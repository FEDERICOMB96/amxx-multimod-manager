#if defined _mm_mapchooser_included_
	#endinput
#endif
#define _mm_mapchooser_included_

MapChooser_Init()
{
	MapChooser_ResetAllData();
}

MapChooser_ResetAllData()
{
	g_bVoteInProgress = false;

	g_iMapVoteNum = 0;
	g_iChangeMapTime = 0;
	g_iVoteMapCount = 0;

	arrayset(g_iVoteMapCountGlobal, 0, MAX_SELECTMAPS);
	arrayset(g_iSVM_MapInMenu, -1, MAX_SELECTMAPS);
	arrayset(g_iSVM_MapInMenu_SecondRound, 0, MAX_SELECTMAPS);

	g_bSVM_MapSecondRound = false;
	g_bSelectedNextMap = false;
	g_bVoteMapHasStarted = false;
	g_bChangeMapOneMoreRound = false;
	
	arrayset(g_VoteMapId, 0, MAX_USERS);

	remove_task(TASK_VOTEMAP);
	remove_task(TASK_TIMELEFT);
	remove_task(TASK_SHOWTIME);
}

MapChooser_InitNextVoteMap(iStartVote)
{
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);

	if(likely(ArraySize(aMods[Maps]) < 2))
	{
		new szMap[MAX_MAPNAME_LENGTH];
		ArrayGetString(aMods[Maps], 0, szMap, MAX_MAPNAME_LENGTH-1);
		set_pcvar_string(g_pCvar_amx_nextmap, szMap);

		client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MAPCHOO_NEXT", szMap);
		
		MapChooser_ResetAllData();
		Nominations_ResetAllData();

		g_bSelectedNextMap = true;

		if(g_bIsRockTheVote)
		{
			RockTheVoteSendIntermission();
			return;
		}

		g_iChangeMapTime = 10;

		ExecChangeTimeleft();
		return;
	}

	if(!g_GlobalConfigs[Nom_Maps_Enabled])
		iStartVote = 0;

	new Float:flStartVote = float(max(10, iStartVote));

	SetAlertStartNextVote(flStartVote - 10.0, 10);

	remove_task(TASK_VOTEMAP);
	set_task(flStartVote, "OnTask_VoteNextMap", TASK_VOTEMAP);

	if(g_GlobalConfigs[Nom_Maps_Enabled])
		client_print_color(0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MAPCHOO_CAN_NOM", iStartVote);
}

MapChooser_ClientPutInServer(const id)
{
	g_VoteMapId[id] = 0;
}

MapChooser_ClientDisconnected(const id)
{
	// Si votó y se fue antes de terminar la votacion, su voto es removido!
	if(g_VoteMapId[id] > 0 && g_bVoteMapHasStarted)
	{
		--g_iVoteMapCountGlobal[g_VoteMapId[id]-1];
		--g_iVoteMapCount;

		g_VoteMapId[id] = 0;
	}
}

public OnTask_VoteNextMap()
{
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);

	new iRecentMaps = Recent_CountRecentMaps(g_iNextSelectMod);
	new iArraySizeMaps = ArraySize(aMods[Maps]);
	
	if(likely(iArraySizeMaps <= iRecentMaps))
		iRecentMaps = 0;

	new iMaxMaps = min(g_GlobalConfigs[MapsInMenu], (iArraySizeMaps - iRecentMaps));

	g_Menu_MapChooser = menu_create(fmt("\y%L", LANG_PLAYER, "MM_SELECT_NEXT_MAP"), "menu__CountVoteMap");

	g_iMapVoteNum = 0;
	for(new iRandom, iMapId, iArraySizeNoms, szMapName[MAX_MAPNAME_LENGTH]; g_iMapVoteNum < iMaxMaps; ++g_iMapVoteNum)
	{
		do {
			if((iArraySizeNoms = ArraySize(g_Array_Nominations)))
			{
				iRandom = random(iArraySizeNoms);
				iMapId = ArrayGetCell(g_Array_Nominations, iRandom);
				ArrayDeleteItem(g_Array_Nominations, iRandom);
			}
			else
				iMapId = random(iArraySizeMaps);

			ArrayGetString(aMods[Maps], iMapId, szMapName, charsmax(szMapName));
		} while(IsMapInMenu(iMapId) || (iRecentMaps && Recent_IsRecentMap(g_iNextSelectMod, szMapName))); // La opcion está en el menú

		g_iSVM_MapInMenu[g_iMapVoteNum] = iMapId;
		menu_additem(g_Menu_MapChooser, fmt("\w%s%s", szMapName, likely(g_iMapVoteNum == (iMaxMaps-1)) ? "^n" : ""));
	}

	menu_additem(g_Menu_MapChooser, fmt("\y%L", LANG_PLAYER, "MM_SELECT_VOTE_RANDOM_ITEM"));

	menu_setprop(g_Menu_MapChooser, MPROP_PERPAGE, 0);
	menu_setprop(g_Menu_MapChooser, MPROP_EXIT, MEXIT_NEVER);

	UTIL_MenuDisplayAll(g_Menu_MapChooser, _, 15);

	arrayset(g_VoteMapId, -1, MAX_USERS);
	arrayset(g_iVoteMapCountGlobal, 0, MAX_SELECTMAPS);
	g_iVoteMapCount = 0;
	g_bVoteMapHasStarted = true;
	g_bSVM_MapSecondRound = false;

	set_task(15.1, "OnTask_CheckVoteMap");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_TIME_CHOOSE_MAP");
	
	ClearSyncHud(0, g_Hud_Vote);
	OnTask_MapChooserHudVote();

	ExecuteForward(g_Forward_StartVotemap, g_Forward_RetVal, false);
}

public menu__CountVoteMap(const id, const menuid, const item)
{
	CHECK_CONNECTED(id)
	CHECK_MENU_EXIT(item)
	CHECK_MENU_TIMEOUT(item)

	new szMap[MAX_MAPNAME_LENGTH];
	new iMapId = item;

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);

	if(iMapId < g_iMapVoteNum)
	{
		ArrayGetString(aMods[Maps], g_iSVM_MapInMenu[iMapId], szMap, MAX_MAPNAME_LENGTH-1);
		
		client_print_color(id, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MAPCHOO_VOTED_FOR", szMap);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%L", LANG_PLAYER, "MM_MAPCHOO_PLAYER_VOTED_FOR", id, szMap);
	}
	else
	{
		iMapId = random(g_iMapVoteNum);
		ArrayGetString(aMods[Maps], g_iSVM_MapInMenu[iMapId], szMap, MAX_MAPNAME_LENGTH-1);

		client_print_color(id, print_team_blue, "%s^1 %L^4 [%L]", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MAPCHOO_VOTED_FOR", szMap, LANG_PLAYER, "MM_RANDOM");
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%L [%L]", LANG_PLAYER, "MM_MAPCHOO_PLAYER_VOTED_FOR", id, szMap, LANG_PLAYER, "MM_RANDOM");
	}
	
	++g_iVoteMapCountGlobal[iMapId];
	++g_iVoteMapCount;

	g_VoteMapId[id] = iMapId+1;
	return PLUGIN_HANDLED;
}

public OnTask_CheckVoteMap()
{
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);

	new iWinner = 0;
	new iResult = 0;

	ResetMapMenuAll();
	menu_destroy(g_Menu_MapChooser);

	ExecuteForward(g_Forward_EndVotemap, g_Forward_RetVal, false);

	client_print(0, print_console, "%L:", LANG_PLAYER, "MM_VOTE_RESULT");

	for(new i = 0; i < g_iMapVoteNum; ++i)
	{
		if(g_iVoteMapCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_iVoteMapCountGlobal[i];
		}

		client_print(0, print_console, "%L: %a - %L: %d - Porcentaje: %d%%", LANG_PLAYER, "MM_MAP",	ArrayGetStringHandle(aMods[Maps], g_iSVM_MapInMenu[i]), 
			LANG_PLAYER, "MM_VOTES", g_iVoteMapCountGlobal[i], LANG_PLAYER, "MM_PERCENT", UTIL_GetPercent(g_iVoteMapCountGlobal[i], g_iVoteMapCount));
	}

	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;
		new szMap[MAX_MAPNAME_LENGTH];

		// Chequeo si hubo 2 o más mapas con los mismos votos
		for(new i = 0; i < g_iMapVoteNum; ++i)
		{
			if(g_iVoteMapCountGlobal[i] == iResult)
				g_iSVM_MapInMenu_SecondRound[j++] = g_iSVM_MapInMenu[i];
		}

		// Hubo 2 o más..
		if(j > 1)
		{
			g_bVoteInProgress = true;
			
			g_bVoteMapHasStarted = false;
			g_bSVM_MapSecondRound = true;
			g_iMapVoteNum = j;

			client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MAPCHOO_NEED_SECOND_VOTE", j, iResult);

			if(g_bCvar_amx_multimod_voice)
				client_cmd(0, "spk ^"run officer(e40) voltage(e30) accelerating(s70) is required^"");

			set_task(10.0, "OnTask_VoteNextMap__FIX");

			SetAlertStartNextVote(5.0, 5);
			return;
		}

		// Solo 1 ganador
		ArrayGetString(aMods[Maps], g_iSVM_MapInMenu[iWinner], szMap, MAX_MAPNAME_LENGTH-1);
		set_pcvar_string(g_pCvar_amx_nextmap, szMap);

		client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MAP_CHO_FIN_NEXT", szMap, iResult, g_iVoteMapCount, UTIL_GetPercent(iResult, g_iVoteMapCount));
		
		MapChooser_ResetAllData();
		Nominations_ResetAllData();

		g_bSelectedNextMap = true;

		if(g_bIsRockTheVote)
		{
			RockTheVoteSendIntermission();
			return;
		}

		g_iChangeMapTime = 10;

		ExecChangeTimeleft();
	}
	else
	{
		iWinner = random(g_iMapVoteNum);

		new szMap[MAX_MAPNAME_LENGTH];
		ArrayGetString(aMods[Maps], g_iSVM_MapInMenu[iWinner], szMap, MAX_MAPNAME_LENGTH-1);
		set_pcvar_string(g_pCvar_amx_nextmap, szMap);

		client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_CHO_NO_VOTES_MAP", szMap);

		MapChooser_ResetAllData();
		Nominations_ResetAllData();

		g_bSelectedNextMap = true;

		g_iChangeMapTime = 10;

		ExecChangeTimeleft();
	}
}

public OnTask_VoteNextMap__FIX()
{
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);

	g_Menu_MapChooser = menu_create(fmt("\y%L", LANG_PLAYER, "MM_SELECT_NEXT_MAP"), "menu__CountVoteMapFIX");

	for(new i = 0; i < g_iMapVoteNum; ++i)
		menu_additem(g_Menu_MapChooser, fmt("\w%a%s", ArrayGetStringHandle(aMods[Maps], g_iSVM_MapInMenu_SecondRound[i]), likely(i == (g_iMapVoteNum-1)) ? "^n^n\d(SEGUNDA VOTACIÓN)" : ""));

	menu_setprop(g_Menu_MapChooser, MPROP_PERPAGE, 0);
	menu_setprop(g_Menu_MapChooser, MPROP_EXIT, MEXIT_NEVER);

	UTIL_MenuDisplayAll(g_Menu_MapChooser, _, 15);

	arrayset(g_VoteMapId, -1, MAX_USERS);
	arrayset(g_iVoteMapCountGlobal, 0, MAX_SELECTMAPS);
	g_iVoteMapCount = 0;
	g_bVoteMapHasStarted = true;
	g_bSVM_MapSecondRound = true;

	set_task(15.1, "OnTask_CheckVoteMapFIX");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_grey, "%s^1 %L^3 (%L)", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_TIME_CHOOSE_MAP", LANG_PLAYER, "MM_SECOND_VOTE");
	
	ClearSyncHud(0, g_Hud_Vote);
	OnTask_MapChooserHudVote();

	ExecuteForward(g_Forward_StartVotemap, g_Forward_RetVal, true);
}

public menu__CountVoteMapFIX(const id, const menuid, const item)
{
	CHECK_CONNECTED(id)
	CHECK_MENU_EXIT(item)
	CHECK_MENU_TIMEOUT(item)

	new szMap[MAX_MAPNAME_LENGTH];
	new iMapId = item;

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);

	ArrayGetString(aMods[Maps], g_iSVM_MapInMenu_SecondRound[iMapId], szMap, MAX_MAPNAME_LENGTH-1);
	
	client_print_color(id, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MAPCHOO_VOTED_FOR", szMap);
	
	if(g_bCvar_amx_vote_answers)
		client_print(0, print_console, "%L", LANG_PLAYER, "MM_MAPCHOO_PLAYER_VOTED_FOR", id, szMap);

	++g_iVoteMapCountGlobal[iMapId];
	++g_iVoteMapCount;

	g_VoteMapId[id] = iMapId+1;
	return PLUGIN_HANDLED;
}

public OnTask_CheckVoteMapFIX()
{
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);

	new iWinner = 0;
	new iResult = 0;

	ResetMapMenuAll();
	menu_destroy(g_Menu_MapChooser);

	ExecuteForward(g_Forward_EndVotemap, g_Forward_RetVal, true);

	client_print(0, print_console, "%L:", LANG_PLAYER, "MM_VOTE_RESULT");

	for(new i = 0; i < g_iMapVoteNum; ++i)
	{
		if(g_iVoteMapCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_iVoteMapCountGlobal[i];
		}

		client_print(0, print_console, "%L: %a - %L: %d - %L: %d%%", LANG_PLAYER, "MM_MAP", ArrayGetStringHandle(aMods[Maps], g_iSVM_MapInMenu_SecondRound[i]), 
			LANG_PLAYER, "MM_VOTES", g_iVoteMapCountGlobal[i], LANG_PLAYER, "MM_PERCENT", UTIL_GetPercent(g_iVoteMapCountGlobal[i], g_iVoteMapCount));
	}

	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;
		new iMapsIds[MAX_SELECTMAPS];
		new szMap[MAX_MAPNAME_LENGTH];

		iWinner = g_iSVM_MapInMenu_SecondRound[iWinner];

		// Chequeo si hubo 2 o más mapas con los mismos votos
		for(new i = 0; i < g_iMapVoteNum; ++i)
		{
			if(g_iVoteMapCountGlobal[i] == iResult)
				iMapsIds[j++] = g_iSVM_MapInMenu_SecondRound[i];
		}

		if(j > 1) // Hubo más de 1 ganador (otra vez)
			iWinner = iMapsIds[random(j)];

		ArrayGetString(aMods[Maps], iWinner, szMap, MAX_MAPNAME_LENGTH-1);
		set_pcvar_string(g_pCvar_amx_nextmap, szMap);
		
		if(j > 1)
		{
			client_print_color(0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MAPCHOO_SECOND_ROUND_NEXT_INFO1");
			client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MAPCHOO_SECOND_ROUND_NEXT_INFO2", szMap);
		}
		else
			client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MAP_CHO_FIN_NEXT", szMap, iResult, g_iVoteMapCount,  UTIL_GetPercent(iResult, g_iVoteMapCount));
		
		MapChooser_ResetAllData();
		Nominations_ResetAllData();

		g_bSelectedNextMap = true;

		if(g_bIsRockTheVote)
		{
			RockTheVoteSendIntermission();
			return;
		}

		g_iChangeMapTime = 10;

		ExecChangeTimeleft();
	}
	else
	{
		iWinner = random(g_iMapVoteNum);

		new szMap[MAX_MAPNAME_LENGTH];
		ArrayGetString(aMods[Maps], g_iSVM_MapInMenu_SecondRound[iWinner], szMap, MAX_MAPNAME_LENGTH-1);
		set_pcvar_string(g_pCvar_amx_nextmap, szMap);

		client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_CHO_NO_VOTES_MAP", szMap);

		MapChooser_ResetAllData();
		Nominations_ResetAllData();

		g_bSelectedNextMap = true;

		g_iChangeMapTime = 10;

		ExecChangeTimeleft();
	}
}

public OnTask_MapChooserHudVote()
{
	if(!g_bVoteMapHasStarted)
		return;

	set_task(0.1, "OnTask_MapChooserHudVote");

	if(!g_iVoteMapCount)
	{
		set_hudmessage(255, 255, 255, 0.75, 0.35, 0, 0.0, 0.2, 0.0, 0.0, -1);
		ShowSyncHudMsg(0, g_Hud_Vote, "%L", LANG_PLAYER, "MM_NO_VOTES_COUNT");
		return;
	}

	new i;
	new iLen = 0;
	new iLoop = 0;
	new iNoFirst = 0;
	new iMapId[MAX_SELECTMAPS];
	new iVoteMaps[MAX_SELECTMAPS];

	new sHud[256];

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);

	iLoop = g_iMapVoteNum;
	
	for(i = 0; i < iLoop; ++i)
	{
		iMapId[i] = (g_bSVM_MapSecondRound) ? g_iSVM_MapInMenu_SecondRound[i] : g_iSVM_MapInMenu[i];
		iVoteMaps[i] = g_iVoteMapCountGlobal[i];
	}

	UTIL_SimpleSortVotes(iVoteMaps, iMapId, g_iMapVoteNum);

	for(i = 0; i < iLoop; ++i)
	{
		if(!iVoteMaps[i])
			continue;

		if(iNoFirst)
			iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "^n");

		iNoFirst = 1;

		iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "%a: %d %L (%d%%%%)", ArrayGetStringHandle(aMods[Maps], iMapId[i]), iVoteMaps[i], LANG_PLAYER, (iVoteMaps[i] == 1) ? "MM_VOTE" : "MM_VOTES", UTIL_GetPercent(iVoteMaps[i], g_iVoteMapCount));
	}

	set_hudmessage(255, 255, 255, 0.75, 0.35, 0, 0.0, 0.3, 0.0, 0.0, -1);
	ShowSyncHudMsg(0, g_Hud_Vote, sHud);
}

public OnTask_ChangeTimeLeft()
{
	g_iNoMoreTime = 1;
	g_RestoreTimelimit = get_pcvar_float(g_pCvar_mp_timelimit);
	set_pcvar_float(g_pCvar_mp_timelimit, 0.0);

	switch(UTIL_GetChangeMapType(g_iCurrentMod))
	{
		case CHANGEMAP_END_OF_ROUND:
		{
			if(g_bCvar_amx_multimod_voice)
				client_cmd(0, "spk ^"%s^"", g_SOUND_ExtendTime);

			client_print_color(0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MAP_CHANGE_END_ROUND");
		}
		case CHANGEMAP_ONE_MORE_ROUND:
		{
			g_bChangeMapOneMoreRound = true;
		}
		default: return;
	}

	if(get_playersnum() > 1)
		return;

	OnCSGameRules_GoToIntermission();
}

RockTheVoteSendIntermission()
{
	g_bVoteRtvResult = true;

	remove_task(TASK_TIMELEFT);
	set_task(2.0, "OnTask_RockTheVoteSendIntermission", TASK_TIMELEFT);
}

public OnTask_RockTheVoteSendIntermission()
{
	OnCSGameRules_GoToIntermission();
}

public OnTask_AlertChangeMap()
{
	if(!g_iChangeMapTime)
	{
		OnCSGameRules_GoToIntermission();
		return;
	}
	
	set_hudmessage(255, 255, 255, -1.0, 0.35, 0, 0.0, 1.1, 0.0, 0.0, -1);
	ShowSyncHudMsg(0, g_Hud_Alert, "%L", g_iChangeMapTime, LANG_PLAYER, "MM_NEXT_MAP_CHANGE_WILL_BE_IN");

	if(g_bCvar_amx_multimod_voice)
		client_cmd(0, "spk ^"fvox/%s^"", g_SOUND_CountDown[g_iChangeMapTime]);

	--g_iChangeMapTime;
	
	remove_task(TASK_TIMELEFT);
	set_task(1.0, "OnTask_AlertChangeMap", TASK_TIMELEFT);
}

public OnTask_ChangeMap(sMap[])
{
	engine_changelevel(sMap);
}

ExecChangeTimeleft()
{
	remove_task(TASK_TIMELEFT);

	(UTIL_GetChangeMapType(g_iCurrentMod) == CHANGEMAP_TIMELEFT)
		? set_task((float(get_timeleft()) - 10.1), "OnTask_AlertChangeMap", TASK_TIMELEFT)
		: set_task((float(get_timeleft()) - 1.1), "OnTask_ChangeTimeLeft", TASK_TIMELEFT);
}

MapChooser_LoadMaps(&Array:aMyArray, const szFileName[])
{
	ArrayClear(aMyArray);

	if(file_exists(szFileName))
	{
		new iFile = fopen(szFileName, "r");

		if(iFile)
		{
			new szLine[PLATFORM_MAX_PATH];
			new szMap[MAX_MAPNAME_LENGTH];

			while(fgets(iFile, szLine, charsmax(szLine)))
			{
				parse(szLine, szMap, charsmax(szMap));

				mb_strtolower(szMap);
				
				if(szMap[0] != ';' && IsValidMap(szMap))
					ArrayPushString(aMyArray, szMap);
			}
			
			fclose(iFile);
		}
	}
}

bool:IsValidMap(szMapName[])
{
	if(is_map_valid(szMapName))
		return true;
	
	new iLen = strlen(szMapName) - 4;
	
	if(iLen < 0)
		return false;
	
	if(equali(szMapName[iLen], ".bsp"))
	{
		szMapName[iLen] = EOS;
		
		if(is_map_valid(szMapName))
			return true;
	}
	
	return false;
}

bool:IsMapInMenu(const i)
{
	for(new j = 0; j < MAX_SELECTMAPS; ++j)
	{
		if(g_iSVM_MapInMenu[j] == i)
			return true;
	}

	return false;
}

bool:ResetMapMenuAll()
{
	for(new i = 1; i < MAX_USERS; ++i) if(GetPlayerBit(g_bConnected, i) && likely(g_VoteMapId[i] == -1))
	{
		menu_cancel(i);
		reset_menu(i);
	}

	return true;
}