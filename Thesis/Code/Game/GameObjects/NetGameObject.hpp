#pragma once

#include "Game/General/GameCommon.hpp"


//-------------------------------------------------------------------------------------------------
class GameObject;


//-------------------------------------------------------------------------------------------------
class NetGameObject
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static size_t const INVALID_NET_ID = 0;
	static size_t s_nextNetID;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	size_t m_netID;
	GameObject * m_localObject;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	NetGameObject( GameObject * linkedGameObject );
	NetGameObject( size_t netID, GameObject * linkedGameObject );
	~NetGameObject( );
	void WriteToMessage( NetMessage * out_message );

	size_t GetID( ) const;
	GameObject * GetLocalObject( ) const;
	eNetGameObjectType GetType( ) const;
};