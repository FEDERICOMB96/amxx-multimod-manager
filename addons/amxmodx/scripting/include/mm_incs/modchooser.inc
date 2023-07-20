#if defined _mm_modchooser_included_
	#endinput
#endif
#define _mm_modchooser_included_

ModChooser_Init()
{
	ModChooser_ResetAllData();
}

ModChooser_ResetAllData()
{
	g_bVoteInProgress = false;

	g_iModVoteNum = 0;
	g_iVoteModCount = 0;

	arrayset(g_iVoteModCountGlobal, 0, MAX_SELECTMODS);
	arrayset(g_iSVM_ModInMenu, -1, MAX_SELECTMODS);
	arrayset(g_iSVM_ModInMenu_SecondRound, 0, MAX_SELECTMODS);

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
	if(g_VoteModId[id] > 0 && g_bVoteModHasStarted)
	{
		--g_iVoteModCountGlobal[g_VoteModId[id]-1];
		--g_iVoteModCount;

		g_VoteModId[id] = 0;
	}
}

public OnTask_VoteNextMod()
{
	new iOffMods = UTIL_GetOffModsCount();
	new iRecentMods = Recent_CountRecentMods();
	
	new aMods[ArrayMods_e];
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);

	if(likely((iArraySizeMods - 1 - iOffMods) < 2)) // No cuento el modo actual (-1) y los modos desactivados (UTIL_GetOffModsCount)
	{
		g_iNextSelectMod = 0;
		MultiMod_SetNextMod(g_iNextSelectMod);

		ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);
		client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MODCHOO_NEXT", aMods[ModName]);
		
		ModChooser_ResetAllData();
		Nominations_ResetAllData();

		g_bSelectedNextMod = true;

		MapChooser_InitNextVoteMap(40);
		return;
	}
	else if(likely((iArraySizeMods - 1 - iOffMods) <= iRecentMods))
		iRecentMods = 0;

	new iMaxMods = min(g_GlobalConfigs[ModsInMenu], (iArraySizeMods - 1 - iOffMods - iRecentMods));

	g_Menu_ModChooser = menu_create(fmt("\y%L", LANG_PLAYER, "MM_SELECT_NEXT_MODE"), "menu__CountVoteMod");

	g_iModVoteNum = 0;
	for(new iRandom, iModId, iArraySizeNoms; g_iModVoteNum < iMaxMods; ++g_iModVoteNum)
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

			ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);
		} while(IsModInMenu(iModId) || g_iCurrentMod == iModId || UTIL_IsModOff(iModId) || (iRecentMods && Recent_IsRecentMod(aMods[ModName]))); // La opcion está en el menú, es el modo actual o se encuentra desactivado

		g_iSVM_ModInMenu[g_iModVoteNum] = iModId;
		menu_additem(g_Menu_ModChooser, fmt("\w%s%s", aMods[ModName], likely(g_iModVoteNum == (iMaxMods-1)) ? "^n" : ""), fmt("%d", g_iModVoteNum));
	}

	if(IsAvailableExtendMap())
	{
		menu_additem(g_Menu_ModChooser, fmt("\w%L^n", LANG_PLAYER, "MM_M_EXTEND_CURRENT_MODE", g_szCurrentMod, g_bCvar_amx_extendmap_step), fmt("%d", OPTION_EXTEND_MOD));
		g_iSVM_ModInMenu[g_iModVoteNum] = OPTION_EXTEND_MOD;
	}

	menu_additem(g_Menu_ModChooser, fmt("\y%L", LANG_PLAYER, "MM_SELECT_VOTE_RANDOM_ITEM"), "Seleccionar al azar");

	menu_setprop(g_Menu_ModChooser, MPROP_PERPAGE, 0);
	menu_setprop(g_Menu_ModChooser, MPROP_EXIT, MEXIT_NEVER);

	UTIL_MenuDisplayAll(g_Menu_ModChooser, _, 15);

	arrayset(g_VoteModId, -1, MAX_USERS);
	arrayset(g_iVoteModCountGlobal, 0, MAX_SELECTMODS);
	g_iVoteModCount = 0;
	g_bVoteModHasStarted = true;
	g_bSVM_ModSecondRound = false;

	set_task(15.1, "OnTaskCheckVoteMod");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_TIME_CHOOSE_MOD");
	
	ClearSyncHud(0, g_Hud_Vote);
	OnTask_ModChooserHudVote();

	ExecuteForward(g_Forward_StartVotemod, g_Forward_RetVal, false);
}

public menu__CountVoteMod(const id, const menuid, const item)
{
	CHECK_CONNECTED(id)
	CHECK_MENU_EXIT(item)
	CHECK_MENU_TIMEOUT(item)

	new iModId = item;

	new access, szInfo[64];
	menu_item_getinfo(menuid, item, access, szInfo, charsmax(szInfo));

	if(equali(szInfo, "Seleccionar al azar"))
	{
		iModId = random_num(0, IsAvailableExtendMap() ? g_iModVoteNum : (g_iModVoteNum - 1));

		// Extender modo..
		if(iModId == g_iModVoteNum)
		{
			client_print_color(id, print_team_blue, "%s^1 %L^4 [%L]", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MODCHOO_VOTED_EXT", g_szCurrentMod, g_bCvar_amx_extendmap_step, LANG_PLAYER, "MM_RANDOM");
		
			if(g_bCvar_amx_vote_answers)
				client_print(0, print_console, "%L [%L]", LANG_PLAYER, "MM_MODCHOO_PLAYER_VOTED_EXT", id, g_szCurrentMod, g_bCvar_amx_extendmap_step, LANG_PLAYER, "MM_RANDOM");
		}
		else
		{
			new aData[ArrayMods_e];
			ArrayGetArray(g_GlobalConfigs[Mods], g_iSVM_ModInMenu[iModId], aData);

			client_print_color(id, print_team_blue, "%s^1 %L^4 [%L]", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MODCHOO_VOTED_FOR", aData[ModName], LANG_PLAYER, "MM_RANDOM");
			
			if(g_bCvar_amx_vote_answers)
				client_print(0, print_console, "%L [%L]", LANG_PLAYER, "MM_MODCHOO_PLAYER_VOTED_FOR", id, aData[ModName], LANG_PLAYER, "MM_RANDOM");
		}
	}
	else if(str_to_num(szInfo) == OPTION_EXTEND_MOD)
	{
		client_print_color(id, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MODCHOO_VOTED_EXT", g_szCurrentMod, g_bCvar_amx_extendmap_step);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%L", LANG_PLAYER, "MM_MODCHOO_PLAYER_VOTED_EXT", id, g_szCurrentMod, g_bCvar_amx_extendmap_step);
	}
	else //if(iModId < g_iModVoteNum)
	{
		new aData[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_iSVM_ModInMenu[iModId], aData);
		
		client_print_color(id, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MODCHOO_VOTED_FOR", aData[ModName]);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%L", LANG_PLAYER, "MM_MODCHOO_PLAYER_VOTED_FOR", id, aData[ModName]);
	}

	++g_iVoteModCountGlobal[iModId];
	++g_iVoteModCount;

	g_VoteModId[id] = iModId+1;
	return PLUGIN_HANDLED;
}

public OnTaskCheckVoteMod()
{
	new aData[ArrayMods_e];
	new iWinner = 0;
	new iResult = 0;

	ResetModMenuAll();
	menu_destroy(g_Menu_ModChooser);

	ExecuteForward(g_Forward_EndVotemod, g_Forward_RetVal, false);

	client_print(0, print_console, "%L:", LANG_PLAYER, "MM_VOTE_RESULT");

	for(new i = 0; i <= g_iModVoteNum; ++i)
	{
		if(g_iVoteModCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_iVoteModCountGlobal[i];
		}

		if(i < g_iModVoteNum)
		{
			ArrayGetArray(g_GlobalConfigs[Mods], g_iSVM_ModInMenu[i], aData);
			client_print(0, print_console, "%L: %s - %L: %d - %L: %d%%", LANG_PLAYER, "MM_MODE", aData[ModName], LANG_PLAYER, "MM_VOTES", g_iVoteModCountGlobal[i], LANG_PLAYER, "MM_PERCENT", UTIL_GetPercent(g_iVoteModCountGlobal[i], g_iVoteModCount));
		}
		else if(IsAvailableExtendMap())
			client_print(0, print_console, "%L %s: %dm - %L: %d - %L: %d%%", LANG_PLAYER, "MM_EXT", g_szCurrentMod, g_bCvar_amx_extendmap_step, LANG_PLAYER, "MM_VOTES", g_iVoteModCountGlobal[i], LANG_PLAYER, "MM_PERCENT", UTIL_GetPercent(g_iVoteModCountGlobal[i], g_iVoteModCount));
	}

	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;

		// Chequeo si hubo 2 o más modos con los mismos votos
		for(new i = 0; i <= g_iModVoteNum; ++i)
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

			client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MODCHOO_NEED_SECOND_VOTE", j, iResult);

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
			client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MOD_CHO_FIN_EXT", g_bCvar_amx_extendmap_step, iResult, g_iVoteModCount, UTIL_GetPercent(iResult, g_iVoteModCount));
			
			ModChooser_ResetAllData();
			Nominations_ResetAllData();
			return;
		}

		g_iNextSelectMod = g_iSVM_ModInMenu[iWinner];
		MultiMod_SetNextMod(g_iNextSelectMod);

		ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aData);
		client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MOD_CHO_FIN_NEXT", aData[ModName], iResult, g_iVoteModCount, UTIL_GetPercent(iResult, g_iVoteModCount));
		
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
			
			client_print_color(0, print_team_red, "%s^3 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_RTV_CHO_NO_RESULT");
			return;
		}

		do {
			iWinner = random(g_iModVoteNum);
		} while(iWinner == g_iModVoteNum);

		g_iNextSelectMod = g_iSVM_ModInMenu[iWinner];
		MultiMod_SetNextMod(g_iNextSelectMod);

		ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aData);
		client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_CHO_NO_VOTES_MOD", aData[ModName]);

		ModChooser_ResetAllData();
		Nominations_ResetAllData();

		g_bSelectedNextMod = true;

		MapChooser_InitNextVoteMap(40);
	}
}

public OnTask_VoteNextMod__FIX()
{
	g_Menu_ModChooser = menu_create(fmt("\y%L", LANG_PLAYER, "MM_SELECT_NEXT_MODE"), "menu__CountVoteModFIX");

	for(new i = 0, aData[ArrayMods_e]; i < g_iModVoteNum; ++i)
	{
		if(g_iSVM_ModInMenu_SecondRound[i] == OPTION_EXTEND_MOD)
		{
			menu_additem(g_Menu_ModChooser, fmt("\w%L%s", LANG_PLAYER, "MM_M_EXTEND_CURRENT_MODE", g_szCurrentMod, g_bCvar_amx_extendmap_step, likely(i == (g_iModVoteNum-1)) ? fmt("^n^n\d(%L)", LANG_PLAYER, "MM_SECOND_VOTE") : ""));
			continue;
		}

		ArrayGetArray(g_GlobalConfigs[Mods], g_iSVM_ModInMenu_SecondRound[i], aData);
		menu_additem(g_Menu_ModChooser, fmt("\w%s%s", aData[ModName], likely(i == (g_iModVoteNum-1)) ? fmt("^n^n\d(%L)", LANG_PLAYER, "MM_SECOND_VOTE") : ""));
	}

	arrayset(g_VoteModId, -1, MAX_USERS);
	arrayset(g_iVoteModCountGlobal, 0, MAX_SELECTMODS);
	g_iVoteModCount = 0;
	g_bVoteModHasStarted = true;
	g_bSVM_ModSecondRound = true;

	menu_setprop(g_Menu_ModChooser, MPROP_PERPAGE, 0);
	menu_setprop(g_Menu_ModChooser, MPROP_EXIT, MEXIT_NEVER);

	UTIL_MenuDisplayAll(g_Menu_ModChooser, _, 15);

	set_task(15.1, "OnTask_CheckVoteModFIX");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_grey, "%s^1 %L^3 (%L)", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_TIME_CHOOSE_MOD", LANG_PLAYER, "MM_SECOND_VOTE");
	
	ClearSyncHud(0, g_Hud_Vote);
	OnTask_ModChooserHudVote();

	ExecuteForward(g_Forward_StartVotemod, g_Forward_RetVal, true);
}

public menu__CountVoteModFIX(const id, const menuid, const item)
{
	CHECK_CONNECTED(id)
	CHECK_MENU_EXIT(item)
	CHECK_MENU_TIMEOUT(item)

	new iModId = item;
	
	if(g_iSVM_ModInMenu_SecondRound[iModId] == OPTION_EXTEND_MOD)
	{
		client_print_color(id, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MODCHOO_VOTED_EXT", g_szCurrentMod, g_bCvar_amx_extendmap_step);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%L", LANG_PLAYER, "MM_MODCHOO_PLAYER_VOTED_EXT", id, g_szCurrentMod, g_bCvar_amx_extendmap_step);
	}
	else
	{
		new aData[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_iSVM_ModInMenu_SecondRound[iModId], aData);

		client_print_color(id, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MODCHOO_VOTED_FOR", aData[ModName]);
		
		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%L", LANG_PLAYER, "MM_MODCHOO_PLAYER_VOTED_FOR", id, aData[ModName]);
	}
	
	++g_iVoteModCountGlobal[iModId];
	++g_iVoteModCount;

	g_VoteModId[id] = iModId+1;
	return PLUGIN_HANDLED;
}

public OnTask_CheckVoteModFIX()
{
	new aData[ArrayMods_e];
	new iWinner = 0;
	new iResult = 0;

	ResetModMenuAll();
	menu_destroy(g_Menu_ModChooser);

	ExecuteForward(g_Forward_EndVotemod, g_Forward_RetVal, true);

	client_print(0, print_console, "%L:", LANG_PLAYER, "MM_VOTE_RESULT");

	for(new i = 0; i < g_iModVoteNum; ++i)
	{
		if(g_iVoteModCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_iVoteModCountGlobal[i];
		}

		if(g_iSVM_ModInMenu_SecondRound[i] == OPTION_EXTEND_MOD)
			client_print(0, print_console, "%L %s: %dm - %L: %d - %L: %d%%", LANG_PLAYER, "MM_EXT", g_szCurrentMod, g_bCvar_amx_extendmap_step, LANG_PLAYER, "MM_VOTES", g_iVoteModCountGlobal[i], LANG_PLAYER, "MM_PERCENT", UTIL_GetPercent(g_iVoteModCountGlobal[i], g_iVoteModCount));
		else
		{
			ArrayGetArray(g_GlobalConfigs[Mods], g_iSVM_ModInMenu_SecondRound[i], aData);
			client_print(0, print_console, "%L: %s - %L: %d - %L: %d%%", LANG_PLAYER, "MM_MODE", aData[ModName], LANG_PLAYER, "MM_VOTES", g_iVoteModCountGlobal[i], LANG_PLAYER, "MM_PERCENT", UTIL_GetPercent(g_iVoteModCountGlobal[i], g_iVoteModCount));
		}
	}

	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;
		new iModsIds[MAX_SELECTMODS];

		iWinner = g_iSVM_ModInMenu_SecondRound[iWinner];

		// Chequeo si hubo 2 o más modos con los mismos votos
		for(new i = 0; i <= g_iModVoteNum; ++i)
		{
			if(g_iVoteModCountGlobal[i] == iResult)
				iModsIds[j++] = g_iSVM_ModInMenu_SecondRound[i];
		}

		if(j > 1) // Hubo más de 1 ganador (otra vez)
			iWinner = iModsIds[random(j)];

		// Ganador, extender el modo
		if(iWinner == OPTION_EXTEND_MOD)
		{
			ExtendTimeleft(g_bCvar_amx_extendmap_step);

			if(j > 1)
			{
				client_print_color(0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MODCHOO_SECOND_ROUND_EXT_INFO1");
				client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MODCHOO_SECOND_ROUND_EXT_INFO2", g_szCurrentMod);
				client_print_color(0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MODCHOO_SECOND_ROUND_EXT_INFO3", g_bCvar_amx_extendmap_step);
			}
			else
				client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MOD_CHO_FIN_EXT", g_bCvar_amx_extendmap_step, iResult, g_iVoteModCount, UTIL_GetPercent(iResult, g_iVoteModCount));

			ModChooser_ResetAllData();
			Nominations_ResetAllData();
			return;
		}

		g_iNextSelectMod = iWinner;
		MultiMod_SetNextMod(g_iNextSelectMod);

		ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aData);
		
		if(j > 1)
		{
			client_print_color(0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MODCHOO_SECOND_ROUND_NEXT_INFO1");
			client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MODCHOO_SECOND_ROUND_NEXT_INFO2", aData[ModName]);
		}
		else
			client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MOD_CHO_FIN_NEXT", aData[ModName], iResult, g_iVoteModCount, UTIL_GetPercent(iResult, g_iVoteModCount));
		
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
			
			client_print_color(0, print_team_red, "%s^3 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_RTV_CHO_NO_RESULT");
			return;
		}

		do {
			iWinner = random(g_iModVoteNum);
		} while(g_iSVM_ModInMenu_SecondRound[iWinner] == OPTION_EXTEND_MOD);

		g_iNextSelectMod = g_iSVM_ModInMenu_SecondRound[iWinner];
		MultiMod_SetNextMod(g_iNextSelectMod);

		ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aData);
		client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_CHO_NO_VOTES_MOD", aData[ModName]);

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
		ShowSyncHudMsg(0, g_Hud_Vote, "%L", LANG_PLAYER, "MM_NO_VOTES_COUNT");
		return;
	}

	new i;
	new iLen = 0;
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

	UTIL_SimpleSortVotes(iVoteMods, iModId, g_iModVoteNum);

	for(i = 0; i < iLoop; ++i)
	{
		if(!iVoteMods[i])
			continue;

		if(iNoFirst)
			iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "^n");

		iNoFirst = 1;

		if(iModId[i] == OPTION_EXTEND_MOD)
			iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "%L %s %dm: %d %L (%d%%%%)", LANG_PLAYER, "MM_EXT", g_szCurrentMod, g_bCvar_amx_extendmap_step, iVoteMods[i], LANG_PLAYER, (iVoteMods[i] == 1) ? "MM_VOTE" : "MM_VOTES", UTIL_GetPercent(iVoteMods[i], g_iVoteModCount));
		else
		{
			ArrayGetArray(g_GlobalConfigs[Mods], iModId[i], aData);
			iLen += formatex(sHud[iLen], charsmax(sHud) - iLen, "%s: %d %L (%d%%%%)", aData[ModName], iVoteMods[i], LANG_PLAYER, (iVoteMods[i] == 1) ? "MM_VOTE" : "MM_VOTES", UTIL_GetPercent(iVoteMods[i], g_iVoteModCount));
		}
	}

	set_hudmessage(255, 255, 255, 0.75, 0.35, 0, 0.0, 0.3, 0.0, 0.0, -1);
	ShowSyncHudMsg(0, g_Hud_Vote, sHud);
}

bool:IsAvailableExtendMap()
{
	return bool:(likely((g_bCvar_mp_winlimit + g_bCvar_mp_maxrounds) == 0) && likely(get_pcvar_float(g_pCvar_mp_timelimit) < float(g_bCvar_amx_extendmap_max)) && !g_bIsRockTheVote);
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

bool:ResetModMenuAll()
{
	for(new i = 1; i < MAX_USERS; ++i) if(GetPlayerBit(g_bConnected, i) && likely(g_VoteModId[i] == -1))
	{
		menu_cancel(i);
		reset_menu(i);
	}

	return true;
}