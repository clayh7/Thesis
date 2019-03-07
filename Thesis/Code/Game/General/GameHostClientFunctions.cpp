#include "Game/General/Game.hpp"

#include "Engine/Core/NamedProperties.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/Net/Session/NetConnection.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/ParticleSystem.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/ParticleEngine.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UIWidget.hpp"
#include "Game/GameObjects/Allies/AllyShip.hpp"
#include "Game/GameObjects/GameObject.hpp"
#include "Game/GameObjects/NetGameObject.hpp"
#include "Game/GameObjects/Enemies/Rock.hpp"
#include "Game/GameObjects/Enemies/EnemyShip.hpp"
#include "Game/GameObjects/Other/Bullet.hpp"
#include "Game/GameObjects/Other/Emote.hpp"
#include "Game/GameObjects/Items/Item.hpp"
#include "Game/GameObjects/Items/Pickup.hpp"
#include "Game/GameObjects/Player/Player.hpp"
#include "Game/GameObjects/Player/PlayerShip.hpp"
#include "Game/General/NetMessageHandling.hpp"
#include "Game/General/SessionCommands.hpp"


//-------------------------------------------------------------------------------------------------
void Game::HostInitializeGame( )
{
	if( !IsHost( ) )
	{
		return;
	}

	//Create Spawn Points
	m_playerSpawnPoint = Vector2f( 0.f, 0.f );
	m_cultistSpawnPoints[0] = Vector2f( -40.f, -40.f );
	m_cultistSpawnPoints[1] = Vector2f( -40.f, 40.f );
	m_cultistSpawnPoints[2] = Vector2f( 40.f, -40.f );
	m_cultistSpawnPoints[3] = Vector2f( 40.f, 40.f );

	//Try to load game from save file
	if( !LoadWorldFromFile( ) )
	{
		//Start a new game if that fails
		CreateNewWorld( );
	}
}


//-------------------------------------------------------------------------------------------------
bool Game::IsHost( ) const
{
	return s_netSession->IsHost( );
}


//-------------------------------------------------------------------------------------------------
void Game::AttemptCreateHost( std::string const & username, std::string const & password )
{
	size_t passwordHash = std::hash<std::string>{ }( password );
	if( Player::CorrectPasswordOrDoesNotExist( username.c_str( ), passwordHash ) )
	{
		//No password
		if( strcmp( password.c_str(), "" ) == 0 )
		{
			Game::s_netSession->Host( username.c_str( ) );
		}
		//With password
		else
		{
			Game::s_netSession->Host( username.c_str( ), passwordHash );
		}
	}
	else
	{
		NamedProperties empty;
		g_GameSystem->OnJoinDeny( empty );
		g_ConsoleSystem->AddLog( "Incorrect Password.", Console::BAD );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::AttemptCreateClient( std::string const & addressString, std::string const & username, std::string const & password )
{
	sockaddr_in address;
	if( SockAddrFromString( &address, addressString.c_str( ) ) )
	{
		//No password
		if( strcmp( password.c_str( ), "" ) == 0 )
		{
			Game::s_netSession->Join( address, username.c_str( ) );
		}
		//With password
		else
		{
			size_t passwordHash = std::hash<std::string>{ }( password );
			Game::s_netSession->Join( address, username.c_str( ), passwordHash );
		}
	}
	else
	{
		g_ConsoleSystem->AddLog( "Invalid Address", Console::BAD );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::HostCreatePlayer( byte_t netIndex, char const * username, size_t password )
{
	if( !IsHost( ) )
	{
		return;
	}

	m_hostPlayers[netIndex] = new Player( netIndex, username, password );
	m_hostPlayers[netIndex]->HostLoadPlayerFromFile( );
	++m_numHostPlayers;
	m_HostGameActivityLog.Printf( "Player (index=%u) Created: %s", m_hostPlayers[netIndex]->GetPlayerIndex( ), m_hostPlayers[netIndex]->GetUsername( ) );

	//Send create message
	NetMessage create( eNetGameMessageType_NET_PLAYER_CREATE );
	create.Write<byte_t>( netIndex );
	create.WriteString( username );
	s_netSession->AddMessageToAllClients( create );
	
	//Immediately send a single Update Message, because players load with saved data and that data is only sent when it changes
	//If this isn't sent, then the player will think they have absolutely nothing until the next update
	//#TODO: Make sure this is actually happening (correctly)
	NetMessage updateOneTime( eNetGameMessageType_NET_PLAYER_UPDATE );
	updateOneTime.Write<byte_t>( m_hostPlayers[netIndex]->GetPlayerIndex( ) );
	m_hostPlayers[netIndex]->WriteToMessage( &updateOneTime );
	s_netSession->AddMessageToAllClients( updateOneTime );
}


//-------------------------------------------------------------------------------------------------
void Game::HostDestroyPlayer( byte_t netIndex )
{
	//Send destroy message
	NetMessage destroy( eNetGameMessageType_NET_PLAYER_DESTROY );
	destroy.Write<byte_t>( netIndex );
	s_netSession->AddMessageToAllClients( destroy );

	//Save Player
	SavePlayerToFile( netIndex );

	//Destroy the player's ship
	if( m_hostPlayers[netIndex]->m_localShip )
	{
		size_t netID = m_hostPlayers[netIndex]->m_localShip->GetID( );
		HostDestroyNetGameObject( netID );
	}

	delete m_hostPlayers[netIndex];
	m_hostPlayers[netIndex] = nullptr;
	--m_numHostPlayers;
}


//-------------------------------------------------------------------------------------------------
Player * Game::HostGetPlayer( byte_t playerIndex ) const
{
	if( !IsHost( ) )
	{
		return nullptr;
	}

	if( playerIndex == GameObject::INVALID_INDEX )
	{
		return nullptr;
	}

	if( playerIndex == GameObject::ENEMY_PLAYER_INDEX )
	{
		return nullptr;
	}

	if( playerIndex == GameObject::ALLY_PLAYER_INDEX )
	{
		return nullptr;
	}

	return m_hostPlayers[playerIndex];
}


//-------------------------------------------------------------------------------------------------
std::vector<Player*> Game::HostGetActivePlayers( ) const
{
	std::vector<Player*> activePlayers;
	for( size_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
	{
		Player * checkPlayer = m_hostPlayers[playerIndex];
		if( checkPlayer )
		{
			activePlayers.push_back( checkPlayer );
		}
	}
	return activePlayers;
}


//-------------------------------------------------------------------------------------------------
Player * Game::HostGetPlayerForPlayerShip( PlayerShip const * playerShip ) const
{
	if( !IsHost( ) )
	{
		return nullptr;
	}

	if( playerShip )
	{
		return m_hostPlayers[playerShip->m_playerIndex];
	}
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
void Game::HostUpdatePlayer( byte_t playerIndex )
{
	NetMessage update( eNetGameMessageType_NET_PLAYER_UPDATE );
	update.Write<byte_t>( m_hostPlayers[playerIndex]->GetPlayerIndex( ) );
	m_hostPlayers[playerIndex]->WriteToMessage( &update );
	s_netSession->AddMessageToAllClients( update );
}


//-------------------------------------------------------------------------------------------------
void Game::HostAddLevelToPlayer( byte_t playerIndex )
{
	m_HostGameActivityLog.Printf( "%s (index=%u) Leveled Up", m_hostPlayers[playerIndex]->GetUsername( ), m_hostPlayers[playerIndex]->GetPlayerIndex( ) );

	if( !IsHost( ) )
	{
		return;
	}

	Player * targetPlayer = m_hostPlayers[playerIndex];
	targetPlayer->HostAddLevel( );

	HostUpdatePlayer( playerIndex );
}


//-------------------------------------------------------------------------------------------------
bool Game::HostAddItemToPlayer( Player * player, uint16_t itemCode )
{
	static bool const SUCCESS = true;
	static bool const FAIL = false;

	if( !IsHost( ) )
	{
		return FAIL;
	}

	//If the item is money, add that directly to the player and update the player
	if( Item::ParseItemType( itemCode ) == eItemType_CUBES )
	{
		player->m_money += Item::ParseCubeAmount( itemCode );
		HostUpdatePlayer( player->GetPlayerIndex( ) );
		m_HostGameActivityLog.Printf( "%s (index=%u) Picked Up %u Cubes (ItemCode=%u)", player->GetUsername( ), player->GetPlayerIndex( ), Item::ParseCubeAmount( itemCode ), itemCode );
		return SUCCESS;
	}

	else if( player->HostAddItemToInventory( itemCode ) )
	{
		HostUpdatePlayer( player->GetPlayerIndex( ) );
		m_HostGameActivityLog.Printf( "%s (index=%u) Picked Up %s (ItemCode=%u)", player->GetUsername( ), player->GetPlayerIndex( ), Item::ParseName( itemCode ).c_str( ), itemCode );
		return SUCCESS;
	}

	return FAIL;
}


//-------------------------------------------------------------------------------------------------
void Game::HostApplyInputEventToPlayer( eGameEvent const & inputEvent, byte_t playerIndex )
{
	//Host only function
	if( !IsHost( ) )
	{
		return;
	}

	//Player that is giving input does not exist
	if( m_hostPlayers[playerIndex] == nullptr )
	{
		return;
	}

	m_hostPlayers[playerIndex]->ApplyInputEvent( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::HostApplyInputToPlayer( uint16_t inputBitfield, byte_t playerIndex )
{
	//Host only function
	if( !IsHost( ) )
	{
		return;
	}

	//Player that is giving input does not exist
	if( m_hostPlayers[playerIndex] == nullptr )
	{
		return;
	}

	m_hostPlayers[playerIndex]->SetInputState( inputBitfield );
}


//-------------------------------------------------------------------------------------------------
ePlayerClass Game::HostGetClassFromPlayerIndex( byte_t playerIndex ) const
{
	if( IsHost( ) )
	{
		return (ePlayerClass) m_hostPlayers[playerIndex]->m_class;
	}
	else
	{
		return (ePlayerClass) m_clientPlayers[playerIndex]->m_class;
	}
}


//-------------------------------------------------------------------------------------------------
void Game::ClientCreatePlayer( byte_t netIndex, char const * username )
{
	m_clientPlayers[netIndex] = new Player( netIndex, username, 0U );
	++m_numClientPlayers;
	m_ClientGameActivityLog.Printf( "Player (index=%u) Created: %s", m_clientPlayers[netIndex]->GetPlayerIndex( ), m_clientPlayers[netIndex]->GetUsername( ) );
}


//-------------------------------------------------------------------------------------------------
void Game::ClientDestroyPlayer( byte_t netIndex )
{
	//Destroy the player
	delete m_clientPlayers[netIndex];
	m_clientPlayers[netIndex] = nullptr;
	--m_numClientPlayers;
}


//-------------------------------------------------------------------------------------------------
Player * Game::ClientGetPlayerSelf( ) const
{
	NetConnection * conn = s_netSession->GetSelf( );
	if( conn && conn->GetIndex( ) != GameObject::INVALID_INDEX )
	{
		return ClientGetPlayer( conn->GetIndex( ) );
	}
	else
	{
		return nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
Player * Game::ClientGetPlayer( byte_t playerIndex ) const
{
	//Only return the player if they exist
	if( playerIndex == GameObject::INVALID_INDEX )
	{
		return nullptr;
	}
	else if( playerIndex == GameObject::ENEMY_PLAYER_INDEX )
	{
		return nullptr;
	}
	else if( playerIndex < MAX_PLAYERS )
	{
		return m_clientPlayers[playerIndex];
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
NetGameObject * Game::HostCreateNetGameObject( GameObject * object )
{

	// Must be host
	if( !IsHost( ) )
	{
		ERROR_AND_DIE( "Must be host make a new Net Game Object" );
	}

	//Add to list of host objects
	NetGameObject * netObject = new NetGameObject( object );
	m_hostObjects.push_back( netObject );
	m_HostGameActivityLog.Printf( "%s (id=%u) Created at (%.2f,%.2f)", object->GetEntityName( ), netObject->GetID( ), object->m_position.x, object->m_position.y );

	//Send create message
	NetMessage create( eNetGameMessageType_NET_OBJECT_CREATE );
	netObject->WriteToMessage( &create );
	s_netSession->AddMessageToAllClients( create );

	return netObject;
}


//-------------------------------------------------------------------------------------------------
void Game::HostDestroyNetGameObject( NetGameObject * netObject )
{
	//Before you die, tigger on death function
	GameObject * hostObject = netObject->GetLocalObject( );
	hostObject->HostOnDestroy( );

	//Send destroy message
	size_t netID = netObject->GetID( );
	NetMessage destroy( eNetGameMessageType_NET_OBJECT_DESTROY );
	destroy.Write<size_t>( netID );
	s_netSession->AddMessageToAllClients( destroy );

	{
		GameObject * checkObject = netObject->GetLocalObject( );
		if( checkObject->m_type == eNetGameObjectType_ENEMYSHIP ||
			checkObject->m_type == eNetGameObjectType_ALLYSHIP )
		{
			EnemyShip * checkEnemyShip = dynamic_cast<EnemyShip*>( checkObject );
			if( checkEnemyShip )
			{
				Player * killer = HostGetPlayer( checkEnemyShip->m_killedByPlayerIndex );
				m_HostGameActivityLog.Printf( "%s (id=%u) at (%.2f,%.2f) Destroyed By %s", netObject->GetLocalObject( )->GetEntityName( ), netObject->GetID( ), checkObject->m_position.x, checkObject->m_position.y, killer != nullptr ? killer->GetUsername( ) : "NPC" );
			}

			AllyShip * checkAllyShip = dynamic_cast<AllyShip*>( checkObject );
			if( checkAllyShip )
			{
				Player * killer = HostGetPlayer( checkAllyShip->m_killedByPlayerIndex );
				m_HostGameActivityLog.Printf( "%s (id=%u) at (%.2f,%.2f) Destroyed By %s", netObject->GetLocalObject( )->GetEntityName( ), netObject->GetID( ), checkObject->m_position.x, checkObject->m_position.y, killer != nullptr ? killer->GetUsername( ) : "NPC" );
			}
		}
		else if( checkObject->m_type == eNetGameObjectType_PLAYERSHIP )
		{
			PlayerShip * checkPlayerShip = dynamic_cast<PlayerShip*>( checkObject );
			if( checkPlayerShip )
			{
				Player * killer = HostGetPlayer( checkPlayerShip->m_killedByPlayerIndex );
				m_HostGameActivityLog.Printf( "%s (id=%u) at (%.2f,%.2f) Destroyed By %s", netObject->GetLocalObject( )->GetEntityName( ), netObject->GetID(), checkObject->m_position.x, checkObject->m_position.y, killer != nullptr ? killer->GetUsername( ) : "NPC" );
			}
		}
		else
		{
			if( checkObject )
			{
				m_HostGameActivityLog.Printf( "%s (id=%u) at (%.2f,%.2f) Destroyed", netObject->GetLocalObject( )->GetEntityName( ), netObject->GetID( ), checkObject->m_position.x, checkObject->m_position.y );
			}
		}
	}

	//clear ship targets
	for( auto hostObjectIter = m_hostObjects.begin( ); hostObjectIter != m_hostObjects.end( ); ++hostObjectIter )
	{
		NetGameObject * checkNetObject = *hostObjectIter;
		GameObject * checkObject = checkNetObject->GetLocalObject( );
		if( checkObject->m_type == eNetGameObjectType_ENEMYSHIP ||
			checkObject->m_type == eNetGameObjectType_ALLYSHIP )
		{
			EnemyShip * checkEnemyShip = dynamic_cast<EnemyShip*>( checkObject );
			if( checkEnemyShip )
			{
				if( checkEnemyShip->m_targetShip == hostObject )
				{
					checkEnemyShip->m_targetShip = nullptr;
				}
			}
			
			AllyShip * checkAllyShip = dynamic_cast<AllyShip*>( checkObject );
			if( checkAllyShip )
			{
				if( checkAllyShip->m_targetShip == hostObject )
				{
					checkAllyShip->m_targetShip = nullptr;
				}
			}
		}
		if( checkObject->m_type == eNetGameObjectType_PLAYERSHIP )
		{
			PlayerShip * checkPlayerShip = dynamic_cast<PlayerShip*>( checkObject );
			if( checkPlayerShip->m_targetGameObject == hostObject )
			{
				checkPlayerShip->m_targetGameObject = nullptr;
				checkPlayerShip->m_lockedOn = false;
			}
		}
	}

	//Remove from list of host objects
	for( auto hostObjectIter = m_hostObjects.begin( ); hostObjectIter != m_hostObjects.end( ); ++hostObjectIter )
	{
		NetGameObject * hostNetObject = *hostObjectIter;
		if( hostNetObject == netObject )
		{
			delete hostNetObject;
			m_hostObjects.erase( hostObjectIter );
			return;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Game::HostDestroyNetGameObject( size_t netID )
{
	NetGameObject * netGameObject = HostGetNetGameObjectFromNetID( netID );
	if( netGameObject )
	{
		HostDestroyNetGameObject( netGameObject );
	}
}


//-------------------------------------------------------------------------------------------------
NetGameObject * Game::HostGetNetGameObjectFromNetID( size_t netID ) const
{
	for( size_t hostObjectIndex = 0; hostObjectIndex < m_hostObjects.size( ); ++hostObjectIndex )
	{
		if( m_hostObjects[hostObjectIndex]->GetID( ) == netID )
		{
			return m_hostObjects[hostObjectIndex];
		}
	}
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
NetGameObject * Game::ClientCreateNetGameObject( size_t netID, GameObject * object )
{
	m_ClientGameActivityLog.Printf( "%s Created at (%.2f,%.2f)", object->GetEntityName( ), object->m_position.x, object->m_position.y );

	NetGameObject * netObject = new NetGameObject( netID, object );
	if( object->m_type == eNetGameObjectType_PLAYERSHIP )
	{
		GameObject * localGameObject = netObject->GetLocalObject( );
		byte playerIndex = localGameObject->m_playerIndex;
		m_clientPlayers[playerIndex]->m_localShip = netObject;
	}
	m_clientObjects.push_back( netObject );
	return netObject;
}


//-------------------------------------------------------------------------------------------------
void Game::ClientDestroyNetGameObject( NetGameObject * netGameObject )
{
	m_HostGameActivityLog.Printf( "%s (id=%u) at (%.2f,%.2f) Destroyed", netGameObject->GetLocalObject( )->GetEntityName( ), netGameObject->GetID( ), netGameObject->GetLocalObject( )->m_position.x, netGameObject->GetLocalObject( )->m_position.y );

	//Before you die, tigger on death function
	GameObject * clientObject = netGameObject->GetLocalObject( );
	clientObject->ClientOnDestroy( );

	//clear ship targets
	for( auto clientObjectIter = m_clientObjects.begin( ); clientObjectIter != m_clientObjects.end( ); ++clientObjectIter )
	{
		NetGameObject * checkNetObject = *clientObjectIter;
		GameObject * checkObject = checkNetObject->GetLocalObject( );
		if( checkObject->m_type == eNetGameObjectType_ENEMYSHIP ||
			checkObject->m_type == eNetGameObjectType_ALLYSHIP )
		{
			EnemyShip * checkEnemyShip = dynamic_cast<EnemyShip*>( checkObject );
			if( checkEnemyShip )
			{
				if( checkEnemyShip->m_targetShip == clientObject )
				{
					checkEnemyShip->m_targetShip = nullptr;
				}
			}

			AllyShip * checkAllyShip = dynamic_cast<AllyShip*>( checkObject );
			if( checkAllyShip )
			{
				if( checkAllyShip->m_targetShip == clientObject )
				{
					checkAllyShip->m_targetShip = nullptr;
				}
			}
		}
		if( checkObject->m_type == eNetGameObjectType_PLAYERSHIP )
		{
			PlayerShip * checkPlayerShip = dynamic_cast<PlayerShip*>( checkObject );
			if( checkPlayerShip->m_targetGameObject == clientObject )
			{
				checkPlayerShip->m_targetGameObject = nullptr;
				checkPlayerShip->m_lockedOn = false;
			}
		}
	}

	//Remove from list of client objects
	for( auto clientObjectIter = m_clientObjects.begin( ); clientObjectIter != m_clientObjects.end( ); ++clientObjectIter )
	{
		NetGameObject * clientNetObject = *clientObjectIter;
		if( clientNetObject == netGameObject )
		{
			delete clientNetObject;
			m_clientObjects.erase( clientObjectIter );
			return;
		}
	}
}



//-------------------------------------------------------------------------------------------------
void Game::ClientDestroyNetGameObject( size_t netID )
{
	NetGameObject * netGameObject = ClientGetNetGameObjectFromNetID( netID );
	if( netGameObject )
	{
		ClientDestroyNetGameObject( netGameObject );
	}
}


//-------------------------------------------------------------------------------------------------
NetGameObject * Game::ClientGetNetGameObjectFromNetID( size_t netID ) const
{
	for( size_t clientObjectIndex = 0; clientObjectIndex < m_clientObjects.size( ); ++clientObjectIndex )
	{
		if( m_clientObjects[clientObjectIndex]->GetID( ) == netID )
		{
			return m_clientObjects[clientObjectIndex];
		}
	}
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
void Game::HostSpawnPlayerShip( byte_t netIndex )
{
	if( !IsHost( ) )
	{
		return;
	}

	PlayerShip * newShip = new PlayerShip( netIndex, true );

	//Try to load last position
	if( m_hostPlayers[netIndex]->ShouldSpawnFromLoad( ) )
	{
		m_hostPlayers[netIndex]->LoadLastData( newShip );
	}

	//Otherwise, just create ship at new spawn location
	else
	{
		newShip->m_position = FindBestRespawnLocation( newShip->HostIsEvil( ) );
	}

	NetGameObject * netObject = HostCreateNetGameObject( newShip );
	m_hostPlayers[netIndex]->m_localShip = netObject;
}


//-------------------------------------------------------------------------------------------------
void Game::HostSpawnRock( )
{
	Vector2f spawnPosition = FindEmptySpawnLocation( );
	HostSpawnRock( spawnPosition );
}


//-------------------------------------------------------------------------------------------------
void Game::HostSpawnRock( Vector2f const & position )
{
	if( !IsHost( ) )
	{
		return;
	}

	Rock * newRock = new Rock( true );
	newRock->m_position = position;

	HostCreateNetGameObject( newRock );
}


//-------------------------------------------------------------------------------------------------
void Game::HostSpawnCubes( Vector2f const & position, uint8_t cubeAmount )
{
	if( !IsHost( ) )
	{
		return;
	}

	uint16_t itemCode = eItemType_CUBES;
	itemCode = itemCode << 8;
	itemCode = itemCode | cubeAmount;
	Pickup * newPickup = new Pickup( itemCode, true );
	newPickup->m_position = position;

	HostCreateNetGameObject( newPickup );
}


//-------------------------------------------------------------------------------------------------
void Game::HostSpawnItem( Vector2f const & position, eItemType const & itemType )
{
	if( !IsHost( ) )
	{
		return;
	}

	Pickup * newPickup = new Pickup( itemType, true );
	newPickup->m_position = position;

	HostCreateNetGameObject( newPickup );
}



//-------------------------------------------------------------------------------------------------
void Game::HostSpawnItem( Vector2f const & position, uint16_t itemCode )
{
	if( !IsHost( ) )
	{
		return;
	}

	Pickup * newPickup = new Pickup( itemCode, true );
	newPickup->m_position = position;

	HostCreateNetGameObject( newPickup );
}


//-------------------------------------------------------------------------------------------------
void Game::HostSpawnCrystal( )
{
	if( !IsHost( ) )
	{
		return;
	}

	AllyShip * newCrystal = new AllyShip( eAllyType_CRYSTAL, true );
	HostCreateNetGameObject( newCrystal );
}


//-------------------------------------------------------------------------------------------------
void Game::HostShowEmote( Vector2f const & position, float const & rotation, eEmoteType const & type )
{
	if( !IsHost( ) )
	{
		return;
	}

	Emote * newEmote = new Emote( type, true );
	newEmote->m_position = position;
	newEmote->m_rotationDegrees = rotation;
	HostCreateNetGameObject( newEmote );
}


//-------------------------------------------------------------------------------------------------
void Game::HostFireWeapon( byte_t playerIndex, uint16_t weaponCode, Vector2f const & bulletPosition, float bulletRotationDegrees, Vector2f const & )
{
	if( !IsHost( ) )
	{
		return;
	}

	eItemType weaponType = Item::ParseItemType( weaponCode );
	if( weaponType == eItemType_WEAPON_PHASER )
	{
		int bulletCount = (int)Item::ParseSizeAttribute( weaponCode );
		float spreadPerBullet = 10.0f;
		float rotationSpread = ( (float) bulletCount - 1.f )*( spreadPerBullet );
		float startRotation = bulletRotationDegrees - rotationSpread / 2.f;
		for( int bulletIndex = 0; bulletIndex < bulletCount; ++bulletIndex )
		{
			float offsetRotation = startRotation + bulletIndex * spreadPerBullet;
			Bullet * firedBullet = new Bullet( playerIndex, weaponCode, true );
			firedBullet->m_position = bulletPosition;
			firedBullet->m_rotationDegrees = WrapDegrees0to360( offsetRotation );
			firedBullet->m_velocity = UnitVectorFromDegrees( firedBullet->m_rotationDegrees ) * Bullet::SPEED;
			g_GameSystem->HostCreateNetGameObject( firedBullet );
		}
	}
	else if( weaponType == eItemType_WEAPON_BOMBER )
	{
		Bullet * firedBullet = new Bullet( playerIndex, weaponCode, true );
		firedBullet->m_position = bulletPosition;
		firedBullet->m_rotationDegrees = bulletRotationDegrees;
		firedBullet->m_velocity = -UnitVectorFromDegrees( firedBullet->m_rotationDegrees ) * ( Bullet::SPEED / 2.f );
		g_GameSystem->HostCreateNetGameObject( firedBullet );
	}
	else
	{
		Bullet * firedBullet = new Bullet( playerIndex, weaponCode, true );
		firedBullet->m_position = bulletPosition;
		firedBullet->m_rotationDegrees = bulletRotationDegrees;
		firedBullet->m_velocity = UnitVectorFromDegrees( firedBullet->m_rotationDegrees ) * Bullet::SPEED;
		g_GameSystem->HostCreateNetGameObject( firedBullet );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::HostCatchupConnectionOnCurrentGameState( NetConnection * conn )
{
	//Create players first
	for( byte_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
	{
		//Player doesn't exist
		if( !m_hostPlayers[playerIndex] )
		{
			continue;
		}

		//Don't send create for themselves
		if( playerIndex != conn->GetIndex( ) )
		{
			NetMessage create( eNetGameMessageType_NET_PLAYER_CREATE );
			create.Write<byte_t>( playerIndex );
			create.WriteString( m_hostPlayers[playerIndex]->GetUsername( ) );
			conn->AddMessage( create );

			NetMessage update( eNetGameMessageType_NET_PLAYER_UPDATE );
			m_hostPlayers[playerIndex]->m_forceUpdate = true;
			update.Write<byte_t>( m_hostPlayers[playerIndex]->GetPlayerIndex( ) );
			m_hostPlayers[playerIndex]->WriteToMessage( &update );
			conn->AddMessage( update );
		}
	}

	//Create all current net objects
	for( size_t netObjectIndex = 0; netObjectIndex < m_hostObjects.size( ); ++netObjectIndex )
	{
		//Send create message
		NetMessage create( eNetGameMessageType_NET_OBJECT_CREATE );
		GameObject * object = m_hostObjects[netObjectIndex]->GetLocalObject( );
		create.Write<size_t>( m_hostObjects[netObjectIndex]->GetID( ) );
		create.Write<eNetGameObjectType>( object->m_type );
		object->WriteToMessage( &create );
		conn->AddMessage( create );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::HostCheckCollisions( NetGameObject * object )
{
	if( !IsHost( ) )
	{
		return;
	}

	// Update Physics
	GameObject * gameObject = object->GetLocalObject( );
	for( size_t hostObjectIndex = 0; hostObjectIndex < m_hostObjects.size( ); ++hostObjectIndex )
	{
		//Don't collide against self
		if( m_hostObjects[hostObjectIndex]->GetID( ) == object->GetID( ) )
		{
			continue;
		}

		GameObject * otherObject = m_hostObjects[hostObjectIndex]->GetLocalObject( );
		otherObject->HostCheckCollision( gameObject );
	}
}


//-------------------------------------------------------------------------------------------------
float Game::HostGetClosestGameObjectWithinDegrees( GameObject const * fromObject, float withinDegrees, GameObject ** out_foundGameObject )
{
	if( !IsHost( ) )
	{
		return -1.f;
	}

	Vector2f startPosition = fromObject->m_position;
	Vector2f startDirection = Vector2f( CosDegrees( fromObject->m_rotationDegrees ), SinDegrees( fromObject->m_rotationDegrees ) );
	startDirection.Normalize( );
	float closestSquareDistance = -1.f;
	for( size_t objectIndex = 0; objectIndex < m_hostObjects.size( ); ++objectIndex )
	{
		NetGameObject * checkObject = m_hostObjects[objectIndex];
		if( checkObject->GetType( ) == eNetGameObjectType_PLAYERSHIP )
		{
			GameObject * checkGameObject = checkObject->GetLocalObject( );
			Vector2f objectDirection = checkGameObject->m_position - startPosition;
			float squareDistance = objectDirection.SquareLength( );
			if( closestSquareDistance == -1.f || closestSquareDistance > squareDistance )
			{
				objectDirection.Normalize( );
				float angleToObject = DotProduct( startDirection, objectDirection );
				float degreesToObject = RadToDeg( acos( angleToObject ) );
				if( degreesToObject < withinDegrees )
				{
					*out_foundGameObject = checkGameObject;
					closestSquareDistance = squareDistance;
				}
			}
		}
	}

	if( closestSquareDistance == -1.f )
	{
		*out_foundGameObject = nullptr;
		return -1.f;
	}

	return sqrt( closestSquareDistance );
}


//-------------------------------------------------------------------------------------------------
float Game::HostGetClosestGoodShip( Vector2f const & position, Ship ** out_foundShip )
{
	if( !IsHost( ) )
	{
		return - 1.f;
	}

	float closestSquareDistance = -1.f;
	for( size_t objectIndex = 0; objectIndex < m_hostObjects.size( ); ++objectIndex )
	{
		bool isGood = false;
		NetGameObject * checkObject = m_hostObjects[objectIndex];
		Ship * checkShip = dynamic_cast<Ship*>( checkObject->GetLocalObject( ) );
		if( checkShip != nullptr )
		{
			if( checkShip->m_type == eNetGameObjectType_ALLYSHIP )
			{
				isGood = true;
			}

			if( checkShip->m_type == eNetGameObjectType_PLAYERSHIP )
			{
				PlayerShip * checkShip2 = dynamic_cast<PlayerShip*>( checkObject->GetLocalObject( ) );
				if( !checkShip2->HostIsEvil( ) )
				{
					isGood = true;
				}
			}

			if( isGood )
			{
				float squareDistance = DistanceBetweenPointsSquared( position, checkShip->m_position );
				if( closestSquareDistance == -1.f || closestSquareDistance > squareDistance )
				{
					*out_foundShip = checkShip;
					closestSquareDistance = squareDistance;
				}
			}
		}
	}

	if( closestSquareDistance == -1.f )
	{
		return -1.f;
	}

	return sqrt( closestSquareDistance );
}


//-------------------------------------------------------------------------------------------------
float Game::HostGetClosestEvilShip( Vector2f const & position, Ship ** out_foundShip )
{
	if( !IsHost( ) )
	{
		return -1.f;
	}

	float closestSquareDistance = -1.f;
	for( size_t objectIndex = 0; objectIndex < m_hostObjects.size( ); ++objectIndex )
	{
		NetGameObject * checkObject = m_hostObjects[objectIndex];
		bool isEvil = false;
		Ship * checkShip = dynamic_cast<Ship*>( checkObject->GetLocalObject( ) );
		if( checkShip != nullptr )
		{
			if( checkShip->m_type == eNetGameObjectType_ENEMYSHIP )
			{
				isEvil = true;
			}

			if( checkShip->m_type == eNetGameObjectType_PLAYERSHIP )
			{
				PlayerShip * checkShip2 = dynamic_cast<PlayerShip*>( checkObject->GetLocalObject( ) );
				if( checkShip2->HostIsEvil( ) )
				{
					isEvil = true;
				}
			}

			if( isEvil )
			{
				float squareDistance = DistanceBetweenPointsSquared( position, checkShip->m_position );
				if( closestSquareDistance == -1.f || closestSquareDistance > squareDistance )
				{
					*out_foundShip = checkShip;
					closestSquareDistance = squareDistance;
				}
			}
		}
	}

	if( closestSquareDistance == -1.f )
	{
		return -1.f;
	}

	return sqrt( closestSquareDistance );
}


//-------------------------------------------------------------------------------------------------
void Game::HostIncreaseSummoningBar( uint8_t amount )
{
	if( !IsHost( ) )
	{
		return;
	}

	m_gameState.HostIncreaseSummoningBar( amount );
}


//-------------------------------------------------------------------------------------------------
void Game::HostSummonCthulhu( )
{
	if( !IsHost( ) )
	{
		return;
	}

	std::vector<Player*> allActiveCultists;

	//Find all cultists
	for( size_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
	{
		Player * checkPlayer = m_hostPlayers[playerIndex];
		if( checkPlayer && checkPlayer->GetPlayerShip() )
		{
			ePlayerClass playerClass = checkPlayer->m_class;
			if( playerClass == ePlayerClass_CULTIST )
			{
				allActiveCultists.push_back( checkPlayer );
			}
		}
	}

	//Choose a random one
	size_t cultistCount = allActiveCultists.size( );
	if( cultistCount > 0 )
	{
		int randomIndex = RandomInt( (int) cultistCount );
		Player * targetPlayer = allActiveCultists[randomIndex];
		m_HostGameActivityLog.Printf( "%s (index=%u) transformed into Cthulhu", targetPlayer->GetUsername( ), targetPlayer->GetPlayerIndex( ) );
		targetPlayer->HostTurnIntoCthulhu( );
		m_gameState.m_hostIsCthulhuActive = true;
		m_gameState.m_hostSummoningAmount = 0;
		HostUpdatePlayer( targetPlayer->GetPlayerIndex( ) );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::HostResetCthulhuSummoning( )
{
	if( !IsHost( ) )
	{
		return;
	}

	m_gameState.m_hostIsCthulhuActive = false;
	m_gameState.m_hostSummoningAmount = 0;
}


//-------------------------------------------------------------------------------------------------
void Game::HostPlayerFeedCrystal( uint8_t playerIndex )
{
	if( !IsHost( ) )
	{
		return;
	}

	Player * player = m_hostPlayers[playerIndex];
	m_HostGameActivityLog.Printf( "%s Fed The Crystal", player->GetUsername( ) );
	PlayerShip * currentShip = player->GetPlayerShip( );
	AllyShip * crystal = m_gameState.m_hostCrystal;
	
	//Player feed crystal
	if( currentShip != nullptr && currentShip->CanFeedCrystal( ) )
	{
		//Heal for feeding crystal
		player->m_money -= PlayerShip::CRYSTAL_FEED_AMOUNT;
		currentShip->m_health = (uint16_t)currentShip->GetMaxHealth( );

		//Only take the heal if it needs it
		if( crystal->m_health < AllyShip::CRYSTAL_MAX_HEALTH )
		{
			crystal->HostHealDamage( PlayerShip::CRYSTAL_FEED_AMOUNT * 10U );
		}
		HostUpdatePlayer( playerIndex );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::HostHealCrystal( uint16_t healAmount )
{
	if( !IsHost( ) )
	{
		return;
	}

	AllyShip * crystal = m_gameState.m_hostCrystal;
	crystal->HostHealDamage( healAmount );
}


//-------------------------------------------------------------------------------------------------
void Game::HostWipeGame( )
{
	m_HostGameActivityLog.Printf( "Crystal Destroyed" );

	//Find all players
	for( size_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
	{
		Player * currentPlayer = m_hostPlayers[playerIndex];
		if( currentPlayer )
		{
			//And destroy them
			PlayerShip * ship = currentPlayer->GetPlayerShip( );
			if( ship )
			{
				ship->m_health = 0;
			}
		}
	}

	//Flush current packets
	for( uint8_t index = 0; index < MAX_PLAYERS; ++index )
	{
		if( !s_netSession->GetNetConnection( index ) )
		{
			continue;
		}
		NamedProperties netEvent;
		netEvent.Set( "connection", s_netSession->GetNetConnection( index ) );
		EventSystem::TriggerEvent( NetSession::PREPARE_PACKET_EVENT, netEvent );
		s_netSession->GetNetConnection( index )->SendPacket( );
	}

	//Boot host, and by consequence, all clients
	m_quitNextFrame = true;
}


//-------------------------------------------------------------------------------------------------
void Game::HostDealAreaDamage( Vector2f const & position, float radius, uint16_t damage, byte_t playerIndexSource )
{
	for( size_t objectIndex = 0; objectIndex < m_hostObjects.size( ); ++objectIndex )
	{
		NetGameObject * checkObject = m_hostObjects[objectIndex];
		GameObject * checkGameObject = checkObject->GetLocalObject( );
		Vector2f objectVector = checkGameObject->m_position - position;
		float squareDistance = objectVector.SquareLength( );
		float squareRadius = radius * radius;
		if( squareDistance < squareRadius )
		{
			Ship * shipGameObject = dynamic_cast<Ship*>( checkGameObject );
			if( shipGameObject )
			{
				shipGameObject->HostTakeDamage( damage, playerIndexSource );
			}
			Rock * rockGameObject = dynamic_cast<Rock*>( checkGameObject );
			if( rockGameObject )
			{
				rockGameObject->HostTakeDamage( damage );
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
float Game::ClientGetClosestGameObjectWithinDegrees( GameObject const * fromObject, float withinDegrees, GameObject ** out_foundGameObject )
{
	Vector2f startPosition = fromObject->m_position;
	Vector2f startDirection = Vector2f( CosDegrees( fromObject->m_rotationDegrees ), SinDegrees( fromObject->m_rotationDegrees ) );
	startDirection.Normalize( );
	float closestSquareDistance = -1.f;
	for( size_t objectIndex = 0; objectIndex < m_clientObjects.size( ); ++objectIndex )
	{
		NetGameObject * checkObject = m_clientObjects[objectIndex];
		GameObject * checkGameObject = checkObject->GetLocalObject( );
		
		//Ignore bullets
		if( checkObject->GetType( ) == eNetGameObjectType_BULLET )
		{
			continue;
		}
		//Ignore emotes
		else if( checkObject->GetType( ) == eNetGameObjectType_EMOTE )
		{
			continue;
		}

		//Ignore timed out objects (Not pickups)
		if( checkGameObject->IsNotRecievingUpdates( ) && checkObject->GetType() != eNetGameObjectType_PICKUP )
		{
			continue;
		}

		Vector2f objectDirection = checkGameObject->m_position - startPosition;
		float squareDistance = objectDirection.SquareLength( );
		if( closestSquareDistance == -1.f || closestSquareDistance > squareDistance )
		{
			objectDirection.Normalize( );
			float angleToObject = DotProduct( startDirection, objectDirection );
			float degreesToObject = RadToDeg( acos( angleToObject ) );
			if( degreesToObject < withinDegrees )
			{
				*out_foundGameObject = checkGameObject;
				closestSquareDistance = squareDistance;
			}
		}
	}

	if( closestSquareDistance == -1.f )
	{
		*out_foundGameObject = nullptr;
		return -1.f;
	}

	return sqrt( closestSquareDistance );
}


//-------------------------------------------------------------------------------------------------
void Game::ClientRequestEvent( eGameEvent const & upgradeEvent )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<eGameEvent>( upgradeEvent );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::ClientRequestRemoveItem( byte_t itemIndex )
{
	NetMessage removeItemEvent( eNetGameMessageType_RELIABLE_INPUT );
	removeItemEvent.Write<eGameEvent>( eGameEvent_EJECT_ITEM );
	removeItemEvent.Write<byte_t>( itemIndex );
	s_netSession->GetHost( )->AddMessage( removeItemEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::ClientRequestRemoveEquipment( eEquipmentSlot const & slot )
{
	NetMessage removeItemEvent( eNetGameMessageType_RELIABLE_INPUT );
	if( slot == eEquipmentSlot_PRIMARY )
	{
		removeItemEvent.Write<eGameEvent>( eGameEvent_REMOVE_PRIMARY );
		s_netSession->GetHost( )->AddMessage( removeItemEvent );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::ClientRequestEquipPrimary( byte_t itemIndex )
{
	NetMessage equipItemEvent( eNetGameMessageType_RELIABLE_INPUT );
	equipItemEvent.Write<eGameEvent>( eGameEvent_EQUIP_PRIMARY );
	equipItemEvent.Write<byte_t>( itemIndex );
	s_netSession->GetHost( )->AddMessage( equipItemEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::ClientLevelUpEffect( Player * player )
{
	if( player )
	{
		if( player->m_localShip )
		{
			PlayerShip * ship = player->GetPlayerShip( );

			//Create bullet pop explosion
			g_ParticleEngine->PlayOnce( "levelup", LAYER_FX, ship->m_position );
		}
	}
}