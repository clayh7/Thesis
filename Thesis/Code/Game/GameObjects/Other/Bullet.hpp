#pragma once

#include <vector>
#include "Game/GameObjects/GameObject.hpp"


//-------------------------------------------------------------------------------------------------
class BulletDefinition
{
public:
	uint16_t m_damage;
	uint16_t m_energyCost;
	float m_fireRate;
	uint8_t m_sizeAttribute;
};


//-------------------------------------------------------------------------------------------------
class ParticleSystem;
class Sprite;


//-------------------------------------------------------------------------------------------------
class Bullet : public GameObject
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static char const * NAME;
	static float const NET_SYNC_PERCENT;
	static float const SPEED;
	static float const RADIUS;
	static float const BULLET_RADIUS_TO_SCALE;
	static float const LIFETIME;
	static float const RADIUS_PER_WEAPON_AOE;

private:
	static GameObjectRegistration s_BulletMessageRegistration;
	static std::map<uint16_t,BulletDefinition> s_bulletRegistry;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static GameObject * MessageCreation( NetSender const & sender, NetMessage const & message );
	//Currently this object does not save to XML, although it's not hard to add

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
//Synced Members
public:
	uint16_t m_itemCode;
	uint16_t m_netItemCode;

//Non-Synced Members
private:
	float m_age;
	ParticleSystem * m_trailEffect;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Bullet( NetSender const & sender, NetMessage const & message );
	virtual void WriteToMessage( NetMessage * out_message ) override;
	virtual void UpdateFromMessage( NetSender const & sender, NetMessage const & message ) override;

	//Bullet( XMLNode const & node );
	virtual void WriteToXMLNode( XMLNode * out_xmlNode ) override;
	virtual void UpdateFromXMLNode( XMLNode const & node ) override;

	Bullet( byte_t playerIndex, uint16_t itemCode, bool hostObject );
	virtual ~Bullet( ) override;
	virtual void Update( ) override;
	virtual void HostHandleCollision( GameObject * gameObject ) override;
	virtual bool HostIsAlive( ) const override;
	virtual char const * GetEntityName( ) const override;
	virtual std::string GetSpriteID( ) const override;
	virtual float GetNetSyncPercent( ) const override;
	virtual float GetPhysicsRadius( ) const override;
	virtual float GetSpriteScale( ) const override;
	virtual void ClientOnDestroy( ) override;

	uint16_t GetDamage( );
	uint16_t GetEnergyCost( );
	float GetFireRate( );
	uint8_t GetSizeAttribute( );

	void HostKillBullet( );
};