#pragma once

#include <vector>
#include "Engine/DebugSystem/DebugLog.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Engine/Net/Session/NetSession.hpp"
#include "Game/GameObjects/Player/Player.hpp"
#include "Game/General/GameCommon.hpp"
#include "Game/General/GameState.hpp"


//-------------------------------------------------------------------------------------------------
class Command;
class EnemyShip;
class Game;
class GameObject;
class Item;
class NetGameObject;
class NetConnection;
class NetMessage;
class NetSender;
class NetSession;
class Player;
class Ship;
class ShipSpawner;
class Sprite;
class TextRenderer;
class UIBox;
class UIButton;
class UILabel;
class UIProgressBar;
class UITextField;
class UIWidget;
enum eEmoteType : uint8_t;
struct XMLNode;


//-------------------------------------------------------------------------------------------------
extern Game * g_GameSystem;


//-------------------------------------------------------------------------------------------------
class Game
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static size_t const MAX_PLAYERS = 252;
	static size_t const MAX_LEADERBOARD_SPOTS = 5;
	static int const NETWORKING_MAX_LINES = 20;
	static bool s_showHost;
	static bool const SHOW_COLLISION;
	static Color const HOST_COLOR;
	static float const MAX_MAP_RADIUS;
	static Vector2f const MINIMAP_CENTER;
	static NetSession * s_netSession;
	static byte_t const CREATE_DESTROY_CHANNEL = 1;
	static byte_t const UPDATE_CHANNEL = 2;
	static byte_t const INPUT_CHANNEL = 3;
	static float const NORMAL_VIEW_SCALE;
	static float const CTHULHU_VIEW_SCALE;
	static float const MAX_AUDIO_DISTANCE;
	static float const MAX_SPEED;
	static int const CULTIST_SPAWN_COUNT = 4;

	static char const * STRING_WORLD_DATA;
	static char const * STRING_GAME_OBJECTS;
	static char const * STRING_GAME_OBJECT_SPAWNERS;
	static char const * UI_ADDRESS_FIELD;
	static char const * UI_USERNAME_FIELD;
	static char const * UI_PASSWORD_FIELD;
	static char const * UI_INVALID_PASSWORD;
	static char const * UI_TITLE_SCREEN;
	static char const * UI_CRYSTAL_DESTROYED;
	static char const * UI_CLASS_SELECT_SCREEN;
	static char const * UI_CLASS_SELECT_BACKGROUND;
	static char const * UI_PLAYER_HUD_SCREEN;
	static char const * UI_TARGET_HUD_SCREEN;
	static char const * UI_DEBUG_HUD_SCREEN;
	static char const * UI_PLAYER_CLASS;
	static char const * UI_PLAYER_NAME;
	static char const * UI_PLAYER_LEVEL;
	static char const * UI_PLAYER_MONEY;
	static char const * UI_UPGRADE_COST;
	static char const * UI_PLAYER_HEALTH;
	static char const * UI_PLAYER_SHIELD;
	static char const * UI_PLAYER_ENERGY;
	static char const * UI_WORLD_CONTAINER;
	static char const * UI_EQUIPMENT_PRIMARY;
	static char const * UI_UPGRADE_HULL_BUTTON;
	static char const * UI_UPGRADE_HEALTH_BUTTON;
	static char const * UI_UPGRADE_SHIELD_BUTTON;
	static char const * UI_UPGRADE_ENERGY_BUTTON;
	static char const * UI_TARGET_NAME;
	static char const * UI_TARGET_LEVEL;
	static char const * UI_TARGET_HEALTH;
	static char const * UI_TARGET_SHIELD;
	static char const * UI_TARGET_ENERGY;
	static char const * UI_STAT_HEALTH;
	static char const * UI_STAT_SHIELD;
	static char const * UI_STAT_ENERGY;
	static char const * UI_TARGET_INFO1;
	static char const * UI_TARGET_INFO2;
	static char const * UI_TARGET_INFO3;
	static char const * UI_TARGET_INFO4;
	static char const * UI_GOD_MODE;
	static char const * UI_WARP_INSTRUCTIONS;
	static char const * UI_WARP_DRIVE;
	static char const * UI_SUMMONING_BAR;
	static char const * UI_MAP_COORDINATES;
	static char const * UI_CONNECTION;
	static char const * UI_DEDICATED_SERVER;
	static char const * UI_FEED_CRYSTAL_BUTTON;
	static char const * UI_FEED_CRYSTAL_LABEL;
	static char const * EQUIP_PRIMARY_EVENT;
	static char const * EJECT_ITEM_EVENT;
	static char const * MOVE_ITEM_TO_INVENTORY_EVENT;
	static char const * BUTTON_DEDICATED_HOST_EVENT;
	static char const * BUTTON_HOST_GAME_EVENT;
	static char const * BUTTON_JOIN_GAME_EVENT;
	static char const * BUTTON_FIGHTER_CLASS_EVENT;
	static char const * BUTTON_WIZARD_CLASS_EVENT;
	static char const * BUTTON_ROGUE_CLASS_EVENT;
	static char const * BUTTON_CULTIST_CLASS_EVENT;
	static char const * BUTTON_UPGRADE_HULL_EVENT;
	static char const * BUTTON_UPGRADE_HEALTH_EVENT;
	static char const * BUTTON_UPGRADE_SHIELD_EVENT;
	static char const * BUTTON_UPGRADE_ENERGY_EVENT;
	static char const * BUTTON_FEED_CRYSTAL_EVENT;
	static char const * BUTTON_EMOTE_HAPPY_EVENT;
	static char const * BUTTON_EMOTE_SAD_EVENT;
	static char const * BUTTON_EMOTE_ANGRY_EVENT;
	static char const * BUTTON_EMOTE_WHOOPS_EVENT;
	static char const * BUTTON_EMOTE_HELP_EVENT;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static uint8_t GetSessionIndex( );
	static void WriteCompressedPosition( NetMessage * out_message, Vector2f const & position );
	static void ReadCompressedPosition( NetMessage const & message, Vector2f * out_position );
	static void WriteCompressedVelocity( NetMessage * out_message, Vector2f const & velocity );
	static void ReadCompressedVelocity( NetMessage const & message, Vector2f * out_velocity );
	static void WriteCompressedSpeed( NetMessage * out_message, float speed );
	static void ReadCompressedSpeed( NetMessage const & message, float * out_speed );
	static void WriteCompressedRotation( NetMessage * out_message, float rotation );
	static void ReadCompressedRotation( NetMessage const & message, float * out_rotation );
	static void WriteCompressedBoolean( NetMessage * out_message, bool actual );
	static void ReadCompressedBoolean( NetMessage const & message, bool * actual );
	static void WriteUncompressedUint8( NetMessage * out_message, uint8_t data );
	static void ReadUncompressedUint8( NetMessage const & message, uint8_t * out_data );
	static void WriteUncompressedUint16( NetMessage * out_message, uint16_t data );
	static void ReadUncompressedUint16( NetMessage const & message, uint16_t * out_data );
	static void WriteUncompressedUint32( NetMessage * out_message, uint32_t data );
	static void ReadUncompressedUint32( NetMessage const & message, uint32_t * out_data );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	//#TODO: Maybe move this all into game state
	//#TODO: Make an array class that bundles a normal array with it's size
	//Host/Client Variables
	std::vector<NetGameObject*> m_hostObjects;
	Player * m_hostPlayers[MAX_PLAYERS];
	size_t m_numHostPlayers;

	std::vector<NetGameObject*> m_clientObjects;
	Player * m_clientPlayers[MAX_PLAYERS];
	size_t m_numClientPlayers;

public:
	DebugLog m_ClientGameActivityLog;
	DebugLog m_HostGameActivityLog;

private:
	//#TODO: May want to move this to the bottom of the member list
	Vector2f m_playerSpawnPoint;
	Vector2f m_cultistSpawnPoints[CULTIST_SPAWN_COUNT];

	//All UI stuff
	UIWidget * m_titleScreen;
	UIBox * m_crystalDestroyedOverlay;
	UIWidget * m_playerHUD;
	UIWidget * m_targetHUD;
	UIWidget * m_debugHUD;
	UITextField * m_usernameField;
	UITextField * m_passwordField;
	UITextField * m_addressField;
	UILabel * m_invalidPasswordLabel;
	UILabel * m_playerClassLabel;
	UILabel * m_playerNameLabel;
	UILabel * m_playerLevelLabel;
	UILabel * m_playerMoneyLabel;
	UILabel * m_upgradeCostLabel;
	UIButton * m_upgradeHullButton;
	UIButton * m_upgradeHealthButton;
	UIButton * m_upgradeShieldButton;
	UIButton * m_upgradeEnergyButton;
	UILabel * m_targetNameLabel;
	UILabel * m_targetLevelLabel;
	UIProgressBar * m_targetHealthBar;
	UIProgressBar * m_targetShieldBar;
	UIProgressBar * m_targetEnergyBar;
	UILabel * m_targetInfo1Label;
	UILabel * m_targetInfo2Label;
	UILabel * m_targetInfo3Label;
	UILabel * m_targetInfo4Label;
	UILabel * m_mapCoordinatesLabel;
	UILabel ** m_debugConnection;
	UILabel * m_dedicatedHostLabel;
	UIProgressBar * m_playerHealthBar;
	UIProgressBar * m_playerShieldBar;
	UIProgressBar * m_playerEnergyBar;
	UIProgressBar * m_playerHealthStat;
	UIProgressBar * m_playerShieldStat;
	UIProgressBar * m_playerEnergyStat;
	UILabel * m_uiGodMode;
	UILabel * m_warpInstruction;
	UIProgressBar * m_warpDriveBar;
	UIProgressBar * m_summoningBar;
	UIWidget * m_classSelectScreen;
	UIBox * m_classSelectBackground;
	UIContainer * m_worldContainer;
	UIContainer * m_equipmentPrimary;
	UIButton * m_feedCrystalButton;
	UILabel * m_feedCrystalLabel;
	Sprite * m_targetingBracket;
	Sprite * m_backgroundGrid;
	Sprite * m_background;
	Sprite * m_minimapCrystal;
	Sprite * m_minimapShip[MAX_PLAYERS];
	Sprite * m_minimap;
	Sprite * m_minimapBorder;
	Sprite * m_minimapBackground;

//Actual Game variables
public:
	GameState m_gameState;
	bool m_showDebug;
	float m_lastInvalidPasswordTime;
	bool m_isDedicatedServer;
	bool m_hostGodMode;
	bool m_stopRendering;
	bool m_crystalDestroyed;
	bool m_quitNextFrame;
	float m_cameraRotation;

private:
	//Used to calculate audio attenuation
	float m_viewSize;
	Vector2f m_cameraPosition;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Game( );
	~Game( );
	void Update( );
	void UpdateInput( );
	void UpdateGameObjects( );
	void UpdateCamera( );
	void UpdatePlayers( );
	void UpdateUI( );
	void UpdateTitleScreenUI( );
	void UpdateDebugUI( );
	void UpdatePlayerUI( );
	void UpdateClassSelectUI( );
	void UpdateTargetingUI( );
	void UpdateTargetingUIForItemCode( uint16_t itemCode );
	void UpdateMinimap( );
	void Render( ) const;
	float GetAudioAttenuationFromCamera( Vector2f const & audioPosition );

private:
	Vector2f FindBestRespawnLocation( bool isCultist );
	Vector2f FindEmptySpawnLocation( );
	bool LoadWorldFromFile( );
	void SaveWorldToFile( );
	void SavePlayerToFile( byte_t netIndex );
	void SaveAllPlayersToFile( );
	void CreateNewWorld( );
	GameObject * CreateGameObjectFromXMLNode( XMLNode const & data );
	void SetupUISystem( );
	void SetupSpriteRenderer( );
	void SetupNetSession( );
	void SetupSprites( );
	void SetupTextRenderers( );
	void CleanupPlayersAndGameObjects( );
	void CleanupSprites( );

//-------------------------------------------------------------------------------------------------
// Events
//-------------------------------------------------------------------------------------------------
public:
	void OnConnectionJoin( NamedProperties & );
	void OnConnectionLeave( NamedProperties & );
	void OnPreparePacket( NamedProperties & );
	void OnJoinRequestValidation( NamedProperties & );
	void OnJoinDeny( NamedProperties & );
	void OnDedicatedHostButton( NamedProperties & );
	void OnGameHostButton( NamedProperties & );
	void OnGameJoinButton( NamedProperties & );
	void OnFighterClassButton( NamedProperties & );
	void OnWizardClassButton( NamedProperties & );
	void OnRogueClassButton( NamedProperties & );
	void OnCultistClassButton( NamedProperties & );
	void OnUpgradeHullButton( NamedProperties & );
	void OnUpgradeHealthButton( NamedProperties & );
	void OnUpgradeShieldButton( NamedProperties & );
	void OnUpgradeEnergyButton( NamedProperties & );
	void OnFeedCrystalButton( NamedProperties & );
	void OnEmoteHappyButton( NamedProperties & );
	void OnEmoteSadButton( NamedProperties & );
	void OnEmoteAngryButton( NamedProperties & );
	void OnEmoteWhoopsButton( NamedProperties & );
	void OnEmoteHelpButton( NamedProperties & );
	void OnEquipPrimary( NamedProperties & );
	void OnEjectItem( NamedProperties & );
	void OnMoveEquipmentToInventory( NamedProperties & );

//-------------------------------------------------------------------------------------------------
// Host / Client Functions
//-------------------------------------------------------------------------------------------------
public:
	void HostInitializeGame( );
	bool IsHost( ) const;
	void AttemptCreateHost( std::string const & username, std::string const & password );
	void AttemptCreateClient( std::string const & addressString, std::string const & username, std::string const & password );

	//Host Player Functions
	void HostCreatePlayer( byte_t netIndex, char const * username, size_t password );
	void HostDestroyPlayer( byte_t netIndex );
	Player * HostGetPlayer( byte_t playerIndex ) const;
	std::vector<Player*> HostGetActivePlayers( ) const;
	Player * HostGetPlayerForPlayerShip( PlayerShip const * playerShip ) const;
	void HostUpdatePlayer( byte_t playerIndex );
	void HostAddLevelToPlayer( byte_t playerIndex );
	bool HostAddItemToPlayer( Player * player, uint16_t itemCode );
	void HostApplyInputEventToPlayer( eGameEvent const & inputEvent, byte_t playerIndex );
	void HostApplyInputToPlayer( uint16_t inputBitfield, byte_t playerIndex );
	ePlayerClass HostGetClassFromPlayerIndex( byte_t playerIndex ) const;

	//Client Player Functions
	void ClientCreatePlayer( byte_t netIndex, char const * username );
	void ClientDestroyPlayer( byte_t netIndex );
	Player * ClientGetPlayerSelf( ) const;
	Player * ClientGetPlayer( byte_t playerIndex ) const;

	//Host Create / Destroy Net Game Objects
	NetGameObject * HostCreateNetGameObject( GameObject * object );
	void HostDestroyNetGameObject( NetGameObject * netObject );
	void HostDestroyNetGameObject( size_t netID );
	NetGameObject * HostGetNetGameObjectFromNetID( size_t netID ) const;
	
	//Client Create / Destroy Net Game Objects
	NetGameObject * ClientCreateNetGameObject( size_t netID, GameObject * object );
	void ClientDestroyNetGameObject( NetGameObject * netGameObject );
	void ClientDestroyNetGameObject( size_t netID );
	NetGameObject * ClientGetNetGameObjectFromNetID( size_t netID ) const;

	//Host Spawn
	void HostSpawnPlayerShip( byte_t netIndex );
	void HostSpawnRock( );
	void HostSpawnRock( Vector2f const & position );
	void HostSpawnCubes( Vector2f const & position, uint8_t cubeAmount );
	void HostSpawnItem( Vector2f const & position, eItemType const & itemType );
	void HostSpawnItem( Vector2f const & position, uint16_t itemCode );
	void HostSpawnCrystal( );
	void HostShowEmote( Vector2f const & position, float const & rotation, eEmoteType const & type );
	void HostFireWeapon( byte_t playerIndex, uint16_t weaponCode, Vector2f const & bulletPosition, float bulletRotationDegrees, Vector2f const & shipVelocity );

	//Host Generic
	void HostCatchupConnectionOnCurrentGameState( NetConnection * conn );
	void HostCheckCollisions( NetGameObject * object );
	float HostGetClosestGameObjectWithinDegrees( GameObject const * fromObject, float withinDegrees, GameObject ** out_foundGameObject );
	float HostGetClosestGoodShip( Vector2f const & position, Ship ** out_foundShip );
	float HostGetClosestEvilShip( Vector2f const & position, Ship ** out_foundShip );
	void HostIncreaseSummoningBar( uint8_t amount );
	void HostSummonCthulhu( );
	void HostResetCthulhuSummoning( );
	void HostPlayerFeedCrystal( uint8_t playerIndex );
	void HostHealCrystal( uint16_t healAmount );
	void HostWipeGame( );
	void HostDealAreaDamage( Vector2f const & position, float radius, uint16_t damage, byte_t playerIndexSource );

	//Client Generic
	float ClientGetClosestGameObjectWithinDegrees( GameObject const * fromObject, float withinDegrees, GameObject ** out_foundGameObject );
	void ClientRequestEvent( eGameEvent const & upgradeEvent );
	void ClientRequestRemoveItem( byte_t itemIndex );
	void ClientRequestRemoveEquipment( eEquipmentSlot const & slot );
	void ClientRequestEquipPrimary( byte_t itemIndex );
	void ClientLevelUpEffect( Player * player );
};