#pragma once

#include "Game/GameObjects/GameObject.hpp"


//-------------------------------------------------------------------------------------------------
enum eEmoteType : uint8_t
{
	eEmoteType_HAPPY,
	eEmoteType_SAD,
	eEmoteType_ANGRY,
	eEmoteType_WHOOPS,
	eEmoteType_HELP,
	eEmoteType_INVALID,
};


//-------------------------------------------------------------------------------------------------
class Emote : public GameObject
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
	static GameObjectRegistration s_EmoteXMLRegistration;
	static GameObjectRegistration s_EmoteMessageRegistration;

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
	eEmoteType m_emoteType;

//Non-Synced Members
public:
	float m_age;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Emote( NetSender const & sender, NetMessage const & message );
	virtual void WriteToMessage( NetMessage * out_message ) override;
	virtual void UpdateFromMessage( NetSender const & sender, NetMessage const & message ) override;
	
	Emote( XMLNode const & node );
	virtual void WriteToXMLNode( XMLNode * out_xmlNode ) override;
	virtual void UpdateFromXMLNode( XMLNode const & node ) override;

	Emote( eEmoteType type, bool hostObject );
	virtual ~Emote( ) override;
	virtual void Update( ) override;
	virtual void HostHandleCollision( GameObject * gameObject ) override;
	virtual bool HostIsAlive( ) const override;
	virtual char const * GetEntityName( ) const override;
	virtual std::string GetSpriteID( ) const override;
	virtual float GetNetSyncPercent( ) const override;
	virtual float GetPhysicsRadius( ) const override;
	virtual float GetSpriteScale( ) const override;

	Color GetSpriteColor( ) const;
};