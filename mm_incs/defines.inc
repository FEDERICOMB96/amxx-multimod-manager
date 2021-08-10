#if defined _mm_defines_included_
	#endinput
#endif
#define _mm_defines_included_

#define MAX_USERS						MAX_CLIENTS+1

#define IsPlayer(%0) 					( 1 <= %0 <= MAX_CLIENTS )

#define GetPlayerBit(%0,%1) 			( IsPlayer(%1) && ( %0 & ( 1 << ( %1 & 31 ) ) ) )
#define SetPlayerBit(%0,%1) 			( IsPlayer(%1) && ( %0 |= ( 1 << ( %1 & 31 ) ) ) )
#define ClearPlayerBit(%0,%1) 			( IsPlayer(%1) && ( %0 &= ~( 1 << ( %1 & 31 ) ) ) )
#define SwitchPlayerBit(%0,%1) 			( IsPlayer(%1) && ( %0 ^= ( 1 << ( %1 & 31 ) ) ) )

#define MAX_ADMIN_VOTEOPTIONS			9
#define MAX_SELECTMODS					9
#define MAX_SELECTMAPS					9

#define OPTION_EXTEND_MOD				-1

#define CHECK_ADMINVOTEINPROGRESS(%0)	if(g_bCvar_amx_last_voting > get_gametime()) { client_print_color(%0, print_team_default, "%s^1 Existe una votación en curso!", g_GlobalConfigs[ChatPrefix]); return PLUGIN_HANDLED; }

enum 
{
	TASK_VOTEMAP = MAX_USERS,
	TASK_VOTEMOD,
	TASK_TIMELEFT,
	TASK_RTV,
	TASK_SHOWTIME,
	TASK_ENDMAP,
	TASK_VOTE_ADMIN
};

const KEYSMENU = (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7) | (1<<8) | (1<<9);

new const g_SOUND_ExtendTime[] = "fvox/time_remaining.wav";
new const g_SOUND_GmanChoose[][] = {"Gman/Gman_Choose1.wav", "Gman/Gman_Choose2.wav"};
new const g_SOUND_CountDown[][] = {"", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"};

enum ChangeMap_e
{
	CHANGEMAP_TIMELEFT,
	CHANGEMAP_END_OF_ROUND,
	CHANGEMAP_ONE_MORE_ROUND
};

enum _:ArrayMods_e 
{
	ModName[64],
	MapsFile[64],
	ChangeMap_e:ChangeMapType,
	Array:Cvars,
	Array:Plugins
};

enum GlobalConfigs_e
{
	// Chat prefix
	ChatPrefix[21],

	// Admin flags
	AdminFlags,

	// Rock The Vote
	bool:RTV_Enabled,
	RTV_Cooldown,
	RTV_MinPlayers,
	RTV_Percentage,

	// Mods/Maps in menu
	ModsInMenu,
	MapsInMenu,

	// Mods
	Array:Mods,
};