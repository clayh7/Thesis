#pragma once

#include "Game/GameObjects/GameObject.hpp"


//-------------------------------------------------------------------------------------------------
enum eItemType : byte_t;


//-------------------------------------------------------------------------------------------------
class Pickup : public GameObject
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static char const * NAME;
	static float const NET_SYNC_PERCENT;
	static float const RADIUS;
	static float const MAX_AGE;

private:
	static GameObjectRegistration s_PickupXMLRegistration;
	static GameObjectRegistration s_PickupMessageRegistration;

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
	uint16_t m_itemCode;
	uint16_t m_netItemCode;

//Non-Synced Members
	bool m_taken;
	float m_age;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Pickup( NetSender const & sender, NetMessage const & message );
	virtual void WriteToMessage( NetMessage * out_message ) override;
	virtual void UpdateFromMessage( NetSender const & sender, NetMessage const & message ) override;
	
	Pickup( XMLNode const & node );
	virtual void WriteToXMLNode( XMLNode * out_xmlNode ) override;
	virtual void UpdateFromXMLNode( XMLNode const & node ) override;

	Pickup( eItemType const & itemType, bool hostObject );
	Pickup( uint16_t itemCode, bool hostObject );
	virtual ~Pickup( ) override;
	virtual void Update( ) override;
	virtual void HostHandleCollision( GameObject * gameObject ) override;
	virtual bool HostIsAlive( ) const override;
	virtual char const * GetEntityName( ) const override;
	virtual std::string GetSpriteID( ) const override;
	virtual float GetNetSyncPercent( ) const override;
	virtual float GetPhysicsRadius( ) const override;
	virtual float GetSpriteScale( ) const override;
};