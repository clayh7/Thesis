#include "Game/GameObjects/Player/Ship.hpp"

#include "Engine/AudioSystem/Audio.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Net/Session/NetMessage.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/ParticleEngine.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/ParticleSystem.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Game/GameObjects/Enemies/Rock.hpp"
#include "Game/GameObjects/Items/Item.hpp"
#include "Game/GameObjects/Other/Bullet.hpp"
#include "Game/GameObjects/Player/PlayerShip.hpp"
#include "Game/General/Game.hpp"


//-------------------------------------------------------------------------------------------------
STATIC float const Ship::IMMUNE_TIME = 3.0f;
STATIC float const Ship::BULLET_START_OFFSET = 0.7f;
STATIC float const Ship::SLOW_DURATION = 3.0f;
STATIC SoundID * Ship::SOUND_FIRE = new SoundID[SOUND_FIRE_COUNT];
STATIC SoundID * Ship::SOUND_EXPLODE = new SoundID[SOUND_EXPLODE_COUNT];


//-------------------------------------------------------------------------------------------------
STATIC void Ship::Initialize( )
{
	SOUND_FIRE[0] = g_AudioSystem->CreateOrGetSound( SOUND_FIRE1_FILE );
	SOUND_FIRE[1] = g_AudioSystem->CreateOrGetSound( SOUND_FIRE2_FILE );
	SOUND_FIRE[2] = g_AudioSystem->CreateOrGetSound( SOUND_FIRE3_FILE );
	SOUND_FIRE[3] = g_AudioSystem->CreateOrGetSound( SOUND_FIRE4_FILE );
	SOUND_FIRE[4] = g_AudioSystem->CreateOrGetSound( SOUND_FIRE5_FILE );
	SOUND_EXPLODE[0] = g_AudioSystem->CreateOrGetSound( SOUND_EXPLODE1_FILE );
	SOUND_EXPLODE[1] = g_AudioSystem->CreateOrGetSound( SOUND_EXPLODE2_FILE );
	SOUND_EXPLODE[2] = g_AudioSystem->CreateOrGetSound( SOUND_EXPLODE3_FILE );
}


//-------------------------------------------------------------------------------------------------
//Host Only
void Ship::WriteToMessage( NetMessage * out_message )
{
	GameObject::WriteToMessage( out_message );

	Game::WriteUncompressedUint16( out_message, m_health );
	Game::WriteUncompressedUint16( out_message, m_shield );
	Game::WriteUncompressedUint16( out_message, m_energy );
	Game::WriteCompressedBoolean( out_message, HostIsSlowed( ) );
}


//-------------------------------------------------------------------------------------------------
//Client Only
void Ship::UpdateFromMessage( NetSender const & sender, NetMessage const & message )
{
	GameObject::UpdateFromMessage( sender, message );

	Game::ReadUncompressedUint16( message, &m_netHealth );
	Game::ReadUncompressedUint16( message, &m_netShield );
	Game::ReadUncompressedUint16( message, &m_netEnergy );
	Game::ReadCompressedBoolean( message, &m_netIsSlow );
}


//-------------------------------------------------------------------------------------------------
//Host Only
void Ship::WriteToXMLNode( XMLNode * out_xmlNode )
{
	out_xmlNode->addAttribute( STRING_POSITION, Stringf( "%f,%f", m_position.x, m_position.y ).c_str( ) );
	out_xmlNode->addAttribute( STRING_VELOCITY, Stringf( "%f,%f", m_velocity.x, m_velocity.y ).c_str( ) );
	out_xmlNode->addAttribute( STRING_ROTATION, Stringf( "%f", m_rotationDegrees ).c_str( ) );
	out_xmlNode->addAttribute( STRING_HEALTH, Stringf( "%u", m_health ).c_str( ) );
	out_xmlNode->addAttribute( STRING_SHIELD, Stringf( "%u", m_shield ).c_str( ) );
	out_xmlNode->addAttribute( STRING_ENERGY, Stringf( "%u", m_energy ).c_str( ) );

	GameObject::WriteToXMLNode( out_xmlNode );
}


//-------------------------------------------------------------------------------------------------
//Host Only
void Ship::UpdateFromXMLNode( XMLNode const & node )
{
	m_position = ReadXMLAttribute( node, STRING_POSITION, m_position );
	m_velocity = ReadXMLAttribute( node, STRING_VELOCITY, m_velocity );
	m_rotationDegrees = ReadXMLAttribute( node, STRING_ROTATION, m_rotationDegrees );
	m_health = ReadXMLAttribute( node, STRING_HEALTH, m_health );
	m_shield = ReadXMLAttribute( node, STRING_SHIELD, m_shield );
	m_energy = ReadXMLAttribute( node, STRING_ENERGY, m_energy );

	GameObject::UpdateFromXMLNode( node );
}


//-------------------------------------------------------------------------------------------------
Ship::Ship( byte_t playerIndex, eNetGameObjectType const & gameObjectType, bool hostObject )
	: GameObject( playerIndex, gameObjectType, hostObject )
	, m_health( 0U )
	, m_netHealth( 0U )
	, m_shield( 0U )
	, m_netShield( 0U )
	, m_energy( 0U )
	, m_netEnergy( 0U )
	, m_fireCooldownTimer( 0.f )
	, m_killedByPlayerIndex( INVALID_INDEX )
	, m_immuneTimer( IMMUNE_TIME )
	, m_slowAmount( 0.f )
	, m_slowEffectTimer( 0.f )
	, m_netIsSlow( false )
{
	m_engineEffect[0] = nullptr;
	m_engineEffect[1] = nullptr;

	//Make this for clients
	if( !m_isHostObject )
	{
		std::string engineID = GetEngineID( );
		m_engineEffect[0] = g_ParticleEngine->Create( engineID, LAYER_FX, m_position );
		m_engineEffect[1] = g_ParticleEngine->Create( engineID, LAYER_FX, m_position );
	}
}


//-------------------------------------------------------------------------------------------------
Ship::~Ship( )
{
	if( m_engineEffect[0] != nullptr && m_engineEffect[1] != nullptr )
	{
		m_engineEffect[0]->Destroy( );
		m_engineEffect[1]->Destroy( );
	}
}


//-------------------------------------------------------------------------------------------------
void Ship::Update( )
{
	UpdateTimers( );

	//Host Update
	if( m_isHostObject )
	{
		//Nothing
	}

	//Client Update
	else if( !m_isHostObject )
	{
		//Update synced members
		m_health = m_netHealth;
		m_shield = m_netShield;
		m_energy = m_netEnergy;
		UpdateFreezeSpriteEffect( );
	}

	//Apply this child's movement control
	ApplyControlInput( );

	//Move Ship
	GameObject::Update( );
	//Post-Move Update

	//Move engine particles to back
	if( m_engineEffect[0] != nullptr && m_engineEffect[1] != nullptr )
	{
		float radius = GetPhysicsRadius( );
		Vector2f back = -Vector2f( CosDegrees( m_rotationDegrees ), SinDegrees( m_rotationDegrees ) ) * radius * 0.9f;
		Vector2f right = Vector2f( -back.y, back.x ) * 0.8f;
		Vector2f left = -right;
		Player * owningPlayer = g_GameSystem->ClientGetPlayer( m_playerIndex );
		if( owningPlayer )
		{
			if( owningPlayer->m_class == ePlayerClass_ROGUE ||
				owningPlayer->m_class == ePlayerClass_FIGHTER ||
				owningPlayer->m_class == ePlayerClass_CULTIST )
			{
				m_engineEffect[0]->SetSpawnPosition( m_position + back + left );
				m_engineEffect[1]->SetSpawnPosition( m_position + back + right );
			}
			else
			{
				m_engineEffect[0]->SetSpawnPosition( m_position + back );
				m_engineEffect[1]->SetSpawnPosition( m_position + back );
			}
		}
		else
		{
			m_engineEffect[0]->SetSpawnPosition( m_position );
			m_engineEffect[1]->SetSpawnPosition( m_position );
		}
		if( IsNotRecievingUpdates( ) )
		{
			m_sprite->SetEnabled( false );
			m_engineEffect[0]->m_isEnabled = false;
			m_engineEffect[1]->m_isEnabled = false;
		}
		else
		{
			m_sprite->SetEnabled( true );
			m_engineEffect[0]->m_isEnabled = true;
			m_engineEffect[1]->m_isEnabled = true;
		}
	}

}


//-------------------------------------------------------------------------------------------------
void Ship::UpdateFreezeSpriteEffect( )
{
	if( m_netIsSlow )
	{
		m_sprite->SetColor( Color( 128, 128, 255, 255 ) );
	}
	else
	{
		m_sprite->SetColor( Color( 255, 255, 255, 255 ) );
	}
}


//-------------------------------------------------------------------------------------------------
void Ship::HostHandleCollision( GameObject * gameObject )
{
	// Collision with Ships
	if( gameObject->m_type == eNetGameObjectType_PLAYERSHIP )
	{
		//PlayerShip * hitShip = (PlayerShip*) gameObject;
		//m_health = 0;
		//hitShip->m_health = 0;
	}

	// Collision with Rocks
	else if( gameObject->m_type == eNetGameObjectType_ROCK )
	{
		// Check collision
		Rock * hitRock = (Rock*) gameObject;
		uint16_t rockDamageToShip = hitRock->m_health;
		uint16_t shipDamageToRock = m_health;
		hitRock->HostTakeDamage( shipDamageToRock );
		HostTakeDamage( rockDamageToShip, hitRock->m_playerIndex );
	}
}


//-------------------------------------------------------------------------------------------------
bool Ship::HostIsAlive( ) const
{
	return m_health > 0;
}


//-------------------------------------------------------------------------------------------------
std::string Ship::GetEngineID( ) const
{
	return "engine";
}


//-------------------------------------------------------------------------------------------------
void Ship::HostOnDestroy( )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	//Increase the summoning bar if killed by a cultist
	Player * killingPlayer = g_GameSystem->HostGetPlayer( m_killedByPlayerIndex );
	if( killingPlayer )
	{
		ePlayerClass playerClass = killingPlayer->m_class;
		if( playerClass == ePlayerClass_CULTIST )
		{
			uint8_t summonBarValue = GetSummoningBarValue( );
			g_GameSystem->HostIncreaseSummoningBar( summonBarValue );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Ship::ClientOnDestroy( )
{
	Player * checkClient = g_GameSystem->ClientGetPlayerSelf( );
	PlayerShip * checkClientShip = checkClient->GetPlayerShip( );
	float attenuation = g_GameSystem->GetAudioAttenuationFromCamera( m_position );

	//Hear yourself blow up
	if( checkClientShip == this )
	{
		attenuation = 1.f;
	}

	//Create ship explosion
	SoundID explosionID = GetRandomExplodeSound( );
	g_AudioSystem->PlayAudio( explosionID, 1.f );
	g_ParticleEngine->PlayOnce( "explode", LAYER_FX, m_position );
}


//-------------------------------------------------------------------------------------------------
void Ship::UpdateTimers( )
{
	//Immunity Cooldown
	if( m_immuneTimer > 0.f )
	{
		m_immuneTimer -= Time::DELTA_SECONDS;
	}

	//Update Fire Cooldown
	if( m_fireCooldownTimer > 0.f )
	{
		m_fireCooldownTimer -= Time::DELTA_SECONDS;
	}

	//Update Slow Effect Timer
	if( m_slowEffectTimer > 0.f )
	{
		m_slowEffectTimer -= Time::DELTA_SECONDS;
	}
}


//-------------------------------------------------------------------------------------------------
bool Ship::CanFire( ) const
{
	Player * owner = g_GameSystem->HostGetPlayer( m_playerIndex );
	bool weaponEquipped = false;
	bool hasRequiredEnergy = false;
	bool weaponOffCooldown = m_fireCooldownTimer <= 0.f;
	if( owner )
	{
		uint16_t itemCode = owner->GetEquippedWeaponCode( );
		if( itemCode != Item::INVALID_ITEM_CODE )
		{
			weaponEquipped = true;
		}
		if( m_energy >= Item::ParseEnergyCost( itemCode ) )
		{
			hasRequiredEnergy = true;
		}
	}

	//This is for no player characters like Allies and Enemies
	else
	{
		weaponEquipped = true;
		hasRequiredEnergy = true;
	}
	return weaponEquipped && weaponOffCooldown && hasRequiredEnergy;
}


//-------------------------------------------------------------------------------------------------
void Ship::ResetFireCooldown( )
{
	//Play Random Fire Sound
	if( !m_isHostObject )
	{
		//equippedWeapon.PlayFireAudio();
		float attenuation = g_GameSystem->GetAudioAttenuationFromCamera( m_position );
		SoundID fireID = GetRandomFireSound( );
		g_AudioSystem->PlayAudio( fireID, attenuation );
	}
	
	float fireRate = GetFireRate( );
	m_fireCooldownTimer = fireRate;

	//Slight buffer for client side
	if( !g_GameSystem->IsHost( ) )
	{
		m_fireCooldownTimer += Time::DELTA_SECONDS;
	}

	Player * owner = g_GameSystem->HostGetPlayer( m_playerIndex );
	if( owner )
	{
		Item * weapon = owner->m_equipment[eEquipmentSlot_PRIMARY];
		uint16_t weaponCode = weapon->m_itemCode;
		m_energy -= Item::ParseEnergyCost( weaponCode );
	}
}


//-------------------------------------------------------------------------------------------------
SoundID Ship::GetRandomFireSound( ) const
{
	int randomSound = RandomInt( SOUND_FIRE_COUNT );
	return SOUND_FIRE[randomSound];
}


//-------------------------------------------------------------------------------------------------
SoundID Ship::GetRandomExplodeSound( ) const
{
	int randomSound = RandomInt( SOUND_EXPLODE_COUNT );
	return SOUND_EXPLODE[randomSound];
}


//-------------------------------------------------------------------------------------------------
bool Ship::HasImmunity( ) const
{
	if( m_playerIndex == 0 && g_GameSystem->IsHost( ) && g_GameSystem->m_hostGodMode )
	{
		return true;
	}
	return m_immuneTimer > 0.f;
}


//-------------------------------------------------------------------------------------------------
void Ship::HostTakeDamage( int incomingDamage, uint8_t sourcePlayerIndex, uint16_t weaponCode )
{
	//No damage during immunity
	if( HasImmunity( ) )
	{
		return;
	}

	//Restart warping when hurt
	PlayerShip * playerShip = dynamic_cast<PlayerShip*>( this );
	if( playerShip )
	{
		if( playerShip->IsWarping( ) )
		{
			playerShip->HostToggleWarping( );
		}
	}

	eItemType weaponType = Item::ParseItemType( weaponCode );
	if( weaponType == eItemType_WEAPON_ZAPPER )
	{
		uint8_t weaponMultiplier = Item::ParseSizeAttribute( weaponCode );

		//Hurt shield first
		int shieldDamage = Min( (int) m_shield, incomingDamage * weaponMultiplier );
		m_shield -= (uint16_t) shieldDamage;

		//If we have no more shield, take hull damage
		if( m_shield == 0 )
		{
			int healthDamage = incomingDamage - shieldDamage / weaponMultiplier;
			healthDamage = Min( (int) m_health, healthDamage );
			m_health -= (uint16_t) healthDamage;
		}
	}
	else if( weaponType == eItemType_WEAPON_ACID )
	{
		uint8_t weaponMultiplier = Item::ParseSizeAttribute( weaponCode );

		//Hurt shield first
		int shieldDamage = Min( (int) m_shield, incomingDamage );
		m_shield -= (uint16_t) shieldDamage;

		//If we have no more shield, take hull damage
		if( m_shield == 0 )
		{
			int healthDamage = incomingDamage - shieldDamage;
			healthDamage = Min( (int) m_health, healthDamage * weaponMultiplier );
			m_health -= (uint16_t) healthDamage;
		}
	}
	else if( weaponType == eItemType_WEAPON_BOMBER || weaponType == eItemType_WEAPON_ROCKET )
	{
		//Nothing (all damage is done with area damage)
	}
	else
	{
		//Hurt shield first
		int shieldDamage = Min( (int) m_shield, incomingDamage );
		m_shield -= (uint16_t) shieldDamage;
		
		//If we have no more shield, take hull damage
		if( m_shield == 0 )
		{
			int healthDamage = incomingDamage - shieldDamage;
			healthDamage = Min( (int) m_health, healthDamage );
			m_health -= (uint16_t) healthDamage;
		}
	}

	if( weaponType == eItemType_WEAPON_FREEZE )
	{
		uint8_t slowNum = Item::ParseSizeAttribute( weaponCode );
		float slowAmount = 0.15f * (float) slowNum;
		HostApplySlow( slowAmount );
	}

	if( m_health == 0 )
	{
		m_killedByPlayerIndex = sourcePlayerIndex;
	}
}


//-------------------------------------------------------------------------------------------------
void Ship::HostTakeDamage( int incomingDamage, uint8_t sourcePlayerIndex )
{
	//No damage during immunity
	if( HasImmunity( ) )
	{
		return;
	}

	//Restart warping when hurt
	PlayerShip * playerShip = dynamic_cast<PlayerShip*>( this );
	if( playerShip )
	{
		if( playerShip->IsWarping( ) )
		{
			playerShip->HostToggleWarping( );
		}
	}

	//Hurt shield first
	int shieldDamage = Min( (int)m_shield, incomingDamage );
	m_shield -= (uint16_t) shieldDamage;

	//If we have no more shield, take hull damage
	if( m_shield == 0 )
	{
		int healthDamage = incomingDamage - shieldDamage;
		healthDamage = Min( (int)m_health, healthDamage );
		m_health -= (uint16_t) healthDamage;
	}

	if( m_health == 0 )
	{
		m_killedByPlayerIndex = sourcePlayerIndex;
	}
}


//-------------------------------------------------------------------------------------------------
Vector2f Ship::GetPositionInfront( float unitsForward, float degreesOffset /*= 0.f*/ ) const
{
	Vector2f currentPosition = m_position;
	float rotation = m_rotationDegrees + degreesOffset;
	Vector2f forwardVector = Vector2f( CosDegrees( rotation ), SinDegrees( rotation ) );
	return currentPosition + forwardVector * unitsForward;
}


//-------------------------------------------------------------------------------------------------
void Ship::HostApplySlow( float amount )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	m_slowAmount = amount;
	m_slowEffectTimer = SLOW_DURATION;
}


//-------------------------------------------------------------------------------------------------
bool Ship::HostIsSlowed( ) const
{
	if( !g_GameSystem->IsHost( ) )
	{
		return false;
	}

	return m_slowEffectTimer > 0.f;
}