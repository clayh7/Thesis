#pragma once
#include "Game/General/GameCommon.hpp"


//-------------------------------------------------------------------------------------------------
class NetSender;
class NetMessage;


//-------------------------------------------------------------------------------------------------
void OnCreatePlayerReceived( NetSender const & sender, NetMessage const & message );
void OnDestroyPlayerReceived( NetSender const & sender, NetMessage const & message );
void OnUpdatePlayerReceived( NetSender const & sender, NetMessage const & message );
void OnCreateMessageReceived( NetSender const & sender, NetMessage const & message );
void OnDestroyMessageReceived( NetSender const & sender, NetMessage const & message );
void OnUpdateMessageReceived( NetSender const & sender, NetMessage const & message );
void OnGameStateMessageReceived( NetSender const & sender, NetMessage const & message );
void OnInputMessageReceived( NetSender const & sender, NetMessage const & message );