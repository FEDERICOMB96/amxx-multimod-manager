#if defined _mm_mapchooser_included_
	#endinput
#endif
#define _mm_mapchooser_included_

#include <amxmodx>
#include "mm_incs/defines"
#include "mm_incs/cvars"

#define MAX_SELECTMAPS			5

new g_VoteMapCount = 0;
new g_VoteMapCountGlobal[MAX_SELECTMAPS + 2];
new g_SVM_MapInMenu[MAX_SELECTMAPS + 1];
new g_SVM_MapInMenu_SecondRound[MAX_SELECTMAPS + 1];
new g_SVM_MapSecondRound = 0;
new g_MapVoteNum = 0;
new g_SelectedNextMap = 0;
new g_VoteMapHasStarted = 0;
new g_ChangeMapTime = 10;
new g_ChangeMapOneMoreRound = 0;

new g_VoteMapId[MAX_USERS];

MapChooser_Init()
{
	MapChooser_ResetAllData();

	register_menu("VoteMap_Menu", KEYSMENU, "menu__CountVoteMap");
	register_menu("VoteMapFIX_Menu", KEYSMENU, "menu__CountVoteMapFIX");
}

MapChooser_ResetAllData()
{
	arrayset(g_VoteMapCountGlobal, 0, (MAX_SELECTMAPS + 2));

	arrayset(g_SVM_MapInMenu, -1, (MAX_SELECTMAPS + 1));
	arrayset(g_SVM_MapInMenu_SecondRound, -1, (MAX_SELECTMAPS + 1));

	g_VoteMapCount = 0;
	g_SVM_MapSecondRound = 0;
	g_MapVoteNum = 0;
	g_VoteMapHasStarted = 0;
	g_ChangeMapTime = 10;
	g_ChangeMapOneMoreRound = 0;
	
	arrayset(g_VoteMapId, 0, MAX_USERS);

	remove_task(TASK_VOTEMAP);
	remove_task(TASK_TIMELEFT);
	remove_task(TASK_SHOWTIME);
}

MapChooser_InitNextVoteMap(iStartVote)
{
	g_ShowTime = 10;

	new Float:flStartVote = float(max(10, iStartVote));

	remove_task(TASK_SHOWTIME);
	remove_task(TASK_VOTEMAP);

	set_task(flStartVote - 10.0, "OnTask_AlertStartNextVote", TASK_SHOWTIME);
	set_task(flStartVote, "OnTask_VoteNextMap", TASK_VOTEMAP);
}

MapChooser_ClientPutInServer(const id)
{
	g_VoteMapId[id] = 0;
}

MapChooser_ClientDisconnected(const id)
{
	// Si votó y se fue antes de terminar la votacion, su voto es removido!
	if(g_VoteMapId[id] && g_VoteMapHasStarted)
	{
		--g_VoteMapCountGlobal[g_VoteMapId[id]-1];
		--g_VoteMapCount;

		g_VoteMapId[id] = 0;
	}
}

public OnTask_VoteNextMap()
{
	new sMenu[400];
	new iMenuKeys = 0;
	new iArraySizeMaps = ArraySize(g_Array_MapName);
	new iMaxMaps = (iArraySizeMaps > MAX_SELECTMAPS) ? MAX_SELECTMAPS : iArraySizeMaps;

	new iLen = formatex(sMenu, charsmax(sMenu), "\yElige el próximo mapa: ¡VOTÁ AHORA!^n^n");

	g_MapVoteNum = 0;
	for(new iRandom; g_MapVoteNum < iMaxMaps; ++g_MapVoteNum)
	{
		if(g_SVM_MapInMenu[g_MapVoteNum] == -1) // Si no existe nominaciones, elegir al azar una opcion
		{
			do {
				iRandom = random(iArraySizeMaps);
			} while(IsMapInMenu(iRandom)); // La opcion está en el menú

			g_SVM_MapInMenu[g_MapVoteNum] = iRandom;
		}

		iMenuKeys |= (1<<g_MapVoteNum);

		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %a^n", (g_MapVoteNum + 1), ArrayGetStringHandle(g_Array_MapName, g_SVM_MapInMenu[g_MapVoteNum]));
	}

	arrayset(g_VoteMapCountGlobal, 0, (MAX_SELECTMAPS + 2));
	g_VoteMapCount = 0;
	g_VoteMapHasStarted = 1;
	g_SVM_MapSecondRound = 0;

	iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r%d.\y Seleccionar al azar", ((iMaxMaps + 2) > 9) ? 0 : (iMaxMaps + 2));
	iMenuKeys |= (1<<iMaxMaps + 1);
	
	show_menu(0, iMenuKeys, sMenu, 15, "VoteMap_Menu");
	set_task(15.1, "OnTaskCheckVotesMap");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_default, "%s^1 Es momento de elegir el próximo mapa!", g_GlobalPrefix);
	
	ClearSyncHud(0, g_HUD_Alert);
	OnTask_MapChooserHudVote();
}

public menu__CountVoteMap(const id, const iKey)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new sMap[32];
	new iMapId = iKey;

	if(iKey < MAX_SELECTMAPS)
	{
		ArrayGetString(g_Array_MapName, g_SVM_MapInMenu[iKey], sMap, 31);
		
		client_print_color(id, print_team_blue, "%s^1 Has elegido el mapa^3 %s", g_GlobalPrefix, sMap);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por el mapa: %s", id, sMap);
	}
	else if(iKey > MAX_SELECTMAPS)
	{
		iMapId = random_num(0, g_MapVoteNum-1);

		ArrayGetString(g_Array_MapName, g_SVM_MapInMenu[iMapId], sMap, 31);

		client_print_color(id, print_team_blue, "%s^1 Has elegido el mapa^3 %s^4 [ELECCIÓN ALEATORIA]", g_GlobalPrefix, sMap);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por el mapa: %s [ELECCIÓN ALEATORIA]", id, sMap);
	}
	
	++g_VoteMapCountGlobal[iMapId];
	++g_VoteMapCount;

	g_VoteMapId[id] = iMapId+1;
}

public OnTaskCheckVotesMap()
{
	new iWinner = 0;
	new iFirst = 0;
	new iResult = 0;
	new i;
	new sMap[32];
	
	for(i = 0; i < g_MapVoteNum; ++i)
	{
		if(!iFirst)
		{
			iWinner = i;
			iFirst = 1;
			iResult = g_VoteMapCountGlobal[i];
		}

		if(g_VoteMapCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_VoteMapCountGlobal[i];
		}
	}

	client_print(0, print_console, "Resultados de la votacion:");
	
	for(i = 0; i < g_MapVoteNum; ++i)
	{
		ArrayGetString(g_Array_MapName, g_SVM_MapInMenu[i], sMap, 31);
		client_print(0, print_console, "Mapa: %s - Votos: %d - Porcentaje: %d%%", sMap, g_VoteMapCountGlobal[i], (g_VoteMapCount > 0) ? ((g_VoteMapCountGlobal[i] * 100) / g_VoteMapCount) : 0);
	}

	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;

		// Chequeo si hubo 2 o más mapas con los mismos votos
		for(i = 0; i < g_MapVoteNum; ++i)
		{
			if(g_VoteMapCountGlobal[i] == iResult)
				g_SVM_MapInMenu_SecondRound[j++] = g_SVM_MapInMenu[i];
		}

		// Hubo 2 o más..
		if(j > 1)
		{
			g_VoteMapHasStarted = 0;
			g_SVM_MapSecondRound = 1;
			g_ShowTime = 5;
			g_MapVoteNum = j;

			client_print_color(0, print_team_blue, "%s^1 Hubo^4 %d mapas^1 con^4 %d^1 voto%s cada uno, siguiente votación en^3 10 segundos^1!", g_GlobalPrefix, j, iResult, (iResult != 1) ? "s" : "");

			client_cmd(0, "spk ^"run officer(e40) voltage(e30) accelerating(s70) is required^"");

			set_task(5.0, "OnTask_AlertStartNextVote", TASK_SHOWTIME);
			set_task(10.0, "OnTask_VoteNextMap__FIX");

			return;
		}

		// Solo 1 ganador
		ArrayGetString(g_Array_MapName, g_SVM_MapInMenu[iWinner], sMap, 31);
		set_pcvar_string(g_pCvar_amx_nextmap, sMap);

		client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el mapa ganador es^3 %s^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", g_GlobalPrefix, sMap, iResult, g_VoteMapCount, (g_VoteMapCount != 1) ? "s" : "", (g_VoteMapCount > 0) ? ((iResult * 100) / g_VoteMapCount) : 0); // , (g_VoteMapCount > 0) ? ((g_VoteMapCountGlobal[j] * 100) / g_VoteMapCount) : 0
		
		MapChooser_ResetAllData();

		g_SelectedNextMap = 1;

		ClearSyncHud(0, g_HUD_Vote);

		if(g_IsRtv)
		{
			g_VoteRtvResult = 1;
			
			remove_task(TASK_TIMELEFT);
			set_task(1.0, "OnTaskChangeTimeLeft", TASK_TIMELEFT);
			
			return;
		}

		g_ChangeMapTime = 10;

		ExecChangeTimeleft();
	}
	else
	{
		do {
			iWinner = random(g_MapVoteNum);
		} while(iWinner == MAX_SELECTMAPS);

		ArrayGetString(g_Array_MapName, g_SVM_MapInMenu[iWinner], sMap, 31);
		set_pcvar_string(g_pCvar_amx_nextmap, sMap);

		client_print_color(0, print_team_blue, "%s^1 Ningún voto fue emitido, por lo tanto el mapa ganador al azar es:^3 %s", g_GlobalPrefix, sMap);

		MapChooser_ResetAllData();

		g_SelectedNextMap = 1;

		ClearSyncHud(0, g_HUD_Vote);

		g_ChangeMapTime = 10;

		ExecChangeTimeleft();
	}
}

public OnTask_VoteNextMap__FIX()
{
	new sMenu[400];
	new iMenuKeys;
	new iLen;
	new i;

	iLen = formatex(sMenu, charsmax(sMenu), "\yElige el próximo mapa: ¡VOTÁ AHORA!^n^n");

	for(i = 0; i < g_MapVoteNum; ++i)
	{
		iMenuKeys |= (1<<i);
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %a^n", i+1, ArrayGetStringHandle(g_Array_MapName, g_SVM_MapInMenu_SecondRound[i]));
	}

	iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n^n\d(SEGUNDA VOTACIÓN)");

	arrayset(g_VoteMapCountGlobal, 0, (MAX_SELECTMAPS + 2));
	g_VoteMapCount = 0;
	g_VoteMapHasStarted = 1;
	g_SVM_MapSecondRound = 1;

	show_menu(0, iMenuKeys, sMenu, 15, "VoteMapFIX_Menu");
	set_task(15.1, "OnTaskCheckVotesMapFIX");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_default, "%s^1 Es momento de elegir el próximo mapa (SEGUNDA VOTACIÓN)!", g_GlobalPrefix);
	
	ClearSyncHud(0, g_HUD_Alert);
	OnTask_MapChooserHudVote();
}

public menu__CountVoteMapFIX(const id, const iKey)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new sMap[32];
	new iMapId = iKey;

	ArrayGetString(g_Array_MapName, g_SVM_MapInMenu_SecondRound[iMapId], sMap, 31);
	
	client_print_color(id, print_team_blue, "%s^1 Has elegido el mapa^3 %s", g_GlobalPrefix, sMap);
	
	if(g_bCvar_amx_vote_answers)
		client_print(0, print_console, "%n ha votado por el mapa: %s", id, sMap);

	++g_VoteMapCountGlobal[iMapId];
	++g_VoteMapCount;

	g_VoteMapId[id] = iMapId+1;
}

public OnTaskCheckVotesMapFIX()
{
	new iWinner = 0;
	new iFirst = 0;
	new iResult = 0;
	new i;
	new sMap[32];
	
	for(i = 0; i < g_MapVoteNum; ++i)
	{
		if(!iFirst)
		{
			iWinner = i;
			iFirst = 1;
			iResult = g_VoteMapCountGlobal[i];
		}

		if(g_VoteMapCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_VoteMapCountGlobal[i];
		}
	}

	client_print(0, print_console, "Resultados de la votacion:");
	
	for(i = 0; i < g_MapVoteNum; ++i)
	{
		ArrayGetString(g_Array_MapName, g_SVM_MapInMenu_SecondRound[i], sMap, 31);
		client_print(0, print_console, "Mapa: %s - Votos: %d - Porcentaje: %d%%", sMap, g_VoteMapCountGlobal[i], (g_VoteMapCount > 0) ? ((g_VoteMapCountGlobal[i] * 100) / g_VoteMapCount) : 0);
	}

	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;
		new iMapsIds[MAX_SELECTMAPS + 1];

		iWinner = g_SVM_MapInMenu_SecondRound[iWinner];

		// Chequeo si hubo 2 o más mapas con los mismos votos
		for(i = 0; i <= g_MapVoteNum; ++i)
		{
			if(g_VoteMapCountGlobal[i] == iResult)
				iMapsIds[j++] = g_SVM_MapInMenu_SecondRound[i];
		}

		if(j > 1) // Hubo más de 1 ganador (otra vez)
			iWinner = iMapsIds[random_num(0, (j - 1))];

		ArrayGetString(g_Array_MapName, iWinner, sMap, 31);
		set_pcvar_string(g_pCvar_amx_nextmap, sMap);
		
		if(j > 1)
		{
			client_print_color(0, print_team_default, "%s^1 Ningún mapa superó^4 la mayoría de los votos^1 por segunda vez.", g_GlobalPrefix);
			client_print_color(0, print_team_blue, "%s^1 Entre los más votados, el mapa ganador al azar es:^3 %s", g_GlobalPrefix, sMap);
		}
		else
			client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el mapa ganador es^3 %s^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", g_GlobalPrefix, sMap, iResult, g_VoteMapCount, (g_VoteMapCount != 1) ? "s" : "", (g_VoteMapCount > 0) ? ((iResult * 100) / g_VoteMapCount) : 0);
		
		MapChooser_ResetAllData();

		g_SelectedNextMap = 1;

		ClearSyncHud(0, g_HUD_Vote);

		if(g_IsRtv)
		{
			g_VoteRtvResult = 1;
			
			remove_task(TASK_TIMELEFT);
			set_task(1.0, "OnTaskChangeTimeLeft", TASK_TIMELEFT);
			
			return;
		}

		g_ChangeMapTime = 10;

		ExecChangeTimeleft();
	}
	else
	{
		do {
			iWinner = random(g_MapVoteNum);
		} while(iWinner == MAX_SELECTMAPS);

		ArrayGetString(g_Array_MapName, g_SVM_MapInMenu_SecondRound[iWinner], sMap, 31);
		set_pcvar_string(g_pCvar_amx_nextmap, sMap);

		client_print_color(0, print_team_blue, "%s^1 Ningún voto fue emitido, por lo tanto el mapa ganador al azar es:^3 %s", g_GlobalPrefix, sMap);

		MapChooser_ResetAllData();

		g_SelectedNextMap = 1;

		ClearSyncHud(0, g_HUD_Vote);

		g_ChangeMapTime = 10;

		ExecChangeTimeleft();
	}
}

public OnTask_MapChooserHudVote()
{
	if(!g_VoteMapHasStarted)
		return;

	set_task(0.1, "OnTask_MapChooserHudVote");

	if(!g_VoteMapCount)
	{
		set_hudmessage(255, 255, 255, 0.75, 0.35, 0, 0.0, 0.2, 0.0, 0.0, -1);
		ShowSyncHudMsg(0, g_HUD_Vote, "Sin votos emitidos");
		return;
	}

	new i;
	new j;
	new iLen = 0;
	new iTemp = 0;
	new iLoop = 0;
	new iNoFirst = 0;
	new iMapId[MAX_SELECTMAPS + 2];
	new iVoteMaps[MAX_SELECTMAPS + 2];

	new sHud[256];

	iLoop = (g_SVM_MapSecondRound) ? g_MapVoteNum : (g_MapVoteNum + 1);
	
	for(i = 0; i < iLoop; ++i)
	{
		iMapId[i] = (g_SVM_MapSecondRound) ? g_SVM_MapInMenu_SecondRound[i] : g_SVM_MapInMenu[i];
		iVoteMaps[i] = g_VoteMapCountGlobal[i];
	}

	// Ordenamiento por seleccion
	for(i = 0; i < g_MapVoteNum; ++i)
	{
		for(j = (i + 1); j < (g_MapVoteNum + 1); ++j)
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

		iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "%a: %d voto%s (%d%%%%)", ArrayGetStringHandle(g_Array_MapName, iMapId[i]), iVoteMaps[i], (iVoteMaps[i] == 1) ? "" : "s", (g_VoteMapCount > 0) ? ((iVoteMaps[i] * 100) / g_VoteMapCount) : 0);
	}

	set_hudmessage(255, 255, 255, 0.75, 0.35, 0, 0.0, 0.3, 0.0, 0.0, -1);
	ShowSyncHudMsg(0, g_HUD_Vote, sHud);
}

public OnTaskChangeTimeLeft()
{
	g_NoMoreTime = 1;
	g_RestoreTimelimit = get_pcvar_float(g_pCvar_mp_timelimit);
	set_pcvar_float(g_pCvar_mp_timelimit, 0.0);

	switch(g_ChangeMapType)
	{
		case CHANGEMAP_END_OF_ROUND:
		{
			client_cmd(0, "spk ^"%s^"", g_SOUND_ExtendTime);
			client_print_color(0, print_team_default, "%s^1 El mapa cambiará al finalizar la ronda!", g_GlobalPrefix);
		}
		case CHANGEMAP_ONE_MORE_ROUND:
		{
			g_ChangeMapOneMoreRound = 1;
		}
	}
}

public OnTaskAlertChangeMap()
{
	if(!g_ChangeMapTime)
	{
		g_NoMoreTime = 2;
		
		message_begin(MSG_ALL, SVC_INTERMISSION);
		message_end();
		
		set_task(2.0, "taskChangeMap", _, g_bCvar_amx_nextmap, sizeof(g_bCvar_amx_nextmap));
		
		client_print_color(0, print_team_blue, "%s^1 El siguiente mapa será:^3 %s", g_GlobalPrefix, g_bCvar_amx_nextmap);
		return;
	}
	
	set_hudmessage(255, 255, 255, -1.0, 0.35, 0, 0.0, 1.1, 0.0, 0.0, -1);
	ShowSyncHudMsg(0, g_HUD_Alert, "El mapa cambiará en %d segundo%s", g_ChangeMapTime, (g_ChangeMapTime != 1) ? "s" : "");

	client_cmd(0, "spk ^"fvox/%s^"", g_SOUND_CountDown[g_ChangeMapTime]);

	--g_ChangeMapTime;
	
	remove_task(TASK_TIMELEFT);
	set_task(1.0, "OnTaskAlertChangeMap", TASK_TIMELEFT);
}

public taskChangeMap(sMap[])
{
	engine_changelevel(sMap);
}

ExecChangeTimeleft()
{
	remove_task(TASK_TIMELEFT);

	(g_ChangeMapType == CHANGEMAP_TIMELEFT)
		? set_task((float(get_timeleft()) - 10.1), "OnTaskAlertChangeMap", TASK_TIMELEFT)
		: set_task((float(get_timeleft()) - 1.1), "OnTaskChangeTimeLeft", TASK_TIMELEFT);
}

MapChooser_LoadMaps(const szFileName[])
{
	if(file_exists(szFileName))
	{
		new iFile = fopen(szFileName, "r");

		if(iFile)
		{
			new szLine[PLATFORM_MAX_PATH];
			new szMap[64];

			while(!feof(iFile))
			{
				fgets(iFile, szLine, charsmax(szLine));
				parse(szLine, szMap, charsmax(szMap));

				mb_strtolower(szMap);
				
				if(szMap[0] != ';' 
					&& IsValidMap(szMap) 
					&& !equali(szMap, g_CurrentMap))
				{
					ArrayPushString(g_Array_MapName, szMap);
				}
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
	for(new j = 0; j < (MAX_SELECTMAPS + 1); ++j)
	{
		if(g_SVM_MapInMenu[j] == i)
			return true;
	}

	return false;
}