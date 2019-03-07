#pragma once

#include <vector>
#include "Engine/Core/Array.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Game/General/GameCommon.hpp"


//-------------------------------------------------------------------------------------------------
class GameObject;
struct XMLNode;


//-------------------------------------------------------------------------------------------------
enum eAllyType : byte_t;
enum eEnemyType : byte_t;


//-------------------------------------------------------------------------------------------------
class ShipSpawner
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static size_t const INVALID_SPAWNER_ID = 0;
	static size_t s_nextSpawnerID;
	static float const RESPAWN_TIME_SEC;
	static size_t constexpr MAX_SPAWNERS = 1024;
	static uint8_t const GUARD_COST;
	static Array<ShipSpawner*> s_spawners;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static void AddExistingObjectToSpawner( GameObject * const existingObject );
	static void RemoveFromSpawner( GameObject * const deadObject );
	static void CreateFromXMLNode( XMLNode const & data );
	static void CreateEnemySpawner( eEnemyType const & type, uint8_t level, int maxObjects = 100, Vector2f const & position = Vector2f::ZERO, size_t spawnerID = INVALID_SPAWNER_ID );
	static void CreateAllySpawner( eAllyType const & type, uint8_t level, int maxObjects = 100, Vector2f const & position = Vector2f::ZERO, size_t spawnerID = INVALID_SPAWNER_ID );
	static void ClearAll( );
	static void UpdateAll( );
	static Vector2f GetLocation( size_t spawnerID );

	static Array<ShipSpawner*> const & GetSpawnerList( );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	size_t m_spawnerID;
	bool m_isEnemy;
	union uSpawnType
	{
		uSpawnType( eAllyType type ) : ally( type ) { }
		uSpawnType( eEnemyType type ) : enemy( type ) { }
		eAllyType ally;
		eEnemyType enemy;
	} m_type;
	uint8_t m_level;
	int m_maxObjects;
	Vector2f m_position;
	std::vector<GameObject*> m_spawnedObjects;

private:
	float m_respawnTimer;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	~ShipSpawner( );
	void Update( );
	void HostSpawnEntity( );
	void WriteToXMLNode( XMLNode * out_xmlNode ) const;

private:
	ShipSpawner( eAllyType const & type, uint8_t level, int maxObjects, Vector2f const & position );
	ShipSpawner( eEnemyType const & type, uint8_t level, int maxObjects, Vector2f const & position );
	void AddObject( GameObject * const object );
};