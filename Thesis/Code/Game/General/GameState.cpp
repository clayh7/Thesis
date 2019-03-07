#include "Game/General/GameState.hpp"

#include "Engine/Net/Session/NetMessage.hpp"
#include "Engine/Net/Session/NetConnection.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Game/GameObjects/Allies/AllyShip.hpp"
#include "Game/General/Game.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * GameState::NAME = "GameState";
STATIC char const * GameState::STRING_SUMMONING_AMOUNT = "summoningAmount";
STATIC uint8_t const GameState::MAX_SUMMONING_BAR = 100;


//-------------------------------------------------------------------------------------------------
STATIC void GameState::SetCrystal( GameObject * gameObject, bool isHost )
{
	GameState & gameState = g_GameSystem->m_gameState;
	if( isHost )
	{
		gameState.m_hostCrystal = (AllyShip*) gameObject;
	}
	else
	{
		gameState.m_clientCrystal = (AllyShip*)gameObject;
	}
}


//-------------------------------------------------------------------------------------------------
GameState::GameState( )
	: m_hostSummoningAmount( 0 )
	, m_clientSummoningAmount( 0 )
	, m_hostCrystal( nullptr )
	, m_clientCrystal( nullptr )
	, m_hostIsCthulhuActive( false )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
// Host Only
void GameState::WriteToMessage( NetMessage * out_message )
{
	out_message->Write<uint8_t>( m_hostSummoningAmount );
}


//-------------------------------------------------------------------------------------------------
// Client Only
void GameState::UpdateFromMessage( NetSender const &, NetMessage const & message )
{
	message.Read<uint8_t>( &m_clientSummoningAmount );
}


//-------------------------------------------------------------------------------------------------
// Host Only
void GameState::WriteToXMLNode( XMLNode * out_xmlNode )
{
	XMLNode node = out_xmlNode->addChild( NAME );
	node.addAttribute( STRING_SUMMONING_AMOUNT, Stringf( "%u", m_hostSummoningAmount ).c_str( ) );
}


//-------------------------------------------------------------------------------------------------
// Host Only
void GameState::UpdateFromXMLNode( XMLNode const & node )
{
	m_hostSummoningAmount = ReadXMLAttribute( node, STRING_SUMMONING_AMOUNT, m_hostSummoningAmount );
}


//-------------------------------------------------------------------------------------------------
void GameState::Update( )
{
	//Host Update
	if( g_GameSystem->IsHost( ) )
	{
		if( m_hostSummoningAmount == MAX_SUMMONING_BAR && !m_hostIsCthulhuActive )
		{
			g_GameSystem->HostSummonCthulhu( );
		}
	}

	//Client Update
	else
	{
		//Nothing
	}
}


//-------------------------------------------------------------------------------------------------
void GameState::HostIncreaseSummoningBar( uint8_t amount )
{
	uint16_t totalAmount = (uint16_t) ( m_hostSummoningAmount + amount );
	
	//Make sure summoning bar does not go over max
	if( totalAmount > MAX_SUMMONING_BAR )
	{
		m_hostSummoningAmount = MAX_SUMMONING_BAR;
	}
	else
	{
		m_hostSummoningAmount = (uint8_t) totalAmount;
	}
}


//-------------------------------------------------------------------------------------------------
bool GameState::HostIsWorldDestroyed( ) const
{
	return m_hostCrystal == nullptr;
}


//-------------------------------------------------------------------------------------------------
float GameState::GetCrystalLifePercent( bool isHost ) const
{
	AllyShip * crystal = nullptr;
	if( isHost )
	{
		crystal = m_hostCrystal;
	}
	else
	{
		crystal = m_clientCrystal;
	}

	if( crystal )
	{
		float currentLife = (float) crystal->m_health;
		float maxLife = (float) AllyShip::CRYSTAL_MAX_HEALTH;
		return currentLife / maxLife;
	}
	else
	{
		return 0.f;
	}
}