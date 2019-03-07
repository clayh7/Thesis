#pragma once

#include "Game/General/GameCommon.hpp"


//-------------------------------------------------------------------------------------------------
class GameObject;
class NetMessage;
class NetSender;
class Sprite;
struct XMLNode;
typedef size_t hash_t; //literally here to remind user that the size_t is a hash
typedef unsigned int SoundID;
typedef GameObject * (GameObjectXMLCreationFunc) ( XMLNode const & node );
typedef GameObject * (GameObjectMessageCreationFunc) ( NetSender const & sender, NetMessage const & message );


//-------------------------------------------------------------------------------------------------
class GameObjectRegistration
{
//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	GameObjectRegistration( std::string const & gameObjectName, GameObjectXMLCreationFunc * creationFunc );
	GameObjectRegistration( eNetGameObjectType const & gameObjectType, GameObjectMessageCreationFunc * creationFunc );
};


//-------------------------------------------------------------------------------------------------
class GameObject
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static byte_t const INVALID_INDEX = 255;
	static byte_t const ENEMY_PLAYER_INDEX = 254;
	static byte_t const ALLY_PLAYER_INDEX = 253;
	static float const MAX_TIMEOUT_SECONDS;
	static SoundID * SOUND_HIT;
	static std::map<hash_t, GameObjectXMLCreationFunc*> * s_registeredGameObjectsXML;
	static std::map<eNetGameObjectType, GameObjectMessageCreationFunc*> * s_registeredGameObjectsMessage;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static void Initialize( );
	static void RegisterGameObject( std::string const & gameObjectName, GameObjectXMLCreationFunc * creationFunc );
	static void RegisterGameObject( eNetGameObjectType const & gameObjectType, GameObjectMessageCreationFunc * creationFunc );
	static GameObject * CreateFromXMLNode( XMLNode const & node );
	static GameObject * CreateFromNetMessage( eNetGameObjectType const & type, NetSender const & sender, NetMessage const & message );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
protected:
	bool m_isHostObject;
	Sprite * m_sprite;
	//#TODO: Setup for debugging purposes
	Sprite * m_netDebugSprite;
	float m_timeOutTimer;

//Synced Members
public:
	byte_t m_playerIndex;
	size_t m_spawnerID;
	eNetGameObjectType m_type;

	Vector2f m_position;
	Vector2f m_netPosition;
	Vector2f m_velocity;
	Vector2f m_netVelocity;
	float m_rotationDegrees;
	float m_netRotationDegrees;
	//Orientation seperate from velocity

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	GameObject( byte_t index, eNetGameObjectType const & type, bool hostObject );
	GameObject( eNetGameObjectType const & type, bool hostObject );
	virtual ~GameObject( );

	virtual void WriteToMessage( NetMessage * out_message );
	virtual void UpdateFromMessage( NetSender const & sender, NetMessage const & message );
	virtual void WriteToXMLNode( XMLNode * out_xmlNode );
	virtual void UpdateFromXMLNode( XMLNode const & node );
	
	virtual void Update( );
	virtual void UpdateSprite( );
	virtual void UpdateNetDebugSprite( );
	virtual bool HostCheckCollision( GameObject * gameObject );
	virtual void HostHandleCollision( GameObject * gameObject ) = 0;
	virtual bool HostIsAlive( ) const = 0;
	virtual char const * GetEntityName( ) const = 0;
	virtual std::string GetSpriteID( ) const;
	virtual float GetNetSyncPercent( ) const = 0;
	virtual float GetPhysicsRadius( ) const = 0;
	virtual float GetSpriteScale( ) const = 0;
	virtual void HostOnDestroy( );
	virtual void ClientOnDestroy( );

	float GetElapsedGameTime( NetSender const & sender ) const;
	void UpdateLocalData( float elapsedLocalGameTime );
	void UpdateNetData( float elapsedNetGameTime );
	bool IsClientOwner( ) const;
	void SetVisibility( bool isVisible );
	float ConvertToSpriteScale( float radius, float desiredSize ) const;
	SoundID GetRandomHitSound( ) const;
	void MessageRecieved( );
	bool IsNotRecievingUpdates( ) const;
};