#if defined _mm_modchooser_included_
	#endinput
#endif
#define _mm_modchooser_included_

ModChooser_Init()
{
	register_menu("VoteMod_Menu", KEYSMENU, "menu__CountVoteMod");
	register_menu("VoteModFIX_Menu", KEYSMENU, "menu__CountVoteModFIX");

	ModChooser_ResetAllData();
}

ModChooser_ResetAllData()
{
	g_bVoteInProgress = false;

	g_iModVoteNum = 0;
	g_iVoteModCount = 0;

	arrayset(g_iVoteModCountGlobal, 0, MAX_SELECTMODS);
	arrayset(g_iSVM_ModInMenu, -1, MAX_SELECTMODS);
	arrayset(g_iSVM_ModInMenu_SecondRound, -1, MAX_SELECTMODS);

	g_bSVM_ModSecondRound = false;
	g_bSelectedNextMod = false;
	g_bVoteModHasStarted = false;

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
	if(g_VoteModId[id] && g_bVoteModHasStarted)
	{
		--g_iVoteModCountGlobal[g_VoteModId[id]-1];
		--g_iVoteModCount;

		g_VoteModId[id] = 0;
	}
}

public OnTask_VoteNextMod()
{
	new sMenu[400];
	new iMenuKeys = 0;
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);
	new iMaxMods = min(g_GlobalConfigs[ModsInMenu], (iArraySizeMods - 1));

	new iLen = formatex(sMenu, charsmax(sMenu), "\yElige el próximo modo: ¡VOTÁ AHORA!^n^n");

	g_iModVoteNum = 0;
	for(new iRandom, iModId, iArraySizeNoms, aData[ArrayMods_e]; g_iModVoteNum < iMaxMods; ++g_iModVoteNum)
	{
		do {
			if((iArraySizeNoms = ArraySize(g_Array_Nominations)))
			{
				iRandom = random(iArraySizeNoms);
				iModId = ArrayGetCell(g_Array_Nominations, iRandom);
				ArrayDeleteItem(g_Array_Nominations, iRandom);
			}
			else
				iModId = random(iArraySizeMods);
		} while(IsModInMenu(iModId) || g_iCurrentMod == iModId); // La opcion está en el menú o es el modo actual

		g_iSVM_ModInMenu[g_iModVoteNum] = iModId;

		ArrayGetArray(g_GlobalConfigs[Mods], g_iSVM_ModInMenu[g_iModVoteNum], aData);
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %s^n", (g_iModVoteNum + 1), aData[ModName]);

		iMenuKeys |= (1<<g_iModVoteNum);
	}

	arrayset(g_iVoteModCountGlobal, 0, MAX_SELECTMODS);
	g_iVoteModCount = 0;
	g_bVoteModHasStarted = true;
	g_bSVM_ModSecondRound = false;

	if(IsAvailableExtendMap())
	{
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r%d.\w Extender\y %s\w %dm^n", iMaxMods + 1, g_szCurrentMod, g_bCvar_amx_extendmap_step);
		iMenuKeys |= (1<<iMaxMods);

		g_iSVM_ModInMenu[g_iModVoteNum] = OPTION_EXTEND_MOD;
	}

	iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r%d.\y Seleccionar al azar", ((iMaxMods + 2) > 9) ? 0 : (iMaxMods + 2));
	iMenuKeys |= (1<<iMaxMods + 1);
	
	show_menu(0, iMenuKeys, sMenu, 15, "VoteMod_Menu");
	set_task(15.1, "OnTaskCheckVoteMod");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_default, "%s^1 Es momento de elegir el próximo modo!", g_GlobalConfigs[ChatPrefix]);
	
	ClearSyncHud(0, g_Hud_Vote);
	OnTask_ModChooserHudVote();
}

public menu__CountVoteMod(const id, const iKey)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new iModId = iKey;
	
	if(iKey == g_iModVoteNum)
	{
		client_print_color(id, print_team_blue, "%s^1 Has elegido extender el modo^4 %s^3 %d minutos más", g_GlobalConfigs[ChatPrefix], g_szCurrentMod, g_bCvar_amx_extendmap_step);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por: extender el modo %s %d minutos más", id, g_szCurrentMod, g_bCvar_amx_extendmap_step);
	}
	else if(iKey < g_iModVoteNum)
	{
		new aData[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_iSVM_ModInMenu[iModId], aData);
		
		client_print_color(id, print_team_blue, "%s^1 Has elegido el modo^3 %s", g_GlobalConfigs[ChatPrefix], aData[ModName]);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por el modo: %s", id, aData[ModName]);
	}
	else
	{
		iModId = random_num(0, IsAvailableExtendMap() ? g_iModVoteNum : (g_iModVoteNum - 1));

		// Extender modo..
		if(iModId == g_iModVoteNum)
		{
			client_print_color(id, print_team_blue, "%s^1 Has elegido extender el modo^4 %s^3 %d minutos más", g_GlobalConfigs[ChatPrefix], g_szCurrentMod, g_bCvar_amx_extendmap_step);
		
			if(g_bCvar_amx_vote_answers)
				client_print(0, print_console, "%n ha votado por: extender el modo %s %d minutos más [ELECCIÓN ALEATORIA]", id, g_szCurrentMod, g_bCvar_amx_extendmap_step);
		}
		else
		{
			new aData[ArrayMods_e];
			ArrayGetArray(g_GlobalConfigs[Mods], g_iSVM_ModInMenu[iModId], aData);

			client_print_color(id, print_team_blue, "%s^1 Has elegido el modo^3 %s^4 [ELECCIÓN ALEATORIA]", g_GlobalConfigs[ChatPrefix], aData[ModName]);
			
			if(g_bCvar_amx_vote_answers)
				client_print(0, print_console, "%n ha votado por el modo: %s [ELECCIÓN ALEATORIA]", id, aData[ModName]);
		}
	}
	
	++g_iVoteModCountGlobal[iModId];
	++g_iVoteModCount;

	g_VoteModId[id] = iModId+1;
}

public OnTaskCheckVoteMod()
{
	new aData[ArrayMods_e];
	new iWinner = 0;
	new iResult = -1;
	new i;

	client_print(0, print_console, "Resultados de la votacion:");

	for(i = 0; i <= g_iModVoteNum; ++i)
	{
		if(g_iVoteModCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_iVoteModCountGlobal[i];
		}

		if(i < g_iModVoteNum)
		{
			ArrayGetArray(g_GlobalConfigs[Mods], g_iSVM_ModInMenu[i], aData);
			client_print(0, print_console, "Modo: %s - Votos: %d - Porcentaje: %d%%", aData[ModName], g_iVoteModCountGlobal[i], UTIL_GetPercent(g_iVoteModCountGlobal[i], g_iVoteModCount));
		}
		else if(IsAvailableExtendMap())
			client_print(0, print_console, "Extender %s: %dm - Votos: %d - Porcentaje: %d%%", g_szCurrentMod, g_bCvar_amx_extendmap_step, g_iVoteModCountGlobal[i], UTIL_GetPercent(g_iVoteModCountGlobal[i], g_iVoteModCount));
	}

	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;

		// Chequeo si hubo 2 o más modos con los mismos votos
		for(i = 0; i <= g_iModVoteNum; ++i)
		{
			if(g_iVoteModCountGlobal[i] == iResult)
			{
				(i == g_iModVoteNum)
					? (g_iSVM_ModInMenu_SecondRound[j] = OPTION_EXTEND_MOD) // Opcion de extender..
					: (g_iSVM_ModInMenu_SecondRound[j] = g_iSVM_ModInMenu[i]);

				j++;
			}
		}

		// Hubo 2 o más..
		if(j > 1)
		{
			g_bVoteInProgress = true;
			
			g_bVoteModHasStarted = false;
			g_bSVM_ModSecondRound = true;
			g_iModVoteNum = j;

			client_print_color(0, print_team_blue, "%s^1 Hubo^4 %d modos^1 con^4 %d^1 voto%s cada uno, siguiente votación en^3 10 segundos^1!", g_GlobalConfigs[ChatPrefix], j, iResult, (iResult != 1) ? "s" : "");

			client_cmd(0, "spk ^"run officer(e40) voltage(e30) accelerating(s70) is required^"");

			set_task(10.0, "OnTask_VoteNextMod__FIX");

			SetAlertStartNextVote(5.0, 5);
			return;
		}

		// Solo 1 ganador

		// Ganador, extender el modo
		if(iWinner == g_iModVoteNum)
		{
			ExtendTimeleft(g_bCvar_amx_extendmap_step);
			client_print_color(0, print_team_blue, "%s^1 El modo actual se extenderá^3 %d minutos más^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", g_GlobalConfigs[ChatPrefix], g_bCvar_amx_extendmap_step, iResult, g_iVoteModCount, (g_iVoteModCount != 1) ? "s" : "", UTIL_GetPercent(iResult, g_iVoteModCount));
			
			ModChooser_ResetAllData();
			Nominations_ResetAllData();
			return;
		}

		g_iNextSelectMod = g_iSVM_ModInMenu[iWinner];
		MultiMod_SetNextMod(g_iNextSelectMod);

		ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aData);
		client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el modo ganador es ^3%s ^1con^4 %d ^1/^4 %d ^1voto%s (%d%%)!", g_GlobalConfigs[ChatPrefix], aData[ModName], iResult, g_iVoteModCount, (g_iVoteModCount != 1) ? "s" : "", UTIL_GetPercent(iResult, g_iVoteModCount));
		
		ModChooser_ResetAllData();
		Nominations_ResetAllData();

		g_bSelectedNextMod = true;

		MapChooser_InitNextVoteMap(40);
	}
	else
	{
		if(g_bIsRockTheVote)
		{
			RockTheVote_ResetAllData();
			ModChooser_ResetAllData();
			Nominations_ResetAllData();
			
			client_print_color(0, print_team_red, "%s^3 (RTV) La votación solicitada por los usuarios no tuvo exito!", g_GlobalConfigs[ChatPrefix]);
			return;
		}

		do {
			iWinner = random(g_iModVoteNum);
		} while(iWinner == g_iModVoteNum);

		g_iNextSelectMod = g_iSVM_ModInMenu[iWinner];
		MultiMod_SetNextMod(g_iNextSelectMod);

		ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aData);
		client_print_color(0, print_team_blue, "%s^1 Ningún voto fue emitido, por lo tanto el modo ganador al azar es:^3 %s", g_GlobalConfigs[ChatPrefix], aData[ModName]);

		ModChooser_ResetAllData();
		Nominations_ResetAllData();

		g_bSelectedNextMod = true;

		MapChooser_InitNextVoteMap(40);
	}
}

public OnTask_VoteNextMod__FIX()
{
	new sMenu[400];
	new iMenuKeys;

	new iLen = formatex(sMenu, charsmax(sMenu), "\yElige el próximo modo: ¡VOTÁ AHORA!^n^n");

	for(new i = 0, aData[ArrayMods_e]; i < g_iModVoteNum; ++i)
	{
		iMenuKeys |= (1<<i);

		if(g_iSVM_ModInMenu_SecondRound[i] == OPTION_EXTEND_MOD)
		{
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w Extender\y %s\w %dm^n", i+1, g_szCurrentMod, g_bCvar_amx_extendmap_step);
			continue;
		}

		ArrayGetArray(g_GlobalConfigs[Mods], g_iSVM_ModInMenu_SecondRound[i], aData);
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %s^n", i+1, aData[ModName]);
	}

	iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n^n\d(SEGUNDA VOTACIÓN)");

	arrayset(g_iVoteModCountGlobal, 0, MAX_SELECTMODS);
	g_iVoteModCount = 0;
	g_bVoteModHasStarted = true;
	g_bSVM_ModSecondRound = true;

	show_menu(0, iMenuKeys, sMenu, 15, "VoteModFIX_Menu");
	set_task(15.1, "OnTaskCheckVoteModFIX");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_default, "%s^1 Es momento de elegir el próximo modo (SEGUNDA VOTACIÓN)!", g_GlobalConfigs[ChatPrefix]);
	
	ClearSyncHud(0, g_Hud_Vote);
	OnTask_ModChooserHudVote();
}

public menu__CountVoteModFIX(const id, const iKey)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new iModId = iKey;
	
	if(g_iSVM_ModInMenu_SecondRound[iModId] == OPTION_EXTEND_MOD)
	{
		client_print_color(id, print_team_blue, "%s^1 Has elegido extender el modo^4 %s^3 %d minutos más", g_GlobalConfigs[ChatPrefix], g_szCurrentMod, g_bCvar_amx_extendmap_step);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por: extender el modo %s %d minutos más", id, g_szCurrentMod, g_bCvar_amx_extendmap_step);
	}
	else
	{
		new aData[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_iSVM_ModInMenu_SecondRound[iModId], aData);

		client_print_color(id, print_team_blue, "%s^1 Has elegido el modo^3 %s", g_GlobalConfigs[ChatPrefix], aData[ModName]);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por el modo: %s", id, aData[ModName]);
	}
	
	++g_iVoteModCountGlobal[iModId];
	++g_iVoteModCount;

	g_VoteModId[id] = iModId+1;
}

public OnTaskCheckVoteModFIX()
{
	new aData[ArrayMods_e];
	new iWinner = 0;
	new iResult = -1;
	new i;

	client_print(0, print_console, "Resultados de la votacion:");

	for(i = 0; i < g_iModVoteNum; ++i)
	{
		if(g_iVoteModCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_iVoteModCountGlobal[i];
		}

		if(g_iSVM_ModInMenu_SecondRound[i] == OPTION_EXTEND_MOD)
			client_print(0, print_console, "Extender modo: %d' - Votos: %d - Porcentaje: %d%%", g_bCvar_amx_extendmap_step, g_iVoteModCountGlobal[i], UTIL_GetPercent(g_iVoteModCountGlobal[i], g_iVoteModCount));
		else
		{
			ArrayGetArray(g_GlobalConfigs[Mods], g_iSVM_ModInMenu_SecondRound[i], aData);
			client_print(0, print_console, "Modo: %s - Votos: %d - Porcentaje: %d%%^n", aData[ModName], g_iVoteModCountGlobal[i], UTIL_GetPercent(g_iVoteModCountGlobal[i], g_iVoteModCount));
		}
	}

	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;
		new iModsIds[MAX_SELECTMODS];

		iWinner = g_iSVM_ModInMenu_SecondRound[iWinner];

		// Chequeo si hubo 2 o más modos con los mismos votos
		for(i = 0; i <= g_iModVoteNum; ++i)
		{
			if(g_iVoteModCountGlobal[i] == iResult)
				iModsIds[j++] = g_iSVM_ModInMenu_SecondRound[i];
		}

		// Hubo más de 1 ganador (otra vez)
		if(j > 1)
			iWinner = iModsIds[random_num(0, (j - 1))];

		// Ganador, extender el modo
		if(iWinner == OPTION_EXTEND_MOD)
		{
			ExtendTimeleft(g_bCvar_amx_extendmap_step);

			if(j > 1)
			{
				client_print_color(0, print_team_default, "%s^1 Ningún modo superó^4 la mayoría de los votos^1 por segunda vez.", g_GlobalConfigs[ChatPrefix]);
				client_print_color(0, print_team_blue, "%s^1 Entre los más votados, el modo ganador al azar es:^3 Extender el modo %s.", g_GlobalConfigs[ChatPrefix], g_szCurrentMod);
				client_print_color(0, print_team_default, "%s^1 El modo actual se extenderá^3 %d minutos más", g_GlobalConfigs[ChatPrefix], g_bCvar_amx_extendmap_step);
			}
			else
				client_print_color(0, print_team_blue, "%s^1 El modo actual se extenderá^3 %d minutos más^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", g_GlobalConfigs[ChatPrefix], g_bCvar_amx_extendmap_step, iResult, g_iVoteModCount, (g_iVoteModCount != 1) ? "s" : "", UTIL_GetPercent(iResult, g_iVoteModCount));

			ModChooser_ResetAllData();
			Nominations_ResetAllData();
			return;
		}

		g_iNextSelectMod = iWinner;
		MultiMod_SetNextMod(g_iNextSelectMod);

		ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aData);
		
		if(j > 1)
		{
			client_print_color(0, print_team_default, "%s^1 Ningún modo superó^4 la mayoría de los votos^1 por segunda vez.", g_GlobalConfigs[ChatPrefix]);
			client_print_color(0, print_team_blue, "%s^1 Entre los más votados, el modo ganador al azar es:^3 %s", g_GlobalConfigs[ChatPrefix], aData[ModName]);
		}
		else
			client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el modo ganador es^3 %s^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", g_GlobalConfigs[ChatPrefix], aData[ModName], iResult, g_iVoteModCount, (g_iVoteModCount != 1) ? "s" : "", UTIL_GetPercent(iResult, g_iVoteModCount));
		
		ModChooser_ResetAllData();
		Nominations_ResetAllData();

		g_bSelectedNextMod = true;

		MapChooser_InitNextVoteMap(40);
	}
	else
	{
		if(g_bIsRockTheVote)
		{
			RockTheVote_ResetAllData();
			ModChooser_ResetAllData();
			Nominations_ResetAllData();
			
			client_print_color(0, print_team_red, "%s^3 La votación solicitada por los usuarios no tuvo exito!", g_GlobalConfigs[ChatPrefix]);
			return;
		}

		do {
			iWinner = random(g_iModVoteNum);
		} while(g_iSVM_ModInMenu_SecondRound[iWinner] == OPTION_EXTEND_MOD);

		g_iNextSelectMod = g_iSVM_ModInMenu_SecondRound[iWinner];
		MultiMod_SetNextMod(g_iNextSelectMod);

		ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aData);
		client_print_color(0, print_team_blue, "%s^1 Ningún voto fue emitido, por lo tanto el modo ganador al azar es:^3 %s", g_GlobalConfigs[ChatPrefix], aData[ModName]);

		ModChooser_ResetAllData();
		Nominations_ResetAllData();

		g_bSelectedNextMod = true;

		MapChooser_InitNextVoteMap(40);
	}
}

public OnTask_ModChooserHudVote()
{
	if(!g_bVoteModHasStarted)
		return;

	set_task(0.1, "OnTask_ModChooserHudVote");

	if(!g_iVoteModCount)
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
	new iModId[MAX_SELECTMODS];
	new iVoteMods[MAX_SELECTMODS];
	new aData[ArrayMods_e];

	new sHud[256];

	iLoop = (g_bSVM_ModSecondRound) ? g_iModVoteNum : (g_iModVoteNum + 1);
	
	for(i = 0; i < iLoop; ++i)
	{
		iModId[i] = (g_bSVM_ModSecondRound) ? g_iSVM_ModInMenu_SecondRound[i] : g_iSVM_ModInMenu[i];
		iVoteMods[i] = g_iVoteModCountGlobal[i];
	}

	// Ordenamiento por seleccion
	for(i = 0; i < g_iModVoteNum; ++i)
	{
		for(j = (i + 1); j < (g_iModVoteNum + 1); ++j)
		{
			if(iVoteMods[j] > iVoteMods[i])
			{
				iTemp = iVoteMods[j];
				iVoteMods[j] = iVoteMods[i];
				iVoteMods[i] = iTemp;

				iTemp = iModId[j];
				iModId[j] = iModId[i];
				iModId[i] = iTemp;
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

		if(iModId[i] == OPTION_EXTEND_MOD)
			iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "Extender modo %dm: %d voto%s (%d%%%%)", g_bCvar_amx_extendmap_step, iVoteMods[i], (iVoteMods[i] == 1) ? "" : "s", UTIL_GetPercent(iVoteMods[i], g_iVoteModCount));
		else
		{
			ArrayGetArray(g_GlobalConfigs[Mods], iModId[i], aData);
			iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "%s: %d voto%s (%d%%%%)", aData[ModName], iVoteMods[i], (iVoteMods[i] == 1) ? "" : "s", UTIL_GetPercent(iVoteMods[i], g_iVoteModCount));
		}
	}

	set_hudmessage(255, 255, 255, 0.75, 0.35, 0, 0.0, 0.3, 0.0, 0.0, -1);
	ShowSyncHudMsg(0, g_Hud_Vote, sHud);
}

bool:IsAvailableExtendMap()
{
	return bool:((get_pcvar_float(g_pCvar_mp_timelimit) < float(g_bCvar_amx_extendmap_max)) && !g_bIsRockTheVote);
}

ExtendTimeleft(const iTime)
{
	set_pcvar_float(g_pCvar_mp_timelimit, (get_pcvar_float(g_pCvar_mp_timelimit) + float(iTime)));
}

bool:IsModInMenu(const i)
{
	for(new j = 0; j < MAX_SELECTMODS; ++j)
	{
		if(g_iSVM_ModInMenu[j] == i)
			return true;
	}

	return false;
}