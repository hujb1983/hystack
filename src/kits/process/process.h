/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once

#include <string>

#define PROCESSES_MAX			 1024

#define PROCESS_NORESPAWN		-1
#define PROCESS_JUST_SPAWN		-2
#define PROCESS_RESPAWN			-3
#define PROCESS_JUST_RESPAWN	-4
#define PROCESS_DETACHED		-5

#define process_helper(n)		#n
#define process_value(n)		 process_helper(n)

#define unix_signal_helper(n)	 SIG##n
#define unix_signal_value(n)	 unix_signal_helper(n)

#define SIGNAL_SHUTDOWN			 QUIT
#define SIGNAL_TERMINATE		 TERM
#define SIGNAL_NOACCEPT			 WINCH
#define SIGNAL_RECONFIGURE		 HUP

#define SIGNAL_REOPEN			 USR1
#define SIGNAL_CHANGEBIN		 USR2

#define PROCESS_SINGLE			 0
#define PROCESS_MASTER			 1
#define PROCESS_SIGNALLER		 2
#define PROCESS_WORKER			 3
#define PROCESS_HELPER			 4

