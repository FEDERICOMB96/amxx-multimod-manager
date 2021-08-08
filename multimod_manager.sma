#pragma semicolon 1

new const PLUGIN_NAME[] = "MultiMod Manager";
new const PLUGIN_VERSION[] = "v2021.08.02";

new const PLUGINS_FILENAME[] = "plugins-multimodmanager.ini";

#include <amxmodx>
#include <amxmisc>
#include <reapi>
#include <engine>
#include <json>
#include "mm_incs/defines"

/*
 * Global vars
 */
new g_bConnected;

new g_szCurrentMap[64];
new g_szCurrentMod[64];
new g_szGlobalPrefix[21];

new g_iCurrentMod = 0;
new g_iNoMoreTime = 0;
new g_iCountdownTime = 0;

new Float:g_RestoreTimelimit = 0.0;

new g_Hud_Vote = 0;
new g_Hud_Alert = 0;

new Array:g_Array_Mods;
new Array:g_Array_MapName;

#include "mm_incs/cvars"
#include "mm_incs/rockthevote"
#include "mm_incs/modchooser"
#include "mm_incs/mapchooser"
#include "mm_incs/utils"

public plugin_init()
{
	register_plugin(PLUGIN_NAME, PLUGIN_VERSION, "FEDERICOMB");

	register_event_ex("TextMsg", "OnEvent_GameRestart", RegisterEvent_Global, "2&#Game_C", "2&#Game_w");
	register_event_ex("TextMsg", "OnEvent_GameRestart", RegisterEvent_Global, "2&#Game_will_restart_in");
	register_event_ex("HLTV", "OnEvent_HLTV", RegisterEvent_Global, "1=0", "2=0");

	get_mapname(g_szCurrentMap, charsmax(g_szCurrentMap));
	mb_strtolower(g_szCurrentMap);

	g_Hud_Vote = CreateHudSyncObj();
	g_Hud_Alert = CreateHudSyncObj();
}

public OnConfigsExecuted()
{
	server_cmd("amx_pausecfg add ^"%s^"", PLUGIN_NAME);

	Cvars_Init();
	MultiMod_Init();
	ModChooser_Init();
	MapChooser_Init();
}

public plugin_end()
{
	if(g_RestoreTimelimit)
		set_pcvar_float(g_pCvar_mp_timelimit, g_RestoreTimelimit);

	if(g_Array_Mods != Invalid_Array)
	{
		new iSize = ArraySize(g_Array_Mods);
		for(new i = 0, aData[ArrayMods_e]; i < iSize; ++i)
		{
			ArrayGetArray(g_Array_Mods, i, aData);

			if(aData[Cvars] != Invalid_Array)
				ArrayDestroy(aData[Cvars]);

			if(aData[Plugins] != Invalid_Array)
				ArrayDestroy(aData[Plugins]);
		}

		ArrayDestroy(g_Array_Mods);
	}

	if(g_Array_MapName != Invalid_Array)
		ArrayDestroy(g_Array_MapName);
}

public client_putinserver(id)
{
	SetPlayerBit(g_bConnected, id);

	ModChooser_ClientPutInServer(id);
	MapChooser_ClientPutInServer(id);
}

public client_disconnected(id, bool:drop, message[], maxlen)
{
	ClearPlayerBit(g_bConnected, id);

	ModChooser_ClientDisconnected(id);
	MapChooser_ClientDisconnected(id);
}

MultiMod_Init()
{
	new szConfigDir[PLATFORM_MAX_PATH], szFileName[PLATFORM_MAX_PATH], szPluginsFile[PLATFORM_MAX_PATH];
	get_configsdir(szConfigDir, charsmax(szConfigDir));

	formatex(szPluginsFile, charsmax(szPluginsFile), "%s/%s", szConfigDir, PLUGINS_FILENAME);
	UTIL_GetCurrentMod(szPluginsFile, g_szCurrentMod, charsmax(g_szCurrentMod));

	formatex(szFileName, charsmax(szFileName), "%s/multimod_manager/configs.json", szConfigDir);

	if(!file_exists(szFileName))
	{
		set_fail_state("[MULTIMOD] Archivo '%s' no se encuentra!", szFileName);
		return;
	}

	new JSON:jsonConfigsFile = json_parse(szFileName, true);

	if(jsonConfigsFile == Invalid_JSON)
	{
		set_fail_state("[MULTIMOD] Archivo JSON invalido '%s'", szFileName);
		return;
	}

	g_Array_Mods = ArrayCreate(ArrayMods_e);
	g_Array_MapName = ArrayCreate(64);

	json_object_get_string(jsonConfigsFile, "global_chat_prefix", g_szGlobalPrefix, charsmax(g_szGlobalPrefix));

	replace_string(g_szGlobalPrefix, charsmax(g_szGlobalPrefix), "!y" , "^1");
	replace_string(g_szGlobalPrefix, charsmax(g_szGlobalPrefix), "!t" , "^3");
	replace_string(g_szGlobalPrefix, charsmax(g_szGlobalPrefix), "!g" , "^4");

	new JSON:jsonObjectMods = json_object_get_value(jsonConfigsFile, "mods");
	new iCount = json_array_get_count(jsonObjectMods);

	for(new i = 0, j, aMod[ArrayMods_e], JSON:jsonArrayValue, JSON:jsonObject, iJsonObjetCount; i < iCount; ++i)
	{
		jsonArrayValue = json_array_get_value(jsonObjectMods, i);
		{
			json_object_get_string(jsonArrayValue, "modname", aMod[ModName], charsmax(aMod));
			json_object_get_string(jsonArrayValue, "mapsfile", aMod[MapsFile], charsmax(aMod));
			aMod[ChangeMapType] = ChangeMap_e:json_object_get_number(jsonArrayValue, "change_map_type");

			aMod[Cvars] = ArrayCreate(128);
			jsonObject = json_object_get_value(jsonArrayValue, "cvars");
			{
				iJsonObjetCount = json_array_get_count(jsonObject);

				j = 0;
				for(new szCvarName[128]; j < iJsonObjetCount; ++j)
				{
					json_array_get_string(jsonObject, j, szCvarName, charsmax(szCvarName));
					ArrayPushString(aMod[Cvars], szCvarName);
				}
			}
			json_free(jsonObject);

			aMod[Plugins] = ArrayCreate(64);
			jsonObject = json_object_get_value(jsonArrayValue, "plugins");
			{
				iJsonObjetCount = json_array_get_count(jsonObject);

				j = 0;
				for(new szPluginName[64]; j < iJsonObjetCount; ++j)
				{
					json_array_get_string(jsonObject, j, szPluginName, charsmax(szPluginName));
					ArrayPushString(aMod[Plugins], szPluginName);
				}
			}
			json_free(jsonObject);

			ArrayPushArray(g_Array_Mods, aMod);

			// Modo actual?
			if(equali(g_szCurrentMod, aMod[ModName]))
			{
				g_iCurrentMod = i;

				new iCvars = ArraySize(aMod[Cvars]);
				for(new i = 0; i < iCvars; ++i)
				{
					server_cmd("%a", ArrayGetStringHandle(aMod[Cvars], i));
				}
			}
		}
		json_free(jsonArrayValue);
	}

	json_free(jsonObjectMods);
	json_free(jsonConfigsFile);
	
	if(!iCount)
	{
		set_fail_state("[MULTIMOD] No se detectaron modos cargados!");
		return;
	}

	OnEvent_GameRestart();
}

public OnEvent_GameRestart()
{
	ModChooser_ResetAllData();
	MapChooser_ResetAllData();

	if(ArraySize(g_Array_Mods))
	{
		remove_task(TASK_ENDMAP);
		set_task(15.0, "OnTask_CheckVoteNextMod", TASK_ENDMAP, .flags = "b");
	}
}

public OnEvent_HLTV()
{
	if((g_iNoMoreTime == 1 && !g_bChangeMapOneMoreRound) || (g_VoteRtvResult && g_iNoMoreTime == 1))
	{
		g_iNoMoreTime = 2;
		
		set_task(2.0, "taskChangeMap", _, g_bCvar_amx_nextmap, sizeof(g_bCvar_amx_nextmap));
		
		message_begin(MSG_ALL, SVC_INTERMISSION);
		message_end();
		
		client_print_color(0, print_team_blue, "%s^1 El siguiente mapa será:^3 %s", g_szGlobalPrefix, g_bCvar_amx_nextmap);
	}

	if(g_bChangeMapOneMoreRound)
	{
		g_bChangeMapOneMoreRound = false;

		client_cmd(0, "spk ^"%s^"", g_SOUND_ExtendTime);
		client_print_color(0, print_team_default, "%s^1 El mapa cambiará al finalizar la ronda!", g_szGlobalPrefix);
	}
}

public OnTask_CheckVoteNextMod()
{
	if(g_bSelectedNextMod || g_bSelectedNextMap)
	{
		remove_task(TASK_ENDMAP);
		return;
	}

	if(g_bCvar_mp_winlimit)
	{
		new a = g_bCvar_mp_winlimit - 3;
		
		if((a > get_member_game(m_iNumCTWins)) && (a > get_member_game(m_iNumTerroristWins)))
			return;
	}
	else if(g_bCvar_mp_maxrounds)
	{
		if((g_bCvar_mp_maxrounds - 3) > (get_member_game(m_iNumCTWins) + get_member_game(m_iNumTerroristWins)))
			return;
	}
	else
	{
		new iTimeleft = get_timeleft();
		
		if(iTimeleft < 1 || iTimeleft > 180)
			return;
	}
	
	if(g_bVoteModHasStarted || g_bSVM_ModSecondRound || g_bVoteMapHasStarted || g_bSVM_MapSecondRound)
		return;

	g_iCountdownTime = 10;

	remove_task(TASK_SHOWTIME);
	remove_task(TASK_VOTEMOD);

	OnTask_AlertStartNextVote();
	set_task(10.1, "OnTask_VoteNextMod", TASK_VOTEMOD);
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
	ArrayGetArray(g_Array_Mods, iNextMod, aDataNextMod);

	new szConfigDir[PLATFORM_MAX_PATH], szFileName[PLATFORM_MAX_PATH];
	get_configsdir(szConfigDir, charsmax(szConfigDir));

	formatex(szFileName, charsmax(szFileName), "%s/%s", szConfigDir, PLUGINS_FILENAME);

	new pPluginsFile = fopen(szFileName, "w+");

	if(pPluginsFile)
	{
		fprintf(pPluginsFile, ";Mod:%s^n^n", aDataNextMod[ModName]);

		new iPlugins = ArraySize(aDataNextMod[Plugins]);
		for(new i = 0; i < iPlugins; ++i)
			fprintf(pPluginsFile, "%a^n", ArrayGetStringHandle(aDataNextMod[Plugins], i));

		fclose(pPluginsFile);
	}
	
	formatex(szFileName, charsmax(szFileName), "%s/multimod_manager/mapsfiles/%s", szConfigDir, aDataNextMod[MapsFile]);
	MapChooser_LoadMaps(szFileName);
}
