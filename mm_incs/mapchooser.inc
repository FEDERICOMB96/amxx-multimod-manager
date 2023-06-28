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

		client_print_color(0, print_team_blue, "%s^1 El próximo mapa será^3 %s^1!", g_GlobalConfigs[ChatPrefix], szMap);
		
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

	new Float:flStartVote = float(max(10, iStartVote));

	SetAlertStartNextVote(flStartVote - 10.0, 10);

	remove_task(TASK_VOTEMAP);
	set_task(flStartVote, "OnTask_VoteNextMap", TASK_VOTEMAP);

	client_print_color(0, print_team_default, "%s^1 Puedes nominar mapas para el próximo modo durante los proximos^4 %d segundos", g_GlobalConfigs[ChatPrefix], iStartVote);
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

	g_Menu_MapChooser = menu_create("\yElige el próximo mapa: ¡VOTÁ AHORA!", "menu__CountVoteMap");

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

	menu_additem(g_Menu_MapChooser, "\ySeleccionar al azar");

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
	client_print_color(0, print_team_default, "%s^1 Es momento de elegir el próximo mapa!", g_GlobalConfigs[ChatPrefix]);
	
	ClearSyncHud(0, g_Hud_Vote);
	OnTask_MapChooserHudVote();
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
		
		client_print_color(id, print_team_blue, "%s^1 Has elegido el mapa^3 %s", g_GlobalConfigs[ChatPrefix], szMap);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por el mapa: %s", id, szMap);
	}
	else
	{
		iMapId = random(g_iMapVoteNum);
		ArrayGetString(aMods[Maps], g_iSVM_MapInMenu[iMapId], szMap, MAX_MAPNAME_LENGTH-1);

		client_print_color(id, print_team_blue, "%s^1 Has elegido el mapa^3 %s^4 [ELECCIÓN ALEATORIA]", g_GlobalConfigs[ChatPrefix], szMap);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por el mapa: %s [ELECCIÓN ALEATORIA]", id, szMap);
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
	new iResult = -1;

	ResetMapMenuAll();
	menu_destroy(g_Menu_MapChooser);

	client_print(0, print_console, "Resultados de la votacion:");

	for(new i = 0; i < g_iMapVoteNum; ++i)
	{
		if(g_iVoteMapCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_iVoteMapCountGlobal[i];
		}

		client_print(0, print_console, "Mapa: %a - Votos: %d - Porcentaje: %d%%", 
			ArrayGetStringHandle(aMods[Maps], g_iSVM_MapInMenu[i]), g_iVoteMapCountGlobal[i], UTIL_GetPercent(g_iVoteMapCountGlobal[i], g_iVoteMapCount));
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

			client_print_color(0, print_team_blue, "%s^1 Hubo^4 %d mapas^1 con^4 %d^1 voto%s cada uno, siguiente votación en^3 10 segundos^1!", g_GlobalConfigs[ChatPrefix], j, iResult, (iResult != 1) ? "s" : "");

			client_cmd(0, "spk ^"run officer(e40) voltage(e30) accelerating(s70) is required^"");

			set_task(10.0, "OnTask_VoteNextMap__FIX");

			SetAlertStartNextVote(5.0, 5);
			return;
		}

		// Solo 1 ganador
		ArrayGetString(aMods[Maps], g_iSVM_MapInMenu[iWinner], szMap, MAX_MAPNAME_LENGTH-1);
		set_pcvar_string(g_pCvar_amx_nextmap, szMap);

		client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el mapa ganador es^3 %s^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", 
			g_GlobalConfigs[ChatPrefix], szMap, iResult, g_iVoteMapCount, (g_iVoteMapCount != 1) ? "s" : "", UTIL_GetPercent(iResult, g_iVoteMapCount));
		
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

		client_print_color(0, print_team_blue, "%s^1 Ningún voto fue emitido, por lo tanto el mapa ganador al azar es:^3 %s", g_GlobalConfigs[ChatPrefix], szMap);

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

	g_Menu_MapChooser = menu_create("\yElige el próximo mapa: ¡VOTÁ AHORA!", "menu__CountVoteMapFIX");

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
	client_print_color(0, print_team_grey, "%s^1 Es momento de elegir el próximo mapa!^3 (SEGUNDA VOTACIÓN)", g_GlobalConfigs[ChatPrefix]);
	
	ClearSyncHud(0, g_Hud_Vote);
	OnTask_MapChooserHudVote();
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
	
	client_print_color(id, print_team_blue, "%s^1 Has elegido el mapa^3 %s", g_GlobalConfigs[ChatPrefix], szMap);
	
	if(g_bCvar_amx_vote_answers)
		client_print(0, print_console, "%n ha votado por el mapa: %s", id, szMap);

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
	new iResult = -1;

	ResetMapMenuAll();
	menu_destroy(g_Menu_MapChooser);

	client_print(0, print_console, "Resultados de la votacion:");

	for(new i = 0; i < g_iMapVoteNum; ++i)
	{
		if(g_iVoteMapCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_iVoteMapCountGlobal[i];
		}

		client_print(0, print_console, "Mapa: %a - Votos: %d - Porcentaje: %d%%", 
			ArrayGetStringHandle(aMods[Maps], g_iSVM_MapInMenu_SecondRound[i]), g_iVoteMapCountGlobal[i], UTIL_GetPercent(g_iVoteMapCountGlobal[i], g_iVoteMapCount));
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
			client_print_color(0, print_team_default, "%s^1 Ningún mapa superó^4 la mayoría de los votos^1 por segunda vez.", g_GlobalConfigs[ChatPrefix]);
			client_print_color(0, print_team_blue, "%s^1 Entre los más votados, el mapa ganador al azar es:^3 %s", g_GlobalConfigs[ChatPrefix], szMap);
		}
		else
			client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el mapa ganador es^3 %s^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", 
				g_GlobalConfigs[ChatPrefix], szMap, iResult, g_iVoteMapCount, (g_iVoteMapCount != 1) ? "s" : "", UTIL_GetPercent(iResult, g_iVoteMapCount));
		
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

		client_print_color(0, print_team_blue, "%s^1 Ningún voto fue emitido, por lo tanto el mapa ganador al azar es:^3 %s", g_GlobalConfigs[ChatPrefix], szMap);

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
		ShowSyncHudMsg(0, g_Hud_Vote, "Sin votos emitidos");
		return;
	}

	new i;
	new j;
	new iLen = 0;
	new iTemp = 0;
	new iLoop = 0;
	new iNoFirst = 0;
	new iMapId[MAX_SELECTMAPS];
	new iVoteMaps[MAX_SELECTMAPS];

	new sHud[256];

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);

	iLoop = (g_bSVM_MapSecondRound) ? g_iMapVoteNum : (g_iMapVoteNum + 1);
	
	for(i = 0; i < iLoop; ++i)
	{
		iMapId[i] = (g_bSVM_MapSecondRound) ? g_iSVM_MapInMenu_SecondRound[i] : g_iSVM_MapInMenu[i];
		iVoteMaps[i] = g_iVoteMapCountGlobal[i];
	}

	// Ordenamiento por seleccion
	for(i = 0; i < (g_iMapVoteNum - 1); ++i)
	{
		for(j = (i + 1); j < g_iMapVoteNum; ++j)
		{
			if(iVoteMaps[j] > iVoteMaps[i])
			{
				iTemp = iVoteMaps[j];
				iVoteMaps[j] = iVoteMaps[i];
				iVoteMaps[i] = iTemp;

				iTemp = iMapId[j];
				iMapId[j] = iMapId[i];
				iMapId[i] = iTemp;
			}
		}
	}

	for(i = 0; i < iLoop; ++i)
	{
		if(!iVoteMaps[i])
			continue;

		if(iNoFirst)
			iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "^n");

		iNoFirst = 1;

		iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "%a: %d voto%s (%d%%%%)", ArrayGetStringHandle(aMods[Maps], iMapId[i]), iVoteMaps[i], (iVoteMaps[i] == 1) ? "" : "s", UTIL_GetPercent(iVoteMaps[i], g_iVoteMapCount));
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
			client_cmd(0, "spk ^"%s^"", g_SOUND_ExtendTime);
			client_print_color(0, print_team_default, "%s^1 El mapa cambiará al finalizar la ronda!", g_GlobalConfigs[ChatPrefix]);
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
	ShowSyncHudMsg(0, g_Hud_Alert, "El mapa cambiará en %d segundo%s", g_iChangeMapTime, (g_iChangeMapTime != 1) ? "s" : "");

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