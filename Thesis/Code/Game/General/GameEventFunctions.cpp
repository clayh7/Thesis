#include "Game/General/Game.hpp"

#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Net/Session/NetConnection.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"
#include "Engine/UISystem/UIContainer.hpp"
#include "Engine/UISystem/UIItem.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UITextField.hpp"
#include "Game/GameObjects/Items/Item.hpp"
#include "Game/GameObjects/NetGameObject.hpp"
#include "Game/GameObjects/Other/Bullet.hpp"
#include "Game/GameObjects/Player/Player.hpp"
#include "Game/GameObjects/Player/PlayerShip.hpp"


//-------------------------------------------------------------------------------------------------
void Game::OnConnectionJoin( NamedProperties & netEvent )
{
	NetConnection * connection;
	netEvent.Get( "connection", connection );

	if( IsHost( ) )
	{
		//Create player
		if( ( connection->IsHost( ) && !m_isDedicatedServer ) ||
			!connection->IsHost( ) )
		{
			HostCreatePlayer( connection->GetIndex( ), connection->GetUsername( ), connection->GetPassword( ) );

			//Send back all other objects that exist
			if( !connection->IsHost( ) )
			{
				HostCatchupConnectionOnCurrentGameState( connection );
			}
		}
		
		//Make this after, because when a world is created, all the data used to make the world is sent
		//Create Game
		if( connection->IsHost( ) )
		{
			HostInitializeGame( );
		}

		m_HostGameActivityLog.Printf( "%s (index=%u) Connected", connection->GetUsername( ), connection->GetIndex( ) );
	}
	else
	{
		m_ClientGameActivityLog.Printf( "%s (index=%u) Connected", connection->GetUsername( ), connection->GetIndex( ) );
	}

}


//-------------------------------------------------------------------------------------------------
void Game::OnConnectionLeave( NamedProperties & netEvent )
{
	NetConnection * connection;
	netEvent.Get( "connection", connection );

	if( IsHost( ) && !m_isDedicatedServer )
	{
		HostDestroyPlayer( connection->GetIndex( ) );
	}

	//The host and self is disconnected seperately (if they're the same)
	//If we're disconnecting the host, who happens to be me
	if( connection->IsHost( ) && IsHost( ) )
	{
		SaveWorldToFile( );
		SaveAllPlayersToFile( );
	}

	else if( connection->IsSelf( ) )
	{
		CleanupPlayersAndGameObjects( );
	}

	if( IsHost( ) )
	{
		m_HostGameActivityLog.Printf( "%s (index=%u) Disconnected", connection->GetUsername( ), connection->GetIndex( ) );
	}
	else
	{
		m_ClientGameActivityLog.Printf( "%s (index=%u) Disconnected", connection->GetUsername( ), connection->GetIndex( ) );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::OnPreparePacket( NamedProperties & netEvent )
{
	NetConnection * connection;
	netEvent.Get( "connection", connection );

	// Send Client info
	Player * currentPlayer = ClientGetPlayerSelf( );
	if( connection->IsHost( ) )
	{
		//Don't send data to yourself
		if( IsHost( ) && m_isDedicatedServer )
		{
			return;
		}

		if( currentPlayer )
		{
			//Send Input State
			NetMessage inputState( eNetGameMessageType_UNRELIABLE_INPUT );
			inputState.Write<uint16_t>( currentPlayer->GetInputBitfield( ) );
			connection->AddMessage( inputState );
		}
	}

	// Then send host info, because if we're host/client we need to make sure our input is sent
	if( IsHost( ) )
	{
		NetMessage gameStateUpdate( eNetGameMessageType_GAME_STATE_UPDATE );
		m_gameState.WriteToMessage( &gameStateUpdate );
		connection->AddMessage( gameStateUpdate );

		//Sync ships first
		for( size_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
		{
			if( m_hostPlayers[playerIndex] == nullptr )
			{
				continue;
			}
			if( m_hostPlayers[playerIndex]->m_localShip == nullptr )
			{
				continue;
			}

			//Send all ships
			NetMessage update( eNetGameMessageType_NET_OBJECT_UPDATE );
			NetGameObject * netShip = m_hostPlayers[playerIndex]->m_localShip;
			netShip->WriteToMessage( &update );
			connection->AddMessage( update );
		}
		//Sync things that are not ships
		for( size_t objectIndex = 0; objectIndex < m_hostObjects.size( ); ++objectIndex )
		{
			//Sync bullets / enemies
			//Only sync if they're close
			//#TODO: Make sure to make this is flexible when more entities need to get networked
			GameObject * object = m_hostObjects[objectIndex]->GetLocalObject( );
			if( object->m_type == eNetGameObjectType_BULLET ||
				object->m_type == eNetGameObjectType_ENEMYSHIP ||
				object->m_type == eNetGameObjectType_ROCK )
			{
				size_t currentIndex = connection->GetIndex( );
				Player * checkPlayer = m_hostPlayers[currentIndex];
				//Only sync players that exist
				if( checkPlayer == nullptr )
				{
					continue;
				}
				Vector2f curretPlayerPosition = checkPlayer->GetLastPosition( );
				float maxDistance = g_SpriteRenderSystem->GetVirtualSize( );
				if( DistanceBetweenPointsSquared( curretPlayerPosition, object->m_position ) > ( maxDistance * maxDistance ) )
				{
					continue;
				}

				NetMessage update( eNetGameMessageType_NET_OBJECT_UPDATE );
				m_hostObjects[objectIndex]->WriteToMessage( &update );
				connection->AddMessage( update );
			}

			//Always sync Crystal
			else if( object->m_type == eNetGameObjectType_ALLYSHIP )
			{
				size_t currentIndex = connection->GetIndex( );
				Player * checkPlayer = m_hostPlayers[currentIndex];
				//Only sync players that exist
				if( checkPlayer == nullptr )
				{
					continue;
				}
				//Vector2f curretPlayerPosition = checkPlayer->GetLastPosition( );
				NetMessage update( eNetGameMessageType_NET_OBJECT_UPDATE );
				m_hostObjects[objectIndex]->WriteToMessage( &update );
				connection->AddMessage( update );
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Game::OnJoinRequestValidation( NamedProperties & netEvent )
{
	JoinRequest * request;
	netEvent.Get( "request", request );

	//This is what it looks like when netEvent enters this function
	//netEvent.Set( "canJoin", true );
	//netEvent.Set( "error", eNetSessionError_NONE );

	//Is the requested join username already playing
	for( size_t playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex )
	{
		if( m_hostPlayers[playerIndex] == nullptr )
		{
			continue;
		}

		//If the names don't match, don't care
		if( strncmp( m_hostPlayers[playerIndex]->GetUsername( ), request->username, Player::MAX_USERNAME_SIZE ) != 0 )
		{
			continue;
		}

		//Password matches
		if( m_hostPlayers[playerIndex]->GetPassword( ) == request->password ||
			m_hostPlayers[playerIndex]->GetPassword( ) == Player::NO_PASSWORD )
		{
			//Get connection to boot
			NetConnection * bootConnection = s_netSession->GetNetConnection( (byte_t) playerIndex );
			if( bootConnection )
			{
				if( bootConnection->IsHost( ) )
				{
					//Login Success but you kick this player off
					netEvent.Set( "canJoin", false );
					netEvent.Set( "error", eGameNetSessionError_LOGGED_IN_AS_HOST ); //(eNetSessionError)
					Game::s_netSession->Leave( );
				}
				else
				{
					//Tell Connection that the host left (This will make them leave)
					NetMessage leave( eNetMessageType_LEAVE, s_netSession->GetHost( )->GetIndex( ) );
					bootConnection->AddMessage( leave );
					bootConnection->SendPacket( );

					//Boot other player
					s_netSession->Disconnect( &bootConnection );
				}
			}
			return;
		}
		else
		{
			netEvent.Set( "canJoin", false );
			netEvent.Set( "error", eGameNetSessionError_WRONG_PASSWORD );
			return;
		}
	}

	//Let's see if we know about the player, but they typed in the wrong password
	if( Player::ExistsButWrongPassword( request->username, request->password ) )
	{
		netEvent.Set( "canJoin", false );
		netEvent.Set( "error", eGameNetSessionError_WRONG_PASSWORD );
	}
}


//-------------------------------------------------------------------------------------------------
void Game::OnJoinDeny( NamedProperties & )
{
	g_GameSystem->m_lastInvalidPasswordTime = Time::TOTAL_SECONDS;
}


//-------------------------------------------------------------------------------------------------
void Game::OnDedicatedHostButton( NamedProperties & )
{
	m_showDebug = true;
	m_isDedicatedServer = true;
	AttemptCreateHost( "", "" );
}


//-------------------------------------------------------------------------------------------------
void Game::OnGameHostButton( NamedProperties & )
{
	m_showDebug = false;
	m_isDedicatedServer = false;

	//Get username from username field
	std::string defaultUsername = NetworkUtils::GetLocalHostName( );
	UITextField * usernameField = g_UISystem->GetWidgetByName<UITextField>( UI_USERNAME_FIELD );
	std::string username = usernameField->GetText( );

	//If the player did not type in a username, use the default username
	if( strcmp( username.c_str( ), "" ) == 0 )
	{
		username = defaultUsername;
	}

	//Get password from password field
	UITextField * passwordField = g_UISystem->GetWidgetByName<UITextField>( UI_PASSWORD_FIELD );
	std::string password = passwordField->GetText( );

	//Host Game
	AttemptCreateHost( username, password );
}


//-------------------------------------------------------------------------------------------------
void Game::OnGameJoinButton( NamedProperties & )
{
	UITextField * addressField = g_UISystem->GetWidgetByName<UITextField>( UI_ADDRESS_FIELD );
	std::string addressString = addressField->GetText( );

	//Get username from username field
	UITextField * usernameField = g_UISystem->GetWidgetByName<UITextField>( UI_USERNAME_FIELD );
	std::string username = usernameField->GetText( );

	//If the player did not type in a username, use the default username
	if( strcmp( username.c_str( ), "" ) == 0 )
	{
		username = NetworkUtils::GetLocalHostName( );
	}

	//Get password from password field
	UITextField * passwordField = g_UISystem->GetWidgetByName<UITextField>( UI_PASSWORD_FIELD );
	std::string password = passwordField->GetText( );

	g_GameSystem->AttemptCreateClient( addressString, username, password );
}


//-------------------------------------------------------------------------------------------------
void Game::OnFighterClassButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_CHOOSE_CLASS_FIGHTER );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnWizardClassButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_CHOOSE_CLASS_WIZARD );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnRogueClassButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_CHOOSE_CLASS_ROGUE );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnCultistClassButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_CHOOSE_CLASS_CULTIST );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnUpgradeHullButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_UPGRADE_HULL );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnUpgradeHealthButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_UPGRADE_HEALTH );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnUpgradeShieldButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_UPGRADE_SHIELD );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnUpgradeEnergyButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_UPGRADE_ENERGY );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnFeedCrystalButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_FEED_CRYSTAL );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnEmoteHappyButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_EMOTE_HAPPY );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnEmoteSadButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_EMOTE_SAD );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnEmoteAngryButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_EMOTE_ANGRY );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnEmoteWhoopsButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_EMOTE_WHOOPS );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnEmoteHelpButton( NamedProperties & )
{
	NetMessage inputEvent( eNetGameMessageType_RELIABLE_INPUT );
	inputEvent.Write<byte_t>( eGameEvent_EMOTE_HELP );
	s_netSession->GetHost( )->AddMessage( inputEvent );
}


//-------------------------------------------------------------------------------------------------
void Game::OnEquipPrimary( NamedProperties & eventParams )
{
	Player * currentPlayer = ClientGetPlayerSelf( );
	if( currentPlayer )
	{
		UIItem * itemToEquip;
		eventParams.Get( "item", itemToEquip );
		byte_t itemIndex;
		if( currentPlayer->GetInventoryIndexForItem( itemToEquip, &itemIndex ) )
		{
			Item * checkItem = currentPlayer->m_inventory[itemIndex];

			//Only equip weapons
			if( checkItem->IsWeapon( ) )
			{
				ClientRequestEquipPrimary( itemIndex );
			}

			//Return non-weapons to inventory
			else
			{
				currentPlayer->m_inventoryContainer->AddItem( itemToEquip );
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Game::OnEjectItem( NamedProperties & eventParams )
{
	Player * currentPlayer = ClientGetPlayerSelf( );
	if( currentPlayer )
	{
		UIItem * itemToEject;
		eventParams.Get( "item", itemToEject );

		byte_t itemIndex;
		eEquipmentSlot equipmentSlot;

		//Find item in inventory
		if( currentPlayer->GetInventoryIndexForItem( itemToEject, &itemIndex ) )
		{
			ClientRequestRemoveItem( itemIndex );

			//Re-add it to the inventory container while we wait for it to leave
			currentPlayer->m_inventoryContainer->AddChild( itemToEject );

			//We don't need to see it pop onto the screen for a few frames
			itemToEject->SetHidden( true );
		}

		//Return item to equipment slot
		else if( currentPlayer->GetEquipmentSlotForItem( itemToEject, &equipmentSlot ) )
		{
			currentPlayer->m_equipmentContainer[equipmentSlot]->AddChild( itemToEject );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Game::OnMoveEquipmentToInventory( NamedProperties & eventParams )
{
	Player * currentPlayer = ClientGetPlayerSelf( );
	if( currentPlayer )
	{
		UIItem * itemToMove;
		eventParams.Get( "item", itemToMove );

		eEquipmentSlot equipmentSlot;

		if( currentPlayer->GetEquipmentSlotForItem( itemToMove, &equipmentSlot ) )
		{
			ClientRequestRemoveEquipment( equipmentSlot );
			itemToMove->SetHidden( true );
		}
	}
}