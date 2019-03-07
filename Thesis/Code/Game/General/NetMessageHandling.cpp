#include "Game/General/NetMessageHandling.hpp"

#include "Engine/Net/Session/NetPacket.hpp"
#include "Engine/Net/Session/NetConnection.hpp"
#include "Game/GameObjects/Enemies/Rock.hpp"
#include "Game/GameObjects/Enemies/EnemyShip.hpp"
#include "Game/GameObjects/Other/Bullet.hpp"
#include "Game/GameObjects/Other/Emote.hpp"
#include "Game/GameObjects/Player/PlayerShip.hpp"
#include "Game/GameObjects/NetGameObject.hpp"
#include "Game/General/Game.hpp"
#include "Game/General/GameState.hpp"


//-------------------------------------------------------------------------------------------------
void OnCreatePlayerReceived( NetSender const &, NetMessage const & message )
{
	byte_t netIndex;
	message.Read<byte_t>( &netIndex );
	char * username;
	message.ReadString( &username );
	g_GameSystem->ClientCreatePlayer( netIndex, username );
}


//-------------------------------------------------------------------------------------------------
void OnDestroyPlayerReceived( NetSender const &, NetMessage const & message )
{
	byte_t netIndex;
	message.Read<byte_t>( &netIndex );
	g_GameSystem->ClientDestroyPlayer( netIndex );
}


//-------------------------------------------------------------------------------------------------
void OnUpdatePlayerReceived( NetSender const & sender, NetMessage const & message )
{
	byte_t netIndex;
	message.Read<byte_t>( &netIndex );
	Player * updatePlayer = g_GameSystem->ClientGetPlayer( netIndex );
	if( updatePlayer )
	{
		updatePlayer->UpdateFromMessage( sender, message );
	}
}


//-------------------------------------------------------------------------------------------------
void OnCreateMessageReceived( NetSender const & sender, NetMessage const & message )
{
	if( !g_GameSystem->m_isDedicatedServer )
	{
		size_t netID;
		message.Read<size_t>( &netID );
		eNetGameObjectType type;
		message.Read<eNetGameObjectType>( &type );
		GameObject * object = GameObject::CreateFromNetMessage( type, sender, message );
		ASSERT_RECOVERABLE( object != nullptr, "OnCreateMessageReceived tried to make an object from a bad type" );
		g_GameSystem->ClientCreateNetGameObject( netID, object );
	}
}


//-------------------------------------------------------------------------------------------------
void OnDestroyMessageReceived( NetSender const &, NetMessage const & message )
{
	if( !g_GameSystem->m_isDedicatedServer )
	{
		size_t netID;
		message.Read<size_t>( &netID );
		g_GameSystem->ClientDestroyNetGameObject( netID );
	}
}


//-------------------------------------------------------------------------------------------------
void OnUpdateMessageReceived( NetSender const & sender, NetMessage const & message )
{
	if( !g_GameSystem->m_isDedicatedServer )
	{
		size_t netID;
		message.Read<size_t>( &netID );

		NetGameObject * netObject = g_GameSystem->ClientGetNetGameObjectFromNetID( netID );

		//Received an update for a destroy object
		if( netObject == nullptr )
		{
			return;
		}

		GameObject * object = netObject->GetLocalObject( );
		object->UpdateFromMessage( sender, message );
	}
}


//-------------------------------------------------------------------------------------------------
void OnGameStateMessageReceived( NetSender const & sender, NetMessage const & message )
{
	if( !g_GameSystem->m_isDedicatedServer )
	{
		GameState & gameState = g_GameSystem->m_gameState;
		gameState.UpdateFromMessage( sender, message );
	}
}


//-------------------------------------------------------------------------------------------------
void OnInputMessageReceived( NetSender const & sender, NetMessage const & message )
{
	//#TODO:  Handle reliable & unreliable input
	//Reliable Input
	if( message.GetNetMessageType( ) == eNetGameMessageType_RELIABLE_INPUT )
	{
		eGameEvent inputPressed;
		message.Read<eGameEvent>( &inputPressed );
		byte_t playerIndex = sender.connection->GetIndex( );
		
		//Handle Eject Item Event
		if( inputPressed == eGameEvent_EJECT_ITEM )
		{
			byte_t itemIndex;
			message.Read<byte_t>( &itemIndex );
			Player * actingPlayer = g_GameSystem->HostGetPlayer( playerIndex );
			if( actingPlayer )
			{
				actingPlayer->HostEjectItemFromInventory( itemIndex );
				g_GameSystem->HostUpdatePlayer( playerIndex );
			}
		}

		else if( inputPressed == eGameEvent_EQUIP_PRIMARY )
		{
			byte_t itemIndex;
			message.Read<byte_t>( &itemIndex );
			Player * actingPlayer = g_GameSystem->HostGetPlayer( playerIndex );
			if( actingPlayer )
			{
				actingPlayer->HostEquipItem( eEquipmentSlot_PRIMARY, itemIndex );

				//#TODO: do i really need to update the player?
				g_GameSystem->HostUpdatePlayer( playerIndex );
			}
		}

		else if( inputPressed == eGameEvent_REMOVE_PRIMARY )
		{
			Player * actingPlayer = g_GameSystem->HostGetPlayer( playerIndex );
			if( actingPlayer )
			{
				actingPlayer->HostRemoveEquipment( eEquipmentSlot_PRIMARY );

				//#TODO: do i really need to update the player?
				g_GameSystem->HostUpdatePlayer( playerIndex );
			}
		}

		else if( inputPressed == eGameEvent_FEED_CRYSTAL )
		{
			g_GameSystem->HostPlayerFeedCrystal( playerIndex );
		}

		//Handle All Other Events
		else
		{
			g_GameSystem->HostApplyInputEventToPlayer( inputPressed, playerIndex );
		}
	}
	//Unreliable Input
	else
	{
		uint16_t inputBitfield;
		message.Read<uint16_t>( &inputBitfield );
		byte_t playerIndex = sender.connection->GetIndex( );
		g_GameSystem->HostApplyInputToPlayer( inputBitfield, playerIndex );
	}
}