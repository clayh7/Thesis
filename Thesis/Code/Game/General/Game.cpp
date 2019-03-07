#include "Game/General/Game.hpp"

#include <stdio.h>
#include "Engine/Core/Time.hpp"
#include "Engine/EventSystem/EventSystem.hpp"
#include "Engine/InputSystem/Input.hpp"
#include "Engine/Net/Session/NetConnection.hpp"
#include "Engine/Net/Session/NetPacket.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/Sprite.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/ParticleEngine.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"
#include "Engine/UISystem/UIBox.hpp"
#include "Engine/UISystem/UIButton.hpp"
#include "Engine/UISystem/UIContainer.hpp"
#include "Engine/UISystem/UIItem.hpp"
#include "Engine/UISystem/UILabel.hpp"
#include "Engine/UISystem/UIProgressBar.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UITextField.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Game/GameObjects/Allies/AllyShip.hpp"
#include "Game/GameObjects/Enemies/EnemyShip.hpp"
#include "Game/GameObjects/Enemies/ShipSpawner.hpp"
#include "Game/GameObjects/Enemies/Rock.hpp"
#include "Game/GameObjects/Items/Item.hpp"
#include "Game/GameObjects/Items/Pickup.hpp"
#include "Game/GameObjects/Other/Bullet.hpp"
#include "Game/GameObjects/Other/Emote.hpp"
#include "Game/GameObjects/Player/Player.hpp"
#include "Game/GameObjects/Player/PlayerShip.hpp"
#include "Game/GameObjects/Player/Ship.hpp"
#include "Game/GameObjects/NetGameObject.hpp"
#include "Game/General/GameCommon.hpp"
#include "Game/General/NetMessageHandling.hpp"
#include "Game/General/SessionCommands.hpp"


//-------------------------------------------------------------------------------------------------
Game * g_GameSystem = nullptr;


//-------------------------------------------------------------------------------------------------
STATIC bool Game::s_showHost = false;
STATIC bool const Game::SHOW_COLLISION = false;
STATIC Color const Game::HOST_COLOR = Color( 0, 0, 0, 255 ); //Color doesn't exist at static initialization
STATIC float const Game::MAX_MAP_RADIUS = 80.f;
STATIC Vector2f const Game::MINIMAP_CENTER( -7.30f, -3.9f );
STATIC NetSession * Game::s_netSession = nullptr;
STATIC float const Game::NORMAL_VIEW_SCALE = 10.f;
STATIC float const Game::CTHULHU_VIEW_SCALE = 20.f;
STATIC float const Game::MAX_AUDIO_DISTANCE = 20.f;
STATIC float const Game::MAX_SPEED = 30.f;

STATIC char const * Game::STRING_WORLD_DATA = "WorldData";
STATIC char const * Game::STRING_GAME_OBJECT_SPAWNERS = "GameObjectSpawners";
STATIC char const * Game::STRING_GAME_OBJECTS = "GameObjects";
STATIC char const * Game::UI_ADDRESS_FIELD = "addressField";
STATIC char const * Game::UI_USERNAME_FIELD = "usernameField";
STATIC char const * Game::UI_PASSWORD_FIELD = "passwordField";
STATIC char const * Game::UI_INVALID_PASSWORD = "invalidPassword";
STATIC char const * Game::UI_TITLE_SCREEN = "titleScreen";
STATIC char const * Game::UI_CRYSTAL_DESTROYED = "crystalDestroyedOverlay";
STATIC char const * Game::UI_CLASS_SELECT_SCREEN = "classSelectScreen";
STATIC char const * Game::UI_CLASS_SELECT_BACKGROUND = "classSelectBackground";
STATIC char const * Game::UI_PLAYER_HUD_SCREEN = "playerHUD";
STATIC char const * Game::UI_TARGET_HUD_SCREEN = "targetingHUD";
STATIC char const * Game::UI_DEBUG_HUD_SCREEN = "debug";
STATIC char const * Game::UI_PLAYER_CLASS = "classLabel";
STATIC char const * Game::UI_PLAYER_NAME = "playerName";
STATIC char const * Game::UI_PLAYER_LEVEL = "playerLevel";
STATIC char const * Game::UI_PLAYER_MONEY = "playerMoney";
STATIC char const * Game::UI_UPGRADE_COST = "upgradeCost";
STATIC char const * Game::UI_PLAYER_HEALTH = "playerHealth";
STATIC char const * Game::UI_PLAYER_SHIELD = "playerShield";
STATIC char const * Game::UI_PLAYER_ENERGY = "playerEnergy";
STATIC char const * Game::UI_STAT_HEALTH = "statHealth";
STATIC char const * Game::UI_STAT_SHIELD = "statShield";
STATIC char const * Game::UI_STAT_ENERGY = "statEnergy";
STATIC char const * Game::UI_WORLD_CONTAINER = "worldContainer";
STATIC char const * Game::UI_EQUIPMENT_PRIMARY = "equpimentPrimary";
STATIC char const * Game::UI_UPGRADE_HULL_BUTTON = "upgradeHullButton";
STATIC char const * Game::UI_UPGRADE_HEALTH_BUTTON = "upgradeHealthButton";
STATIC char const * Game::UI_UPGRADE_SHIELD_BUTTON = "upgradeShieldButton";
STATIC char const * Game::UI_UPGRADE_ENERGY_BUTTON = "upgradeEnergyButton";
STATIC char const * Game::UI_TARGET_NAME = "targetName";
STATIC char const * Game::UI_TARGET_LEVEL = "targetLevel";
STATIC char const * Game::UI_TARGET_HEALTH = "targetHealth";
STATIC char const * Game::UI_TARGET_SHIELD = "targetShield";
STATIC char const * Game::UI_TARGET_ENERGY = "targetEnergy";
STATIC char const * Game::UI_TARGET_INFO1 = "targetInfo1";
STATIC char const * Game::UI_TARGET_INFO2 = "targetInfo2";
STATIC char const * Game::UI_TARGET_INFO3 = "targetInfo3";
STATIC char const * Game::UI_TARGET_INFO4 = "targetInfo4";
STATIC char const * Game::UI_GOD_MODE = "godModeText";
STATIC char const * Game::UI_WARP_INSTRUCTIONS = "warpToCrystalInstructions";
STATIC char const * Game::UI_WARP_DRIVE = "warpDrive";
STATIC char const * Game::UI_SUMMONING_BAR = "summoningBar";
STATIC char const * Game::UI_MAP_COORDINATES = "mapCoordinates";
STATIC char const * Game::UI_CONNECTION = "connection";
STATIC char const * Game::UI_DEDICATED_SERVER = "dedicatedServerLabel";
STATIC char const * Game::UI_FEED_CRYSTAL_BUTTON = "feedCrystalButton";
STATIC char const * Game::UI_FEED_CRYSTAL_LABEL = "feedCrystalLabel";
STATIC char const * Game::EQUIP_PRIMARY_EVENT = "EquipPrimaryEvent";
STATIC char const * Game::EJECT_ITEM_EVENT = "EjectItemEvent";
STATIC char const * Game::MOVE_ITEM_TO_INVENTORY_EVENT = "MoveItemToInventoryEvent";
STATIC char const * Game::BUTTON_DEDICATED_HOST_EVENT = "DedicatedHostEvent";
STATIC char const * Game::BUTTON_HOST_GAME_EVENT = "HostGameEvent";
STATIC char const * Game::BUTTON_JOIN_GAME_EVENT = "JoinGameEvent";
STATIC char const * Game::BUTTON_FIGHTER_CLASS_EVENT = "ChooseFighterEvent";
STATIC char const * Game::BUTTON_WIZARD_CLASS_EVENT = "ChooseWizardEvent";
STATIC char const * Game::BUTTON_ROGUE_CLASS_EVENT = "ChooseRogueEvent";
STATIC char const * Game::BUTTON_CULTIST_CLASS_EVENT = "ChooseCultistEvent";
STATIC char const * Game::BUTTON_UPGRADE_HULL_EVENT = "UpgradeHullEvent";
STATIC char const * Game::BUTTON_UPGRADE_HEALTH_EVENT = "UpgradeHealthEvent";
STATIC char const * Game::BUTTON_UPGRADE_SHIELD_EVENT = "UpgradeShieldEvent";
STATIC char const * Game::BUTTON_UPGRADE_ENERGY_EVENT = "UpgradeEnergyEvent";
STATIC char const * Game::BUTTON_FEED_CRYSTAL_EVENT = "FeedCrystalEvent";
STATIC char const * Game::BUTTON_EMOTE_HAPPY_EVENT = "EmoteHappyEvent";
STATIC char const * Game::BUTTON_EMOTE_SAD_EVENT = "EmoteSadEvent";
STATIC char const * Game::BUTTON_EMOTE_ANGRY_EVENT = "EmoteAngryEvent";
STATIC char const * Game::BUTTON_EMOTE_WHOOPS_EVENT = "EmoteWhoopsEvent";
STATIC char const * Game::BUTTON_EMOTE_HELP_EVENT = "EmoteHelpEvent";


//-------------------------------------------------------------------------------------------------
STATIC uint8_t Game::GetSessionIndex( )
{
	return s_netSession->GetSelf( )->GetIndex( );
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::WriteCompressedPosition( NetMessage * out_message, Vector2f const & position )
{
	Vector2f positionBounds = Vector2f( MAX_MAP_RADIUS + 10.f, MAX_MAP_RADIUS + 10.f );
	uint32_t compressedPosition = CompressVector2fToUint32( position, -positionBounds, positionBounds );
	out_message->Write<uint32_t>( compressedPosition );
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::ReadCompressedPosition( NetMessage const & message, Vector2f * out_position )
{
	uint32_t compressedPosition;
	message.Read<uint32_t>( &compressedPosition );
	Vector2f positionBounds = Vector2f( MAX_MAP_RADIUS + 10.f, MAX_MAP_RADIUS + 10.f );
	*out_position = DecompressUint32ToVector2f( compressedPosition, -positionBounds, positionBounds );
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::WriteCompressedVelocity( NetMessage * out_message, Vector2f const & velocity )
{
	Vector2f velocityBounds = Vector2f( MAX_SPEED, MAX_SPEED );
	uint32_t compressedVelocity = CompressVector2fToUint32( velocity, -velocityBounds, velocityBounds );
	out_message->Write<uint32_t>( compressedVelocity );
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::ReadCompressedVelocity( NetMessage const & message, Vector2f * out_velocity )
{
	uint32_t compressedVelocity;
	message.Read<uint32_t>( &compressedVelocity );
	Vector2f velocityBounds = Vector2f( MAX_SPEED, MAX_SPEED );
	*out_velocity = DecompressUint32ToVector2f( compressedVelocity, -velocityBounds, velocityBounds );
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::WriteCompressedSpeed( NetMessage * out_message, float speed )
{
	uint16_t compressedSpeed = CompressFloat32ToUint16( speed, -MAX_SPEED, MAX_SPEED );
	out_message->Write<uint16_t>( compressedSpeed );

}


//-------------------------------------------------------------------------------------------------
STATIC void Game::ReadCompressedSpeed( NetMessage const & message, float * out_speed )
{
	uint16_t compressedSpeed;
	message.Read<uint16_t>( &compressedSpeed );
	*out_speed = DecompressUint16ToFloat32( compressedSpeed, -MAX_SPEED, MAX_SPEED );
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::WriteCompressedRotation( NetMessage * out_message, float rotation )
{
	uint8_t compressedRotation = CompressFloat32ToUint8( rotation, 0.f, 360.f );
	out_message->Write<uint8_t>( compressedRotation );
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::ReadCompressedRotation( NetMessage const & message, float * out_rotation )
{
	uint8_t compressedRotation = 0U;
	message.Read<uint8_t>( &compressedRotation );
	*out_rotation = DecompressUint8ToFloat32( compressedRotation, 0.f, 360.f );
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::WriteCompressedBoolean( NetMessage * out_message, bool actual )
{
	out_message->Write<uint8_t>( actual ? 1U : 0U );
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::ReadCompressedBoolean( NetMessage const & message, bool * actual )
{
	uint8_t compressed = 0U;
	message.Read<uint8_t>( &compressed );
	*actual = compressed == 1U;
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::WriteUncompressedUint8( NetMessage * out_message, uint8_t data )
{
	out_message->Write<uint8_t>( data );
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::ReadUncompressedUint8( NetMessage const & message, uint8_t * out_data )
{
	message.Read<uint8_t>( out_data );
}

//-------------------------------------------------------------------------------------------------
STATIC void Game::WriteUncompressedUint16( NetMessage * out_message, uint16_t data )
{
	out_message->Write<uint16_t>( data );
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::ReadUncompressedUint16( NetMessage const & message, uint16_t * out_data )
{
	message.Read<uint16_t>( out_data );
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::WriteUncompressedUint32( NetMessage * out_message, uint32_t data )
{
	out_message->Write<uint32_t>( data );
}


//-------------------------------------------------------------------------------------------------
STATIC void Game::ReadUncompressedUint32( NetMessage const & message, uint32_t * out_data )
{
	message.Read<uint32_t>( out_data );
}


//-------------------------------------------------------------------------------------------------
Game::Game( )
	: m_numHostPlayers( 0 )
	, m_numClientPlayers( 0 )
	, m_ClientGameActivityLog( "Data/Logs/GameplayActivity_Client.txt" )
	, m_HostGameActivityLog( "Data/Logs/GameplayActivity_Host.txt" )
	, m_playerSpawnPoint( 0.f )
	, m_titleScreen( nullptr )
	, m_crystalDestroyedOverlay( nullptr )
	, m_playerHUD( nullptr )
	, m_targetHUD( nullptr )
	, m_debugHUD( nullptr )
	, m_usernameField( nullptr )
	, m_passwordField( nullptr )
	, m_addressField( nullptr )
	, m_invalidPasswordLabel( nullptr )
	, m_playerClassLabel( nullptr )
	, m_playerNameLabel( nullptr )
	, m_playerLevelLabel( nullptr )
	, m_playerMoneyLabel( nullptr )
	, m_upgradeCostLabel( nullptr )
	, m_targetNameLabel( nullptr )
	, m_targetLevelLabel( nullptr )
	, m_targetHealthBar( nullptr )
	, m_targetShieldBar( nullptr )
	, m_targetEnergyBar( nullptr )
	, m_mapCoordinatesLabel( nullptr )
	, m_debugConnection( new UILabel*[MAX_PLAYERS] )
	, m_dedicatedHostLabel( nullptr )
	, m_playerHealthBar( nullptr )
	, m_playerShieldBar( nullptr )
	, m_playerEnergyBar( nullptr )
	, m_playerHealthStat( nullptr )
	, m_playerShieldStat( nullptr )
	, m_playerEnergyStat( nullptr )
	, m_uiGodMode( nullptr )
	, m_warpInstruction( nullptr )
	, m_warpDriveBar( nullptr )
	, m_summoningBar( nullptr )
	, m_classSelectScreen( nullptr )
	, m_worldContainer( nullptr )
	, m_equipmentPrimary( nullptr )
	, m_feedCrystalButton( nullptr )
	, m_feedCrystalLabel( nullptr )
	, m_targetInfo1Label( nullptr )
	, m_targetInfo2Label( nullptr )
	, m_targetInfo3Label( nullptr )
	, m_targetInfo4Label( nullptr )
	, m_targetingBracket( nullptr )
	, m_backgroundGrid( nullptr )
	, m_background( nullptr )
	, m_minimapCrystal( nullptr )
	, m_minimap( nullptr )
	, m_minimapBorder( nullptr )
	, m_minimapBackground( nullptr )
	, m_gameState( )
	, m_showDebug( false )
	, m_lastInvalidPasswordTime( 0.f )
	, m_isDedicatedServer( false )
	, m_hostGodMode( false )
	, m_stopRendering( false )
	, m_crystalDestroyed( false )
	, m_quitNextFrame( false )
	, m_cameraRotation( 0.f )
	, m_viewSize( NORMAL_VIEW_SCALE )
	, m_cameraPosition( Vector2f::ZERO )
{
	SetupUISystem( );
	SetupSpriteRenderer( );
	SetupNetSession( );
	SetupSprites( );
	SetupTextRenderers( );
	RegisterConsoleCommands( );

	//Run through and initialize static variables
	GameObject::Initialize( );
	Ship::Initialize( );
	Player::Initialize( );
}


//-------------------------------------------------------------------------------------------------
Game::~Game( )
{
	//Courtesy leave when you close the app
	Game::s_netSession->Leave( );

	//This thing has a red squiggly line under it, but that can be ignored. MVS glitch
	EventSystem::Unregister( this );

	CleanupPlayersAndGameObjects( );
	CleanupSprites( );

	delete m_debugConnection;
	m_debugConnection = nullptr;

	delete g_ParticleEngine;
	g_ParticleEngine = nullptr;

	delete s_netSession;
	s_netSession = nullptr;
}


//-------------------------------------------------------------------------------------------------
void Game::Update( )
{
	if( m_quitNextFrame )
	{
		s_netSession->Leave( );
		m_quitNextFrame = false;
	}
	UpdateInput( );
	m_gameState.Update( );
	UpdateGameObjects( );
	UpdateCamera( );
	UpdateUI( );
	m_HostGameActivityLog.WriteToFile( );
	m_ClientGameActivityLog.WriteToFile( );
}


//-------------------------------------------------------------------------------------------------
void Game::UpdateInput( )
{
	if( s_netSession->GetState( ) == eNetSessionState_DISCONNECTED )
	{
		//Tab advances current selection
		if( g_InputSystem->WasKeyJustPressed( Input::KEY_TAB ) )
		{
			UIWidget * selected = g_UISystem->GetSelectedWidget( );
			if( m_usernameField == selected )
			{
				m_passwordField->Select( );
			}
			else if( m_passwordField == selected )
			{
				m_addressField->Select( );
			}
			else if( m_addressField == selected )
			{
				m_usernameField->Select( );
			}
			else
			{
				m_usernameField->Select( );
			}
		}

		//Quit to main menu, then quit
		if( g_InputSystem->WasKeyJustPressed( Input::KEY_ESCAPE ) )
		{
			g_isQuitting = true;
		}
	}

	if( s_netSession->GetState( ) != eNetSessionState_CONNECTED )
	{
		return;
	}

	uint16_t inputBitfield = 0;

	if( g_InputSystem->IsKeyDown( 'W' ) )
	{
		SetBit( &inputBitfield, eGameButton_UP );
	}

	if( g_InputSystem->IsKeyDown( 'S' ) )
	{
		SetBit( &inputBitfield, eGameButton_DOWN );
	}

	if( g_InputSystem->IsKeyDown( 'A' ) )
	{
		SetBit( &inputBitfield, eGameButton_LEFT );
	}

	if( g_InputSystem->IsKeyDown( 'D' ) )
	{
		SetBit( &inputBitfield, eGameButton_RIGHT );
	}

	if( m_isDedicatedServer )
	{
		if( g_InputSystem->IsKeyDown( 'W' ) )
		{
			m_cameraPosition.y -= 5.0f * Time::DELTA_SECONDS;
		}
		
		if( g_InputSystem->IsKeyDown( 'S' ) )
		{
			m_cameraPosition.y += 5.0f * Time::DELTA_SECONDS;
		}
		
		if( g_InputSystem->IsKeyDown( 'A' ) )
		{
			m_cameraPosition.x += 5.0f * Time::DELTA_SECONDS;
		}
		
		if( g_InputSystem->IsKeyDown( 'D' ) )
		{
			m_cameraPosition.x -= 5.0f * Time::DELTA_SECONDS;
		}
	}
	else
	{
		//Dummy Name Properties
		NamedProperties empty = NamedProperties( );
		if( g_InputSystem->WasKeyJustReleased( '1' ) )
		{
			Game::OnEmoteHappyButton( empty );
		}
		if( g_InputSystem->WasKeyJustReleased( '2' ) )
		{
			Game::OnEmoteSadButton( empty );
		}
		if( g_InputSystem->WasKeyJustReleased( '3' ) )
		{
			Game::OnEmoteAngryButton( empty );
		}
		if( g_InputSystem->WasKeyJustReleased( '4' ) )
		{
			Game::OnEmoteWhoopsButton( empty );
		}
		if( g_InputSystem->WasKeyJustReleased( '5' ) )
		{
			Game::OnEmoteHelpButton( empty );
		}
		if( g_InputSystem->WasKeyJustReleased( 'F' ) )
		{
			Game::OnFeedCrystalButton( empty );
		}
	}

	if( IsHost( ) )
	{
		Player * currentHost = HostGetPlayer( 0 );
		//Toggle god mode
		if( g_InputSystem->WasKeyJustPressed( 'G' ) )
		{
			m_hostGodMode = !m_hostGodMode;
		}
		m_uiGodMode->SetHidden( !m_hostGodMode );
		//Add 100 cubes
		if( g_InputSystem->WasKeyJustPressed( 'C' ) )
		{
			currentHost->m_money += 100;
			HostUpdatePlayer( 0 );
		}
		//Spawn random level 1 weapon
		if( g_InputSystem->WasKeyJustPressed( 'I' ) )
		{
			HostSpawnItem( currentHost->GetLastPosition( ), Item::GenerateRandomWeapon( 1U ) );
		}
		//Magnet a random bad guy to yourself
		if( g_InputSystem->WasKeyJustPressed( 'B' ) )
		{
			Ship * evilShip;
			g_GameSystem->HostGetClosestEvilShip( currentHost->GetLastPosition( ) + RandomUnitVectorCircle( ) * 20.f, &evilShip );
			if( evilShip )
			{
				Vector2f adjacentPosition = currentHost->GetLastPosition( ) + RandomUnitVectorCircle( );
				evilShip->m_position = adjacentPosition;
			}
		}
		//Kill yourself
		if( g_InputSystem->WasKeyJustPressed( 'K' ) )
		{
			PlayerShip * hostShip = currentHost->GetPlayerShip( );
			if( hostShip )
			{
				hostShip->m_health = 0;
			}
		}
		//Hurt Crystal
		if( g_InputSystem->WasKeyJustPressed( 'L' ) )
		{
			Ship * crystal = m_gameState.m_hostCrystal;
			crystal->HostTakeDamage( 1000U, 0U );
		}
		//Increase Summoning Bar
		if( g_InputSystem->WasKeyJustPressed( 'X' ) )
		{
			HostIncreaseSummoningBar( 9U );
		}
		//Show Client-Side Prediction
		if( g_InputSystem->WasKeyJustPressed( 'H' ) )
		{
			Game::s_showHost = !Game::s_showHost;
		}
	}

	//Toggle rendering
	if( g_InputSystem->WasKeyJustPressed( 'P' ) )
	{
		m_stopRendering = !m_stopRendering;
	}

	//Quit to main menu, then quit
	if( g_InputSystem->WasKeyJustPressed( Input::KEY_ESCAPE ) )
	{
		if( s_netSession->IsConnected( ) )
		{
			s_netSession->Leave( );
		}
	}

	Player * currentPlayer = ClientGetPlayerSelf( );
	if( currentPlayer )
	{
		//Set Input State
		PlayerShip * currentShip = currentPlayer->GetPlayerShip( );
		currentPlayer->SetInputState( inputBitfield );
		
		//Send Input Events
		if( currentShip )
		{
			//Player shooting
			if( g_InputSystem->IsKeyDown( ' ' ) )
			{
				if( currentShip->CanFire( ) )
				{
					ClientRequestEvent( eGameEvent_PRESS_SHOOT );
					currentShip->ResetFireCooldown( );
				}
			}

			//Player warping
			if( g_InputSystem->WasKeyJustPressed( 'E' ) )
			{
				if( !currentPlayer->IsEvilClass( ) )
				{
					ClientRequestEvent( eGameEvent_WARP_DRIVE );
				}
			}

			//Player ejecting item
			if( g_InputSystem->WasKeyJustPressed( Input::KEY_TAB ) )
			{
				if( currentShip->m_isWarping && currentPlayer->m_inventorySize > 0 )
				{
					byte_t itemIndex = currentPlayer->GetLastInventoryIndex( );
					ClientRequestRemoveItem( itemIndex );
				}
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Game::UpdateGameObjects( )
{
	g_ParticleEngine->Update( );

	// Update Players
	UpdatePlayers( );

	// Update Host Objects
	if( IsHost( ) )
	{
		//Update Spawners
		ShipSpawner::UpdateAll( );

		//Update
		for( size_t hostObjectIndex = 0; hostObjectIndex < m_hostObjects.size( ); ++hostObjectIndex )
		{
			m_hostObjects[hostObjectIndex]->GetLocalObject( )->Update( );
		}

		//Collide
		for( size_t hostObjectIndex = 0; hostObjectIndex < m_hostObjects.size( ); ++hostObjectIndex )
		{
			HostCheckCollisions( m_hostObjects[hostObjectIndex] );
		}

		//Cleanup
		for( size_t hostObjectIndex = 0; hostObjectIndex < m_hostObjects.size( ); ++hostObjectIndex )
		{
			NetGameObject * hostObject = m_hostObjects[hostObjectIndex];
			GameObject * localGameObject = hostObject->GetLocalObject( );
			if( !localGameObject->HostIsAlive( ) )
			{
				HostDestroyNetGameObject( m_hostObjects[hostObjectIndex] );
			}
		}
	}

	// Update Client Objects
	for( size_t clientObjectIndex = 0; clientObjectIndex < m_clientObjects.size( ); ++clientObjectIndex )
	{
		m_clientObjects[clientObjectIndex]->GetLocalObject( )->Update( );
	}
}


//-------------------------------------------------------------------------------------------------
//#TODO: Make this much faster
Vector2f CalcScreenPosition( Vector2f const & shipPosition )
{
	Matrix4f view = g_SpriteRenderSystem->GetView( );
	Vector2f camPosition = view.GetWorldPositionFromViewNoScale( ).XY( );
	Vector2f offsetFromCenter = shipPosition - camPosition;
	Vector3f right;
	Vector3f up;
	Vector3f forward;
	Vector3f pos;
	view.GetBasis( &right, &up, &forward, &pos );
	Matrix4f rotMat;
	rotMat.SetBasis( right, up, forward );
	//rotMat.SetRotation( Euler( 0.f, 0.f, m_clientPlayers[0]->GetPlayerShip( )->m_rotationDegrees - 90.f ) );
	return ( Vector4f( offsetFromCenter, 0.f, 0.f ) * rotMat ).XY( );
}


//-------------------------------------------------------------------------------------------------
void Game::UpdateCamera( )
{
	//If we have a player
	Player * player = ClientGetPlayerSelf( );
	if( player )
	{
		PlayerShip * ship = player->GetPlayerShip( );
		//And if the player owns a ship
		if( ship )
		{
			if( ship->GetPlayerClass( ) == ePlayerClass_CTHULHU )
			{
				m_viewSize = Lerp( m_viewSize, CTHULHU_VIEW_SCALE, 0.1f );
			}
			else
			{
				m_viewSize = Lerp( m_viewSize, NORMAL_VIEW_SCALE, 0.1f );
			}

			Matrix4f viewPosition;
			Matrix4f viewRotation;
			Matrix4f viewOffset;
			static Euler cameraRotation( 0.f, 0.f, 0.f );
			static Vector3f cameraPosition( 0.f, 0.f, 0.f );
			float currentShipRotation = ship->m_rotationDegrees - 90.f; //Rotate 90 so the ship is pointing up
			Vector3f currentShipPosition( -ship->m_position, 0.f );
			float rotationDistance = ShortestSignedAngularDistance( cameraRotation.m_rollDegreesAboutZ, currentShipRotation );
			cameraRotation.m_rollDegreesAboutZ += rotationDistance * 0.1f;
			cameraPosition = Lerp( cameraPosition, currentShipPosition, 0.1f );
			viewPosition.SetTranslation( cameraPosition );
			viewRotation.SetRotation( cameraRotation );
			viewOffset.SetTranslation( Vector3f( 0.f, -3.f, 0.f ) );
			Matrix4f view = viewPosition * viewRotation * viewOffset;

			m_cameraRotation = cameraRotation.m_rollDegreesAboutZ;
			m_cameraPosition = -cameraPosition.XY( );
			g_SpriteRenderSystem->SetView( view );
			g_SpriteRenderSystem->SetVirtualSize( m_viewSize );
		}
	}
	//Manual camera movement
	else if( m_isDedicatedServer )
	{
		Matrix4f viewPosition;
		Matrix4f viewRotation;
		Matrix4f viewOffset;
		static Euler cameraRotation( 0.f, 0.f, 0.f );
		static Vector3f cameraPosition( 0.f, 0.f, 0.f );
		float currentShipRotation = 0.f;
		Vector3f currentShipPosition( m_cameraPosition, 0.f );
		float rotationDistance = ShortestSignedAngularDistance( cameraRotation.m_rollDegreesAboutZ, currentShipRotation );
		cameraRotation.m_rollDegreesAboutZ += rotationDistance * 0.1f;
		cameraPosition = Lerp( cameraPosition, currentShipPosition, 0.1f );
		viewPosition.SetTranslation( cameraPosition );
		viewRotation.SetRotation( cameraRotation );
		viewOffset.SetTranslation( Vector3f( 0.f, -3.f, 0.f ) );
		Matrix4f view = viewPosition * viewRotation * viewOffset;

		g_SpriteRenderSystem->SetView( view );
		g_SpriteRenderSystem->SetVirtualSize( m_viewSize );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::UpdatePlayers( )
{
	if( !IsHost( ) )
	{
		return;
	}

	for( byte_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
	{
		Player * currentPlayer = m_hostPlayers[playerIndex];
		if( currentPlayer == nullptr )
		{
			continue;
		}

		currentPlayer->SetLastPosition( );

		//Update respawn
		currentPlayer->CheckRespawn( );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::UpdateUI( )
{
	//Title Screen
	UpdateTitleScreenUI( );

	//Debug
	UpdateDebugUI( );

	//HUD
	UpdatePlayerUI( );

	//Class Select
	UpdateClassSelectUI( );

	//Targeting
	UpdateTargetingUI( );

	//#TODO: Remove and update
	//Update minimap sprites
	UpdateMinimap( );
}


//-------------------------------------------------------------------------------------------------
void Game::UpdateTitleScreenUI( )
{
	static float crystalDestroyedOverlayTimer = 0.f;
	static float overlayDuration = 5.0f;

	if( m_crystalDestroyed )
	{
		m_crystalDestroyed = false;
		crystalDestroyedOverlayTimer = 1.0f;
		m_crystalDestroyedOverlay->SetHidden( false );
	}
	else
	{
		if( crystalDestroyedOverlayTimer > 0.f )
		{
			crystalDestroyedOverlayTimer -= Time::DELTA_SECONDS / overlayDuration;
		}
		if( crystalDestroyedOverlayTimer < 0.f )
		{
			crystalDestroyedOverlayTimer = 0.f;
			m_crystalDestroyedOverlay->SetHidden( true );
		}
	}

	if( s_netSession->GetState( ) == eNetSessionState_DISCONNECTED )
	{
		m_titleScreen->SetHidden( false );
		float elapsedTime = Time::TOTAL_SECONDS - m_lastInvalidPasswordTime;
		float colorAlpha = RangeMap( 0.f, 1.f, 1.f - elapsedTime, 0.f, 255.f );
		m_invalidPasswordLabel->SetProperty( UITextField::PROPERTY_TEXT_COLOR, Color( 255, 0, 0, (byte_t) colorAlpha ) );
		m_invalidPasswordLabel->SetProperty( UITextField::PROPERTY_BACKGROUND_COLOR, Color( 0, 0, 0, (byte_t) ( colorAlpha / 2.f ) ) );
	}
	else
	{
		m_titleScreen->SetHidden( true );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::UpdateDebugUI( )
{
	if( m_showDebug )
	{
		if( m_isDedicatedServer )
		{
			m_dedicatedHostLabel->SetHidden( false );
		}

		m_debugHUD->SetHidden( false );
		for( uint8_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
		{
			NetConnection * connection = s_netSession->GetNetConnection( playerIndex );
			uint8_t uiIndex = playerIndex;
			if( m_isDedicatedServer )
			{
				if( playerIndex == 0 )
				{
					continue;
				}
				uiIndex = playerIndex - 1;
			}
			if( connection )
			{
				//Not enough space to show all the connections
				if( m_debugConnection[uiIndex] == nullptr )
				{
					continue;
				}

				int ping = (int) ( connection->GetRoundTripTime( ) * 1000.0 );
				char const * username = strncmp( connection->GetUsername( ), "", 255 ) == 0 ? "Host" : connection->GetUsername( );
				std::string connectionString = Stringf( "%.10s | P=%u | M=%u | Bytes=(%u/%u) | Ping=%dms",
					username,
					connection->m_lastOutgoingPackageCount,
					connection->m_lastOutgoingMessageCount,
					connection->m_lastOutgoingByteCountPacketHeader + connection->m_lastOutgoingByteCountMessages,
					NetPacket::MAX_SIZE,
					ping );
				m_debugConnection[uiIndex]->SetProperty( UILabel::PROPERTY_TEXT, connectionString );
				int lowPing = 100;
				int mediumPing = 200;
				if( ping < lowPing )
				{
					m_debugConnection[uiIndex]->SetProperty( UILabel::PROPERTY_TEXT_COLOR, Color::GREEN );
				}
				else if( ping < mediumPing )
				{
					m_debugConnection[uiIndex]->SetProperty( UILabel::PROPERTY_TEXT_COLOR, Color::ORANGE );
				}
				else
				{
					m_debugConnection[uiIndex]->SetProperty( UILabel::PROPERTY_TEXT_COLOR, Color::RED );
				}
			}
			else
			{
				if( playerIndex < MAX_PLAYERS && m_debugConnection[playerIndex] != nullptr )
				{
					m_debugConnection[uiIndex]->SetProperty( UILabel::PROPERTY_TEXT, "" );
				}
			}
		}
	}
	else
	{
		m_dedicatedHostLabel->SetHidden( true );
		m_debugHUD->SetHidden( true );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::UpdatePlayerUI( )
{
	Player * currentPlayer = ClientGetPlayerSelf( );
	if( currentPlayer )
	{
		if( currentPlayer->IsEvilClass( ) )
		{
			m_feedCrystalButton->SetHidden( true );
			m_feedCrystalLabel->SetHidden( true );
		}
		else
		{
			m_feedCrystalButton->SetHidden( false );
			m_feedCrystalLabel->SetHidden( false );
		}

		PlayerShip * currentShip = currentPlayer->GetPlayerShip( );
		if( currentShip )
		{	
			//Show how to warp
			if( currentPlayer->IsEvilClass( ) )
			{
				m_warpInstruction->SetHidden( true );
			}
			else
			{
				m_warpInstruction->SetHidden( false );
			}

			//Show HUD
			m_playerHUD->SetHidden( false );
			m_worldContainer->SetHidden( false );
			m_mapCoordinatesLabel->SetHidden( false );

			//Set Properties
			m_playerClassLabel->SetProperty( UILabel::PROPERTY_TEXT, Player::ParseClassName( currentPlayer->m_class ) );
			m_playerClassLabel->SetProperty( UILabel::PROPERTY_BACKGROUND_COLOR, Player::ParseClassColor( currentPlayer->m_class ) );
			m_playerNameLabel->SetProperty( UILabel::PROPERTY_TEXT, currentPlayer->GetUsername( ) );
			std::string levelString = Stringf( "%u", currentPlayer->m_level );
			m_playerLevelLabel->SetProperty( UILabel::PROPERTY_TEXT, levelString );
			std::string moneyString = Stringf( "%u", currentPlayer->m_money );
			m_playerMoneyLabel->SetProperty( UILabel::PROPERTY_TEXT, moneyString );
			std::string upgradeCostString = Stringf( "Cost %u", currentPlayer->GetNextUpgradeCost( ) );
			m_upgradeCostLabel->SetProperty( UILabel::PROPERTY_TEXT, upgradeCostString );
			std::string mapCoords = Stringf( "(%.2f,%.2f)", currentShip->m_position.x, currentShip->m_position.y );
			m_mapCoordinatesLabel->SetProperty( UILabel::PROPERTY_TEXT, mapCoords );
			m_playerHealthBar->SetAmount( currentShip->m_health, currentShip->GetMaxHealth( ) );
			m_playerHealthBar->m_text->SetText( Stringf("%u / %u", currentShip->m_health, currentShip->GetMaxHealth( ) ) );
			m_playerHealthStat->SetAmount( currentShip->GetMaxHealth( ), (int) Player::GetClassStatMax( currentShip->GetPlayerClass( ), eStat_HEALTH ) );
			m_playerShieldBar->SetAmount( currentShip->m_shield, currentShip->GetMaxShield( ) );
			m_playerShieldBar->m_text->SetText( Stringf( "%u / %u", currentShip->m_shield, currentShip->GetMaxShield( ) ) );
			m_playerShieldStat->SetAmount( currentShip->GetMaxShield( ), (int) Player::GetClassStatMax( currentShip->GetPlayerClass( ), eStat_SHIELD ) );
			m_playerEnergyBar->SetAmount( currentShip->m_energy, currentShip->GetMaxEnergy( ) );
			m_playerEnergyBar->m_text->SetText( Stringf( "%u / %u", currentShip->m_energy, currentShip->GetMaxEnergy( ) ) );
			m_playerEnergyStat->SetAmount( currentShip->GetMaxEnergy( ), (int) Player::GetClassStatMax( currentShip->GetPlayerClass( ), eStat_ENERGY ) );
			m_warpDriveBar->SetHidden( !currentShip->m_isWarping );
			m_warpDriveBar->SetAmount( currentShip->GetCurrentWarpTime( ), currentShip->GetMaxWarpTime( ) );

			//Show leveling button
			if( !currentPlayer->IsMaxLevel( ) )
			{
				m_upgradeHullButton->SetHidden( false );
				m_upgradeHealthButton->SetHidden( true );
				m_upgradeShieldButton->SetHidden( true );
				m_upgradeEnergyButton->SetHidden( true );
			}
			//Show max level buttons
			else
			{
				m_upgradeHullButton->SetHidden( true );
				bool hideHealth = currentPlayer->IsHealthStatMaxed( );
				m_upgradeHealthButton->SetHidden( hideHealth );
				bool hideShield = currentPlayer->IsShieldStatMaxed( );
				m_upgradeShieldButton->SetHidden( hideShield );
				bool hideEnergy = currentPlayer->IsEnergyStatMaxed( );
				m_upgradeEnergyButton->SetHidden( hideEnergy );
				bool upgradesComplete = hideHealth && hideShield && hideEnergy;
				m_upgradeCostLabel->SetHidden( upgradesComplete );
			}

			//Disable/Enable Upgrade buttons
			currentPlayer->CanUpgradeHull( ) ? m_upgradeHullButton->Enable( ) : m_upgradeHullButton->Disable( );
			currentPlayer->CanUpgradeHealth( ) ? m_upgradeHealthButton->Enable( ) : m_upgradeHealthButton->Disable( );
			currentPlayer->CanUpgradeShield( ) ? m_upgradeShieldButton->Enable( ) : m_upgradeShieldButton->Disable( );
			currentPlayer->CanUpgradeEnergy( ) ? m_upgradeEnergyButton->Enable( ) : m_upgradeEnergyButton->Disable( );

			//If enemy, show the top bar as the cthulhu summoning bar
			if( currentPlayer->IsEvilClass( ) )
			{
				m_summoningBar->SetAmount( m_gameState.m_clientSummoningAmount, GameState::MAX_SUMMONING_BAR );
				m_summoningBar->m_text->SetText( "Cthulhu Summoning Progress" );
				m_summoningBar->SetProperty( UIProgressBar::PROPERTY_BAR_COLOR, Color::PURPLE );
			}

			//If ally, show the top bar as the Crystal life bar
			else
			{
				float crystalLifePercentage = m_gameState.GetCrystalLifePercent( false );
				m_summoningBar->SetAmount( crystalLifePercentage );
				m_summoningBar->m_text->SetText( "Crystal Health" );
				m_summoningBar->SetProperty( UIProgressBar::PROPERTY_BAR_COLOR, Color::LIGHT_BLUE );
			}
		}
		else
		{
			//Hide HUD
			m_playerHUD->SetHidden( true );
			m_worldContainer->SetHidden( true );
			m_mapCoordinatesLabel->SetHidden( true );
		}
	}
	else
	{
		//Hide HUD
		m_playerHUD->SetHidden( true );
		//m_debugHUD->SetHidden( true );
		m_worldContainer->SetHidden( true );
		m_mapCoordinatesLabel->SetHidden( true );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::UpdateClassSelectUI( )
{
	Player * currentPlayer = ClientGetPlayerSelf( );
	static float backgroundFade = 0.f;
	static float fadeTimeTotalSeconds = 3.f;

	if( currentPlayer )
	{
		if( currentPlayer->CanChooseClass( ) )
		{
			if( backgroundFade < 0.8f )
			{
				backgroundFade += Time::DELTA_SECONDS / fadeTimeTotalSeconds;
			}
			else
			{
				backgroundFade = 0.8f;
			}
			m_classSelectBackground->SetProperty( UIBox::PROPERTY_BACKGROUND_COLOR, Color( 0, 0, 0, (uint8_t) ( 255.f * backgroundFade ) ) );
			m_classSelectScreen->SetHidden( false );
		}
		else
		{
			backgroundFade = 0.f;
			m_classSelectScreen->SetHidden( true );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Game::UpdateTargetingUI( )
{
	//Assume it's disable, unless we need it
	m_targetingBracket->SetEnabled( false );
	m_targetHUD->SetHidden( true );

	Player * currentPlayer = ClientGetPlayerSelf( );
	if( currentPlayer )
	{
		PlayerShip * currentShip = currentPlayer->GetPlayerShip( );
		if( currentShip )
		{
			//Show Targeting Frame
			m_targetHUD->SetHidden( false );

			//Get item associated with widget
			bool targetingHighlightedItem = false;
			UIItem * highlightedWidget = dynamic_cast<UIItem*>( g_UISystem->GetHighlightedWidget( ) );
			if( highlightedWidget )
			{
				byte_t inventoryIndex = 0U;
				eEquipmentSlot equipmentSlot = eEquipmentSlot_PRIMARY;
				if( currentPlayer->GetInventoryIndexForItem( highlightedWidget, &inventoryIndex ) )
				{
					targetingHighlightedItem = true;

					Item * inventoryItem = currentPlayer->m_inventory[inventoryIndex];
					uint16_t targetItemCode = inventoryItem->m_itemCode;
					UpdateTargetingUIForItemCode( targetItemCode );
				}
				else if( currentPlayer->GetEquipmentSlotForItem( highlightedWidget, &equipmentSlot ) )
				{
					targetingHighlightedItem = true;

					Item * equippedItem = currentPlayer->m_equipment[equipmentSlot];
					uint16_t targetItemCode = equippedItem->m_itemCode;
					UpdateTargetingUIForItemCode( targetItemCode );
				}
			}

			if( currentShip->HasTarget( ) )
			{
				m_targetingBracket->SetEnabled( true );

				//Update targeting bracket
				GameObject * currentTarget = currentShip->GetTarget( );
				float spriteScale = currentTarget->GetSpriteScale( );
				std::string spriteID = currentShip->GetTargetBracketSpriteID( );
				m_targetingBracket->SetID( spriteID );
				m_targetingBracket->SetScale( spriteScale );
				m_targetingBracket->SetPosition( currentTarget->m_position );

				//Update target frame
				if( targetingHighlightedItem )
				{
					//Nothing
				}

				else if( currentTarget->m_type == eNetGameObjectType_ALLYSHIP ||
						 currentTarget->m_type == eNetGameObjectType_ENEMYSHIP ||
						 currentTarget->m_type == eNetGameObjectType_PLAYERSHIP )
				{
					//Parse target name
					if( currentTarget->m_type == eNetGameObjectType_PLAYERSHIP )
					{
						PlayerShip * targetShip = dynamic_cast<PlayerShip*>( currentTarget );
						Player * targetPlayer = ClientGetPlayer( targetShip->m_playerIndex );
						m_targetNameLabel->SetProperty( UILabel::PROPERTY_TEXT, targetPlayer->GetUsername( ) );
					}
					else
					{
						m_targetNameLabel->SetProperty( UILabel::PROPERTY_TEXT, currentTarget->GetEntityName( ) );
					}

					m_targetInfo1Label->SetProperty( UILabel::PROPERTY_TEXT, "" );
					m_targetInfo2Label->SetProperty( UILabel::PROPERTY_TEXT, "" );
					m_targetInfo3Label->SetProperty( UILabel::PROPERTY_TEXT, "" );
					m_targetInfo4Label->SetProperty( UILabel::PROPERTY_TEXT, "" );

					//Parse target level
					uint8_t level = 0U;
					if( currentTarget->m_type == eNetGameObjectType_ALLYSHIP )
					{
						level = 10U;
						AllyShip * targetShip = dynamic_cast<AllyShip*>( currentTarget );
						if( targetShip->IsCrystal() )
						{
							m_targetInfo1Label->SetProperty( UILabel::PROPERTY_TEXT, "Provides life to the" );
							m_targetInfo2Label->SetProperty( UILabel::PROPERTY_TEXT, "world." );
							//m_targetInfo2Label->SetProperty( UILabel::PROPERTY_TEXT, "beast." );
						}
						else
						{
							m_targetInfo1Label->SetProperty( UILabel::PROPERTY_TEXT, "Devoted to protecting" );
							m_targetInfo2Label->SetProperty( UILabel::PROPERTY_TEXT, "the Crystal at all" );
							m_targetInfo3Label->SetProperty( UILabel::PROPERTY_TEXT, "costs." );
						}
					}
					else if (currentTarget->m_type == eNetGameObjectType_ENEMYSHIP )
					{
						EnemyShip * targetShip = dynamic_cast<EnemyShip*>( currentTarget );
						if( targetShip )
						{
							level = targetShip->GetLevel( );
						}
						if( targetShip->IsBoss( ) )
						{
							m_targetInfo1Label->SetProperty( UILabel::PROPERTY_TEXT, "Large stationary" );
							m_targetInfo2Label->SetProperty( UILabel::PROPERTY_TEXT, "beast." );
						}
						else
						{
							m_targetInfo1Label->SetProperty( UILabel::PROPERTY_TEXT, "Long-ranged grunt." );
						}
					}
					else if( currentTarget->m_type == eNetGameObjectType_PLAYERSHIP )
					{
						PlayerShip * targetShip = dynamic_cast<PlayerShip*>( currentTarget );
						Player * targetPlayer = ClientGetPlayer( targetShip->m_playerIndex );
						if( targetShip )
						{
							level = targetPlayer->m_level;
						}
					}

					Ship * targetShip = static_cast<Ship*>( currentTarget );
					m_targetLevelLabel->SetProperty( UILabel::PROPERTY_TEXT, Stringf( "%u", level ).c_str( ) );
					m_targetHealthBar->SetHidden( false );
					m_targetHealthBar->SetAmount( targetShip->m_health, targetShip->GetMaxHealth( ) );
					m_targetHealthBar->m_text->SetText( Stringf( "%u / %u", targetShip->m_health, targetShip->GetMaxHealth( ) ) );
					m_targetShieldBar->SetHidden( false );
					m_targetShieldBar->SetAmount( targetShip->m_shield, targetShip->GetMaxShield( ) );
					m_targetShieldBar->m_text->SetText( Stringf( "%u / %u", targetShip->m_shield, targetShip->GetMaxShield( ) ) );
					m_targetEnergyBar->SetHidden( false );
					m_targetEnergyBar->SetAmount( targetShip->m_energy, targetShip->GetMaxEnergy( ) );
					m_targetEnergyBar->m_text->SetText( Stringf( "%u / %u", targetShip->m_energy, targetShip->GetMaxEnergy( ) ) );
				}

				else if( currentTarget->m_type == eNetGameObjectType_PICKUP )
				{
					Pickup * targetPickup = (Pickup*) currentTarget;
					uint16_t targetItemCode = targetPickup->m_itemCode;
					UpdateTargetingUIForItemCode( targetItemCode );
				}

				else if( currentTarget->m_type == eNetGameObjectType_ROCK )
				{
					m_targetNameLabel->SetProperty( UILabel::PROPERTY_TEXT, currentTarget->GetEntityName( ) );
					m_targetHealthBar->SetHidden( false );
					m_targetShieldBar->SetHidden( true );
					m_targetEnergyBar->SetHidden( true );

					Rock * targetRock = static_cast<Rock*>( currentTarget );
					m_targetHealthBar->SetAmount( targetRock->m_health, targetRock->GetMaxHealth( ) );
					m_targetHealthBar->m_text->SetText( Stringf( "%u / %u", targetRock->m_health, targetRock->GetMaxHealth( ) ) );
					m_targetInfo1Label->SetProperty( UILabel::PROPERTY_TEXT, "Space rubble." );
					m_targetInfo2Label->SetProperty( UILabel::PROPERTY_TEXT, "Avoid crashing into it." );
					m_targetInfo3Label->SetProperty( UILabel::PROPERTY_TEXT, "" );
					m_targetInfo4Label->SetProperty( UILabel::PROPERTY_TEXT, "" );
				}
			}
			//No target
			else if( !highlightedWidget )
			{
				m_targetNameLabel->SetProperty( UILabel::PROPERTY_TEXT, "" );
				m_targetHealthBar->SetHidden( true );
				m_targetShieldBar->SetHidden( true );
				m_targetEnergyBar->SetHidden( true );
				m_targetLevelLabel->SetProperty( UILabel::PROPERTY_TEXT, "" );
				m_targetInfo1Label->SetProperty( UILabel::PROPERTY_TEXT, "" );
				m_targetInfo2Label->SetProperty( UILabel::PROPERTY_TEXT, "" );
				m_targetInfo3Label->SetProperty( UILabel::PROPERTY_TEXT, "" );
				m_targetInfo4Label->SetProperty( UILabel::PROPERTY_TEXT, "" );
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Game::UpdateTargetingUIForItemCode( uint16_t itemCode )
{
	m_targetHealthBar->SetHidden( true );
	m_targetShieldBar->SetHidden( true );
	m_targetEnergyBar->SetHidden( true );
	
	Item checkItem = Item( itemCode );
	if( checkItem.IsWeapon( ) )
	{
		byte_t level = Item::ParseLevel( itemCode );
		uint16_t damage = Item::ParseDamage( itemCode );
		float fireRate = Item::ParseFireRate( itemCode );
		uint16_t energyCost = Item::ParseEnergyCost( itemCode );
		m_targetNameLabel->SetProperty( UILabel::PROPERTY_TEXT, Item::ParseName( itemCode ) );
		m_targetLevelLabel->SetProperty( UILabel::PROPERTY_TEXT, Stringf( "%u", level ).c_str( ) );
		m_targetInfo1Label->SetProperty( UILabel::PROPERTY_TEXT, Stringf( "Damage: %u", damage ).c_str( ) );
		m_targetInfo2Label->SetProperty( UILabel::PROPERTY_TEXT, Stringf( "Cooldown: %.2f sec", fireRate ).c_str( ) );
		m_targetInfo3Label->SetProperty( UILabel::PROPERTY_TEXT, Stringf( "Cost: %u", energyCost ).c_str( ) );
		m_targetInfo4Label->SetProperty( UILabel::PROPERTY_TEXT, Item::ParseEffectString( itemCode ) );
	}
	else if( checkItem.GetItemType() == eItemType_CUBES )
	{
		m_targetNameLabel->SetProperty( UILabel::PROPERTY_TEXT, Item::ParseName( itemCode ) );
		m_targetLevelLabel->SetProperty( UILabel::PROPERTY_TEXT, "" );
		m_targetInfo1Label->SetProperty( UILabel::PROPERTY_TEXT, Item::ParseEffectString( itemCode ) );
		m_targetInfo2Label->SetProperty( UILabel::PROPERTY_TEXT, "" );
		m_targetInfo3Label->SetProperty( UILabel::PROPERTY_TEXT, "" );
		m_targetInfo4Label->SetProperty( UILabel::PROPERTY_TEXT, "" );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::UpdateMinimap( )
{
	//Show / Hide Entire minimap
	Player * self = ClientGetPlayerSelf( );
	if( self )
	{
		PlayerShip * myShip = self->GetPlayerShip( );
		if( myShip )
		{
			m_minimapCrystal->SetEnabled( true );
			m_minimapCrystal->SetPosition( MINIMAP_CENTER * ( m_viewSize / 10.f ) );
			m_minimap->SetEnabled( true );
			m_minimap->SetPosition( MINIMAP_CENTER * ( m_viewSize / 10.f ) );
			m_minimapBorder->SetEnabled( true );
			m_minimapBorder->SetPosition( MINIMAP_CENTER * ( m_viewSize / 10.f ) );
			m_minimapBackground->SetEnabled( true );
			m_minimapBackground->SetPosition( MINIMAP_CENTER * ( m_viewSize / 10.f ) );

		}
		else
		{
			m_minimapCrystal->SetEnabled( false );
			m_minimap->SetEnabled( false );
			m_minimapBorder->SetEnabled( false );
			m_minimapBackground->SetEnabled( false );
		}
	}
	else
	{
		m_minimapCrystal->SetEnabled( false );
		m_minimap->SetEnabled( false );
		m_minimapBorder->SetEnabled( false );
		m_minimapBackground->SetEnabled( false );
	}
	m_backgroundGrid->SetScale( 10.f / g_SpriteRenderSystem->GetVirtualSize( ) );

	//Show / Hide individual people
	for( size_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
	{
		Player * client = m_clientPlayers[playerIndex];
		if( client == nullptr )
		{
			m_minimapShip[playerIndex]->SetEnabled( false );
			continue;
		}

		if( self )
		{
			bool alignmentMatches = self->IsEvilClass( ) == client->IsEvilClass( );
			bool isCthulhu = client->m_class == ePlayerClass_CTHULHU;
			if( client->m_localShip && ( alignmentMatches || isCthulhu ) )
			{
				PlayerShip * ship = client->GetPlayerShip( );
				Color classColor = Player::ParseClassColor( ship->GetPlayerClass( ) );
				classColor.Multiply( 2.f ); //This makes it 25% brighter
				m_minimapShip[playerIndex]->SetColor( classColor );
				Vector2f shipPosition = MINIMAP_CENTER + ship->m_position / MAX_MAP_RADIUS;
				m_minimapShip[playerIndex]->SetPosition( shipPosition * ( m_viewSize / 10.f ) );
				m_minimapShip[playerIndex]->SetRotation( -ship->m_rotationDegrees );
				m_minimapShip[playerIndex]->SetEnabled( true );
			}
			else
			{
				m_minimapShip[playerIndex]->SetEnabled( false );
			}	
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Game::Render( ) const
{
	if( m_isDedicatedServer && m_stopRendering )
	{
		//Nothing
	}
	else
	{
		g_SpriteRenderSystem->Render( );
	}
}


//-------------------------------------------------------------------------------------------------
float Game::GetAudioAttenuationFromCamera( Vector2f const & audioPosition )
{
	Player * checkClient = ClientGetPlayerSelf( );
	PlayerShip * checkClientShip = checkClient->GetPlayerShip( );
	if( checkClientShip )
	{
		Vector2f audioVectorToCamera = checkClientShip->m_position - audioPosition;
		float audioDistance = audioVectorToCamera.Length( );
		return audioDistance < MAX_AUDIO_DISTANCE ? 1.0f : 0.f;
	}
	return 0.f;
}


//-------------------------------------------------------------------------------------------------
Vector2f Game::FindBestRespawnLocation( bool isCultist )
{
	if( isCultist )
	{
		int randomSpawn = RandomInt( 4 );
		return m_cultistSpawnPoints[randomSpawn];
	}
	else
	{
		return m_playerSpawnPoint;
	}
}


//-------------------------------------------------------------------------------------------------
Vector2f Game::FindEmptySpawnLocation( )
{
	float radius = RandomFloat( MAX_MAP_RADIUS - 10.f ) + 10.f;
	Vector2f spawnLocation = RandomUnitVectorCircle( );
	return radius * spawnLocation;
}


//-------------------------------------------------------------------------------------------------
bool Game::LoadWorldFromFile( )
{
	bool const LOAD_SUCCESS = true;
	bool const LOAD_FAILED = false;

	//Find all saved worlds
	std::vector<std::string> worldSaveFiles = EnumerateFilesInFolder( "Data/Saves/World/", "*.World.xml" );

	//Needs to be at least one
	if( worldSaveFiles.size( ) == 0 )
	{
		return LOAD_FAILED;
	}

	//Just load the first one
	std::string & worldSavePath = worldSaveFiles[0];
	XMLNode world = XMLNode::openFileHelper( worldSavePath.c_str() ).getChildNode( STRING_WORLD_DATA );

	//Read GameState
	XMLNode gameStateData = world.getChildNode( 0 );
	m_gameState.UpdateFromXMLNode( gameStateData );

	//Read Spawners
	XMLNode gameObjectSpawners = world.getChildNode( STRING_GAME_OBJECT_SPAWNERS );
	for( int childIndex = 0; childIndex < gameObjectSpawners.nChildNode( ); ++childIndex )
	{
		XMLNode spawnerData = gameObjectSpawners.getChildNode( childIndex );
		ShipSpawner::CreateFromXMLNode( spawnerData );
	}

	//Read GameObjects
	XMLNode gameObjects = world.getChildNode( STRING_GAME_OBJECTS );
	for( int childIndex = 0; childIndex < gameObjects.nChildNode( ); ++childIndex )
	{
		XMLNode objectData = gameObjects.getChildNode( childIndex );
		GameObject * object = GameObject::CreateFromXMLNode( objectData );
		if( object )
		{
			ShipSpawner::AddExistingObjectToSpawner( object );
			HostCreateNetGameObject( object );
		}
	}

	return LOAD_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
void Game::SaveWorldToFile( )
{
	std::string saveFilePath = Stringf( "Data/Saves/World/ShipGame_v%d.World.xml", GAME_VERSION );
	
	//World crystal destroyed
	if( m_gameState.HostIsWorldDestroyed( ) )
	{
		//Delete world file
		std::remove( saveFilePath.c_str( ) );
	}

	//World crystal intact
	else
	{
		XMLNode saveWorld = XMLNode::createXMLTopNode( STRING_WORLD_DATA );

		//Write Game State
		m_gameState.WriteToXMLNode( &saveWorld );

		//Write Spawners
		XMLNode saveSpawners = saveWorld.addChild( STRING_GAME_OBJECT_SPAWNERS );
		Array<ShipSpawner*> const & objectSpawners = ShipSpawner::GetSpawnerList( );
		for( size_t spawnerIndex = 0; spawnerIndex < objectSpawners.Size( ); ++spawnerIndex )
		{
			ShipSpawner * spawner = objectSpawners[spawnerIndex];
			spawner->WriteToXMLNode( &saveSpawners );
		}

		//Write Game Objects
		XMLNode saveObjects = saveWorld.addChild( STRING_GAME_OBJECTS );
		for( NetGameObject * hostObject : m_hostObjects )
		{
			hostObject->GetLocalObject( )->WriteToXMLNode( &saveObjects );
		}

		saveWorld.writeToFile( saveFilePath.c_str( ) );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::SavePlayerToFile( byte_t netIndex )
{
	Player * currentPlayer = m_hostPlayers[netIndex];
	if( currentPlayer )
	{
		XMLNode savePlayer = XMLNode::createXMLTopNode( "PlayerData" );
		currentPlayer->WriteToXMLNode( &savePlayer );
		//#TODO: Limit usernames to only certain characters, else it'll fuck up saving
		std::string saveFilePath = Stringf( "Data/Saves/Players/%s.Player.xml", currentPlayer->GetUsername( ) );
		savePlayer.writeToFile( saveFilePath.c_str( ) );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::SaveAllPlayersToFile( )
{
	for( size_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
	{
		SavePlayerToFile( (byte_t) playerIndex );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::CreateNewWorld( )
{
	//Boss Spawners
	ShipSpawner::CreateEnemySpawner( eEnemyType_BOSS_RED, 10U, 1, Vector2f( 0.f, 60.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_BOSS_RED, 10U, 1, Vector2f( 0.f, -60.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_BOSS_RED, 10U, 1, Vector2f( 60.f, 0.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_BOSS_RED, 10U, 1, Vector2f( -60.f, 0.f ) );
	
	//Level 1 Ring
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 1U, 2, Vector2f( 0.f, 15.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 1U, 2, Vector2f( 0.f, -15.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 1U, 2, Vector2f( -15.f, 0.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 1U, 2, Vector2f( 15.f, 0.f ) );
	
	//Level 2 Ring
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 2U, 2, Vector2f( 15.f, 15.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 2U, 2, Vector2f( 15.f, -15.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 2U, 2, Vector2f( -15.f, 15.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 2U, 2, Vector2f( -15.f, -15.f ) );
	
	//Level 3 Ring
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 3U, 2, Vector2f( 0.f, 25.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 3U, 2, Vector2f( 0.f, -25.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 3U, 2, Vector2f( -25.f, 0.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 3U, 2, Vector2f( 25.f, 0.f ) );
	
	//Level 4 Ring
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 4U, 2, Vector2f( 25.f, 25.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 4U, 2, Vector2f( 25.f, -25.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 4U, 2, Vector2f( -25.f, 25.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 4U, 2, Vector2f( -25.f, -25.f ) );
	
	//Level 5 Ring
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 5U, 4, Vector2f( 0.f, 35.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 5U, 4, Vector2f( 0.f, -35.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 5U, 4, Vector2f( -35.f, 0.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 5U, 4, Vector2f( 35.f, 0.f ) );
	
	//Level 6 Ring
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 6U, 2, Vector2f( 35.f, 35.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 6U, 2, Vector2f( 35.f, -35.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 6U, 2, Vector2f( -35.f, 35.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 6U, 2, Vector2f( -35.f, -35.f ) );
	
	//Level 7 Ring
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 7U, 2, Vector2f( 0.f, 45.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 7U, 2, Vector2f( 0.f, -45.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 7U, 2, Vector2f( -45.f, 0.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 7U, 2, Vector2f( 45.f, 0.f ) );
	
	//Level 8 Ring
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 8U, 2, Vector2f( 45.f, 45.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 8U, 2, Vector2f( 45.f, -45.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 8U, 2, Vector2f( -45.f, 45.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 8U, 2, Vector2f( -45.f, -45.f ) );
	
	//Level 9 Ring
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 9U, 2, Vector2f( 0.f, 55.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 9U, 2, Vector2f( 0.f, -55.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 9U, 2, Vector2f( -55.f, 0.f ) );
	ShipSpawner::CreateEnemySpawner( eEnemyType_ARCHER, 9U, 2, Vector2f( 55.f, 0.f ) );
	
	ShipSpawner::CreateAllySpawner( eAllyType_GUARD, 10U, 3, Vector2f( 0.f, 0.f ) );

	//Crystal
	HostSpawnCrystal( );

	//Spawn Rocks
	for( int rockCount = 0; rockCount < 200; ++rockCount )
	{
		HostSpawnRock( );
	}
}


//-------------------------------------------------------------------------------------------------
//#TODO: Make these objects automatically register themselves in a gameobject Registry
GameObject * Game::CreateGameObjectFromXMLNode( XMLNode const & data )
{
	size_t GAME_OBJECT_NAME_MAX_LENGTH = 256;
	char const * objectName = data.getName( );
	if( strncmp( objectName, "Rock", GAME_OBJECT_NAME_MAX_LENGTH ) == 0 )
	{
		return new Rock( data );
	}
	else if( strncmp( objectName, "ExpOrb", GAME_OBJECT_NAME_MAX_LENGTH ) == 0 )
	{
		return new Emote( data );
	}
	else if( strncmp( objectName, "BasicEnemy", GAME_OBJECT_NAME_MAX_LENGTH ) == 0 )
	{
		return new EnemyShip( data );
	}
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
void Game::SetupUISystem( )
{
	m_titleScreen = g_UISystem->GetWidgetByName<UIWidget>( UI_TITLE_SCREEN );
	m_crystalDestroyedOverlay = g_UISystem->GetWidgetByName<UIBox>( UI_CRYSTAL_DESTROYED );
	m_playerHUD = g_UISystem->GetWidgetByName<UIWidget>( UI_PLAYER_HUD_SCREEN );
	m_targetHUD = g_UISystem->GetWidgetByName<UIWidget>( UI_TARGET_HUD_SCREEN );
	m_debugHUD = g_UISystem->GetWidgetByName<UIWidget>( UI_DEBUG_HUD_SCREEN );
	m_usernameField = g_UISystem->GetWidgetByName<UITextField>( UI_USERNAME_FIELD );
	m_passwordField = g_UISystem->GetWidgetByName<UITextField>( UI_PASSWORD_FIELD );
	m_addressField = g_UISystem->GetWidgetByName<UITextField>( UI_ADDRESS_FIELD );
	m_invalidPasswordLabel = g_UISystem->GetWidgetByName<UILabel>( UI_INVALID_PASSWORD );
	m_playerClassLabel = g_UISystem->GetWidgetByName<UILabel>( UI_PLAYER_CLASS );
	m_playerNameLabel = g_UISystem->GetWidgetByName<UILabel>( UI_PLAYER_NAME );
	m_playerLevelLabel = g_UISystem->GetWidgetByName<UILabel>( UI_PLAYER_LEVEL );
	m_playerMoneyLabel = g_UISystem->GetWidgetByName<UILabel>( UI_PLAYER_MONEY );
	m_upgradeCostLabel = g_UISystem->GetWidgetByName<UILabel>( UI_UPGRADE_COST );
	m_upgradeHullButton = g_UISystem->GetWidgetByName<UIButton>( UI_UPGRADE_HULL_BUTTON );
	m_upgradeHealthButton = g_UISystem->GetWidgetByName<UIButton>( UI_UPGRADE_HEALTH_BUTTON );
	m_upgradeShieldButton = g_UISystem->GetWidgetByName<UIButton>( UI_UPGRADE_SHIELD_BUTTON );
	m_upgradeEnergyButton = g_UISystem->GetWidgetByName<UIButton>( UI_UPGRADE_ENERGY_BUTTON );
	m_mapCoordinatesLabel = g_UISystem->GetWidgetByName<UILabel>( UI_MAP_COORDINATES );
	m_targetNameLabel = g_UISystem->GetWidgetByName<UILabel>( UI_TARGET_NAME );
	m_targetLevelLabel = g_UISystem->GetWidgetByName<UILabel>( UI_TARGET_LEVEL );
	m_targetHealthBar = g_UISystem->GetWidgetByName<UIProgressBar>( UI_TARGET_HEALTH );
	m_targetShieldBar = g_UISystem->GetWidgetByName<UIProgressBar>( UI_TARGET_SHIELD );
	m_targetEnergyBar = g_UISystem->GetWidgetByName<UIProgressBar>( UI_TARGET_ENERGY );
	m_targetInfo1Label = g_UISystem->GetWidgetByName<UILabel>( UI_TARGET_INFO1 );
	m_targetInfo2Label = g_UISystem->GetWidgetByName<UILabel>( UI_TARGET_INFO2 );
	m_targetInfo3Label = g_UISystem->GetWidgetByName<UILabel>( UI_TARGET_INFO3 );
	m_targetInfo4Label = g_UISystem->GetWidgetByName<UILabel>( UI_TARGET_INFO4 );
	for( size_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
	{
		std::string uiName = Stringf( "%s%u", UI_CONNECTION, playerIndex+1 );
		m_debugConnection[playerIndex] = g_UISystem->GetWidgetByName<UILabel>( uiName );
	}
	m_dedicatedHostLabel = g_UISystem->GetWidgetByName<UILabel>( UI_DEDICATED_SERVER );
	m_playerHealthBar = g_UISystem->GetWidgetByName<UIProgressBar>( UI_PLAYER_HEALTH );
	m_playerShieldBar = g_UISystem->GetWidgetByName<UIProgressBar>( UI_PLAYER_SHIELD );
	m_playerEnergyBar = g_UISystem->GetWidgetByName<UIProgressBar>( UI_PLAYER_ENERGY );
	m_playerHealthStat = g_UISystem->GetWidgetByName<UIProgressBar>( UI_STAT_HEALTH );
	m_playerShieldStat = g_UISystem->GetWidgetByName<UIProgressBar>( UI_STAT_SHIELD );
	m_playerEnergyStat = g_UISystem->GetWidgetByName<UIProgressBar>( UI_STAT_ENERGY );
	m_uiGodMode = g_UISystem->GetWidgetByName<UILabel>( UI_GOD_MODE );
	m_warpInstruction = g_UISystem->GetWidgetByName<UILabel>( UI_WARP_INSTRUCTIONS );
	m_warpDriveBar = g_UISystem->GetWidgetByName<UIProgressBar>( UI_WARP_DRIVE );
	m_summoningBar = g_UISystem->GetWidgetByName<UIProgressBar>( UI_SUMMONING_BAR );
	m_classSelectScreen = g_UISystem->GetWidgetByName<UIWidget>( UI_CLASS_SELECT_SCREEN );
	m_classSelectBackground = g_UISystem->GetWidgetByName<UIBox>( UI_CLASS_SELECT_BACKGROUND );
	m_worldContainer = g_UISystem->GetWidgetByName<UIContainer>( UI_WORLD_CONTAINER );
	m_equipmentPrimary = g_UISystem->GetWidgetByName<UIContainer>( UI_EQUIPMENT_PRIMARY );
	m_feedCrystalButton = g_UISystem->GetWidgetByName<UIButton>( UI_FEED_CRYSTAL_BUTTON );
	m_feedCrystalLabel = g_UISystem->GetWidgetByName<UILabel>( UI_FEED_CRYSTAL_LABEL );

	//#TODO: Make this select password if username data already exists
	m_usernameField->Select( );
	std::vector<std::string> configFiles = EnumerateFilesInFolder( "Data/", "config.xml" );
	for( std::string & configPath : configFiles )
	{
		XMLNode configData = XMLNode::openFileHelper( configPath.c_str( ) ).getChildNode( "Config" );
		std::string ipAddress = ReadXMLAttribute( configData, "IPAddress", "" );
		m_addressField->SetText( ipAddress );
	}

	EventSystem::RegisterEvent( BUTTON_DEDICATED_HOST_EVENT, this, &Game::OnDedicatedHostButton );
	EventSystem::RegisterEvent( BUTTON_HOST_GAME_EVENT, this, &Game::OnGameHostButton );
	EventSystem::RegisterEvent( BUTTON_JOIN_GAME_EVENT, this, &Game::OnGameJoinButton );
	EventSystem::RegisterEvent( BUTTON_FIGHTER_CLASS_EVENT, this, &Game::OnFighterClassButton );
	EventSystem::RegisterEvent( BUTTON_WIZARD_CLASS_EVENT, this, &Game::OnWizardClassButton );
	EventSystem::RegisterEvent( BUTTON_ROGUE_CLASS_EVENT, this, &Game::OnRogueClassButton );
	EventSystem::RegisterEvent( BUTTON_CULTIST_CLASS_EVENT, this, &Game::OnCultistClassButton );
	EventSystem::RegisterEvent( BUTTON_UPGRADE_HULL_EVENT, this, &Game::OnUpgradeHullButton );
	EventSystem::RegisterEvent( BUTTON_UPGRADE_HEALTH_EVENT, this, &Game::OnUpgradeHealthButton );
	EventSystem::RegisterEvent( BUTTON_UPGRADE_SHIELD_EVENT, this, &Game::OnUpgradeShieldButton );
	EventSystem::RegisterEvent( BUTTON_UPGRADE_ENERGY_EVENT, this, &Game::OnUpgradeEnergyButton );
	EventSystem::RegisterEvent( BUTTON_FEED_CRYSTAL_EVENT, this, &Game::OnFeedCrystalButton );
	EventSystem::RegisterEvent( BUTTON_EMOTE_HAPPY_EVENT, this, &Game::OnEmoteHappyButton );
	EventSystem::RegisterEvent( BUTTON_EMOTE_SAD_EVENT, this, &Game::OnEmoteSadButton );
	EventSystem::RegisterEvent( BUTTON_EMOTE_ANGRY_EVENT, this, &Game::OnEmoteAngryButton );
	EventSystem::RegisterEvent( BUTTON_EMOTE_WHOOPS_EVENT, this, &Game::OnEmoteWhoopsButton );
	EventSystem::RegisterEvent( BUTTON_EMOTE_HELP_EVENT, this, &Game::OnEmoteHelpButton );
	EventSystem::RegisterEvent( EQUIP_PRIMARY_EVENT, this, &Game::OnEquipPrimary );
	EventSystem::RegisterEvent( EJECT_ITEM_EVENT, this, &Game::OnEjectItem );
	EventSystem::RegisterEvent( MOVE_ITEM_TO_INVENTORY_EVENT, this, &Game::OnMoveEquipmentToInventory );
}


//-------------------------------------------------------------------------------------------------
void Game::SetupSpriteRenderer( )
{
	g_ParticleEngine = new ParticleEngine( );
	Vector2i windowDimensions = GetWindowDimensions( );
	g_SpriteRenderSystem->SetAspectRatio( (float) windowDimensions.x, (float) windowDimensions.y );
	g_SpriteRenderSystem->SetImportSize( 256.f );
	g_SpriteRenderSystem->SetVirtualSize( 10.f );
	g_SpriteRenderSystem->SetClearColor( Color::WHITE );
}


//-------------------------------------------------------------------------------------------------
void Game::SetupNetSession( )
{
	s_netSession = new NetSession( GAME_VERSION );
	byte_t optionFlags, channel;

	// Create Player Message
	// Connection Required, Reliable, Sequence
	optionFlags = NetMessageDefinition::RELIABLE_OPTION_FLAG | NetMessageDefinition::SEQUENCE_OPTION_FLAG;
	channel = CREATE_DESTROY_CHANNEL;
	s_netSession->RegisterMessage( (eNetMessageType) eNetGameMessageType_NET_PLAYER_CREATE, OnCreatePlayerReceived, 0, optionFlags, channel );

	// Destroy Player Message
	// Connection Required, Reliable, Sequence
	optionFlags = NetMessageDefinition::RELIABLE_OPTION_FLAG | NetMessageDefinition::SEQUENCE_OPTION_FLAG;
	channel = CREATE_DESTROY_CHANNEL;
	s_netSession->RegisterMessage( (eNetMessageType) eNetGameMessageType_NET_PLAYER_DESTROY, OnDestroyPlayerReceived, 0, optionFlags, channel );

	// Update Player Message
	// Connection Required, Reliable, Sequence
	optionFlags = NetMessageDefinition::RELIABLE_OPTION_FLAG | NetMessageDefinition::SEQUENCE_OPTION_FLAG;
	channel = UPDATE_CHANNEL;
	s_netSession->RegisterMessage( (eNetMessageType) eNetGameMessageType_NET_PLAYER_UPDATE, OnUpdatePlayerReceived, 0, optionFlags, channel );

	// Create Objects Message
	// Connection Required, Reliable, Sequence
	optionFlags = NetMessageDefinition::RELIABLE_OPTION_FLAG | NetMessageDefinition::SEQUENCE_OPTION_FLAG;
	channel = CREATE_DESTROY_CHANNEL;
	s_netSession->RegisterMessage( (eNetMessageType) eNetGameMessageType_NET_OBJECT_CREATE, OnCreateMessageReceived, 0, optionFlags, channel );

	// Destroy Objects Message
	// Connection Required, Reliable, Sequence
	optionFlags = NetMessageDefinition::RELIABLE_OPTION_FLAG | NetMessageDefinition::SEQUENCE_OPTION_FLAG;
	channel = CREATE_DESTROY_CHANNEL;
	s_netSession->RegisterMessage( (eNetMessageType) eNetGameMessageType_NET_OBJECT_DESTROY, OnDestroyMessageReceived, 0, optionFlags, channel );

	// Update Objects Message
	// Connection Required, Unreliable, Sequence
	//CURRENT NOT SEQUENCED
	s_netSession->RegisterMessage( (eNetMessageType) eNetGameMessageType_NET_OBJECT_UPDATE, OnUpdateMessageReceived, 0, 0 );

	// Update Game State Message
	// Connection Required, Unreliable, Sequence
	//CURRENT NOT SEQUENCED
	s_netSession->RegisterMessage( (eNetMessageType) eNetGameMessageType_GAME_STATE_UPDATE, OnGameStateMessageReceived, 0, 0 );

	// Update Reliable Input
	// Connection Required, Reliable
	optionFlags = NetMessageDefinition::RELIABLE_OPTION_FLAG;
	s_netSession->RegisterMessage( (eNetMessageType) eNetGameMessageType_RELIABLE_INPUT, OnInputMessageReceived, 0, optionFlags );

	// Update Unreliable Input
	// Connection Required
	s_netSession->RegisterMessage( (eNetMessageType) eNetGameMessageType_UNRELIABLE_INPUT, OnInputMessageReceived, 0, 0 );

	// NetSession Events
	EventSystem::RegisterEvent( NetSession::ON_CONNECTION_JOIN_EVENT, this, &Game::OnConnectionJoin );
	EventSystem::RegisterEvent( NetSession::ON_CONNECTION_LEAVE_EVENT, this, &Game::OnConnectionLeave );
	EventSystem::RegisterEvent( NetSession::PREPARE_PACKET_EVENT, this, &Game::OnPreparePacket );
	EventSystem::RegisterEvent( NetSession::ON_GAME_JOIN_VALIDATION_EVENT, this, &Game::OnJoinRequestValidation );
	EventSystem::RegisterEvent( NetSession::ON_JOIN_DENY_EVENT, this, &Game::OnJoinDeny );

	// Start session, check port range (default = 8)
	s_netSession->Start( NetworkSystem::GAME_PORT );
}


//-------------------------------------------------------------------------------------------------
void Game::SetupSprites( )
{
	//Setup game background
	m_targetingBracket = SpriteGameRenderer::Create( "targeting", LAYER_UI_FOREGROUND );
	m_backgroundGrid = SpriteGameRenderer::Create( "grid", LAYER_BACKGROUND );
	m_background = SpriteGameRenderer::Create( "square", LAYER_BACKGROUND );
	m_background->SetColor( Color::DARK_GREY );
	m_background->SetScale( 16.f );

	//minimap
	for( size_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
	{
		m_minimapShip[playerIndex] = SpriteGameRenderer::Create( "mapIcon", LAYER_UI_FOREGROUND, true );
		m_minimapShip[playerIndex]->SetScale( 0.025f );
		m_minimapShip[playerIndex]->SetColor( GetColorFromIndex( (byte_t) playerIndex ) );
		m_minimapShip[playerIndex]->SetEnabled( false );
	}
	m_minimapCrystal = SpriteGameRenderer::Create( "circle", LAYER_UI_FOREGROUND, true );
	m_minimapCrystal->SetScale( 1 / MAX_MAP_RADIUS );
	m_minimapCrystal->SetPosition( MINIMAP_CENTER );
	m_minimapCrystal->SetColor( Color::LIGHT_BLUE );
	m_minimap = SpriteGameRenderer::Create( "grid", LAYER_UI_FOREGROUND, true );
	m_minimap->SetScale( 1 / MAX_MAP_RADIUS );
	m_minimap->SetPosition( MINIMAP_CENTER );
	m_minimap->SetColor( Color::LIGHT_GREY );
	m_minimapBorder = SpriteGameRenderer::Create( "square", LAYER_UI_FOREGROUND, true );
	m_minimapBorder->SetScale( 1 / MAX_MAP_RADIUS * 16.f );
	m_minimapBorder->SetColor( Color::DARK_GREY );
	m_minimapBorder->SetPosition( MINIMAP_CENTER );
	m_minimapBackground = SpriteGameRenderer::Create( "square", LAYER_UI_FOREGROUND, true );
	m_minimapBackground->SetScale( 1 / MAX_MAP_RADIUS * 28.f );
	m_minimapBackground->SetColor( Color::LIGHT_GREY );
	m_minimapBackground->SetPosition( MINIMAP_CENTER );
}


//-------------------------------------------------------------------------------------------------
void Game::SetupTextRenderers( )
{
	//Clear players
	for( size_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
	{
		m_numHostPlayers = 0;
		m_numClientPlayers = 0;
		m_hostPlayers[playerIndex] = nullptr;
		m_clientPlayers[playerIndex] = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
void Game::CleanupPlayersAndGameObjects( )
{
	//Clean up players
	for( size_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
	{
		if( m_hostPlayers[playerIndex] )
		{
			delete m_hostPlayers[playerIndex];
			m_hostPlayers[playerIndex] = nullptr;
		}

		if( m_clientPlayers[playerIndex] )
		{
			delete m_clientPlayers[playerIndex];
			m_clientPlayers[playerIndex] = nullptr;
		}
	}
	m_numHostPlayers = 0;
	m_numClientPlayers = 0;

	for( size_t objectIndex = 0; objectIndex < m_hostObjects.size( ); ++objectIndex )
	{
		delete m_hostObjects[objectIndex];
		m_hostObjects[objectIndex] = nullptr;
	}
	m_hostObjects.clear( );

	for( size_t objectIndex = 0; objectIndex < m_clientObjects.size( ); ++objectIndex )
	{
		delete m_clientObjects[objectIndex];
		m_clientObjects[objectIndex] = nullptr;
	}
	m_clientObjects.clear( );

	//Clean up Spawners
	ShipSpawner::ClearAll( );
}


//-------------------------------------------------------------------------------------------------
void Game::CleanupSprites( )
{
	delete m_targetingBracket;
	m_targetingBracket = nullptr;

	delete m_backgroundGrid;
	m_backgroundGrid = nullptr;

	delete m_background;
	m_background = nullptr;

	delete m_minimapCrystal;
	m_minimapCrystal = nullptr;

	delete m_minimap;
	m_minimap = nullptr;

	delete m_minimapBorder;
	m_minimapBorder = nullptr;

	delete m_minimapBackground;
	m_minimapBackground = nullptr;

	for( size_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
	{
		delete m_minimapShip[playerIndex];
		m_minimapShip[playerIndex] = nullptr;
	}
}