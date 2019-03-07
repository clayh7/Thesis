#pragma once

#include "Game/GameObjects/Player/Ship.hpp"


//-------------------------------------------------------------------------------------------------
enum eAllyType : uint8_t
{
	eAllyType_CRYSTAL,
	eAllyType_GUARD,
	eAllyType_INVALID,
};


//-------------------------------------------------------------------------------------------------
class EnemyShip;


//-------------------------------------------------------------------------------------------------
class AllyShip : public Ship
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static char const * NAME;
	static char const * CRYSTAL_NAME;
	static float const NET_SYNC_PERCENT;
	static float const RADIUS;
	static float const SIGHT_RADIUS;
	static float const SPEED;
	static float const SPEED_DECAY_RATE;
	static float const TURN_SPEED;
	static float const SAFE_DISTANCE;
	static float const SAFE_RADIUS;
	static float const FIRE_RATE_SECONDS;
	static float const MAX_WANDER_DISTANCE;
	static float const WANDER_THRESHOLD;
	static float const SHIELD_REGEN_COOLDOWN;
	static uint16_t const MAX_HEALTH;
	static uint16_t const MAX_SHIELD;
	static uint16_t const MAX_ENERGY;
	static float const CRYSTAL_RADIUS;
	static uint16_t const CRYSTAL_MAX_HEALTH;
	static uint16_t const CRYSTAL_MAX_SHIELD;
	static uint16_t const CRYSTAL_MAX_ENERGY;

private:
	static GameObjectRegistration s_AllyXMLRegistration;
	static GameObjectRegistration s_AllyMessageRegistration;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static GameObject * MessageCreation( NetSender const &, NetMessage const & );
	static GameObject * XMLCreation( XMLNode const & );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
//Synced Members
public:
	eAllyType m_allyType;

//Non-Synced Members
private:
	Sprite * m_crystalBubble;
	float m_aiTimer;
	float m_shieldRegenTimer;

public:
	Ship * m_targetShip;
	Vector2f m_wanderTarget;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	AllyShip( NetSender const & sender, NetMessage const & message );
	virtual void WriteToMessage( NetMessage * out_message ) override;
	virtual void UpdateFromMessage( NetSender const & sender, NetMessage const & message ) override;

	AllyShip( XMLNode const & node );
	virtual void WriteToXMLNode( XMLNode * out_xmlNode ) override;
	virtual void UpdateFromXMLNode( XMLNode const & node ) override;

	AllyShip( eAllyType const & allyType, bool hostObject );
	virtual ~AllyShip( ) override;
	virtual void Update( ) override;
	virtual void HostHandleCollision( GameObject * gameObject ) override;
	virtual bool HostIsAlive( ) const override;
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

	void HostHealDamage( uint16_t incomingHeal );
	bool IsCrystal( ) const;

protected:
	void UpdateTimers( );
	void HostApplyAIInput( );
	void HostAcquireNewTarget( );
	void HostTurnTowardsTarget( );
	void HostMoveTowardsSafeZone( );
	void HostMoveTowardsWanderTarget( );
	//void HostMoveInSquare( );
	bool HasTarget( ) const;
	void CreateOrUpdateCrystalBubble( );
};