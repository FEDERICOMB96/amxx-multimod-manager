#if defined _mm_mapchooser_included_
	#endinput
#endif
#define _mm_mapchooser_included_

#include <amxmodx>
#include <multimod_manager/defines>
#include <multimod_manager/cvars>

#define MAX_SELECTMAPS			5
#define MAX_RECENT_MAPS			5

new g_VoteCount = 0;
new g_VoteCountGlobal[MAX_SELECTMAPS + 2];
new g_SVM_MapInMenu[MAX_SELECTMAPS + 1];
new g_SVM_MapInMenu__FuckingFIX[MAX_SELECTMAPS + 1];
new g_SVM_MapName[MAX_SELECTMAPS + 1][64];
new g_SVM_MapSecondRound = 0;
new g_MapVoteNum = 0;
new g_SelectedNextMap = 0;
new g_VoteMapHasStarted = 0;
new g_ChangeMapTime = 10;
new g_ChangeMapOneMoreRound = 0;
new g_RecentMaps[MAX_RECENT_MAPS][32];
new g_RecentMapsCount = 0;

new g_VoteMapId[MAX_USERS];

new Array:g_Array_MapName;

MapChooser_Init()
{
	for(new i = 0; i < (MAX_SELECTMAPS + 1); ++i)
	{
		g_SVM_MapInMenu[i] = -1;
		g_SVM_MapName[i][0] = EOS;
		g_SVM_MapInMenu__FuckingFIX[i] = -1;
	}

	register_menu("VoteMap_Menu", KEYSMENU, "menu__CountVoteMap");
	register_menu("VoteMapFIX_Menu", KEYSMENU, "menu__CountVoteMapFIX");
	
	g_Array_MapName = ArrayCreate(32);
}

MapChooser_ResetAllData()
{
	for(new i = 0; i < (MAX_SELECTMAPS + 2); ++i)
		g_VoteCountGlobal[i] = 0;

	for(new i = 0; i < (MAX_SELECTMAPS + 1); ++i)
	{
		g_SVM_MapInMenu[i] = -1;
		g_SVM_MapInMenu__FuckingFIX[i] = -1;
		g_SVM_MapName[i][0] = EOS;
	}

	g_VoteCount = 0;
	g_SVM_MapSecondRound = 0;
	g_MapVoteNum = 0;
	g_ChangeMapTime = 10;
	g_NoMoreTime = 0;
	g_ShowTime = 10;
	g_ChangeMapOneMoreRound = 0;
	g_VoteMapHasStarted = 0;

	for(new i = 0; i < MAX_USERS; ++i)
	{
		RockTheVote_ResetUserData(i);
		g_VoteMapId[i] = 0;
	}

	remove_task(TASK_VOTEMAP);
	remove_task(TASK_VOTEMOD);
	remove_task(TASK_TIMELEFT);
	remove_task(TASK_RTV);
	remove_task(TASK_SHOWTIME);
}

MapChooser_InitNextVoteMap()
{
	g_ShowTime = 10;

	new Float:flTimeStartVote = (get_pcvar_float(mp_timelimit) - 2.0) * 60.0;
	
	remove_task(TASK_VOTEMAP);
	set_task(flTimeStartVote, "OnTaskVoteNextMap", TASK_VOTEMAP);
	
	remove_task(TASK_SHOWTIME);
	set_task(flTimeStartVote - 10.0, "OnTaskSpamStartVote", TASK_SHOWTIME);
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
		--g_VoteCountGlobal[g_VoteMapId[id]-1];
		--g_VoteCount;

		g_VoteMapId[id] = 0;
	}
}

public OnTaskVoteNextMap()
{
	new sMenu[400];
	new iMenuKeys;
	new iRandom;
	new j;
	new iLen;
	new iMaxMaps;
	new iStart;
	new iOk;
	new iArraySizeMaps = ArraySize(g_Array_MapName);

	iMaxMaps = (iArraySizeMaps > MAX_SELECTMAPS) ? MAX_SELECTMAPS : iArraySizeMaps;
	iMenuKeys = (1<<iMaxMaps + 1);
	
	iLen = formatex(sMenu, charsmax(sMenu), "\yElige el próximo mapa: ¡VOTÁ AHORA!^n^n");
	j = 0;
	
	for(g_MapVoteNum = 0; g_MapVoteNum < iMaxMaps; ++g_MapVoteNum)
	{
		if(g_SVM_MapInMenu[g_MapVoteNum] == -1)
		{
			iOk = 1;
			iStart = iRandom = random_num(0, (iArraySizeMaps - 1));
			
			while(IsMapInMenu(iRandom))
			{
				if(++iRandom >= iArraySizeMaps)
					iRandom = 0;

				if(iRandom == iStart) // Fix infinite bucle
				{
					iOk = 0;
					break;
				}
			}

			if(!iOk)
				continue;

			g_SVM_MapInMenu[g_MapVoteNum] = iRandom;
		}

		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %a^n", ++j, ArrayGetStringHandle(g_Array_MapName, g_SVM_MapInMenu[g_MapVoteNum]));
		
		iMenuKeys |= (1<<g_MapVoteNum);
	}
	
	for(j = 0; j < (MAX_SELECTMAPS + 2); ++j)
		g_VoteCountGlobal[j] = 0;
	
	g_VoteCount = 0;
	g_VoteMapHasStarted = 1;
	g_SVM_MapSecondRound = 0;

	iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r%d.\y Seleccionar al azar", ((iMaxMaps + 2) > 9) ? 0 : (iMaxMaps + 2));
	
	show_menu(0, iMenuKeys, sMenu, 15, "VoteMap_Menu");
	set_task(15.1, "OnTaskCheckVotesMap");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_default, "%sEs momento de elegir el próximo mapa!", g_GlobalPrefix);
	
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
		
		client_print_color(id, print_team_blue, "%s^1Has elegido el mapa^3 %s", g_GlobalPrefix, sMap);
		
		if(amx_vote_answers)
			client_print(0, print_console, "%n ha votado por el mapa: %s", id, sMap);
	}
	else if(iKey > MAX_SELECTMAPS)
	{
		iMapId = random_num(0, g_MapVoteNum-1);

		ArrayGetString(g_Array_MapName, g_SVM_MapInMenu[iMapId], sMap, 31);

		client_print_color(id, print_team_blue, "%sHas elegido el mapa^3 %s^4 [ELECCIÓN ALEATORIA]", g_GlobalPrefix, sMap);
		
		if(amx_vote_answers)
			client_print(0, print_console, "%n ha votado por el mapa: %s [ELECCIÓN ALEATORIA]", id, sMap);
	}
	
	++g_VoteCountGlobal[iMapId];
	++g_VoteCount;

	g_VoteMapId[id] = iMapId+1;
}

public OnTaskCheckVotesMap()
{
	new iWinner = 0;
	new iFirst = 0;
	new iResult = 0;
	new i;
	new sMap[32];
	
	for(i = 0; i <= g_MapVoteNum; ++i)
	{
		if(!iFirst)
		{
			iWinner = i;
			iFirst = 1;
			iResult = g_VoteCountGlobal[i];
		}

		if(g_VoteCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_VoteCountGlobal[i];
		}
	}

	client_print(0, print_console, "Resultados de la votacion:");
	
	for(i = 0; i < g_MapVoteNum; ++i)
	{
		ArrayGetString(g_Array_MapName, g_SVM_MapInMenu[i], sMap, 31);
		
		client_print(0, print_console, "Mapa: %s - Votos: %d - Porcentaje: %d%%", sMap, g_VoteCountGlobal[i], (g_VoteCount > 0) ? ((g_VoteCountGlobal[i] * 100) / g_VoteCount) : 0);
	}
	
	client_print(0, print_console, "Extender mapa: %dm - Votos: %d - Porcentaje: %d%%", amx_extendmap_step, g_VoteCountGlobal[MAX_SELECTMAPS], (g_VoteCount > 0) ? ((g_VoteCountGlobal[MAX_SELECTMAPS] * 100) / g_VoteCount) : 0);

	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;

		// Chequeo si hubo 2 o más mapas con los mismos votos
		for(i = 0; i <= g_MapVoteNum; ++i)
		{
			if(g_VoteCountGlobal[i] == iResult)
			{
				if(i != MAX_SELECTMAPS) // MAX_SELECTMAPS == extender mapa
					g_SVM_MapInMenu__FuckingFIX[j++] = g_SVM_MapInMenu[i];
			}
		}

		// Hubo 2 o más..
		if(j > 1)
		{
			g_VoteMapHasStarted = 0;
			g_SVM_MapSecondRound = 1;
			g_ShowTime = 5;
			g_MapVoteNum = j;

			client_print_color(0, print_team_blue, "%sHubo^4 %d mapas^1 con^4 %d^1 voto%s cada uno, siguiente votación en^3 5 segundos^1!", g_GlobalPrefix, j, iResult, (iResult != 1) ? "s" : "");

			client_cmd(0, "spk ^"run officer(e40) voltage(e30) accelerating(s70) is required^"");

			set_task(5.0, "OnTaskSpamStartVote", TASK_SHOWTIME);
			set_task(10.0, "OnTaskVoteNextMap__FIX");
		}

		// Solo 1 ganador
		else
		{
			ArrayGetString(g_Array_MapName, g_SVM_MapInMenu[iWinner], sMap, 31);
			set_pcvar_string(pamx_nextmap, sMap);

			client_print_color(0, print_team_blue, "%sVotación finalizada, el mapa ganador es ^3%s ^1con^4 %d ^1/^4 %d ^1voto%s (%d%%)!", g_GlobalPrefix, sMap, iResult, g_VoteCount, (g_VoteCount != 1) ? "s" : "", (g_VoteCount > 0) ? ((iResult * 100) / g_VoteCount) : 0); // , (g_VoteCount > 0) ? ((g_VoteCountGlobal[j] * 100) / g_VoteCount) : 0
			
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
	}
	else
	{
		client_print_color(0, print_team_blue, "%sVotación finalizada, el mapa ganador fue seleccionado al azar con^4 0^1 votos!", g_GlobalPrefix, amx_nextmap);
		
		MapChooser_ResetAllData();

		g_SelectedNextMap = 1;

		ClearSyncHud(0, g_HUD_Vote);

		g_ChangeMapTime = 10;

		ExecChangeTimeleft();
	}
}

public OnTaskVoteNextMap__FIX()
{
	new sMenu[400];
	new iMenuKeys;
	new iLen;
	new i;

	iLen = formatex(sMenu, charsmax(sMenu), "\yElige el próximo mapa: ¡VOTÁ AHORA!^n^n");

	for(i = 0; i < g_MapVoteNum; ++i)
	{
		iMenuKeys |= (1<<i);
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %a^n", i+1, ArrayGetStringHandle(g_Array_MapName, g_SVM_MapInMenu__FuckingFIX[i]));
	}

	iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n^n\d(SEGUNDA VOTACIÓN)");

	for(i = 0; i < (MAX_SELECTMAPS + 2); ++i)
		g_VoteCountGlobal[i] = 0;

	g_VoteCount = 0;
	g_VoteMapHasStarted = 1;
	g_SVM_MapSecondRound = 1;

	show_menu(0, iMenuKeys, sMenu, 15, "VoteMapFIX_Menu");
	set_task(15.1, "OnTaskCheckVotesMapFIX");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_default, "%sEs momento de elegir el próximo mapa (SEGUNDA VOTACIÓN)!", g_GlobalPrefix);
	
	ClearSyncHud(0, g_HUD_Alert);
	OnTask_MapChooserHudVote();
}

public menu__CountVoteMapFIX(const id, const iKey)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new sMap[32];
	new iMapId = iKey;

	ArrayGetString(g_Array_MapName, g_SVM_MapInMenu__FuckingFIX[iMapId], sMap, 31);
	
	client_print_color(id, print_team_blue, "%s^1Has elegido el mapa^3 %s", g_GlobalPrefix, sMap);
	
	if(amx_vote_answers)
		client_print(0, print_console, "%n ha votado por el mapa: %s", id, sMap);

	++g_VoteCountGlobal[iMapId];
	++g_VoteCount;

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
			iResult = g_VoteCountGlobal[i];
		}

		if(g_VoteCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_VoteCountGlobal[i];
		}
	}

	client_print(0, print_console, "Resultados de la votacion:");
	
	for(i = 0; i < g_MapVoteNum; ++i)
	{
		ArrayGetString(g_Array_MapName, g_SVM_MapInMenu__FuckingFIX[i], sMap, 31);
		client_print(0, print_console, "Mapa: %s - Votos: %d - Porcentaje: %d%%", sMap, g_VoteCountGlobal[i], (g_VoteCount > 0) ? ((g_VoteCountGlobal[i] * 100) / g_VoteCount) : 0);
	}

	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;
		new iMapsId[MAX_SELECTMAPS + 1];

		iWinner = g_SVM_MapInMenu__FuckingFIX[iWinner];

		// Chequeo si hubo 2 o más mapas con los mismos votos
		for(i = 0; i <= g_MapVoteNum; ++i)
		{
			if(g_VoteCountGlobal[i] == iResult)
			{
				iMapsId[j] = g_SVM_MapInMenu__FuckingFIX[i];

				j++;
			}
		}

		if(j > 1) // Hubo más de 1 ganador (otra vez)
			iWinner = iMapsId[random_num(0, (j - 1))];

		ArrayGetString(g_Array_MapName, iWinner, sMap, 31);
		set_pcvar_string(pamx_nextmap, sMap);
		
		if(j > 1)
		{
			client_print_color(0, print_team_default, "%sNingún mapa superó^4 la mayoría de los votos^1 por segunda vez.", g_GlobalPrefix);
			client_print_color(0, print_team_blue, "%sEntre los más votados, el mapa ganador al azar es:^3 %s", g_GlobalPrefix, sMap);
		}
		else
			client_print_color(0, print_team_blue, "%sVotación finalizada, el mapa ganador es ^3%s ^1con^4 %d ^1/^4 %d ^1voto%s (%d%%)!", g_GlobalPrefix, sMap, iResult, g_VoteCount, (g_VoteCount != 1) ? "s" : "", (g_VoteCount > 0) ? ((iResult * 100) / g_VoteCount) : 0);
		
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
		client_print_color(0, print_team_blue, "%sNingún voto fue emitido, por lo tanto el mapa ganador al azar es:^3 %s", g_GlobalPrefix, amx_nextmap);
		
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

	if(!g_VoteCount)
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
		iMapId[i] = (g_SVM_MapSecondRound) ? g_SVM_MapInMenu__FuckingFIX[i] : g_SVM_MapInMenu[i];
		iVoteMaps[i] = g_VoteCountGlobal[i];
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

		iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "%a: %d voto%s (%d%%%%)", ArrayGetStringHandle(g_Array_MapName, iMapId[i]), iVoteMaps[i], (iVoteMaps[i] == 1) ? "" : "s", (g_VoteCount > 0) ? ((iVoteMaps[i] * 100) / g_VoteCount) : 0);
	}

	set_hudmessage(255, 255, 255, 0.75, 0.35, 0, 0.0, 0.3, 0.0, 0.0, -1);
	ShowSyncHudMsg(0, g_HUD_Vote, sHud);
}

public OnTaskChangeTimeLeft()
{
	g_NoMoreTime = 1;
	set_pcvar_float(mp_timelimit, 0.0);

	switch(g_ChangeMapType)
	{
		case CHANGEMAP_END_OF_ROUND:
		{
			client_cmd(0, "spk ^"%s^"", g_SOUND_ExtendTime);
			client_print_color(0, print_team_default, "%s^1El mapa cambiará al finalizar la ronda!", g_GlobalPrefix);
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
		
		set_task(2.0, "taskChangeMap", _, amx_nextmap, sizeof(amx_nextmap));
		
		client_print_color(0, print_team_blue, "%s^1El siguiente mapa será: ^3%s", g_GlobalPrefix, amx_nextmap);
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
	new iFile;
	new sLine[256];
	new sMap[64];
	
	g_RecentMapsCount = 0;

	/*copy(sLine, charsmax(sLine), "addons/amxmodx/configs/recentmaps/maps.txt");

	if(file_exists(sLine)) {
		iFile = fopen(sLine, "r");

		if(iFile) {
			while(!feof(iFile)) {
				fgets(iFile, sLine, charsmax(sLine));

				trim(sLine);

				if(!strlen(sLine)) {
					continue;
				}

				copy(g_RecentMaps[g_RecentMapsCount], charsmax(g_RecentMaps[]), sLine);

				if(++g_RecentMapsCount >= MAX_RECENT_MAPS) {
					break;
				}
			}

			fclose(iFile);
		}
	}*/
	
	if(file_exists(szFileName))
	{
		iFile = fopen(szFileName, "r");

		if(iFile)
		{
			new j;
			new iOk;

			while(!feof(iFile))
			{
				sLine[0] = EOS;
				sMap[0] = EOS;
				
				fgets(iFile, sLine, 255);
				parse(sLine, sMap, 63);

				mb_strtolower(sMap);
				
				if(sMap[0] != ';' && validMap(sMap) && !equali(sMap, g_CurrentMap) && !equali(sMap, g_LastMap) && (containi(sMap, g_CurrentMap) == -1) && (containi(sMap, g_LastMap) == -1))
				{
					iOk = 1;

					for(j = 0; j < MAX_RECENT_MAPS; ++j) {
						if(equali(sMap, g_RecentMaps[j])) {
							iOk = 0;
							break;
						}
					}

					if(!iOk) {
						continue;
					}

					ArrayPushString(g_Array_MapName, sMap);
				}
			}
			
			fclose(iFile);
		}
	}
}

stock validMap(mapname[]) {
	if(is_map_valid(mapname)) {
		return 1;
	}
	
	new iLen = strlen(mapname) - 4;
	
	if(iLen < 0) {
		return 0;
	}
	
	if(equali(mapname[iLen], ".bsp")) {
		mapname[iLen] = EOS;
		
		if(is_map_valid(mapname)) {
			return 1;
		}
	}
	
	return 0;
}

stock IsMapInMenu(const i)
{
	new j;
	for(j = 0; j < (MAX_SELECTMAPS + 1); ++j)
	{
		if(i == g_SVM_MapInMenu[j]) {
			return 1;
		}
	}

	if((ArraySize(g_Array_MapName) + MAX_RECENT_MAPS) > MAX_RECENT_MAPS) {
		new sMap[64];
		formatex(sMap, 63, "%a", ArrayGetStringHandle(g_Array_MapName, i));

		for(j = 0; j < MAX_RECENT_MAPS; ++j) {
			if(equali(sMap, g_RecentMaps[j])) {
				return 1;
			}
		}
	}
	
	return 0;
}