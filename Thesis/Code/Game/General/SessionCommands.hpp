#pragma once


//-------------------------------------------------------------------------------------------------
class Command;


//-------------------------------------------------------------------------------------------------
void RegisterConsoleCommands( );
void SessionStartCommand( Command const & );
void SessionStopCommand( Command const & );
void SessionHostCommand( Command const & );
void SessionJoinCommand( Command const & );
void SessionLeaveCommand( Command const & );
void SessionPingCommand( Command const & );
void SessionCreateConnectionCommand( Command const & );

void SessionDropRateCommand( Command const & );
void SessionLatencyCommand( Command const & );
void SessionToggleTimeoutsCommand( Command const & );
void SessionDebug( Command const & );