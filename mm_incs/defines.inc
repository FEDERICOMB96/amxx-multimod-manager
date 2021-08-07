#if defined _mm_defines_included_
	#endinput
#endif
#define _mm_defines_included_

#include <reapi>

#define cvar_t 							new

#define MAX_USERS						MAX_CLIENTS+1

#define IsPlayer(%0) 					( 1 <= %0 <= MAX_CLIENTS )

#define GetPlayerBit(%0,%1) 			( IsPlayer(%1) && ( %0 & ( 1 << ( %1 & 31 ) ) ) )
#define SetPlayerBit(%0,%1) 			( IsPlayer(%1) && ( %0 |= ( 1 << ( %1 & 31 ) ) ) )
#define ClearPlayerBit(%0,%1) 			( IsPlayer(%1) && ( %0 &= ~( 1 << ( %1 & 31 ) ) ) )
#define SwitchPlayerBit(%0,%1) 			( IsPlayer(%1) && ( %0 ^= ( 1 << ( %1 & 31 ) ) ) )

enum 
{
	TASK_VOTEMAP = MAX_USERS,
	TASK_VOTEMOD,
	TASK_TIMELEFT,
	TASK_RTV,
	TASK_SHOWTIME,
	TASK_ENDMAP
};

const KEYSMENU = (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7) | (1<<8) | (1<<9);

new const g_SOUND_ExtendTime[] = "fvox/time_remaining.wav";
new const g_SOUND_GmanChoose[][] = {"Gman/Gman_Choose1.wav", "Gman/Gman_Choose2.wav"};
new const g_SOUND_CountDown[][] = {"", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"};

enum _:ArrayMods_e 
{
	ModName[64],
	MapsFile[64],
	ChangeMapType,
	Array:Cvars,
	Array:Plugins
};

enum ChangeMap_e
{
	CHANGEMAP_TIMELEFT,
	CHANGEMAP_END_OF_ROUND,
	CHANGEMAP_ONE_MORE_ROUND
};