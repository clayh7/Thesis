#pragma once

#include "Game/GameObjects/GameObject.hpp"


//-------------------------------------------------------------------------------------------------
struct XMLNode;


//-------------------------------------------------------------------------------------------------
class Rock : public GameObject
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static char const * NAME;
	static float const NET_SYNC_PERCENT;
	static float const RADIUS;
	static float const ROCK_RADIUS_TO_SCALE;
	static float const MAX_SPIN_SPEED;
	static int const MAX_HEALTH;

private:
	static GameObjectRegistration s_RockMessageRegistration;
	static GameObjectRegistration s_RockXMLRegistration;

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
	uint16_t m_health;
	uint16_t m_netHealth;

//Non-Synced Members
private:
	float m_spinSpeed;
	int m_randomSpriteNum;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Rock( NetSender const & sender, NetMessage const & message );
	virtual void WriteToMessage( NetMessage * out_message ) override;
	virtual void UpdateFromMessage( NetSender const & sender, NetMessage const & message ) override;
	
	Rock( XMLNode const & node );
	virtual void WriteToXMLNode( XMLNode * out_xmlNode ) override;
	virtual void UpdateFromXMLNode( XMLNode const & node ) override;

	Rock( bool hostObject );
	virtual ~Rock( ) override;
	virtual void Update( ) override;
	virtual void HostHandleCollision( GameObject * gameObject ) override;
	virtual bool HostIsAlive( ) const override;
	virtual char const * GetEntityName( ) const override;
	virtual std::string GetSpriteID( ) const override;
	virtual float GetNetSyncPercent( ) const override;
	virtual float GetPhysicsRadius( ) const override;
	virtual float GetSpriteScale( ) const override;
	virtual void HostOnDestroy( ) override;
	virtual void ClientOnDestroy( ) override;

	void HostTakeDamage( uint16_t incomingDamage );
	int GetMaxHealth( ) const;
};