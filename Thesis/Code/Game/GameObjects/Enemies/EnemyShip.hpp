#pragma once

#include "Game/GameObjects/Player/Ship.hpp"


//-------------------------------------------------------------------------------------------------
enum eEnemyType : byte_t //Only use 16 values max (4 bits)
{
	eEnemyType_BASIC,
	eEnemyType_ARCHER,
	eEnemyType_BOSS_RED,
	eEnemyType_COUNT,
	eEnemyType_INVALID,
};


//-------------------------------------------------------------------------------------------------
class PlayerShip;


//-------------------------------------------------------------------------------------------------
class EnemyShip : public Ship
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static char const * NAME;
	static float const NET_SYNC_PERCENT;
	static float const SPEED;
	static float const SPEED_DECAY_RATE;
	static float const TURN_SPEED;
	static float const RADIUS;
	static float const SIGHT_RADIUS;
	static float const SAFE_DISTANCE;
	static float const SAFE_RADIUS;
	static float const FIRE_RATE_SECONDS;
	static uint8_t const CUBES_DROPPED_PER_LEVEL;
	static float const DROP_WEAPON_CHANCE;
	static float const BOSS_RED_RADIUS;
	static float const BOSS_RED_FIRE_RATE_SECONDS;
	static float const BOSS_RED_TURN_SPEED;
	static float const MAX_WANDER_DISTANCE;
	static float const WANDER_THRESHOLD;
	static int const BASE_HEALTH;
	static int const HEALTH_PER_LEVEL;
	static float const SHIELD_REGEN_COOLDOWN;
	static int const BASE_SHIELD;
	static int const SHIELD_PER_LEVEL;
	static int const MAX_ENERGY;
	static uint16_t CRYSTAL_HEAL_REWARD;

private:
	static GameObjectRegistration s_EnemyXMLRegistration;
	static GameObjectRegistration s_EnemyMessageRegistration;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static GameObject * MessageCreation( NetSender const &, NetMessage const & );
	static GameObject * XMLCreation( XMLNode const & );

	static eEnemyType ParseEnemyType( uint8_t enemyCode );
	static uint8_t ParseLevel( uint8_t enemyCode );
	static uint8_t GenerateCode( eEnemyType type, uint8_t level );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
//Synced Members
public:
	// 4 bits (level) 4 bits (enemy type)
	uint8_t m_enemyCode;
	uint8_t m_netEnemyCode;

//Non-Synced Members
private:
	float m_aiTimer;
	float m_shieldRegenTimer;

public:
	Ship * m_targetShip;
	Vector2f m_wanderTarget;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	EnemyShip( NetSender const & sender, NetMessage const & message );
	virtual void WriteToMessage( NetMessage * out_message ) override;
	virtual void UpdateFromMessage( NetSender const & sender, NetMessage const & message ) override;

	EnemyShip( XMLNode const & node );
	virtual void WriteToXMLNode( XMLNode * out_xmlNode ) override;
	virtual void UpdateFromXMLNode( XMLNode const & node ) override;

	EnemyShip( eEnemyType const & enemyType, uint8_t level, bool hostObject );
	virtual ~EnemyShip( ) override;
	virtual void Update( ) override;
	virtual void HostHandleCollision( GameObject * gameObject ) override;
	virtual void HostFireWeapon( ) const override;
	virtual char const * GetEntityName( ) const override;
	virtual std::string GetSpriteID( ) const override;
	virtual float GetNetSyncPercent( ) const override;
	virtual float GetPhysicsRadius( ) const override;
	virtual float GetSpriteScale( ) const override;
	virtual void HostOnDestroy( ) override;
	virtual void ClientOnDestroy( ) override;

	virtual void ApplyControlInput( ) override;
	virtual int GetMaxHealth( ) const override;
	virtual int GetMaxShield( ) const override;
	virtual int GetMaxEnergy( ) const override;
	virtual float GetMaxSpeed( ) const override;
	virtual float GetFireRate( ) const override;
	virtual uint8_t GetSummoningBarValue( ) const override;
	virtual Vector2f GetHomeLocation( ) const;
	eEnemyType GetEnemyType( ) const;
	uint8_t GetLevel( ) const;

protected:
	void UpdateTimers( );
	void HostApplyAIInput( );
	void HostAcquireNewTarget( );
	void HostTurnTowardsTarget( );
	void HostMoveTowardsSafeZone( );
	void HostMoveTowardsWanderTarget( );
	bool HasTarget( ) const;

public:
	bool IsBoss( ) const;
};