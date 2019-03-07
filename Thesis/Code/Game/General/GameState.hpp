#pragma once

#include "Game/General/GameCommon.hpp"


//-------------------------------------------------------------------------------------------------
class AllyShip;
class GameObject;
class NetMessage;
struct XMLNode;


//-------------------------------------------------------------------------------------------------
class GameState
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static char const * NAME;
	static char const * STRING_SUMMONING_AMOUNT;
	static uint8_t const MAX_SUMMONING_BAR;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static void SetCrystal( GameObject * gameObject, bool isHost );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	uint8_t m_hostSummoningAmount;
	uint8_t m_clientSummoningAmount;
	AllyShip * m_hostCrystal;
	AllyShip * m_clientCrystal;
	bool m_hostIsCthulhuActive;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	GameState( );
	void WriteToMessage( NetMessage * out_message );
	void UpdateFromMessage( NetSender const & sender, NetMessage const & message );
	void WriteToXMLNode( XMLNode * out_xmlNode );
	void UpdateFromXMLNode( XMLNode const & node );

	void Update( );

	void HostIncreaseSummoningBar( uint8_t amount );
	bool HostIsWorldDestroyed( ) const;
	float GetCrystalLifePercent( bool isHost ) const;
};