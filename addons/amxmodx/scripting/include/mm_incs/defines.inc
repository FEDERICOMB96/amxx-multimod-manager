#if defined _mm_defines_included_
	#endinput
#endif
#define _mm_defines_included_

#define PLUGIN_NAME                     "MultiMod Manager CS"
#define PLUGIN_URL                      "https://github.com/FEDERICOMB96/amxx-multimod-manager"

#define MM_PLUGINS_FILENAME             "plugins-MultimodManagerPluginsMods.ini"

#define MM_CONFIG_FOLDER                "multimod_manager"
#define MM_MAPSFILE_FOLDER              "mapsfiles"

#define MM_CONFIG_FILENAME              "configs.json"
#define MM_RECENT_MODS_FILENAME         "recent_mods.json"
#define MM_RECENT_MAPS_FILENAME         "recent_maps.json"
#define MM_OFF_MODS_FILENAME            "off_mods.json"

#define MM_LOG_FILENAME                 "error_multimod_manager.log"

#define MAX_USERS                       MAX_CLIENTS+1

#define IsPlayer(%0)                    ( 1 <= %0 <= MAX_CLIENTS )

#define GetPlayerBit(%0,%1)             ( IsPlayer(%1) && ( %0 & ( 1 << ( %1 & 31 ) ) ) )
#define SetPlayerBit(%0,%1)             ( IsPlayer(%1) && ( %0 |= ( 1 << ( %1 & 31 ) ) ) )
#define ClearPlayerBit(%0,%1)           ( IsPlayer(%1) && ( %0 &= ~( 1 << ( %1 & 31 ) ) ) )
#define SwitchPlayerBit(%0,%1)          ( IsPlayer(%1) && ( %0 ^= ( 1 << ( %1 & 31 ) ) ) )

#define MAX_MODNAME_LENGTH              64
#define MAX_MAPNAME_LENGTH              64
#define MAX_RESEMICLIP_LENGTH           64

#define MAX_ADMIN_VOTEOPTIONS           9
#define MAX_SELECTMODS                  9
#define MAX_SELECTMAPS                  9

#define OPTION_EXTEND_MOD               -1

#define CHECK_CONNECTED(%0)             if(!GetPlayerBit(g_bConnected, %0)) { return PLUGIN_HANDLED; }
#define CHECK_ADMINVOTEINPROGRESS(%0)   if(g_bCvar_amx_last_voting > get_gametime()) { client_print_color(%0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ALREADY_VOTING"); return PLUGIN_HANDLED; }

#define CHECK_CONNECTED_NEWMENU(%0,%1)  if(!GetPlayerBit(g_bConnected, %0)) { menu_destroy(%1); return PLUGIN_HANDLED; }
#define CHECK_EXIT_NEWMENU(%0,%1,%2)    if(%2 == MENU_EXIT) { menu_destroy(%1); return PLUGIN_HANDLED; }
#define CHECK_TIMEOUT_NEWMENU(%0,%1,%2) if(%2 == MENU_TIMEOUT) { menu_destroy(%1); return PLUGIN_HANDLED; }

#define CHECK_MENU_EXIT(%0)             if(%0 == MENU_EXIT) { return PLUGIN_HANDLED; }
#define CHECK_MENU_TIMEOUT(%0)          if(%0 == MENU_TIMEOUT) { return PLUGIN_HANDLED; }

#define CHECK_ADMIN_ACCESS(%0,%1)       if(~get_user_flags(%0) & %1) { client_print_color(%0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_NO_ACC"); return PLUGIN_HANDLED; }

enum (+= 12345)
{
	TASK_VOTEMAP = MAX_USERS,
	TASK_VOTEMOD,
	TASK_TIMELEFT,
	TASK_RTV,
	TASK_SHOWTIME,
	TASK_ENDMAP,
	TASK_VOTE_ADMIN,
	TASK_FORCE_CHANGE_MAP,
};

const KEYSMENU = (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7) | (1<<8) | (1<<9);

new const g_SOUND_ExtendTime[]          = "fvox/time_remaining.wav";
new const g_SOUND_GmanChoose[][]        = {"Gman/Gman_Choose1.wav", "Gman/Gman_Choose2.wav"};
new const g_SOUND_CountDown[][]         = {"", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"};

enum ChangeMap_e
{
	CHANGEMAP_TIMELEFT,
	CHANGEMAP_END_OF_ROUND,
	CHANGEMAP_ONE_MORE_ROUND,
};

enum _:ArrayMods_e 
{
	bool:Enabled,
	ModName[MAX_MODNAME_LENGTH],
	ModTag[MAX_MODNAME_LENGTH],
	ChangeMap_e:ChangeMapType,
	Array:Maps,
	Array:Cvars,
	Array:Plugins,
	Array:ReSemiclip,
};

enum _:ArrayRecentMods_e
{
	RECENT_MOD_NAME[MAX_MODNAME_LENGTH],
	RECENT_MOD_SYSTIME,
};

enum _:ArrayRecentMaps_e
{
	RECENT_MOD_NAME[MAX_MODNAME_LENGTH],
	RECENT_MAP_NAME[MAX_MAPNAME_LENGTH],
	RECENT_MAP_COUNT,
	RECENT_MAP_SYSTIME,
};

enum _:ArrayReSemiclip_e
{
	KEY_NAME[MAX_RESEMICLIP_LENGTH],
	KEY_VALUE[MAX_RESEMICLIP_LENGTH],
};

enum GlobalConfigs_e
{
	// Chat prefix
	ChatPrefix[21],

	// Admin flags
	AdminFlags_Menu,
	AdminFlags_ManageMods,
	AdminFlags_SelectMenu,
	AdminFlags_ForceVoteMod,
	AdminFlags_VoteMenu,

	// Rock The Vote
	bool:RTV_Enabled,
	RTV_Cooldown,
	RTV_MinPlayers,
	RTV_Percentage,
	bool:RTV_IgnoreChangeMapType,

	// Nomination
	bool:Nom_Mods_Enabled,
	bool:Nom_Maps_Enabled,

	// Admin options in menu
	AdminMaxOptionsInMenu,

	// Mods/Maps in menu
	ModsInMenu,
	MapsInMenu,

	// Max recent Mods/Maps
	MaxRecentMods,
	MaxRecentMaps,

	// Overwrite Mapcycle
	bool:OverwriteMapcycle,

	// resemiclip_path
	ReSemiclipPath[PLATFORM_MAX_PATH],

	// Change Game Description
	bool:ChangeGameDescription,

	// Mods
	Array:Mods,

	// Recent Mods
	Array:RecentMods,

	// Recent Maps
	Array:RecentMaps,
};