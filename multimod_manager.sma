#pragma semicolon 1

new const PLUGIN_NAME[] = "MultiMod Manager";
new const PLUGIN_VERSION[] = "v2021.08.26";

new const PLUGINS_FILENAME[] = "plugins-multimodmanager.ini";

#include <amxmodx>
#include <amxmisc>
#include <reapi>
#include <fakemeta>
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
	register_native("mm_get_mod_id", "_mm_get_mod_id");
	register_native("mm_get_mod_name", "_mm_get_mod_name");
	register_native("mm_get_mod_tag", "_mm_get_mod_tag");
	register_native("mm_get_nextmod_id", "_mm_get_nextmod_id");
	register_native("mm_get_nextmod_name", "_mm_get_nextmod_name");
}

public plugin_precache()
{
	get_mapname(g_szCurrentMap, charsmax(g_szCurrentMap));
	mb_strtolower(g_szCurrentMap);

	g_GlobalConfigs[Mods] = ArrayCreate(ArrayMods_e);
	g_GlobalConfigs[RecentMods] = ArrayCreate(MAX_MODNAME_LENGTH);
	g_GlobalConfigs[RecentMaps] = ArrayCreate(ArrayRecentMaps_e);
	g_Array_Nominations = ArrayCreate(1);

	Cvars_Init();
	MultiMod_Init();
	MultiMod_ExecCvars(g_iCurrentMod);
}

public plugin_init()
{
	register_plugin(PLUGIN_NAME, PLUGIN_VERSION, "FEDERICOMB");

	register_event_ex("TextMsg", "OnEvent_GameRestart", RegisterEvent_Global, "2&#Game_C", "2&#Game_w");
	register_event_ex("TextMsg", "OnEvent_GameRestart", RegisterEvent_Global, "2&#Game_will_restart_in");
	register_event_ex("HLTV", "OnEvent_HLTV", RegisterEvent_Global, "1=0", "2=0");

	RegisterHookChain(RG_CSGameRules_GoToIntermission, "OnCSGameRules_GoToIntermission", 0);

	UTIL_RegisterClientCommandAll("nextmod", "OnClientCommand_NextMod");
	UTIL_RegisterClientCommandAll("nextmap", "OnClientCommand_NextMap");
	UTIL_RegisterClientCommandAll("timeleft", "OnClientCommand_Timeleft");

	register_forward(FM_GetGameDescription, "OnGetGameDescription_Pre", 0);

	g_Hud_Vote = CreateHudSyncObj();
	g_Hud_Alert = CreateHudSyncObj();

	AdminCmd_Init();
	ModChooser_Init();
	MapChooser_Init();
	RockTheVote_Init();
	Nominations_Init();

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
	get_configsdir(szConfigDir, PLATFORM_MAX_PATH-1);

	formatex(szPluginsFile, PLATFORM_MAX_PATH-1, "%s/%s", szConfigDir, PLUGINS_FILENAME);
	UTIL_GetCurrentMod(szPluginsFile, g_szCurrentMod, MAX_MODNAME_LENGTH-1, szDefaultCurrentMap, MAX_MAPNAME_LENGTH-1);

	formatex(szFileName, PLATFORM_MAX_PATH-1, "%s/multimod_manager/configs.json", szConfigDir);

	if(!file_exists(szFileName))
	{
		set_fail_state("Error al encontrar el archivo [%s]", szFileName);
		return;
	}

	new JSON:jConfigsFile = json_parse(szFileName, true, true);

	if(jConfigsFile == Invalid_JSON)
	{
		set_fail_state("Error al analizar el archivo [%s]", szFileName);
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

		json_object_get_string(jConfigsFile, "adminflags.votemenu", szReadFlags, charsmax(szReadFlags), true);
		g_GlobalConfigs[AdminFlags_VoteMenu] = read_flags(szReadFlags);

		g_GlobalConfigs[RTV_Enabled] = json_object_get_bool(jConfigsFile, "rockthevote.enable", true);
		g_GlobalConfigs[RTV_Cooldown] = max(0, json_object_get_number(jConfigsFile, "rockthevote.cooldown", true));
		g_GlobalConfigs[RTV_MinPlayers] = clamp(json_object_get_number(jConfigsFile, "rockthevote.minplayers", true), 0, MAX_CLIENTS);
		g_GlobalConfigs[RTV_Percentage] = clamp(json_object_get_number(jConfigsFile, "rockthevote.percentage", true), 0, 100);
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
			format(szMapFile, PLATFORM_MAX_PATH-1, "%s/multimod_manager/mapsfiles/%s", szConfigDir, szMapFile);
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

				aMod[Plugins] = ArrayCreate(64);
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
				log_amx("Modo: '%s' lista de mapas inválida. Modo omitido", aMod[ModName]);
			}

			json_free(jArrayValue);
		}

		json_free(jArrayMods);
	}

	json_free(jConfigsFile);
	
	if(ArraySize(g_GlobalConfigs[Mods]) < 1)
	{
		set_fail_state("[MULTIMOD] No se detectaron modos cargados!");
		return;
	}

	if(bReloadMod)
	{
		g_iNextSelectMod = 0;
		MultiMod_SetNextMod(g_iNextSelectMod); // First mod as default
		UTIL_GetCurrentMod(szPluginsFile, g_szCurrentMod, MAX_MODNAME_LENGTH-1, szDefaultCurrentMap, MAX_MAPNAME_LENGTH-1);

		set_task(2.0, "OnTask_ChangeMap", 0, IsValidMap(szDefaultCurrentMap) ? szDefaultCurrentMap : g_szCurrentMap, MAX_MAPNAME_LENGTH-1);
		return;
	}

	MultiMod_ClearPluginsFile();
	MultiMod_OverwriteMapCycle(g_iCurrentMod);
	MultiMod_GetOffMods();
	Recent_LoadRecentModsMaps();
	Recent_SaveRecentModsMaps(g_iCurrentMod, g_szCurrentMap);
	OnEvent_GameRestart();
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
		client_print_color(0, print_team_default, "%s^1 El mapa cambiará al finalizar la ronda!", g_GlobalConfigs[ChatPrefix]);
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
	
	client_print_color(0, print_team_blue, "%s^1 El siguiente mapa será:^3 %s", g_GlobalConfigs[ChatPrefix], g_bCvar_amx_nextmap);

	g_bGameOver = true;
	set_member_game(m_iEndIntermissionButtonHit, 0);
	set_member_game(m_iSpawnPointCount_Terrorist, 0);
	set_member_game(m_iSpawnPointCount_CT, 0);
	set_member_game(m_bLevelInitialized, false);

	return HC_BREAK;
}

public OnClientCommand_NextMod(const id)
{
	CHECK_CONNECTED(id)

	if(g_bSelectedNextMod)
	{
		new aMod[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMod);

		client_print_color(id, print_team_blue, "%s^1 El siguiente modo será:^3 %s", g_GlobalConfigs[ChatPrefix], aMod[ModName]);
		return PLUGIN_HANDLED;
	}

	client_print_color(id, print_team_default, "%s^1 El siguiente modo todavía no ha sido elegido!", g_GlobalConfigs[ChatPrefix]);
	return PLUGIN_HANDLED;
}

public OnClientCommand_NextMap(const id)
{
	CHECK_CONNECTED(id)

	if(g_bSelectedNextMap)
	{
		client_print_color(id, print_team_blue, "%s^1 El siguiente mapa será:^3 %s", g_GlobalConfigs[ChatPrefix], g_bCvar_amx_nextmap);
		return PLUGIN_HANDLED;
	}

	client_print_color(id, print_team_default, "%s^1 El siguiente mapa todavía no ha sido elegido!", g_GlobalConfigs[ChatPrefix]);
	return PLUGIN_HANDLED;
}

public OnClientCommand_Timeleft(const id)
{
	CHECK_CONNECTED(id)

	switch(g_iNoMoreTime)
	{
		case 1: client_print_color(id, print_team_default, "%s^1 Esperando a que finalice la ronda actual para cambiar de mapa!", g_GlobalConfigs[ChatPrefix]);
		case 2: client_print_color(id, print_team_blue, "%s^1 El siguiente mapa será:^3 %s", g_GlobalConfigs[ChatPrefix], g_bCvar_amx_nextmap);
		default:
		{
			if((g_bCvar_mp_maxrounds != 0) || (g_bCvar_mp_winlimit != 0))
			{
				if(g_bCvar_mp_maxrounds != 0)
				{
					client_print_color(id, print_team_blue, "%s^1 Máximo de rondas:^3 %d^1 - Rondas restantes:^3 %d", g_GlobalConfigs[ChatPrefix], 
						g_bCvar_mp_maxrounds, (g_bCvar_mp_maxrounds - (get_member_game(m_iTotalRoundsPlayed))));
				}

				if(g_bCvar_mp_winlimit != 0)
				{
					client_print_color(id, print_team_blue, "%s^1 El primer equipo en llegar a^3 %d ronda%c ganada%c^1 ganará la partida!", 
						g_GlobalConfigs[ChatPrefix], g_bCvar_mp_winlimit, likely(g_bCvar_mp_winlimit == 1) ? 0 : 115, likely(g_bCvar_mp_winlimit == 1) ? 0 : 115);

					client_print_color(id, print_team_default, "%s^1 T:^4 %d^1 | CT:^4 %d", g_GlobalConfigs[ChatPrefix], get_member_game(m_iNumTerroristWins), get_member_game(m_iNumCTWins));
				}
			}
			else if(g_bCvar_mp_timelimit <= 0.0)
				client_print_color(id, print_team_blue, "%s^1 Tiempo restante:^3 Ilimitado", g_GlobalConfigs[ChatPrefix]);
			else
			{
				new z = get_timeleft();
				client_print_color(id, print_team_blue, "%s^1 Tiempo restante:^3 %d:%02d", g_GlobalConfigs[ChatPrefix], (z / 60), (z % 60));
			}
		}
	}

	return PLUGIN_HANDLED;
}

public OnGetGameDescription_Pre()
{
	if(likely(g_GlobalConfigs[ChangeGameDescription] == true))
	{
		new aMod[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_iCurrentMod, aMod);

		forward_return(FMV_STRING, aMod[ModName]);
		return FMRES_SUPERCEDE;
	}

	return FMRES_IGNORED;
}

public OnTask_CheckVoteNextMod()
{
	if(g_bSelectedNextMod || g_bSelectedNextMap)
	{
		remove_task(TASK_ENDMAP);
		return;
	}

	if(CanStartVoteNextMod())
	{
		g_bVoteInProgress = true;

		SetAlertStartNextVote(0.0, 10);

		remove_task(TASK_VOTEMOD);
		set_task(10.1, "OnTask_VoteNextMod", TASK_VOTEMOD);
	}
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
	ShowSyncHudMsg(0, g_Hud_Alert, "La votación comenzará en %d segundo%s", g_iCountdownTime, (g_iCountdownTime != 1) ? "s" : "");

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
	get_configsdir(szFileName, PLATFORM_MAX_PATH-1);
	add(szFileName, PLATFORM_MAX_PATH-1, fmt("/%s", PLUGINS_FILENAME));

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
	get_configsdir(szFileName, PLATFORM_MAX_PATH-1);
	add(szFileName, PLATFORM_MAX_PATH-1, fmt("/%s", PLUGINS_FILENAME));

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
	get_configsdir(szFileName, PLATFORM_MAX_PATH-1);
	add(szFileName, PLATFORM_MAX_PATH-1, "/multimod_manager/off_mods.json");

	if(!file_exists(szFileName))
		return;

	new JSON:jConfigsFile = json_parse(szFileName, true);

	if(jConfigsFile == Invalid_JSON)
	{
		abort(AMX_ERR_GENERAL, "Archivo JSON invalido [%s]", szFileName);
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
	get_configsdir(szFileName, charsmax(szFileName));
	add(szFileName, charsmax(szFileName), "/multimod_manager/off_mods.json");

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

	server_print("[MultiMod] Executing Cvars from Mod: %s (Count: %d)", aMod[ModName], iCvars);
	return iCvars;
}