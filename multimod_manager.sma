#pragma semicolon 1

new const PLUGIN_NAME[] = "MultiMod Manager";
new const PLUGIN_VERSION[] = "v2021.08.02";

new const PLUGINS_FILENAME[] = "plugins-multimodmanager.ini";

#include <amxmodx>
#include <amxmisc>
#include <reapi>
#include <engine>
#include <json>
#include <multimod_manager/defines>

new g_bConnected;

new g_GlobalPrefix[21];
new g_TimeleftTrigger;
new Array:g_aModNames;
new ChangeMap_e:g_ChangeMapType;

new g_CurrentMap[64];
new g_LastMap[64];
new g_CurrentMod[64];
new g_NoMoreTime = 0;
new g_ShowTime = 0;
new Float:g_RestoreTimelimit = 0.0;
new g_HUD_Vote = 0;
new g_HUD_Alert = 0;


#include <multimod_manager/cvars>
#include <multimod_manager/rockthevote>
#include <multimod_manager/modchooser>
#include <multimod_manager/mapchooser>


public plugin_precache()
{
	precache_sound(g_SOUND_ExtendTime);
	
	for(new i = 0; i < sizeof(g_SOUND_GmanChoose); ++i)
		precache_sound(g_SOUND_GmanChoose[i]);
	
	for(new i = 0, szBuffer[64]; i < sizeof(g_SOUND_CountDown); ++i)
	{
		formatex(szBuffer, charsmax(szBuffer), "fvox/%s.wav", g_SOUND_CountDown[i]);
		precache_sound(szBuffer);
	}
}

public plugin_init()
{
	register_plugin(PLUGIN_NAME, PLUGIN_VERSION, "FEDERICOMB");

	CvarsInit();
	MultiModInit();
	ModChooser_Init();
	MapChooser_Init();

	register_event_ex("TextMsg", "OnEvent_GameRestart", RegisterEvent_Global, "2&#Game_C", "2&#Game_w");
	register_event_ex("TextMsg", "OnEvent_GameRestart", RegisterEvent_Global, "2&#Game_will_restart_in");
	register_event_ex("HLTV", "OnEvent_HLTV", RegisterEvent_Global, "1=0", "2=0");

	get_mapname(g_CurrentMap, charsmax(g_CurrentMap));
	mb_strtolower(g_CurrentMap);

	get_localinfo("mm_lastmap", g_LastMap, charsmax(g_LastMap));
	mb_strtolower(g_LastMap);

	g_HUD_Vote = CreateHudSyncObj();
	g_HUD_Alert = CreateHudSyncObj();
}

public plugin_cfg()
{
	server_cmd("amx_pausecfg add ^"%s^"", PLUGIN_NAME);
	server_cmd("sv_restart 1");
}

public plugin_end()
{
	set_localinfo("mm_lastmap", g_CurrentMap);
	set_pcvar_float(mp_timelimit, g_RestoreTimelimit);
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

MultiModInit()
{
	new szConfigDir[STRLEN_PATH], szFileName[STRLEN_PATH], szPluginsFile[STRLEN_PATH];
	get_configsdir(szConfigDir, charsmax(szConfigDir));

	formatex(szPluginsFile, charsmax(szPluginsFile), "%s/%s", szConfigDir, PLUGINS_FILENAME);
	MultiMod_GetCurrentMod(szPluginsFile, g_CurrentMod, charsmax(g_CurrentMod));

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

	g_aModNames = ArrayCreate(32);

	json_object_get_string(jsonConfigsFile, "global_chat_prefix", g_GlobalPrefix, charsmax(g_GlobalPrefix));

	replace_string(g_GlobalPrefix, charsmax(g_GlobalPrefix), "!y" , "^1");
	replace_string(g_GlobalPrefix, charsmax(g_GlobalPrefix), "!t" , "^3");
	replace_string(g_GlobalPrefix, charsmax(g_GlobalPrefix), "!g" , "^4");

	g_TimeleftTrigger = json_object_get_number(jsonConfigsFile, "timeleft_trigger");

	new JSON:jsonObjectMods = json_object_get_value(jsonConfigsFile, "mods");
	new iCount = json_array_get_count(jsonObjectMods);

	new Array:aExecCFGs = ArrayCreate(128);

	for(new i = 0, szModName[32], JSON:jsonArrayValue; i < iCount; ++i)
	{
		jsonArrayValue = json_array_get_value(jsonObjectMods, i);

		json_object_get_string(jsonArrayValue, "modname", szModName, charsmax(szModName));
		ArrayPushString(g_aModNames, szModName);

		// Modo actual? Ejecutar CFGs
		if(equali(g_CurrentMod, szModName))
		{
			g_ChangeMapType = ChangeMap_e:json_object_get_number(jsonArrayValue, "change_map_type");

			new JSON:jsonObjectCvars = json_object_get_value(jsonArrayValue, "cvars");
			new iCvars = json_array_get_count(jsonObjectCvars);

			for(new j = 0, szCvarName[128]; j < iCvars; ++j)
			{
				json_array_get_string(jsonObjectCvars, j, szCvarName, charsmax(szCvarName));
				ArrayPushString(aExecCFGs, szCvarName);
			}

			json_free(jsonObjectCvars);
		}

		json_free(jsonArrayValue);
	}

	json_free(jsonObjectMods);
	json_free(jsonConfigsFile);

	new iCfgsCount = ArraySize(aExecCFGs);
	for(new i = 0; i < iCfgsCount; ++i)
	{
		server_cmd("%a", ArrayGetStringHandle(aExecCFGs, i));
	}

	ArrayDestroy(aExecCFGs);

	if(!iCount)
	{
		set_fail_state("[MULTIMOD] No se detectaron modos cargados!");
		return;
	}

	OnEvent_GameRestart();
}

MultiMod_GetCurrentMod(const szFilePath[STRLEN_PATH], szOut[], const iLen)
{
	if(file_exists(szFilePath))
	{
		new iFile = fopen(szFilePath, "r");

		if(iFile)
		{
			new szLine[128];
			fgets(iFile, szLine, charsmax(szLine));
			trim(szLine);

			replace_string(szLine, charsmax(szLine), ";Mod:", "");
			replace_string(szLine, charsmax(szLine), "^"", "");
			
			copy(szOut, iLen, szLine);

			fclose(iFile);
		}
	}
}

public OnEvent_GameRestart()
{
	ModChooser_ResetAllData();
	MapChooser_ResetAllData();

	if(ArraySize(g_aModNames))
	{
		remove_task(TASK_ENDMAP);
		set_task(15.0, "OnTask_CheckVoteNextMod", TASK_ENDMAP, .flags = "b");
	}
}

public OnEvent_HLTV()
{
	if((g_NoMoreTime == 1 && !g_ChangeMapOneMoreRound) || (g_VoteRtvResult && g_NoMoreTime == 1))
	{
		g_NoMoreTime = 2;
		
		set_task(2.0, "taskChangeMap", _, amx_nextmap, sizeof(amx_nextmap));
		
		message_begin(MSG_ALL, SVC_INTERMISSION);
		message_end();
		
		client_print_color(0, print_team_blue, "%s^1 El siguiente mapa será: ^3%s", g_GlobalPrefix, amx_nextmap);
	}

	if(g_ChangeMapOneMoreRound)
	{
		g_ChangeMapOneMoreRound = 0;

		client_cmd(0, "spk ^"%s^"", g_SOUND_ExtendTime);
		client_print_color(0, print_team_default, "%s^1 El mapa cambiará al finalizar la ronda!", g_GlobalPrefix);
	}
}

public OnTask_CheckVoteNextMod()
{
	if(g_SelectedNextMod || g_SelectedNextMap)
	{
		remove_task(TASK_ENDMAP);
		return;
	}

	if(mp_winlimit)
	{
		new a = mp_winlimit - 2;
		
		if((a > get_member_game(m_iNumCTWins)) && (a > get_member_game(m_iNumTerroristWins)))
			return;
	}
	else if(mp_maxrounds)
	{
		if((mp_maxrounds - 2) > (get_member_game(m_iNumCTWins) + get_member_game(m_iNumTerroristWins)))
			return;
	}
	else
	{
		new iTimeleft = get_timeleft();
		
		if(iTimeleft < 1 || iTimeleft > g_TimeleftTrigger)
			return;
	}
	
	if(g_VoteModHasStarted || g_VoteMapHasStarted)
		return;

	g_ShowTime = 10;

	remove_task(TASK_SHOWTIME);
	remove_task(TASK_VOTEMOD);

	OnTask_AlertStartNextVote();
	set_task(10.1, "OnTask_VoteNextMod", TASK_VOTEMOD);
}

public OnTask_AlertStartNextVote()
{
	if(!g_ShowTime)
	{
		ClearSyncHud(0, g_HUD_Alert);
		return;
	}

	if(g_ShowTime == 10)
		client_cmd(0, "spk ^"get red(e80) ninety(s45) to check(e20) use bay(s18) mass(e42) cap(s50)^"");

	set_hudmessage(255, 255, 255, -1.0, 0.35, 0, 0.0, 1.1, 0.0, 0.0, -1);
	ShowSyncHudMsg(0, g_HUD_Alert, "La votación comenzará en %d segundo%s", g_ShowTime, (g_ShowTime != 1) ? "s" : "");

	if(g_ShowTime <= 5)
		client_cmd(0, "spk ^"fvox/%s^"", g_SOUND_CountDown[g_ShowTime]);
	
	--g_ShowTime;
	
	remove_task(TASK_SHOWTIME);
	set_task(1.0, "OnTask_AlertStartNextVote", TASK_SHOWTIME);
}

MultiMod_SetNextMod(const iMod)
{
	new szFileName[128], szConfigDir[128];
	get_configsdir(szConfigDir, charsmax(szConfigDir));

	formatex(szFileName, charsmax(szFileName), "%s/multimod_manager/configs.json", szConfigDir);

	if(!file_exists(szFileName))
	{
		set_fail_state("[MULTIMOD] MultiMod_SetNextMod: Archivo '%s' no se encuentra!", szFileName);
		return;
	}

	new szPluginsFile[256];
	formatex(szPluginsFile, charsmax(szPluginsFile), "%s/%s", szConfigDir, PLUGINS_FILENAME);
	
	new pPluginsFile = fopen(szPluginsFile, "w+");

	if(pPluginsFile)
	{
		new JSON:jsonConfigsFile = json_parse(szFileName, true);

		if(jsonConfigsFile == Invalid_JSON)
		{
			fclose(pPluginsFile);
			return;
		}

		new JSON:jsonObjectMods = json_object_get_value(jsonConfigsFile, "mods");
		new JSON:jsonArrayMods = json_array_get_value(jsonObjectMods, iMod);

		new szModName[32];
		json_object_get_string(jsonArrayMods, "modname", szModName, charsmax(szModName));

		fprintf(pPluginsFile, ";Mod:^"%s^"^n^n", szModName);

		new JSON:jsonObjectPlugins = json_object_get_value(jsonArrayMods, "plugins");
		new iPlugins = json_array_get_count(jsonObjectPlugins);

		for(new i = 0, szPluginName[32]; i < iPlugins; ++i)
		{
			json_array_get_string(jsonObjectPlugins, i, szPluginName, charsmax(szPluginName));
			fprintf(pPluginsFile, "%s^n", szPluginName);
		}

		new szMapFile[128];
		json_object_get_string(jsonArrayMods, "mapsfile", szMapFile, charsmax(szMapFile));
		format(szMapFile, charsmax(szMapFile), "%s/multimod_manager/%s", szConfigDir, szMapFile);
		MapChooser_LoadMaps(szMapFile);

		json_free(jsonObjectPlugins);
		json_free(jsonArrayMods);
		json_free(jsonObjectMods);
		json_free(jsonConfigsFile);

		fclose(pPluginsFile);
	}
}
