#if defined _mm_modchooser_included_
	#endinput
#endif
#define _mm_modchooser_included_

#include <amxmodx>
#include "mm_incs/defines"
#include "mm_incs/cvars"

#define MAX_SELECTMODS			5

new g_VoteModCount = 0;
new g_VoteModCountGlobal[MAX_SELECTMODS + 2];
new g_SVM_ModInMenu[MAX_SELECTMODS + 1];
new g_SVM_ModInMenu_SecondRound[MAX_SELECTMODS + 1];
new g_SVM_ModSecondRound = 0;
new g_ModVoteNum = 0;
new g_SelectedNextMod = 0;
new g_VoteModHasStarted = 0;

new g_VoteModId[MAX_USERS];

ModChooser_Init()
{
	ModChooser_ResetAllData();

	register_menu("VoteMod_Menu", KEYSMENU, "menu__CountVoteMod");
	register_menu("VoteModFIX_Menu", KEYSMENU, "menu__CountVoteModFIX");
}

ModChooser_ResetAllData()
{
	arrayset(g_VoteModCountGlobal, 0, (MAX_SELECTMODS + 2));

	arrayset(g_SVM_ModInMenu, -1, (MAX_SELECTMODS + 1));
	arrayset(g_SVM_ModInMenu_SecondRound, -1, (MAX_SELECTMODS + 1));

	g_VoteModCount = 0;
	g_SVM_ModSecondRound = 0;
	g_ModVoteNum = 0;
	g_VoteModHasStarted = 0;

	arrayset(g_VoteModId, 0, MAX_USERS);

	remove_task(TASK_VOTEMOD);
	remove_task(TASK_SHOWTIME);
}

ModChooser_ClientPutInServer(const id)
{
	g_VoteModId[id] = 0;
}

ModChooser_ClientDisconnected(const id)
{
	// Si votó y se fue antes de terminar la votacion, su voto es removido!
	if(g_VoteModId[id] && g_VoteModHasStarted)
	{
		--g_VoteModCountGlobal[g_VoteModId[id]-1];
		--g_VoteModCount;

		g_VoteModId[id] = 0;
	}
}

public OnTask_VoteNextMod()
{
	new sMenu[400];
	new iMenuKeys = 0;
	new iArraySizeMods = ArraySize(g_Array_Mods);
	new iMaxMods = (iArraySizeMods > MAX_SELECTMODS) ? MAX_SELECTMODS : (iArraySizeMods - 1);

	new iLen = formatex(sMenu, charsmax(sMenu), "\yElige el próximo modo: ¡VOTÁ AHORA!^n^n");

	g_ModVoteNum = 0;
	for(new iRandom, aData[ArrayMods_e]; g_ModVoteNum < iMaxMods; ++g_ModVoteNum)
	{
		if(g_SVM_ModInMenu[g_ModVoteNum] == -1) // Si no existe nominaciones, elegir al azar una opcion
		{
			do {
				iRandom = random(iArraySizeMods);
			} while(IsModInMenu(iRandom) || g_iCurrentMod == iRandom); // La opcion está en el menú o es el modo actual

			g_SVM_ModInMenu[g_ModVoteNum] = iRandom;
		}

		iMenuKeys |= (1<<g_ModVoteNum);

		ArrayGetArray(g_Array_Mods, g_SVM_ModInMenu[g_ModVoteNum], aData);
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %s^n", (g_ModVoteNum + 1), aData[ModName]);
	}

	arrayset(g_VoteModCountGlobal, 0, (MAX_SELECTMODS + 2));
	g_VoteModCount = 0;
	g_VoteModHasStarted = 1;
	g_SVM_ModSecondRound = 0;

	if(IsAvailableExtendMap())
	{
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r%d.\w Extender\y %s\w %dm^n", iMaxMods + 1, g_szCurrentMod, g_bCvar_amx_extendmap_step);
		iMenuKeys |= (1<<iMaxMods);

		g_SVM_ModInMenu[g_ModVoteNum] = g_ModVoteNum;
	}

	iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r%d.\y Seleccionar al azar", ((iMaxMods + 2) > 9) ? 0 : (iMaxMods + 2));
	iMenuKeys |= (1<<iMaxMods + 1);
	
	show_menu(0, iMenuKeys, sMenu, 15, "VoteMod_Menu");
	set_task(15.1, "OnTaskCheckVoteMod");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_default, "%s^1 Es momento de elegir el próximo modo!", g_GlobalPrefix);
	
	ClearSyncHud(0, g_HUD_Vote);
	OnTask_ModChooserHudVote();
}

public menu__CountVoteMod(const id, const iKey)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new iModId = iKey;
	
	if(iKey == MAX_SELECTMODS)
	{
		client_print_color(id, print_team_blue, "%s^1 Has elegido extender el modo^4 %s^3 %d minutos más", g_GlobalPrefix, g_szCurrentMod, g_bCvar_amx_extendmap_step);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por: extender el modo %s %d minutos más", id, g_szCurrentMod, g_bCvar_amx_extendmap_step);
	}
	else if(iKey < MAX_SELECTMODS)
	{
		new aData[ArrayMods_e];
		ArrayGetArray(g_Array_Mods, g_SVM_ModInMenu[iModId], aData);
		
		client_print_color(id, print_team_blue, "%s^1 Has elegido el modo^3 %s", g_GlobalPrefix, aData[ModName]);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por el modo: %s", id, aData[ModName]);
	}
	else
	{
		iModId = random_num(0, IsAvailableExtendMap() ? g_ModVoteNum : (g_ModVoteNum - 1));

		// Extender modo..
		if(iModId == g_ModVoteNum)
		{
			client_print_color(id, print_team_blue, "%s^1 Has elegido extender el modo^4 %s^3 %d minutos más", g_GlobalPrefix, g_szCurrentMod, g_bCvar_amx_extendmap_step);
		
			if(g_bCvar_amx_vote_answers)
				client_print(0, print_console, "%n ha votado por: extender el modo %s %d minutos más [ELECCIÓN ALEATORIA]", id, g_szCurrentMod, g_bCvar_amx_extendmap_step);
		}
		else
		{
			new aData[ArrayMods_e];
			ArrayGetArray(g_Array_Mods, g_SVM_ModInMenu[iModId], aData);

			client_print_color(id, print_team_blue, "%sHas elegido el modo^3 %s^4 [ELECCIÓN ALEATORIA]", g_GlobalPrefix, aData[ModName]);
			
			if(g_bCvar_amx_vote_answers)
				client_print(0, print_console, "%n ha votado por el modo: %s [ELECCIÓN ALEATORIA]", id, aData[ModName]);
		}
	}
	
	++g_VoteModCountGlobal[iModId];
	++g_VoteModCount;

	g_VoteModId[id] = iModId+1;
}

public OnTaskCheckVoteMod()
{
	new iWinner = 0;
	new iFirst = 0;
	new iResult = 0;
	new i;
	new aData[ArrayMods_e];
	
	for(i = 0; i <= g_ModVoteNum; ++i)
	{
		if(!iFirst)
		{
			iWinner = i;
			iFirst = 1;
			iResult = g_VoteModCountGlobal[i];
		}

		if(g_VoteModCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_VoteModCountGlobal[i];
		}
	}

	client_print(0, print_console, "Resultados de la votacion:");
	
	for(i = 0; i < g_ModVoteNum; ++i)
	{
		ArrayGetArray(g_Array_Mods, g_SVM_ModInMenu[i], aData);
		client_print(0, print_console, "Modo: %s - Votos: %d - Porcentaje: %d%%", aData[ModName], g_VoteModCountGlobal[i], (g_VoteModCount > 0) ? ((g_VoteModCountGlobal[i] * 100) / g_VoteModCount) : 0);
	}
	
	client_print(0, print_console, "Extender %s: %dm - Votos: %d - Porcentaje: %d%%", g_szCurrentMod, g_bCvar_amx_extendmap_step, g_VoteModCountGlobal[MAX_SELECTMODS], (g_VoteModCount > 0) ? ((g_VoteModCountGlobal[MAX_SELECTMODS] * 100) / g_VoteModCount) : 0);
	
	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;

		// Chequeo si hubo 2 o más modos con los mismos votos
		for(i = 0; i <= g_ModVoteNum; ++i)
		{
			if(g_VoteModCountGlobal[i] == iResult)
			{
				(i == MAX_SELECTMODS)
					? (g_SVM_ModInMenu_SecondRound[j] = -1) // Opcion de extender..
					: (g_SVM_ModInMenu_SecondRound[j] = g_SVM_ModInMenu[i]);

				j++;
			}
		}

		// Hubo 2 o más..
		if(j > 1)
		{
			g_VoteModHasStarted = 0;
			g_SVM_ModSecondRound = 1;
			g_ShowTime = 5;
			g_ModVoteNum = j;

			client_print_color(0, print_team_blue, "%s^1 Hubo^4 %d modos^1 con^4 %d^1 voto%s cada uno, siguiente votación en^3 10 segundos^1!", g_GlobalPrefix, j, iResult, (iResult != 1) ? "s" : "");

			client_cmd(0, "spk ^"run officer(e40) voltage(e30) accelerating(s70) is required^"");

			set_task(5.0, "OnTask_AlertStartNextVote", TASK_SHOWTIME);
			set_task(10.0, "OnTask_VoteNextMod__FIX");

			return;
		}

		// Solo 1 ganador

		// Ganador, extender el modo
		if(iWinner == MAX_SELECTMODS)
		{
			ExtendTimeleft(g_bCvar_amx_extendmap_step);
			client_print_color(0, print_team_blue, "%s^1 El modo actual se extenderá^3 %d minutos más^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", g_GlobalPrefix, g_bCvar_amx_extendmap_step, iResult, g_VoteModCount, (g_VoteModCount != 1) ? "s" : "", (g_VoteModCount > 0) ? ((iResult * 100) / g_VoteModCount) : 0); // , (g_VoteModCount > 0) ? ((g_VoteModCountGlobal[MAX_SELECTMODS] * 100) / g_VoteModCount) : 0
			
			ModChooser_ResetAllData();
			return;
		}

		MultiMod_SetNextMod(g_SVM_ModInMenu[iWinner]);

		ArrayGetArray(g_Array_Mods, g_SVM_ModInMenu[iWinner], aData);
		client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el modo ganador es ^3%s ^1con^4 %d ^1/^4 %d ^1voto%s (%d%%)!", g_GlobalPrefix, aData[ModName], iResult, g_VoteModCount, (g_VoteModCount != 1) ? "s" : "", (g_VoteModCount > 0) ? ((iResult * 100) / g_VoteModCount) : 0); // , (g_VoteModCount > 0) ? ((g_VoteModCountGlobal[j] * 100) / g_VoteModCount) : 0
		
		ModChooser_ResetAllData();

		g_SelectedNextMod = 1;

		MapChooser_InitNextVoteMap(60);
	}
	else
	{
		if(g_IsRtv)
		{
			g_IsRtv = 0;
			g_VoteRtvResult = 0;
			g_VoteCountRtv = 0;
			g_IsVotingRtv = 0;
			g_VoteModHasStarted = 0;
			
			ModChooser_ResetAllData();
			
			client_print_color(0, print_team_red, "%s^3 (RTV) La votación solicitada por los usuarios no tuvo exito!", g_GlobalPrefix);
			return;
		}

		do {
			iWinner = random(g_ModVoteNum);
		} while(iWinner == MAX_SELECTMODS);

		MultiMod_SetNextMod(g_SVM_ModInMenu[iWinner]);

		ArrayGetArray(g_Array_Mods, g_SVM_ModInMenu[iWinner], aData);
		client_print_color(0, print_team_blue, "%s^1 Ningún voto fue emitido, por lo tanto el modo ganador al azar es:^3 %s", g_GlobalPrefix, aData[ModName]);

		ModChooser_ResetAllData();

		g_SelectedNextMod = 1;

		MapChooser_InitNextVoteMap(60);
	}
}

public OnTask_VoteNextMod__FIX()
{
	new sMenu[400];
	new iMenuKeys;

	new iLen = formatex(sMenu, charsmax(sMenu), "\yElige el próximo modo: ¡VOTÁ AHORA!^n^n");

	for(new i = 0, aData[ArrayMods_e]; i < g_ModVoteNum; ++i)
	{
		iMenuKeys |= (1<<i);

		if(g_SVM_ModInMenu_SecondRound[i] == -1)
		{
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w Extender\y %s\w %dm^n", i+1, g_szCurrentMod, g_bCvar_amx_extendmap_step);
			continue;
		}

		ArrayGetArray(g_Array_Mods, g_SVM_ModInMenu_SecondRound[i], aData);
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %s^n", i+1, aData[ModName]);
	}

	iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n^n\d(SEGUNDA VOTACIÓN)");

	arrayset(g_VoteModCountGlobal, 0, (MAX_SELECTMODS + 2));
	g_VoteModCount = 0;
	g_VoteModHasStarted = 1;
	g_SVM_ModSecondRound = 1;

	show_menu(0, iMenuKeys, sMenu, 15, "VoteModFIX_Menu");
	set_task(15.1, "OnTaskCheckVoteModFIX");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_default, "%s^1 Es momento de elegir el próximo modo (SEGUNDA VOTACIÓN)!", g_GlobalPrefix);
	
	ClearSyncHud(0, g_HUD_Vote);
	OnTask_ModChooserHudVote();
}

public menu__CountVoteModFIX(const id, const iKey)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new iModId = iKey;
	
	if(g_SVM_ModInMenu_SecondRound[iModId] == -1)
	{
		client_print_color(id, print_team_blue, "%s^1 Has elegido extender el modo^4 %s^3 %d minutos más", g_GlobalPrefix, g_szCurrentMod, g_bCvar_amx_extendmap_step);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por: extender el modo %s %d minutos más", id, g_szCurrentMod, g_bCvar_amx_extendmap_step);
	}
	else
	{
		new aData[ArrayMods_e];
		ArrayGetArray(g_Array_Mods, g_SVM_ModInMenu_SecondRound[iModId], aData);

		client_print_color(id, print_team_blue, "%s^1 Has elegido el modo^3 %s", g_GlobalPrefix, aData[ModName]);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por el modo: %s", id, aData[ModName]);
	}
	
	++g_VoteModCountGlobal[iModId];
	++g_VoteModCount;

	g_VoteModId[id] = iModId+1;
}

public OnTaskCheckVoteModFIX()
{
	new iWinner = 0;
	new iFirst = 0;
	new iResult = 0;
	new i;
	new aData[ArrayMods_e];

	for(i = 0; i < g_ModVoteNum; ++i)
	{
		if(!iFirst)
		{
			iWinner = i;
			iFirst = 1;
			iResult = g_VoteModCountGlobal[i];
		}

		if(g_VoteModCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_VoteModCountGlobal[i];
		}
	}

	client_print(0, print_console, "Resultados de la votacion:");
	
	for(i = 0; i < g_ModVoteNum; ++i)
	{
		if(g_SVM_ModInMenu_SecondRound[i] == -1)
		{
			client_print(0, print_console, "Extender modo: %d' - Votos: %d - Porcentaje: %d%%", g_bCvar_amx_extendmap_step, g_VoteModCountGlobal[i], (g_VoteModCount > 0) ? ((g_VoteModCountGlobal[i] * 100) / g_VoteModCount) : 0);
			continue;
		}

		ArrayGetArray(g_Array_Mods, g_SVM_ModInMenu_SecondRound[i], aData);
		client_print(0, print_console, "Modo: %s - Votos: %d - Porcentaje: %d%%^n", aData[ModName], g_VoteModCountGlobal[i], (g_VoteModCount > 0) ? ((g_VoteModCountGlobal[i] * 100) / g_VoteModCount) : 0);
	}

	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;
		new iModsIds[MAX_SELECTMODS + 1];

		iWinner = g_SVM_ModInMenu_SecondRound[iWinner];

		// Chequeo si hubo 2 o más modos con los mismos votos
		for(i = 0; i <= g_ModVoteNum; ++i)
		{
			if(g_VoteModCountGlobal[i] == iResult)
				iModsIds[j++] = g_SVM_ModInMenu_SecondRound[i];
		}

		// Hubo más de 1 ganador (otra vez)
		if(j > 1)
			iWinner = iModsIds[random_num(0, (j - 1))];

		// Ganador, extender el modo
		if(iWinner == -1)
		{
			ExtendTimeleft(g_bCvar_amx_extendmap_step);

			if(j > 1)
			{
				client_print_color(0, print_team_default, "%s^1 Ningún modo superó^4 la mayoría de los votos^1 por segunda vez.", g_GlobalPrefix);
				client_print_color(0, print_team_blue, "%s^1 Entre los más votados, el modo ganador al azar es:^3 Extender el modo %s.", g_GlobalPrefix, g_szCurrentMod);
				client_print_color(0, print_team_default, "%s^1 El modo actual se extenderá^3 %d minutos más", g_GlobalPrefix, g_bCvar_amx_extendmap_step);
			}
			else
				client_print_color(0, print_team_blue, "%s^1 El modo actual se extenderá^3 %d minutos más^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", g_GlobalPrefix, g_bCvar_amx_extendmap_step, iResult, g_VoteModCount, (g_VoteModCount != 1) ? "s" : "", (g_VoteModCount > 0) ? ((iResult * 100) / g_VoteModCount) : 0);

			ModChooser_ResetAllData();
			return;
		}

		ArrayGetArray(g_Array_Mods, iWinner, aData);
		
		if(j > 1)
		{
			client_print_color(0, print_team_default, "%s^1 Ningún modo superó^4 la mayoría de los votos^1 por segunda vez.", g_GlobalPrefix);
			client_print_color(0, print_team_blue, "%s^1 Entre los más votados, el modo ganador al azar es:^3 %s", g_GlobalPrefix, aData[ModName]);
		}
		else
			client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el modo ganador es^3 %s^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", g_GlobalPrefix, aData[ModName], iResult, g_VoteModCount, (g_VoteModCount != 1) ? "s" : "", (g_VoteModCount > 0) ? ((iResult * 100) / g_VoteModCount) : 0);
		
		ModChooser_ResetAllData();

		g_SelectedNextMod = 1;

		MapChooser_InitNextVoteMap(60);
	}
	else
	{
		if(g_IsRtv)
		{
			g_IsRtv = 0;
			g_VoteRtvResult = 0;
			g_VoteCountRtv = 0;
			g_IsVotingRtv = 0;
			g_VoteModHasStarted = 0;
			
			ModChooser_ResetAllData();
			
			client_print_color(0, print_team_red, "%s^3 La votación solicitada por los usuarios no tuvo exito!", g_GlobalPrefix);
			return;
		}

		do {
			iWinner = random(g_ModVoteNum);
		} while(g_SVM_ModInMenu_SecondRound[iWinner] == -1);

		MultiMod_SetNextMod(g_SVM_ModInMenu_SecondRound[iWinner]);

		ArrayGetArray(g_Array_Mods, g_SVM_ModInMenu_SecondRound[iWinner], aData);
		client_print_color(0, print_team_blue, "%s^1 Ningún voto fue emitido, por lo tanto el modo ganador al azar es:^3 %s", g_GlobalPrefix, aData[ModName]);

		ModChooser_ResetAllData();

		g_SelectedNextMod = 1;

		MapChooser_InitNextVoteMap(60);
	}
}

public OnTask_ModChooserHudVote()
{
	if(!g_VoteModHasStarted)
		return;

	set_task(0.1, "OnTask_ModChooserHudVote");

	if(!g_VoteModCount)
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
	new iModId[MAX_SELECTMODS + 2];
	new iVoteMods[MAX_SELECTMODS + 2];
	new iExtendMod[MAX_SELECTMODS + 2];
	new aData[ArrayMods_e];

	new sHud[256];

	iLoop = (g_SVM_ModSecondRound) ? g_ModVoteNum : (g_ModVoteNum + 1);
	
	for(i = 0; i < iLoop; ++i)
	{
		iModId[i] = (g_SVM_ModSecondRound) ? g_SVM_ModInMenu_SecondRound[i] : g_SVM_ModInMenu[i];
		iVoteMods[i] = g_VoteModCountGlobal[i];
		iExtendMod[i] = (i == MAX_SELECTMODS || iModId[i] == -1) ? 1 : 0;
	}

	// Ordenamiento por seleccion
	for(i = 0; i < g_ModVoteNum; ++i)
	{
		for(j = (i + 1); j < (g_ModVoteNum + 1); ++j)
		{
			if(iVoteMods[j] > iVoteMods[i])
			{
				iTemp = iVoteMods[j];
				iVoteMods[j] = iVoteMods[i];
				iVoteMods[i] = iTemp;

				iTemp = iModId[j];
				iModId[j] = iModId[i];
				iModId[i] = iTemp;

				iTemp = iExtendMod[j];
				iExtendMod[j] = iExtendMod[i];
				iExtendMod[i] = iTemp;
			}
		}
	}

	for(i = 0; i < iLoop; ++i)
	{
		if(!iVoteMods[i])
			continue;

		if(iNoFirst)
			iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "^n");

		iNoFirst = 1;

		if(iExtendMod[i])
			iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "Extender modo %dm: %d voto%s (%d%%%%)", g_bCvar_amx_extendmap_step, iVoteMods[i], (iVoteMods[i] == 1) ? "" : "s", (g_VoteModCount > 0) ? ((iVoteMods[i] * 100) / g_VoteModCount) : 0);
		else
		{
			ArrayGetArray(g_Array_Mods, iModId[i], aData);
			iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "%s: %d voto%s (%d%%%%)", aData[ModName], iVoteMods[i], (iVoteMods[i] == 1) ? "" : "s", (g_VoteModCount > 0) ? ((iVoteMods[i] * 100) / g_VoteModCount) : 0);
		}
	}

	set_hudmessage(255, 255, 255, 0.75, 0.35, 0, 0.0, 0.3, 0.0, 0.0, -1);
	ShowSyncHudMsg(0, g_HUD_Vote, sHud);
}

bool:IsAvailableExtendMap()
{
	return bool:((get_pcvar_float(g_pCvar_mp_timelimit) < float(g_bCvar_amx_extendmap_max)) && !g_IsRtv);
}

ExtendTimeleft(const iTime)
{
	set_pcvar_float(g_pCvar_mp_timelimit, (get_pcvar_float(g_pCvar_mp_timelimit) + float(iTime)));
}

bool:IsModInMenu(const i)
{
	for(new j = 0; j < (MAX_SELECTMODS + 1); ++j)
	{
		if(g_SVM_ModInMenu[j] == i)
			return true;
	}

	return false;
}