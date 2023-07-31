#pragma semicolon 1

#include <amxmodx>
#include <amxmisc>
#include <reapi>
#include <json>
#include "mm_incs/defines"
#include "mm_incs/global"
#include "mm_incs/natives"
#include "mm_incs/cvars"
#include "mm_incs/recent_mods_maps"
#include "mm_incs/admincmds"
#include "mm_incs/modchooser"
#include "mm_incs/mapchooser"
#include "mm_incs/rockthevote"
#include "mm_incs/nominations"
#include "mm_incs/utils"

public plugin_natives()
{
	register_native("mm_get_mods_count", "_mm_get_mods_count");
	register_native("mm_is_mod_enabled", "_mm_is_mod_enabled");
	register_native("mm_get_mod_name", "_mm_get_mod_name");
	register_native("mm_get_mod_tag", "_mm_get_mod_tag");
	register_native("mm_get_mod_changemap_type", "_mm_get_mod_changemap_type");
	register_native("mm_get_mod_maps", "_mm_get_mod_maps");
	register_native("mm_get_mod_cvars", "_mm_get_mod_cvars");
	register_native("mm_get_mod_plugins", "_mm_get_mod_plugins");
	register_native("mm_get_currentmod_id", "_mm_get_currentmod_id");
	register_native("mm_get_nextmod_id", "_mm_get_nextmod_id");
	register_native("mm_force_votemod", "_mm_force_votemod");
	register_native("mm_force_change_map", "_mm_force_change_map");
}

public plugin_precache()
{
	register_dictionary("common.txt");
	register_dictionary("multimod_manager.txt");

	get_mapname(g_szCurrentMap, charsmax(g_szCurrentMap));
	mb_strtolower(g_szCurrentMap);

	g_GlobalConfigs[Mods] = ArrayCreate(ArrayMods_e);
	g_GlobalConfigs[RecentMods] = ArrayCreate(ArrayRecentMods_e);
	g_GlobalConfigs[RecentMaps] = ArrayCreate(ArrayRecentMaps_e);
	g_Array_Nominations = ArrayCreate(1);

	g_Forward_VersionCheck = CreateMultiForward("__multimod_version_check", ET_IGNORE, FP_CELL, FP_CELL);
	g_Forward_StartVotemod = CreateMultiForward("multimod_start_votemod", ET_IGNORE, FP_CELL);
	g_Forward_EndVotemod = CreateMultiForward("multimod_end_votemod", ET_IGNORE, FP_CELL);
	g_Forward_StartVotemap = CreateMultiForward("multimod_start_votemap", ET_IGNORE, FP_CELL);
	g_Forward_EndVotemap = CreateMultiForward("multimod_end_votemap", ET_IGNORE, FP_CELL);
	g_Forward_AdminForceVotemod = CreateMultiForward("multimod_admin_force_votemod", ET_STOP, FP_CELL);

	Cvars_Init();
	MultiMod_Init();
	MultiMod_ExecCvars(g_iCurrentMod);
}

public plugin_init()
{
	register_plugin(PLUGIN_NAME, PLUGIN_VERSION, "FEDERICOMB");

	register_dictionary("common.txt");
	register_dictionary("multimod_manager.txt");

	register_event_ex("TextMsg", "OnEvent_GameRestart", RegisterEvent_Global, "2&#Game_C", "2&#Game_w");
	register_event_ex("TextMsg", "OnEvent_GameRestart", RegisterEvent_Global, "2&#Game_will_restart_in");
	register_event_ex("HLTV", "OnEvent_HLTV", RegisterEvent_Global, "1=0", "2=0");

	RegisterHookChain(RG_CSGameRules_GoToIntermission, "OnCSGameRules_GoToIntermission", 0);

	UTIL_RegisterClientCommandAll("recentmods", "OnClientCommand_RecentMods");
	UTIL_RegisterClientCommandAll("recentmaps", "OnClientCommand_RecentMaps");
	UTIL_RegisterClientCommandAll("currentmod", "OnClientCommand_CurrentMod");
	UTIL_RegisterClientCommandAll("currentmap", "OnClientCommand_CurrentMap");
	UTIL_RegisterClientCommandAll("nextmod", "OnClientCommand_NextMod");
	UTIL_RegisterClientCommandAll("nextmap", "OnClientCommand_NextMap");
	UTIL_RegisterClientCommandAll("timeleft", "OnClientCommand_Timeleft");

	g_Hud_Vote = CreateHudSyncObj();
	g_Hud_Alert = CreateHudSyncObj();

	AdminCmd_Init();
	ModChooser_Init();
	MapChooser_Init();
	RockTheVote_Init();
	Nominations_Init();

	MultiMod_SetGameDescription(g_iCurrentMod);

	g_bGameOver = false;
}

public OnConfigsExecuted()
{
	server_cmd("amx_pausecfg add ^"%s^"", PLUGIN_NAME);

	MultiMod_ExecCvars(g_iCurrentMod);
}

public plugin_end()
{
	if(g_RestoreTimelimit)
		set_pcvar_float(g_pCvar_mp_timelimit, g_RestoreTimelimit);
	
	if(g_RestoreChattime)
		set_pcvar_float(g_pCvar_mp_chattime, g_RestoreChattime);

	MultiMod_OverwriteMapCycle(g_iNextSelectMod);

	if(g_GlobalConfigs[Mods] != Invalid_Array)
	{
		new iSize = ArraySize(g_GlobalConfigs[Mods]);
		for(new i = 0, aData[ArrayMods_e]; i < iSize; ++i)
		{
			ArrayGetArray(g_GlobalConfigs[Mods], i, aData);

			if(aData[Maps] != Invalid_Array)
				ArrayDestroy(aData[Maps]);

			if(aData[Cvars] != Invalid_Array)
				ArrayDestroy(aData[Cvars]);

			if(aData[Plugins] != Invalid_Array)
				ArrayDestroy(aData[Plugins]);

			if(aData[ReSemiclip] != Invalid_Array)
				ArrayDestroy(aData[ReSemiclip]);
		}

		ArrayDestroy(g_GlobalConfigs[Mods]);
	}

	if(g_GlobalConfigs[RecentMods] != Invalid_Array)
		ArrayDestroy(g_GlobalConfigs[RecentMods]);

	if(g_GlobalConfigs[RecentMaps] != Invalid_Array)
		ArrayDestroy(g_GlobalConfigs[RecentMaps]);

	if(g_Array_Nominations != Invalid_Array)
		ArrayDestroy(g_Array_Nominations);
}

public client_putinserver(id)
{
	SetPlayerBit(g_bConnected, id);

	AdminCmd_ClientPutInServer(id);
	ModChooser_ClientPutInServer(id);
	MapChooser_ClientPutInServer(id);
	RockTheVote_ClientPutInServer(id);
	Nominations_ClientPutInServer(id);
}

public client_disconnected(id, bool:drop, message[], maxlen)
{
	ClearPlayerBit(g_bConnected, id);

	AdminCmd_ClientDisconnected(id);
	ModChooser_ClientDisconnected(id);
	MapChooser_ClientDisconnected(id);
	RockTheVote_ClientDisconnected(id);
	Nominations_ClientDisconnected(id);
}

MultiMod_Init()
{
	new szDefaultCurrentMap[MAX_MAPNAME_LENGTH];

	new szConfigDir[PLATFORM_MAX_PATH], szFileName[PLATFORM_MAX_PATH], szPluginsFile[PLATFORM_MAX_PATH];
	get_configsdir(szConfigDir, charsmax(szConfigDir));

	formatex(szPluginsFile, charsmax(szPluginsFile), "%s/%s", szConfigDir, MM_PLUGINS_FILENAME);
	UTIL_GetCurrentMod(szPluginsFile, g_szCurrentMod, MAX_MODNAME_LENGTH-1, szDefaultCurrentMap, MAX_MAPNAME_LENGTH-1);

	formatex(szFileName, charsmax(szFileName), "%s/%s/%s", szConfigDir, MM_CONFIG_FOLDER, MM_CONFIG_FILENAME);

	if(!file_exists(szFileName))
	{
		set_fail_state("[MULTIMOD] %L [%s]", LANG_SERVER, "MM_ERR_FIND_FILE", szFileName);
		return;
	}

	new JSON:jConfigsFile = json_parse(szFileName, true, true);

	if(jConfigsFile == Invalid_JSON)
	{
		set_fail_state("[MULTIMOD] %L [%s]", LANG_SERVER, "MM_ERR_PARSE_FILE", szFileName);
		return;
	}

	new bool:bReloadMod = true;

	if(json_is_object(jConfigsFile))
	{
		json_object_get_string(jConfigsFile, "global_chat_prefix", g_GlobalConfigs[ChatPrefix], charsmax(g_GlobalConfigs[ChatPrefix]));

		replace_string(g_GlobalConfigs[ChatPrefix], charsmax(g_GlobalConfigs[ChatPrefix]), "!y" , "^1");
		replace_string(g_GlobalConfigs[ChatPrefix], charsmax(g_GlobalConfigs[ChatPrefix]), "!t" , "^3");
		replace_string(g_GlobalConfigs[ChatPrefix], charsmax(g_GlobalConfigs[ChatPrefix]), "!g" , "^4");

		new szReadFlags[30];
		json_object_get_string(jConfigsFile, "adminflags.menu", szReadFlags, charsmax(szReadFlags), true);
		g_GlobalConfigs[AdminFlags_Menu] = read_flags(szReadFlags);

		json_object_get_string(jConfigsFile, "adminflags.managemods", szReadFlags, charsmax(szReadFlags), true);
		g_GlobalConfigs[AdminFlags_ManageMods] = read_flags(szReadFlags);

		json_object_get_string(jConfigsFile, "adminflags.selectmenu", szReadFlags, charsmax(szReadFlags), true);
		g_GlobalConfigs[AdminFlags_SelectMenu] = read_flags(szReadFlags);

		json_object_get_string(jConfigsFile, "adminflags.forcevotemod", szReadFlags, charsmax(szReadFlags), true);
		g_GlobalConfigs[AdminFlags_ForceVoteMod] = read_flags(szReadFlags);

		json_object_get_string(jConfigsFile, "adminflags.votemenu", szReadFlags, charsmax(szReadFlags), true);
		g_GlobalConfigs[AdminFlags_VoteMenu] = read_flags(szReadFlags);

		g_GlobalConfigs[RTV_Enabled] = json_object_get_bool(jConfigsFile, "rockthevote.enable", true);
		g_GlobalConfigs[RTV_Cooldown] = max(0, json_object_get_number(jConfigsFile, "rockthevote.cooldown", true));
		g_GlobalConfigs[RTV_MinPlayers] = clamp(json_object_get_number(jConfigsFile, "rockthevote.minplayers", true), 0, MAX_CLIENTS);
		g_GlobalConfigs[RTV_Percentage] = clamp(json_object_get_number(jConfigsFile, "rockthevote.percentage", true), 0, 100);
		g_GlobalConfigs[Nom_Mods_Enabled] = json_object_get_bool(jConfigsFile, "nomination.mods", true);
		g_GlobalConfigs[Nom_Maps_Enabled] = json_object_get_bool(jConfigsFile, "nomination.maps", true);
		g_GlobalConfigs[AdminMaxOptionsInMenu] = clamp(json_object_get_number(jConfigsFile, "admin_max_options_in_menu"), 2, MAX_ADMIN_VOTEOPTIONS);
		g_GlobalConfigs[ModsInMenu] = clamp(json_object_get_number(jConfigsFile, "mods_in_menu"), 2, (MAX_SELECTMODS - 1)); // La ultima opción se reserva para extender unicamente.
		g_GlobalConfigs[MapsInMenu] = clamp(json_object_get_number(jConfigsFile, "maps_in_menu"), 2, MAX_SELECTMAPS);
		g_GlobalConfigs[MaxRecentMods] = max(0, json_object_get_number(jConfigsFile, "max_recent_mods"));
		g_GlobalConfigs[MaxRecentMaps] = max(0, json_object_get_number(jConfigsFile, "max_recent_maps"));
		g_GlobalConfigs[OverwriteMapcycle] = json_object_get_bool(jConfigsFile, "overwrite_mapcycle");
		json_object_get_string(jConfigsFile, "resemiclip_path", g_GlobalConfigs[ReSemiclipPath], PLATFORM_MAX_PATH-1);
		g_GlobalConfigs[ChangeGameDescription] = json_object_get_bool(jConfigsFile, "change_game_description");

		new JSON:jArrayMods = json_object_get_value(jConfigsFile, "mods");
		new iCount = json_array_get_count(jArrayMods);

		for(new i = 0, j,
			szCvarName[128], szPluginName[64],
			szMapFile[PLATFORM_MAX_PATH],
			aMod[ArrayMods_e], aReSemiclip[ArrayReSemiclip_e],
			JSON:jArrayValue, 
			JSON:jObjectMod, iObjetCount; i < iCount; ++i)
		{
			jArrayValue = json_array_get_value(jArrayMods, i);

			aMod[Enabled] = true;

			json_object_get_string(jArrayValue, "modname", aMod[ModName], MAX_MODNAME_LENGTH-1);
			json_object_get_string(jArrayValue, "mod_tag", aMod[ModTag], MAX_MODNAME_LENGTH-1);

			aMod[ChangeMapType] = ChangeMap_e:json_object_get_number(jArrayValue, "change_map_type");

			aMod[Maps] = ArrayCreate(MAX_MAPNAME_LENGTH);
			json_object_get_string(jArrayValue, "mapsfile", szMapFile, charsmax(szMapFile));
			format(szMapFile, PLATFORM_MAX_PATH-1, "%s/%s/%s/%s", szConfigDir, MM_CONFIG_FOLDER, MM_MAPSFILE_FOLDER, szMapFile);
			MapChooser_LoadMaps(aMod[Maps], szMapFile);

			// El modo contiene mapas
			if(ArraySize(aMod[Maps]))
			{
				// Modo actual?
				if(equali(g_szCurrentMod, aMod[ModName]))
				{
					bReloadMod = false;
					g_iCurrentMod = i;
				}

				aMod[Cvars] = ArrayCreate(PLATFORM_MAX_PATH);
				jObjectMod = json_object_get_value(jArrayValue, "cvars");
				for(j = 0, iObjetCount = json_array_get_count(jObjectMod); j < iObjetCount; ++j)
				{
					json_array_get_string(jObjectMod, j, szCvarName, charsmax(szCvarName));
					ArrayPushString(aMod[Cvars], szCvarName);
				}
				json_free(jObjectMod);

				aMod[Plugins] = ArrayCreate(PLATFORM_MAX_PATH);
				jObjectMod = json_object_get_value(jArrayValue, "plugins");
				for(j = 0, iObjetCount = json_array_get_count(jObjectMod); j < iObjetCount; ++j)
				{
					json_array_get_string(jObjectMod, j, szPluginName, charsmax(szPluginName));
					ArrayPushString(aMod[Plugins], szPluginName);
				}
				json_free(jObjectMod);

				aMod[ReSemiclip] = ArrayCreate(ArrayReSemiclip_e);
				jObjectMod = json_object_get_value(jArrayValue, "resemiclip_config");
				for(j = 0, iObjetCount = json_object_get_count(jObjectMod); j < iObjetCount; ++j)
				{
					json_object_get_name(jObjectMod, j, aReSemiclip[KEY_NAME], MAX_RESEMICLIP_LENGTH-1);
					json_object_get_string(jArrayValue, fmt("resemiclip_config.%s", aReSemiclip[KEY_NAME]), aReSemiclip[KEY_VALUE], MAX_RESEMICLIP_LENGTH-1, true);

					ArrayPushArray(aMod[ReSemiclip], aReSemiclip);
				}
				json_free(jObjectMod);

				ArrayPushArray(g_GlobalConfigs[Mods], aMod);
			}
			else
			{
				ArrayDestroy(aMod[Maps]);
				log_amx("%L", LANG_SERVER, "MM_ERR_INVALID_MAP_LIST", aMod[ModName]);
			}

			json_free(jArrayValue);
		}

		json_free(jArrayMods);
	}

	json_free(jConfigsFile);
	
	if(ArraySize(g_GlobalConfigs[Mods]) < 1)
	{
		set_fail_state("[MULTIMOD] %L", LANG_SERVER, "MM_ERR_NO_LOADED_MODES");
		return;
	}

	if(bReloadMod || !IsValidMapForMod(g_iCurrentMod, g_szCurrentMap))
	{
		g_iNextSelectMod = bReloadMod ? 0 : g_iCurrentMod; // 0 = First mod as default
		MultiMod_SetNextMod(g_iNextSelectMod); 
		UTIL_GetCurrentMod(szPluginsFile, g_szCurrentMod, MAX_MODNAME_LENGTH-1, szDefaultCurrentMap, MAX_MAPNAME_LENGTH-1);

		if(!GetRandomMapForMod(g_iNextSelectMod, szDefaultCurrentMap, MAX_MAPNAME_LENGTH-1))
			copy(szDefaultCurrentMap, MAX_MAPNAME_LENGTH-1, g_szCurrentMap);

		set_task(2.0, "OnTask_ChangeMap", 0, szDefaultCurrentMap, MAX_MAPNAME_LENGTH-1);
		return;
	}

	MultiMod_ClearPluginsFile();
	MultiMod_OverwriteMapCycle(g_iCurrentMod);
	MultiMod_GetOffMods();
	Recent_LoadRecentModsMaps();
	Recent_SaveRecentModsMaps();
	OnEvent_GameRestart();

	new iRes;
	ExecuteForward(g_Forward_VersionCheck, iRes, MM_VERSION_MAJOR, MM_VERSION_MINOR);
}

public OnEvent_GameRestart()
{
	ModChooser_ResetAllData();
	MapChooser_ResetAllData();
	RockTheVote_ResetAllData();
	Nominations_ResetAllData();

	if(ArraySize(g_GlobalConfigs[Mods]))
	{
		remove_task(TASK_ENDMAP);
		set_task(15.0, "OnTask_CheckVoteNextMod", TASK_ENDMAP, .flags = "b");
	}
}

public OnEvent_HLTV()
{
	if((g_iNoMoreTime == 1 && !g_bChangeMapOneMoreRound) || (g_bVoteRtvResult && g_iNoMoreTime == 1))
	{
		OnCSGameRules_GoToIntermission();
		return;
	}

	if(g_bChangeMapOneMoreRound)
	{
		g_bChangeMapOneMoreRound = false;

		client_cmd(0, "spk ^"%s^"", g_SOUND_ExtendTime);
		client_print_color(0, print_team_default, "%s^1 %L", LANG_PLAYER, "MM_MAP_CHANGE_END_ROUND", g_GlobalConfigs[ChatPrefix]);
	}
}

public OnCSGameRules_GoToIntermission()
{
	if(g_bGameOver)
		return HC_BREAK;

	g_RestoreChattime = floatclamp(g_bCvar_mp_chattime, 0.1, 120.0);
	set_pcvar_float(g_pCvar_mp_chattime, g_RestoreChattime + 2.0);
	
	set_task(g_RestoreChattime, "OnTask_ChangeMap", 1, g_bCvar_amx_nextmap, charsmax(g_bCvar_amx_nextmap));

	g_iNoMoreTime = 2;

	message_begin(MSG_ALL, SVC_INTERMISSION);
	message_end();
	
	client_print_color(0, print_team_blue, "%s^1 %L:^3 %s", LANG_PLAYER, "MM_NEXT_MAP", g_GlobalConfigs[ChatPrefix], g_bCvar_amx_nextmap);

	g_bGameOver = true;
	set_member_game(m_iEndIntermissionButtonHit, 0);
	set_member_game(m_iSpawnPointCount_Terrorist, 0);
	set_member_game(m_iSpawnPointCount_CT, 0);
	set_member_game(m_bLevelInitialized, false);

	return HC_BREAK;
}

public OnClientCommand_RecentMods(const id)
{
	CHECK_CONNECTED(id)

	if(Recent_CountRecentMods() < 1)
	{
		client_print_color(id, id, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NO_RECENT_MODES_PLAYED");
		return PLUGIN_HANDLED;
	}

	ShowMenu_RecentMods(id);
	return PLUGIN_HANDLED;
}

ShowMenu_RecentMods(const id, menupage=0)
{
	CHECK_CONNECTED(id)

	new iMenu = menu_create(fmt("\y%L:\R", LANG_PLAYER, "MM_RECENT_MODES"), "menu_RecentMods");

	new iArraySizeMods = ArraySize(g_GlobalConfigs[RecentMods]);

	for(new iModId = 0, aRecents[ArrayRecentMods_e], szTimeAgo[11]; iModId < iArraySizeMods; ++iModId)
	{
		ArrayGetArray(g_GlobalConfigs[RecentMods], iModId, aRecents);

		UTIL_GetTimeElapsed((get_systime() - aRecents[RECENT_MOD_SYSTIME]), szTimeAgo, charsmax(szTimeAgo));

		menu_additem(iMenu, fmt("%s\y (%L)", aRecents[RECENT_MOD_NAME], LANG_PLAYER, "MM_AGO", szTimeAgo));
	}
	
	menu_setprop(iMenu, MPROP_NEXTNAME, fmt("%L", LANG_PLAYER, "MM_MORE"));
	menu_setprop(iMenu, MPROP_BACKNAME, fmt("%L", LANG_PLAYER, "MM_BACK"));
	menu_setprop(iMenu, MPROP_EXITNAME, fmt("%L", LANG_PLAYER, "MM_EXIT"));

	menu_display(id, iMenu, min(menupage, menu_pages(iMenu) - 1));
	return PLUGIN_HANDLED;
}

public menu_RecentMods(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)
	CHECK_EXIT_NEWMENU(id, menuid, item)
	
	new iNothing, iMenuPage;
	player_menu_info(id, iNothing, iNothing, iMenuPage);

	menu_destroy(menuid);
	ShowMenu_RecentMods(id, iMenuPage);
	return PLUGIN_HANDLED;
}

public OnClientCommand_RecentMaps(const id)
{
	CHECK_CONNECTED(id)

	if(Recent_CountRecentMaps(g_iCurrentMod) < 1)
	{
		client_print_color(id, id, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NO_RECENT_MAPS_PLAYED");
		return PLUGIN_HANDLED;
	}

	ShowMenu_RecentMaps(id);
	return PLUGIN_HANDLED;
}

ShowMenu_RecentMaps(const id, menupage=0)
{
	CHECK_CONNECTED(id)

	new iMenu = menu_create(fmt("\y%L\d [%s]\y:\R", LANG_PLAYER, "MM_RECENT_MAPS", g_szCurrentMod), "menu_RecentMaps");

	new iArraySizeMaps = ArraySize(g_GlobalConfigs[RecentMaps]);

	for(new iMapId = 0, aRecents[ArrayRecentMaps_e], szTimeAgo[11]; iMapId < iArraySizeMaps; ++iMapId)
	{
		ArrayGetArray(g_GlobalConfigs[RecentMaps], iMapId, aRecents);

		if(unlikely(g_iCurrentMod == UTIL_GetModId(aRecents[RECENT_MOD_NAME])))
			continue;

		UTIL_GetTimeElapsed((get_systime() - aRecents[RECENT_MAP_SYSTIME]), szTimeAgo, charsmax(szTimeAgo));

		menu_additem(iMenu, fmt("%s\y (%L)", aRecents[RECENT_MAP_NAME], LANG_PLAYER, "MM_AGO", szTimeAgo));
	}
	
	menu_setprop(iMenu, MPROP_NEXTNAME, fmt("%L", LANG_PLAYER, "MM_MORE"));
	menu_setprop(iMenu, MPROP_BACKNAME, fmt("%L", LANG_PLAYER, "MM_BACK"));
	menu_setprop(iMenu, MPROP_EXITNAME, fmt("%L", LANG_PLAYER, "MM_EXIT"));

	menu_display(id, iMenu, min(menupage, menu_pages(iMenu) - 1));
	return PLUGIN_HANDLED;
}

public menu_RecentMaps(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)
	CHECK_EXIT_NEWMENU(id, menuid, item)
	
	new iNothing, iMenuPage;
	player_menu_info(id, iNothing, iNothing, iMenuPage);

	menu_destroy(menuid);
	ShowMenu_RecentMaps(id, iMenuPage);
	return PLUGIN_HANDLED;
}

public OnClientCommand_CurrentMod(const id)
{
	CHECK_CONNECTED(id)

	client_print_color(id, print_team_blue, "%s^1 %L:^3 %s", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_CURRENT_MODE", g_szCurrentMod);
	return PLUGIN_HANDLED;
}

public OnClientCommand_CurrentMap(const id)
{
	CHECK_CONNECTED(id)

	client_print_color(id, print_team_blue, "%s^1 %L:^3 %s", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_CURRENT_MAP", g_szCurrentMap);
	return PLUGIN_HANDLED;
}

public OnClientCommand_NextMod(const id)
{
	CHECK_CONNECTED(id)

	if(g_bSelectedNextMod)
	{
		new aMod[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMod);

		client_print_color(id, print_team_blue, "%s^1 %L:^3 %s", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NEXT_MODE", aMod[ModName]);
		return PLUGIN_HANDLED;
	}

	client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NEXT_MODE_HASNT_BEEN_CHOOSED");
	return PLUGIN_HANDLED;
}

public OnClientCommand_NextMap(const id)
{
	CHECK_CONNECTED(id)

	if(g_bSelectedNextMap)
	{
		client_print_color(id, print_team_blue, "%s^1 %L:^3 %s", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NEXT_MAP", g_bCvar_amx_nextmap);
		return PLUGIN_HANDLED;
	}

	client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NEXT_MAP_HASNT_BEEN_CHOOSED");
	return PLUGIN_HANDLED;
}

public OnClientCommand_Timeleft(const id)
{
	CHECK_CONNECTED(id)

	switch(g_iNoMoreTime)
	{
		case 1: client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MAP_CHANGE_END_ROUND");
		case 2: client_print_color(id, print_team_blue, "%s^1 %L:^3 %s", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NEXT_MAP", g_bCvar_amx_nextmap);
		default:
		{
			if((g_bCvar_mp_maxrounds != 0) || (g_bCvar_mp_winlimit != 0))
			{
				if(g_bCvar_mp_maxrounds != 0)
				{
					client_print_color(id, print_team_blue, "%s^1 %L:^3 %d^1 - %L:^3 %d", g_GlobalConfigs[ChatPrefix], 
						LANG_PLAYER, "MM_MAX_ROUNDS", g_bCvar_mp_maxrounds, LANG_PLAYER, "MM_ROUNDS_LEFT", (g_bCvar_mp_maxrounds - (get_member_game(m_iTotalRoundsPlayed))));
				}

				if(g_bCvar_mp_winlimit != 0)
				{
					client_print_color(id, print_team_blue, "%s^1 %L!", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_WIN_LIMIT", g_bCvar_mp_winlimit);
					client_print_color(id, print_team_default, "%s^1 T:^4 %d^1 | CT:^4 %d", g_GlobalConfigs[ChatPrefix], get_member_game(m_iNumTerroristWins), get_member_game(m_iNumCTWins));
				}
			}
			else if(g_bCvar_mp_timelimit <= 0.0)
				client_print_color(id, print_team_blue, "%s^1 %L:^3 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_TIMELEFT", LANG_PLAYER, "MM_NO_T_LIMIT");
			else
			{
				new z = get_timeleft();
				client_print_color(id, print_team_blue, "%s^1 %L:^3 %d:%02d", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_TIMELEFT", (z / 60), (z % 60));
			}
		}
	}

	return PLUGIN_HANDLED;
}

public OnTask_CheckVoteNextMod()
{
	if(g_bSelectedNextMod || g_bSelectedNextMap)
	{
		remove_task(TASK_ENDMAP);
		return;
	}

	if(CanStartVoteNextMod())
		StartVoteNextMod();
}

bool:CanStartVoteNextMod()
{
	if(g_bVoteModHasStarted || g_bSVM_ModSecondRound || g_bVoteMapHasStarted || g_bSVM_MapSecondRound || g_bIsVotingRtv || g_bVoteInProgress)
		return false;

	if((g_bCvar_mp_maxrounds != 0) && (get_member_game(m_iTotalRoundsPlayed) >= (g_bCvar_mp_maxrounds - 3)))
		return true;

	if((g_bCvar_mp_winlimit != 0) && ((get_member_game(m_iNumCTWins) >= (g_bCvar_mp_winlimit - 3)) || (get_member_game(m_iNumTerroristWins) >= (g_bCvar_mp_winlimit - 3))))
		return true;

	if((g_bCvar_mp_timelimit != 0.0) && (1 <= get_timeleft() <= 180))
		return true;

	return false;
}

bool:CanForceVoteNextMod()
{
	if(g_bVoteModHasStarted || g_bSVM_ModSecondRound || g_bVoteMapHasStarted || g_bSVM_MapSecondRound || g_bIsVotingRtv || g_bVoteInProgress || g_bSelectedNextMod || g_bSelectedNextMap)
		return false;
	
	return true;
}

StartVoteNextMod()
{
	g_bVoteInProgress = true;

	SetAlertStartNextVote(0.0, 10);

	remove_task(TASK_VOTEMOD);
	set_task(10.1, "OnTask_VoteNextMod", TASK_VOTEMOD);
}

SetAlertStartNextVote(const Float:flStart, const iCountdown)
{
	g_iCountdownTime = iCountdown;

	if(flStart)
	{
		remove_task(TASK_SHOWTIME);
		set_task(flStart, "OnTask_AlertStartNextVote", TASK_SHOWTIME);
	}
	else
		OnTask_AlertStartNextVote();
}

public OnTask_AlertStartNextVote()
{
	if(!g_iCountdownTime)
	{
		ClearSyncHud(0, g_Hud_Alert);
		return;
	}

	if(g_iCountdownTime == 10)
		client_cmd(0, "spk ^"get red(e80) ninety(s45) to check(e20) use bay(s18) mass(e42) cap(s50)^"");

	set_hudmessage(255, 255, 255, -1.0, 0.35, 0, 0.0, 1.1, 0.0, 0.0, -1);
	ShowSyncHudMsg(0, g_Hud_Alert, "%L", LANG_PLAYER, "MM_NEXT_VOTE_WILL_START_IN", g_iCountdownTime);

	if(g_iCountdownTime <= 5)
		client_cmd(0, "spk ^"fvox/%s^"", g_SOUND_CountDown[g_iCountdownTime]);
	
	--g_iCountdownTime;
	
	remove_task(TASK_SHOWTIME);
	set_task(1.0, "OnTask_AlertStartNextVote", TASK_SHOWTIME);
}

MultiMod_SetNextMod(const iNextMod)
{
	new aDataNextMod[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iNextMod, aDataNextMod);

	new szFileName[PLATFORM_MAX_PATH];
	new iLen = get_configsdir(szFileName, charsmax(szFileName));
	formatex(szFileName[iLen], charsmax(szFileName) - iLen, "/%s", MM_PLUGINS_FILENAME);

	new pPluginsFile = fopen(szFileName, "w+");

	if(pPluginsFile)
	{
		fprintf(pPluginsFile, ";Mod:%s^n;Map:%a^n^n", aDataNextMod[ModName], ArrayGetStringHandle(aDataNextMod[Maps], 0));

		for(new i = 0, iPlugins = ArraySize(aDataNextMod[Plugins]); i < iPlugins; ++i)
			fprintf(pPluginsFile, "%a^n", ArrayGetStringHandle(aDataNextMod[Plugins], i));

		fclose(pPluginsFile);
	}

	if(strlen(g_GlobalConfigs[ReSemiclipPath]) && dir_exists(g_GlobalConfigs[ReSemiclipPath]))
	{
		new pConfigSemiclip = fopen(fmt("%s/config.ini", g_GlobalConfigs[ReSemiclipPath]), "w+");

		if(pConfigSemiclip)
		{
			for(new i = 0, aReSemiclip[ArrayReSemiclip_e], 
				iConfigCount = ArraySize(aDataNextMod[ReSemiclip]); i < iConfigCount; ++i)
			{
				ArrayGetArray(aDataNextMod[ReSemiclip], i, aReSemiclip);
				fprintf(pConfigSemiclip, "%s = %s;^n", aReSemiclip[KEY_NAME], aReSemiclip[KEY_VALUE]);
			}

			fclose(pConfigSemiclip);
		}
	}
}

MultiMod_ClearPluginsFile()
{
	new szFileName[PLATFORM_MAX_PATH];
	new iLen = get_configsdir(szFileName, charsmax(szFileName));
	formatex(szFileName[iLen], charsmax(szFileName) - iLen, "/%s", MM_PLUGINS_FILENAME);

	new pPluginsFile = fopen(szFileName, "w+");

	if(pPluginsFile)
		fclose(pPluginsFile);
}

MultiMod_OverwriteMapCycle(const iMod)
{
	if(likely(g_GlobalConfigs[OverwriteMapcycle] == true))
	{
		new aDataNextMod[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], iMod, aDataNextMod);

		new pMapCycle = fopen("mapcycle.txt", "w+");

		if(pMapCycle)
		{
			for(new i = 0, iMaps = ArraySize(aDataNextMod[Maps]); i < iMaps; ++i)
				fprintf(pMapCycle, "%a^n", ArrayGetStringHandle(aDataNextMod[Maps], i));

			fclose(pMapCycle);
		}
	}
}

MultiMod_GetOffMods()
{
	new szFileName[PLATFORM_MAX_PATH];
	new iLen = get_configsdir(szFileName, charsmax(szFileName));
	formatex(szFileName[iLen], charsmax(szFileName) - iLen, "/%s/%s", MM_CONFIG_FOLDER, MM_OFF_MODS_FILENAME);

	if(!file_exists(szFileName))
		return;

	new JSON:jConfigsFile = json_parse(szFileName, true);

	if(jConfigsFile == Invalid_JSON)
	{
		abort(AMX_ERR_GENERAL, "[MULTIMOD] %L [%s]", LANG_SERVER, "MM_INVALID_JSON_FILE", szFileName);
		return;
	}

	if(json_is_object(jConfigsFile) && json_object_has_value(jConfigsFile, "off_mods", JSONArray))
	{
		new JSON:jArrayMods = json_object_get_value(jConfigsFile, "off_mods");

		for(new i = 0, j,
			bool:bFoundIt,
			aMods[ArrayMods_e],
			szModName[MAX_MODNAME_LENGTH],
			iCount = json_array_get_count(jArrayMods),
			iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]); i < iCount; ++i)
		{
			json_array_get_string(jArrayMods, i, szModName, MAX_MODNAME_LENGTH-1);

			j = 0;
			bFoundIt = false;
			while(j < iArraySizeMods && likely(bFoundIt == false))
			{
				ArrayGetArray(g_GlobalConfigs[Mods], j, aMods);

				if(equali(aMods[ModName], szModName))
				{
					aMods[Enabled] = false;
					ArraySetArray(g_GlobalConfigs[Mods], j, aMods);
					
					bFoundIt = true;
				}

				++j;
			}
		}

		json_free(jArrayMods);
	}

	json_free(jConfigsFile);
}

bool:MultiMod_SaveOffMods()
{
	new JSON:root_value = json_init_object();
	new JSON:array = json_init_array();

	for(new i = 0, aMods[ArrayMods_e], iArraySize = ArraySize(g_GlobalConfigs[Mods]); i < iArraySize; ++i)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], i, aMods);

		if(likely(aMods[Enabled] == false))
			json_array_append_string(array, aMods[ModName]);
	}

	json_object_set_value(root_value, "off_mods", array);
	json_free(array);

	new szFileName[PLATFORM_MAX_PATH];
	new iLen = get_configsdir(szFileName, charsmax(szFileName));
	formatex(szFileName[iLen], charsmax(szFileName) - iLen, "/%s/%s", MM_CONFIG_FOLDER, MM_OFF_MODS_FILENAME);

	new bool:bRet = json_serial_to_file(root_value, szFileName, true);
	json_free(root_value);

	return bRet;
}

MultiMod_ExecCvars(const iMod)
{
	if(iMod < 0 || iMod > ArraySize(g_GlobalConfigs[Mods]))
		return 0;

	new aMod[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iMod, aMod);

	new iCvars = ArraySize(aMod[Cvars]);
	for(new i = 0; i < iCvars; ++i)
	{
		server_cmd("%a", ArrayGetStringHandle(aMod[Cvars], i));
	}

	server_print("[MULTIMOD] %L (%L: %d)", LANG_SERVER, "MM_EXECUTING_CVARS_MODE", aMod[ModName], LANG_SERVER, "MM_COUNT", iCvars);
	return iCvars;
}

MultiMod_SetGameDescription(const iMod)
{
	if(iMod < 0 || iMod > ArraySize(g_GlobalConfigs[Mods]))
		return 0;
		
	if(likely(g_GlobalConfigs[ChangeGameDescription] == true))
	{
		new aMod[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], iMod, aMod);

		set_member_game(m_GameDesc, aMod[ModName]);
		return 1;
	}

	return 0;
}

bool:IsValidMapForMod(const iModId, const szMapName[MAX_MAPNAME_LENGTH])
{
	if(iModId < 0 || iModId > ArraySize(g_GlobalConfigs[Mods]))
		return false;

	new aMod[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMod);

	new iMaps = ArraySize(aMod[Maps]);
	for(new i = 0, szMap[MAX_MAPNAME_LENGTH]; i < iMaps; ++i)
	{
		ArrayGetString(aMod[Maps], i, szMap, charsmax(szMap));

		if(equali(szMapName, szMap))
			return true;
	}

	return false;
}

GetRandomMapForMod(const iModId, szMap[], const iLen)
{
	if(iModId < 0 || iModId > ArraySize(g_GlobalConfigs[Mods]))
		return 0;

	new aMod[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMod);

	new iMaps = ArraySize(aMod[Maps]);
	if(iMaps)
	{
		new szBuff[MAX_MAPNAME_LENGTH];
		new iMapId = random_num(0, iMaps - 1);
		new iMapPass = 0;
		new iFinal = -1;

		while(iMapPass <= iMaps)
		{
			if(iMapPass == iMaps)
				break;

			if(++iMapId >= iMaps)
				iMapId = 0;

			++iMapPass;

			iFinal = iMapId;
			ArrayGetString(aMod[Maps], iMapId, szBuff, charsmax(szBuff));
			
			if(!IsValidMap(szBuff))
				iFinal = -1;
			
			if(iFinal == -1)
				continue;
			
			if(iFinal != -1)
				break;
		}
		
		if(iFinal != -1)
			return copy(szMap, iLen, szBuff);
	}
	
	return 0;
}

public OnTask_ForceChangeMap()
{
	OnCSGameRules_GoToIntermission();
}