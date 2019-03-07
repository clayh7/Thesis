#include "Game/GameObjects/Enemies/ShipSpawner.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Game/General/GameCommon.hpp"
#include "Game/GameObjects/Enemies/EnemyShip.hpp"
#include "Game/GameObjects/Allies/AllyShip.hpp"
#include "Game/General/Game.hpp"


//-------------------------------------------------------------------------------------------------
STATIC size_t ShipSpawner::s_nextSpawnerID = 1;
STATIC float const ShipSpawner::RESPAWN_TIME_SEC = 30.0f;
STATIC uint8_t const ShipSpawner::GUARD_COST = 10U;
STATIC Array<ShipSpawner*> ShipSpawner::s_spawners( MAX_SPAWNERS );


//-------------------------------------------------------------------------------------------------
STATIC void ShipSpawner::AddExistingObjectToSpawner( GameObject * const existingObject )
{
	size_t spawnerID = existingObject->m_spawnerID;
	if( spawnerID == INVALID_SPAWNER_ID )
	{
		return;
	}
	for( size_t spawnerIndex = 0; spawnerIndex < s_spawners.Size(); ++spawnerIndex )
	{
		ShipSpawner * currentSpawner = s_spawners[spawnerIndex];
		if( currentSpawner->m_spawnerID == spawnerID )
		{
			currentSpawner->m_spawnedObjects.push_back( existingObject );
			return;
		}
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void ShipSpawner::RemoveFromSpawner( GameObject * const deadObject )
{
	size_t spawnerID = deadObject->m_spawnerID;
	if( spawnerID == INVALID_SPAWNER_ID )
	{
		return;
	}
	for( size_t spawnerIndex = 0; spawnerIndex < s_spawners.Size(); ++spawnerIndex )
	{
		ShipSpawner * currentSpawner = s_spawners[spawnerIndex];
		if( currentSpawner->m_spawnerID == spawnerID )
		{
			std::vector<GameObject*> & spawnedObjects = currentSpawner->m_spawnedObjects;
			for( auto currentObjectIter = spawnedObjects.begin(); currentObjectIter != spawnedObjects.end(); ++currentObjectIter )
			{
				if( *currentObjectIter == deadObject )
				{
					spawnedObjects.erase( currentObjectIter );
					return;
				}
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void ShipSpawner::CreateFromXMLNode( XMLNode const & data )
{
	size_t spawnerID = ReadXMLAttribute( data, "id", INVALID_SPAWNER_ID );
	bool isEnemy = ReadXMLAttribute( data, "isEnemy", true );
	byte_t type = ReadXMLAttribute( data, "type", (byte_t)0U );
	uint8_t level = ReadXMLAttribute( data, "level", (uint8_t) 0U );
	int maxObjects = ReadXMLAttribute( data, "max", 100 );
	Vector2f position = ReadXMLAttribute( data, "position", Vector2f::ZERO );
	if( isEnemy )
	{
		CreateEnemySpawner( (eEnemyType) type, level, maxObjects, position, spawnerID );
	}
	else
	{
		CreateAllySpawner( (eAllyType) type, level, maxObjects, position, spawnerID );
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void ShipSpawner::CreateEnemySpawner( eEnemyType const & type, uint8_t level, int maxObjects /*= 100*/, Vector2f const & position /*= Vector2f::ZERO*/, size_t spawnerID /*= INVALID_SPAWNER_ID */ )
{
	ShipSpawner * spawner = new ShipSpawner( type, level, maxObjects, position );

	//Set spawner ID
	if( spawnerID == INVALID_SPAWNER_ID )
	{
		spawner->m_spawnerID = s_nextSpawnerID;
		++s_nextSpawnerID;
	}

	//Update spawner ID
	else
	{
		spawner->m_spawnerID = spawnerID;
		if( s_nextSpawnerID <= spawnerID )
		{
			s_nextSpawnerID = spawnerID + 1;
		}
	}

	s_spawners.Push( spawner );
}


//-------------------------------------------------------------------------------------------------
STATIC void ShipSpawner::CreateAllySpawner( eAllyType const & type, uint8_t level, int maxObjects /*= 100*/, Vector2f const & position /*= Vector2f::ZERO*/, size_t spawnerID /*= INVALID_SPAWNER_ID */ )
{
	ShipSpawner * spawner = new ShipSpawner( type, level, maxObjects, position );

	//Set spawner ID
	if( spawnerID == INVALID_SPAWNER_ID )
	{
		spawner->m_spawnerID = s_nextSpawnerID;
		++s_nextSpawnerID;
	}

	//Update spawner ID
	else
	{
		spawner->m_spawnerID = spawnerID;
		if( s_nextSpawnerID <= spawnerID )
		{
			s_nextSpawnerID = spawnerID + 1;
		}
	}

	s_spawners.Push( spawner );
}


//-------------------------------------------------------------------------------------------------
STATIC void ShipSpawner::ClearAll( )
{
	s_spawners.DeleteAndClear( );
}


//-------------------------------------------------------------------------------------------------
STATIC void ShipSpawner::UpdateAll( )
{
	for( size_t spawnerIndex = 0; spawnerIndex < s_spawners.Size( ); ++spawnerIndex )
	{
		s_spawners[spawnerIndex]->Update( );
	}
}


//-------------------------------------------------------------------------------------------------
STATIC Vector2f ShipSpawner::GetLocation( size_t spawnerID )
{
	for( size_t spawnerIndex = 0; spawnerIndex < s_spawners.Size( ); ++spawnerIndex )
	{
		if( s_spawners[spawnerIndex]->m_spawnerID == spawnerID )
		{
			return s_spawners[spawnerIndex]->m_position;
		}
	}
	return Vector2f::ZERO;
}


//-------------------------------------------------------------------------------------------------
STATIC Array<ShipSpawner*> const & ShipSpawner::GetSpawnerList( )
{
	return s_spawners;
}


//-------------------------------------------------------------------------------------------------
void ShipSpawner::Update( )
{
	//Tick respawn timers
	if( (int) m_spawnedObjects.size( ) < m_maxObjects )
	{
		m_respawnTimer += Time::DELTA_SECONDS;
	}

	//Spawn a new entity
	if( m_respawnTimer > RESPAWN_TIME_SEC )
	{
		m_respawnTimer = 0.f;
		HostSpawnEntity( );
	}
}


//-------------------------------------------------------------------------------------------------
void ShipSpawner::HostSpawnEntity( )
{
	if( m_isEnemy )
	{
		EnemyShip * enemy = new EnemyShip( m_type.enemy, m_level, true );
		enemy->m_position = m_position;
		enemy->m_wanderTarget = m_position;
		AddObject( enemy );
	}
	else
	{
		AllyShip * ally = new AllyShip( m_type.ally, true );
		ally->m_position = m_position;

		//Cost's the crystal's life to spawn guards
		if( ally->m_allyType == eAllyType_GUARD )
		{
			AllyShip * crystal = g_GameSystem->m_gameState.m_hostCrystal;
			crystal->HostTakeDamage( GUARD_COST, GameObject::ALLY_PLAYER_INDEX );
		}

		AddObject( ally );
	}
}


//-------------------------------------------------------------------------------------------------
void ShipSpawner::WriteToXMLNode( XMLNode * out_xmlNode ) const
{
	XMLNode node = out_xmlNode->addChild( "EnemySpawner" );
	node.addAttribute( "id", Stringf( "%u", m_spawnerID ).c_str( ) );
	node.addAttribute( "isEnemy", m_isEnemy ? "true" : "false" );
	node.addAttribute( "type", Stringf( "%u", m_type ).c_str( ) );
	node.addAttribute( "level", Stringf( "%u", m_level ).c_str( ) );
	node.addAttribute( "max", Stringf( "%d", m_maxObjects ).c_str( ) );
	node.addAttribute( "position", Stringf( "%f,%f", m_position.x, m_position.y ).c_str( ) );
}


//-------------------------------------------------------------------------------------------------
ShipSpawner::ShipSpawner( eAllyType const & type, uint8_t level, int maxObjects, Vector2f const & position )
	: m_spawnerID( INVALID_SPAWNER_ID )
	, m_isEnemy( false )
	, m_type( type )
	, m_level( level )
	, m_maxObjects( maxObjects )
	, m_position( position )
	, m_respawnTimer( 0.f )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
ShipSpawner::ShipSpawner( eEnemyType const & type, uint8_t level, int maxObjects, Vector2f const & position )
	: m_spawnerID( INVALID_SPAWNER_ID )
	, m_isEnemy( true )
	, m_type( type )
	, m_level( level )
	, m_maxObjects( maxObjects )
	, m_position( position )
	, m_respawnTimer( 0.f )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
ShipSpawner::~ShipSpawner( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void ShipSpawner::AddObject( GameObject * const object )
{
	object->m_spawnerID = m_spawnerID;

	if( g_GameSystem->IsHost( ) )
	{
		g_GameSystem->HostCreateNetGameObject( object );
	}

	m_spawnedObjects.push_back( object );
}