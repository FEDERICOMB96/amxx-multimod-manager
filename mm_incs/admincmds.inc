#if defined _mm_admincmds_included_
	#endinput
#endif
#define _mm_admincmds_included_

AdminCmd_Init()
{
	register_clcmd("amx_multimod", "ClientCommand__AdminMultimod", g_GlobalConfigs[AdminFlags]);

	register_menu("SelectVoteMod_AdminMenu", KEYSMENU, "menu_SelectVoteMod");
	register_menu("VoteNextMod_AdminMenu", KEYSMENU, "menu_VoteNextMod");
	register_menu("SelectVoteMap_AdminMenu", KEYSMENU, "menu_SelectVoteMap");
	register_menu("VoteNextMap_AdminMenu", KEYSMENU, "menu_VoteNextMap");
}

AdminCmd_ResetUserData(const id)
{
	arrayset(g_Admin_VoteOptions[id], 0, MAX_ADMIN_VOTEOPTIONS);
	g_Admin_VoteSelectedOptions[id] = 0;
	g_Admin_VoteCurrentPage[id] = 0;
	g_Admin_SelectedMod[id] = g_iCurrentMod;
	g_Admin_SelectedMap[id] = 0;

	remove_task(id + TASK_VOTE_ADMIN);
}

AdminCmd_ClientPutInServer(const id)
{
	AdminCmd_ResetUserData(id);
}

AdminCmd_ClientDisconnected(const id)
{
	if(task_exists(id + TASK_VOTE_ADMIN) && g_Admin_VoteUserId == id)
	{
		remove_task(id + TASK_VOTE_ADMIN);

		client_print_color(0, print_team_blue, "%s^3 %n^1 se desconectó, por lo tanto, su votación fue anulada!", g_GlobalConfigs[ChatPrefix], id);
	}
	
	if(g_Admin_VoteUserId == id)
		g_Admin_VoteUserId = -1;

	AdminCmd_ResetUserData(id);
}

public ClientCommand__AdminMultimod(const id, const level, const cid)
{
	if(!GetPlayerBit(g_bConnected, id) || !cmd_access(id, level, cid, 1))
		return PLUGIN_HANDLED;

	CHECK_ADMINVOTEINPROGRESS(id)

	AdminCmd_ResetUserData(id);

	ShowMenu_AdminMultimod(id);
	return PLUGIN_HANDLED;
}

ShowMenu_AdminMultimod(const id)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iCurrentMod, aMods);

	new iMenu = menu_create(fmt("\yMULTIMOD MANAGER^n\dModo actual: [ %s ]\y\R", aMods[ModName]), "menu_AdminMultimod");

	ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);
	menu_additem(iMenu, fmt("Elegir un\y siguiente modo\r [ %s ]", aMods[ModName]));

	if(g_Admin_SelectedMap[id])
		menu_additem(iMenu, fmt("Elegir un\y siguiente mapa\r [ %a ]", ArrayGetStringHandle(aMods[Maps], g_Admin_SelectedMap[id]-1)));
	else
		menu_additem(iMenu, "Elegir un\y siguiente mapa\r [\d sin definir\r ]");

	if(g_Admin_SelectedMap[id])
		menu_additem(iMenu, fmt("Cambiar a Modo\y [ %s ]\w Mapa\r [ %a ]^n", aMods[ModName],  ArrayGetStringHandle(aMods[Maps], g_Admin_SelectedMap[id]-1)));
	else
		menu_additem(iMenu, fmt("Cambiar a Modo [ %s ] Mapa [ sin definir ]^n", aMods[ModName]), .callback=menu_makecallback("UTIL_DisableItem"));

	menu_additem(iMenu, "Administrar\y modos^n");

	menu_additem(iMenu, "Iniciar una\y votación de modos");
	menu_additem(iMenu, fmt("Iniciar una\y votación de mapas\r [ %s ]", aMods[ModName]));

	menu_setprop(iMenu, MPROP_EXITNAME, "Salir");
	menu_display(id, iMenu, 0);
}

public menu_AdminMultimod(const id, const menuid, const item)
{
	menu_destroy(menuid);

	if(!GetPlayerBit(g_bConnected, id) || item == MENU_EXIT)
		return;

	switch(item)
	{
		case 0: ShowMenu_SelectMod(id);
		case 1: ShowMenu_SelectMap(id);
		case 2: OnFunc_ChangeModAndMap(id);
		case 3: ShowMenu_ManageMods(id);
		case 4:
		{
			arrayset(g_Admin_VoteOptions[id], 0, MAX_ADMIN_VOTEOPTIONS);
			g_Admin_VoteSelectedOptions[id] = g_Admin_VoteCurrentPage[id] = 0;
			ShowMenu_Votemod(id);
		}
		case 5:
		{
			arrayset(g_Admin_VoteOptions[id], 0, MAX_ADMIN_VOTEOPTIONS);
			g_Admin_VoteSelectedOptions[id] = g_Admin_VoteCurrentPage[id] = 0;
			ShowMenu_Votemap(id);
		}
	}
}

ShowMenu_SelectMod(const id)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new iMenu = menu_create("\ySelecciona un Modo\r:\R", "menu_SelectMod");

	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);
	new aMods[ArrayMods_e];

	for(new iModId = 0; iModId < iArraySizeMods; ++iModId)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

		(g_iCurrentMod == iModId)
			? menu_additem(iMenu, fmt("%s \r(ACTUAL)", aMods[ModName]), .callback=menu_makecallback("UTIL_DisableItem"))
			: menu_additem(iMenu, fmt("%s%s", aMods[ModName], IsNominated(iModId) ? ((iModId == (g_SVM_MyNominate[id]-1)) ? "\y [TU NOMINASTE]" : "\y (NOMINADO)"): ""));
	}

	menu_setprop(iMenu, MPROP_NEXTNAME, "Siguiente");
	menu_setprop(iMenu, MPROP_BACKNAME, "Atrás");
	menu_setprop(iMenu, MPROP_EXITNAME, "Salir");

	menu_display(id, iMenu, 0);
}

public menu_SelectMod(const id, const menuid, const item)
{
	menu_destroy(menuid);

	if(!GetPlayerBit(g_bConnected, id) || item == MENU_EXIT)
		return;

	new iModId = item;
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	g_Admin_SelectedMod[id] = iModId;
	client_print_color(id, print_team_default, "%s^1 Has seleccionado el modo^4 %s^1!", g_GlobalConfigs[ChatPrefix], aMods[ModName]);
	client_print_color(id, print_team_default, "%s^1 Ahora puedes cambiar a un mapa del modo seleccionado o iniciar una votación!", g_GlobalConfigs[ChatPrefix]);

	ShowMenu_AdminMultimod(id);
}

ShowMenu_SelectMap(const id)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);

	new iArraySize = ArraySize(aMods[Maps]);
	if(iArraySize < 1)
	{
		client_print_color(id, print_team_default, "%s^1 No hay mapas disponibles para seleccionar!", g_GlobalConfigs[ChatPrefix]);
		return;
	}

	new iMenu = menu_create(fmt("\ySelecciona un Mapa\r:^n\dLista de mapas del modo [ %s ]\R", aMods[ModName]), "menu_SelectMap");

	for(new iMapId = 0; iMapId < iArraySize; ++iMapId)
		menu_additem(iMenu, fmt("%a", ArrayGetStringHandle(aMods[Maps], iMapId)));

	menu_setprop(iMenu, MPROP_NEXTNAME, "Siguiente");
	menu_setprop(iMenu, MPROP_BACKNAME, "Atrás");
	menu_setprop(iMenu, MPROP_EXITNAME, "Salir");

	menu_display(id, iMenu, 0);
}

public menu_SelectMap(const id, const menuid, const item)
{
	menu_destroy(menuid);

	if(!GetPlayerBit(g_bConnected, id) || item == MENU_EXIT)
		return;

	new iMapId = item;
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);

	g_Admin_SelectedMap[id] = iMapId+1;
	client_print_color(id, print_team_default, "%s^1 Has seleccionado el mapa^4 %a^1 del modo^3 %s^1!", g_GlobalConfigs[ChatPrefix], ArrayGetStringHandle(aMods[Maps], iMapId), aMods[ModName]);
	client_print_color(id, print_team_default, "%s^1 Ahora puedes cambiar al mapa seleccionado!", g_GlobalConfigs[ChatPrefix]);

	ShowMenu_AdminMultimod(id);
}

OnFunc_ChangeModAndMap(const id)
{
	if(g_Admin_SelectedMap[id])
	{
		MultiMod_SetNextMod(g_Admin_SelectedMod[id]);

		new aMods[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);

		new szNextMap[64];
		ArrayGetString(aMods[Maps], g_Admin_SelectedMap[id]-1, szNextMap, charsmax(szNextMap));

		message_begin(MSG_ALL, SVC_INTERMISSION);
		message_end();
		
		set_task(2.0, "taskChangeMap", _, szNextMap, sizeof(szNextMap));
		
		client_print_color(0, print_team_blue, "%s^3 %n^1 cambió el modo a^4 %s^1 y el mapa a^4 %s", g_GlobalConfigs[ChatPrefix], id, aMods[ModName], szNextMap);
	}
}

ShowMenu_ManageMods(const id)
{
	
}

ShowMenu_Votemod(const id)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new sMenu[512];
	new aMods[ArrayMods_e];
	new iArraySize = ArraySize(g_GlobalConfigs[Mods]);
	new iStartLoop = g_Admin_VoteCurrentPage[id] * 7;
	new iEndLoop = clamp(((g_Admin_VoteCurrentPage[id] + 1) * 7), 0, iArraySize);
	new iKeys = MENU_KEY_0;
	new j = 0;

	new iLen = formatex(sMenu, charsmax(sMenu), "\yVotación de Modos\r:\R\y%d/%d^n^n", (g_Admin_VoteCurrentPage[id] + 1), ((iArraySize / 7) + ((iArraySize % 7) ? 1 : 0)));
	
	for(new i = iStartLoop; i < iEndLoop; ++i)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], i, aMods);

		if(g_Admin_VoteSelectedOptions[id] == MAX_ADMIN_VOTEOPTIONS || IsOptionSelected(id, (g_Admin_VoteCurrentPage[id] * 7) + j))
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\d%d. %s%s^n", ++j, aMods[ModName], (g_iCurrentMod == i) ? " (ACTUAL)" : "");
		else
		{
			iKeys |= (1<<j);
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %s%s^n", ++j, aMods[ModName], (g_iCurrentMod == i) ? "\y (ACTUAL)" : "");
		}
	}
	
	if(g_Admin_VoteSelectedOptions[id] > 1)
	{
		iKeys |= MENU_KEY_8;
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r8.\w Comenzar la votación^n");
	}
	else
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\d8. Tenés que seleccionar al menos 2 modos (%d máx)^n", MAX_ADMIN_VOTEOPTIONS);
	
	if(iEndLoop != iArraySize)
	{
		iKeys |= MENU_KEY_9;
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r9.\w Siguiente^n\r0.\w %s^n", iStartLoop ? "Volver" : "Salir");
	}
	else
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r0.\w %s", iStartLoop ? "Volver" : "Salir");
	
	if(g_Admin_VoteSelectedOptions[id])
	{
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n^n\yModos seleccionados\r:^n");
		
		for(j = 0; j < g_Admin_VoteSelectedOptions[id]; ++j)
		{
			ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_VoteOptions[id][j], aMods);
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\w%s^n", aMods[ModName]);
		}
	}
	
	show_menu(id, iKeys, sMenu, -1, "SelectVoteMod_AdminMenu");
}

public menu_SelectVoteMod(const id, const key)
{
	if(!GetPlayerBit(g_bConnected, id))
		return PLUGIN_HANDLED;

	switch(key)
	{
		case 7:
		{
			CHECK_ADMINVOTEINPROGRESS(id)

			new sMenu[512];
			new iKeys;
			new iLen = formatex(sMenu, charsmax(sMenu), "\yVotación de\r:\w %n^n^n\yElige el próximo modo\r:\y ¡VOTÁ AHORA!^n^n", id);

			g_Admin_VoteUserId = id;
			g_Admin_VoteCountTotal = 0;
			arrayset(g_Admin_iVoteCountId, 0, MAX_ADMIN_VOTEOPTIONS);

			set_pcvar_float(g_pCvar_amx_last_voting, get_gametime() + 15.0 + 2.0 + float(g_bCvar_amx_vote_delay_mm));

			for(new i = 0, aMods[ArrayMods_e]; i < g_Admin_VoteSelectedOptions[id]; ++i)
			{
				ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_VoteOptions[id][i], aMods);
				iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %s^n", (i + 1), aMods[ModName]);
				
				iKeys |= (1<<i);
			}
			
			iKeys |= MENU_KEY_0;
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r0.\y Seleccionar al azar");
			
			show_menu(0, iKeys, sMenu, 15, "VoteNextMod_AdminMenu");
			
			remove_task(id + TASK_VOTE_ADMIN);
			set_task(15.1, "OnTask_CheckAdminVotemod", id + TASK_VOTE_ADMIN);
			
			client_print_color(0, print_team_blue, "%s^3 %n^1 comenzó una votación para el cambio de modo!", g_GlobalConfigs[ChatPrefix], id);
			return PLUGIN_HANDLED;
		}
		case 8: ++g_Admin_VoteCurrentPage[id];
		case 9: --g_Admin_VoteCurrentPage[id];
		default: g_Admin_VoteOptions[id][g_Admin_VoteSelectedOptions[id]++] = ((g_Admin_VoteCurrentPage[id] * 7) + key);
	}
	
	ShowMenu_Votemod(id);
	return PLUGIN_HANDLED;
}

public menu_VoteNextMod(const id, const iKey)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;
	
	if(task_exists(g_Admin_VoteUserId + TASK_VOTE_ADMIN))
	{
		new iModId = iKey;
		new bool:bRandom = false;
		new aMods[ArrayMods_e];

		if(iKey == 9)
		{
			bRandom = true;
			iModId = random(g_Admin_VoteSelectedOptions[g_Admin_VoteUserId]);
		}

		++g_Admin_iVoteCountId[iModId];
		++g_Admin_VoteCountTotal;

		ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_VoteOptions[g_Admin_VoteUserId][iModId], aMods);

		client_print_color(id, print_team_default, "%s^1 Has votado por la opción^4 #%d^1 (^4%s^1)!%s", 
			g_GlobalConfigs[ChatPrefix], (iModId + 1), aMods[ModName], (bRandom) ? "^3 [ELECCIÓN ALEATORIA]" : "");

		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por la opcion #%d (%s)%s", id, (iModId + 1), aMods[ModName], (bRandom) ? " [ELECCIÓN ALEATORIA]" : "");
	}
}

public OnTask_CheckAdminVotemod(const taskid)
{
	new id = (taskid - TASK_VOTE_ADMIN);

	if(!GetPlayerBit(g_bConnected, id))
		return;
	
	new iMaxVotes = -1;

	g_Admin_SelectedNextMod = -1;
	
	for(new i = 0; i < g_Admin_VoteSelectedOptions[id]; ++i)
	{
		if(g_Admin_iVoteCountId[i] > iMaxVotes)
		{
			iMaxVotes = g_Admin_iVoteCountId[i];
			g_Admin_SelectedNextMod = i+1;
		}
	}
	
	if(iMaxVotes)
	{
		new aMods[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_VoteOptions[id][g_Admin_SelectedNextMod-1], aMods);

		client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el modo ganador es^3 %s^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", 
			g_GlobalConfigs[ChatPrefix], aMods[ModName], iMaxVotes, g_Admin_VoteCountTotal, (g_Admin_VoteCountTotal != 1) ? "s" : "", UTIL_GetPercent(iMaxVotes, g_Admin_VoteCountTotal));
	}
	else
	{
		g_Admin_VoteUserId = -1;
		g_Admin_VoteCountTotal = 0;

		client_print_color(0, print_team_default, "%s^1 La votación para el cambio de modo no tuvo exito!", g_GlobalConfigs[ChatPrefix]);
	}

	arrayset(g_Admin_iVoteCountId, 0, MAX_ADMIN_VOTEOPTIONS);
}

ShowMenu_Votemap(const id)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);

	new iArraySize = ArraySize(aMods[Maps]);
	if(iArraySize < 1)
	{
		client_print_color(id, print_team_default, "%s^1 No hay mapas disponibles para seleccionar!", g_GlobalConfigs[ChatPrefix]);
		return;
	}

	new sMenu[512];
	new iStartLoop = g_Admin_VoteCurrentPage[id] * 7;
	new iEndLoop = clamp(((g_Admin_VoteCurrentPage[id] + 1) * 7), 0, iArraySize);
	new iKeys = MENU_KEY_0;
	new j = 0;

	new iLen = formatex(sMenu, charsmax(sMenu), "\yVotación de Mapas\r:^n\dLista de mapas del modo [ %s ]\R\y%d/%d^n^n", aMods[ModName], (g_Admin_VoteCurrentPage[id] + 1), ((iArraySize / 7) + ((iArraySize % 7) ? 1 : 0)));
	
	for(new i = iStartLoop; i < iEndLoop; ++i)
	{
		if(g_Admin_VoteSelectedOptions[id] == MAX_ADMIN_VOTEOPTIONS || IsOptionSelected(id, (g_Admin_VoteCurrentPage[id] * 7) + j))
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\d%d. %a^n", ++j, ArrayGetStringHandle(aMods[Maps], i));
		else
		{
			iKeys |= (1<<j);
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %a^n", ++j, ArrayGetStringHandle(aMods[Maps], i));
		}
	}
	
	if(g_Admin_VoteSelectedOptions[id] > 1)
	{
		iKeys |= MENU_KEY_8;
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r8.\w Comenzar la votación^n");
	}
	else
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\d8. Tenés que seleccionar al menos 2 mapas (%d máx)^n", MAX_ADMIN_VOTEOPTIONS);
	
	if(iEndLoop != iArraySize)
	{
		iKeys |= MENU_KEY_9;
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r9.\w Siguiente^n\r0.\w %s^n", iStartLoop ? "Volver" : "Salir");
	}
	else
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r0.\w %s", iStartLoop ? "Volver" : "Salir");
	
	if(g_Admin_VoteSelectedOptions[id])
	{
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n^n\yMapas seleccionados\r:^n");
		
		for(j = 0; j < g_Admin_VoteSelectedOptions[id]; ++j)
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\w%a^n", ArrayGetStringHandle(aMods[Maps], g_Admin_VoteOptions[id][j]));
	}
	
	show_menu(id, iKeys, sMenu, -1, "SelectVoteMap_AdminMenu");
}

public menu_SelectVoteMap(const id, const key)
{
	if(!GetPlayerBit(g_bConnected, id))
		return PLUGIN_HANDLED;

	switch(key)
	{
		case 7:
		{
			CHECK_ADMINVOTEINPROGRESS(id)
			
			new sMenu[512];
			new iKeys;
			new iLen = formatex(sMenu, charsmax(sMenu), "\yVotación de\r:\w %n^n^n\yElige el próximo mapa\r:\y ¡VOTÁ AHORA!^n^n", id);

			g_Admin_VoteUserId = id;
			g_Admin_VoteCountTotal = 0;
			arrayset(g_Admin_iVoteCountId, 0, MAX_ADMIN_VOTEOPTIONS);

			set_pcvar_float(g_pCvar_amx_last_voting, get_gametime() + 15.0 + 2.0 + float(g_bCvar_amx_vote_delay_mm));

			new aMods[ArrayMods_e];
			ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);

			for(new i = 0; i < g_Admin_VoteSelectedOptions[id]; ++i)
			{
				iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %a^n", (i + 1), ArrayGetStringHandle(aMods[Maps], g_Admin_VoteOptions[id][i]));
				
				iKeys |= (1<<i);
			}
			
			iKeys |= MENU_KEY_0;
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r0.\y Seleccionar al azar");
			
			show_menu(0, iKeys, sMenu, 15, "VoteNextMap_AdminMenu");
			
			remove_task(id + TASK_VOTE_ADMIN);
			set_task(15.1, "OnTask_CheckAdminVotemap", id + TASK_VOTE_ADMIN);
			
			client_print_color(0, print_team_blue, "%s^3 %n^1 comenzó una votación para el cambio de mapa!", g_GlobalConfigs[ChatPrefix], id);
			return PLUGIN_HANDLED;
		}
		case 8: ++g_Admin_VoteCurrentPage[id];
		case 9: --g_Admin_VoteCurrentPage[id];
		default: g_Admin_VoteOptions[id][g_Admin_VoteSelectedOptions[id]++] = ((g_Admin_VoteCurrentPage[id] * 7) + key);
	}
	
	ShowMenu_Votemap(id);
	return PLUGIN_HANDLED;
}

public menu_VoteNextMap(const id, const iKey)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;
	
	if(task_exists(g_Admin_VoteUserId + TASK_VOTE_ADMIN))
	{
		new iMapId = iKey;
		new bool:bRandom = false;
		new szMap[64];

		if(iKey == 9)
		{
			bRandom = true;
			iMapId = random(g_Admin_VoteSelectedOptions[g_Admin_VoteUserId]);
		}

		++g_Admin_iVoteCountId[iMapId];
		++g_Admin_VoteCountTotal;

		new aMods[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);
		ArrayGetString(aMods[Maps], g_Admin_VoteOptions[g_Admin_VoteUserId][iMapId], szMap, charsmax(szMap));

		client_print_color(id, print_team_default, "%s^1 Has votado por la opción^4 #%d^1 (^4%s^1)!%s", 
			g_GlobalConfigs[ChatPrefix], (iMapId + 1), szMap, (bRandom) ? "^3 [ELECCIÓN ALEATORIA]" : "");

		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por la opcion #%d (%s)%s", id, (iMapId + 1), szMap, (bRandom) ? " [ELECCIÓN ALEATORIA]" : "");
	}
}

public OnTask_CheckAdminVotemap(const taskid)
{
	new id = (taskid - TASK_VOTE_ADMIN);

	if(!GetPlayerBit(g_bConnected, id))
		return;
	
	new iMaxVotes = -1;

	g_Admin_SelectedNextMod = -1;
	
	for(new i = 0; i < g_Admin_VoteSelectedOptions[id]; ++i)
	{
		if(g_Admin_iVoteCountId[i] > iMaxVotes)
		{
			iMaxVotes = g_Admin_iVoteCountId[i];
			g_Admin_SelectedNextMod = i+1;
		}
	}
	
	if(iMaxVotes)
	{
		new aMods[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);

		client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el mapa ganador es^3 %a^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", 
			g_GlobalConfigs[ChatPrefix], ArrayGetStringHandle(aMods[Maps], g_Admin_VoteOptions[id][g_Admin_SelectedNextMod-1]), 
			iMaxVotes, g_Admin_VoteCountTotal, (g_Admin_VoteCountTotal != 1) ? "s" : "", UTIL_GetPercent(iMaxVotes, g_Admin_VoteCountTotal));
	}
	else
	{
		g_Admin_VoteUserId = -1;
		g_Admin_VoteCountTotal = 0;

		client_print_color(0, print_team_default, "%s^1 La votación para el cambio de modo no tuvo exito!", g_GlobalConfigs[ChatPrefix]);
	}

	arrayset(g_Admin_iVoteCountId, 0, MAX_ADMIN_VOTEOPTIONS);
}

bool:IsOptionSelected(const id, const position)
{
	for(new i = 0; i < g_Admin_VoteSelectedOptions[id]; ++i)
	{
		if(g_Admin_VoteOptions[id][i] == position)
			return true;
	}
	
	return false;
}