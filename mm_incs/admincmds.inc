#if defined _mm_admincmds_included_
	#endinput
#endif
#define _mm_admincmds_included_

AdminCmd_Init()
{
	register_clcmd("amx_multimod", "ClientCommand__AdminMultimod", g_GlobalConfigs[AdminFlags_Menu]);

	register_menu("SelectVoteMod_AdminMenu", KEYSMENU, "menu_SelectVoteMod");
	register_menu("SelectVoteMap_AdminMenu", KEYSMENU, "menu_SelectVoteMap");
}

AdminCmd_ResetGlobalData()
{
	g_Admin_VoteUserId = 0;
	g_Admin_VoteCountTotal = 0;

	arrayset(g_Admin_iVoteCountId, 0, MAX_ADMIN_VOTEOPTIONS);

	arrayset(g_Admin_VoteId, -1, MAX_USERS);
}

AdminCmd_ResetUserData(const id)
{
	arrayset(g_Admin_VoteOptions[id], 0, MAX_ADMIN_VOTEOPTIONS);
	g_Admin_VoteSelectedOptions[id] = 0;
	g_Admin_VoteCurrentPage[id] = 0;
	g_Admin_SelectedMod[id] = g_iCurrentMod;
	g_Admin_SelectedMap[id] = 0;

	g_Admin_VoteId[id] = 0;

	g_Admin_VoteStartInMod[id] = false;

	remove_task(id + TASK_VOTE_ADMIN);
}

AdminCmd_ClientPutInServer(const id)
{
	AdminCmd_ResetUserData(id);
}

AdminCmd_ClientDisconnected(const id)
{
	if(IsPlayer(g_Admin_VoteUserId) && likely(g_Admin_VoteUserId == id) && task_exists(id + TASK_VOTE_ADMIN))
	{
		remove_task(id + TASK_VOTE_ADMIN);
		client_print_color(0, print_team_blue, "%s^3 %n^1 se desconectó, por lo tanto, su votación fue anulada!", g_GlobalConfigs[ChatPrefix], id);
	}
	
	if(likely(g_Admin_VoteUserId == id))
		g_Admin_VoteUserId = 0;

	// Si votó y se fue antes de terminar la votacion, su voto es removido!
	if(g_Admin_VoteId[id] > 0 && IsPlayer(g_Admin_VoteUserId) && task_exists(g_Admin_VoteUserId + TASK_VOTE_ADMIN))
	{
		--g_Admin_iVoteCountId[g_Admin_VoteId[id]-1];
		--g_Admin_VoteCountTotal;

		g_Admin_VoteId[id] = 0;
	}

	AdminCmd_ResetUserData(id);
}

public ClientCommand__AdminMultimod(const id, const level, const cid)
{
	CHECK_CONNECTED(id)

	if(!cmd_access(id, level, cid, 1))
		return PLUGIN_HANDLED;

	CHECK_ADMINVOTEINPROGRESS(id)

	AdminCmd_ResetUserData(id);

	ShowMenu_AdminMultimod(id);
	return PLUGIN_HANDLED;
}

ShowMenu_AdminMultimod(const id)
{
	CHECK_CONNECTED(id)

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
	return PLUGIN_HANDLED;
}

public menu_AdminMultimod(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)

	if(item == MENU_EXIT)
	{
		menu_destroy(menuid);
		return PLUGIN_HANDLED;
	}

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

	menu_destroy(menuid);
	return PLUGIN_HANDLED;
}

ShowMenu_SelectMod(const id)
{
	CHECK_CONNECTED(id)
	CHECK_ADMIN_ACCESS(id, g_GlobalConfigs[AdminFlags_SelectMenu])

	new iMenu = menu_create("\ySELECCIONA UN MODO\R", "menu_SelectMod");

	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);
	new aMods[ArrayMods_e];

	for(new iModId = 0; iModId < iArraySizeMods; ++iModId)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);
		menu_additem(iMenu, fmt("%s%s", aMods[ModName], likely(g_Admin_SelectedMod[id] == iModId) ? "\y (SELECCIONADO)" : (likely(g_iCurrentMod == iModId) ? "\y (ACTUAL)" : "")));
	}

	menu_setprop(iMenu, MPROP_NEXTNAME, "Siguiente");
	menu_setprop(iMenu, MPROP_BACKNAME, "Atrás");
	menu_setprop(iMenu, MPROP_EXITNAME, "Salir");

	menu_display(id, iMenu, 0);
	return PLUGIN_HANDLED;
}

public menu_SelectMod(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)

	if(item == MENU_EXIT)
	{
		menu_destroy(menuid);
		return PLUGIN_HANDLED;
	}

	new iModId = item;
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	g_Admin_SelectedMod[id] = iModId;
	g_Admin_SelectedMap[id] = 0;
	client_print_color(id, print_team_default, "%s^1 Has seleccionado el modo^4 %s^1!", g_GlobalConfigs[ChatPrefix], aMods[ModName]);
	client_print_color(id, print_team_default, "%s^1 Ahora puedes cambiar a un mapa del modo seleccionado o iniciar una votación!", g_GlobalConfigs[ChatPrefix]);

	menu_destroy(menuid);

	ShowMenu_AdminMultimod(id);
	return PLUGIN_HANDLED;
}

ShowMenu_SelectMap(const id)
{
	CHECK_CONNECTED(id)
	CHECK_ADMIN_ACCESS(id, g_GlobalConfigs[AdminFlags_SelectMenu])

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);

	new iArraySize = ArraySize(aMods[Maps]);
	if(iArraySize < 1)
	{
		client_print_color(id, print_team_default, "%s^1 No hay mapas disponibles para seleccionar!", g_GlobalConfigs[ChatPrefix]);
		return PLUGIN_HANDLED;
	}

	new iMenu = menu_create(fmt("\ySELECCIONA UN MAPA^n\dLista de mapas del modo [ %s ]\R", aMods[ModName]), "menu_SelectMap");

	for(new iMapId = 0; iMapId < iArraySize; ++iMapId)
		menu_additem(iMenu, fmt("%a%s", ArrayGetStringHandle(aMods[Maps], iMapId), likely(g_Admin_SelectedMap[id] == iMapId+1) ? "\y (SELECCIONADO)" : ""));

	menu_setprop(iMenu, MPROP_NEXTNAME, "Siguiente");
	menu_setprop(iMenu, MPROP_BACKNAME, "Atrás");
	menu_setprop(iMenu, MPROP_EXITNAME, "Salir");

	menu_display(id, iMenu, 0);
	return PLUGIN_HANDLED;
}

public menu_SelectMap(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)

	if(item == MENU_EXIT)
	{
		menu_destroy(menuid);
		return PLUGIN_HANDLED;
	}

	new iMapId = item;
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);

	g_Admin_SelectedMap[id] = iMapId+1;
	client_print_color(id, print_team_default, "%s^1 Has seleccionado el mapa^4 %a^1 del modo^3 %s^1!", g_GlobalConfigs[ChatPrefix], ArrayGetStringHandle(aMods[Maps], iMapId), aMods[ModName]);
	client_print_color(id, print_team_default, "%s^1 Ahora puedes cambiar al mapa seleccionado!", g_GlobalConfigs[ChatPrefix]);

	menu_destroy(menuid);

	ShowMenu_AdminMultimod(id);
	return PLUGIN_HANDLED;
}

OnFunc_ChangeModAndMap(const id)
{
	CHECK_ADMIN_ACCESS(id, g_GlobalConfigs[AdminFlags_SelectMenu])

	if(g_Admin_SelectedMap[id])
	{
		MultiMod_SetNextMod(g_Admin_SelectedMod[id]);

		new aMods[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);

		new szNextMap[MAX_MAPNAME_LENGTH];
		ArrayGetString(aMods[Maps], g_Admin_SelectedMap[id]-1, szNextMap, MAX_MAPNAME_LENGTH-1);

		message_begin(MSG_ALL, SVC_INTERMISSION);
		message_end();
		
		set_task(2.0, "taskChangeMap", _, szNextMap, sizeof(szNextMap));
		
		client_print_color(0, print_team_blue, "%s^3 %n^1 cambió el modo a^4 %s^1 y el mapa a^4 %s", g_GlobalConfigs[ChatPrefix], id, aMods[ModName], szNextMap);
	}

	return PLUGIN_HANDLED;
}

ShowMenu_ManageMods(const id, menupage=0)
{
	CHECK_CONNECTED(id)
	CHECK_ADMIN_ACCESS(id, g_GlobalConfigs[AdminFlags_ManageMods])

	new iMenu = menu_create("\yADMINISTRAR MODOS\R", "menu_ManageMods");

	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);
	new aMods[ArrayMods_e];

	for(new iModId = 0; iModId < iArraySizeMods; ++iModId)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);
		menu_additem(iMenu, fmt("%s%s", aMods[ModName], likely(aMods[Enabled] == true) ? "" : "\r (DESACTIVADO)"));
	}

	menu_setprop(iMenu, MPROP_NEXTNAME, "Siguiente");
	menu_setprop(iMenu, MPROP_BACKNAME, "Atrás");
	menu_setprop(iMenu, MPROP_EXITNAME, "Salir");

	menu_display(id, iMenu, min(menupage, menu_pages(iMenu) - 1));
	return PLUGIN_HANDLED;
}

public menu_ManageMods(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)

	if(item == MENU_EXIT)
	{
		menu_destroy(menuid);
		return PLUGIN_HANDLED;
	}

	new iNothing, iMenuPage;
	player_menu_info(id, iNothing, iNothing, iMenuPage);

	new iModId = item;
	if(likely(iModId == 0))
	{
		menu_destroy(menuid);

		client_print_color(id, print_team_blue, "%s^1 Este modo no se puede desactivar (MODO POR DEFECTO)!", g_GlobalConfigs[ChatPrefix]);
		ShowMenu_ManageMods(id, iMenuPage);
		return PLUGIN_HANDLED;
	}

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	//likely(aMods[Enabled] == true) ? (aMods[Enabled] = false) : (aMods[Enabled] = true);
	//aMods[Enabled] = !aMods[Enabled];
	#emit ADDR.alt aMods
	#emit CONST.pri 0
	#emit LIDX
	#emit NOT
	#emit STOR.S.pri aMods
	client_print_color(id, print_team_blue, "%s^1 Has^3 %s^1 el modo^4 %s^1!", g_GlobalConfigs[ChatPrefix], likely(aMods[Enabled] == true) ? "ACTIVADO" : "DESACTIVADO", aMods[ModName]);

	ArraySetArray(g_GlobalConfigs[Mods], iModId, aMods);
	MultiMod_SaveOffMods();

	menu_destroy(menuid);
	ShowMenu_ManageMods(id, iMenuPage);
	return PLUGIN_HANDLED;
}

ShowMenu_Votemod(const id)
{
	CHECK_CONNECTED(id)
	CHECK_ADMIN_ACCESS(id, g_GlobalConfigs[AdminFlags_VoteMenu])

	if(g_Admin_VoteCurrentPage[id] < 0)
		return PLUGIN_HANDLED;

	new iArraySize = ArraySize(g_GlobalConfigs[Mods]);
	if(iArraySize < 2)
	{
		client_print_color(id, print_team_default, "%s^1 No hay modos disponibles para seleccionar!", g_GlobalConfigs[ChatPrefix]);
		return PLUGIN_HANDLED;
	}

	new sMenu[MAX_MENU_LENGTH];
	new aMods[ArrayMods_e];
	new iStartLoop = g_Admin_VoteCurrentPage[id] * 7;
	new iEndLoop = clamp(((g_Admin_VoteCurrentPage[id] + 1) * 7), 0, iArraySize);
	new iKeys = MENU_KEY_0;
	new j = 0;

	new iLen = formatex(sMenu, charsmax(sMenu), "\yVOTACIÓN DE MODOS\R\y%d/%d^n^n", (g_Admin_VoteCurrentPage[id] + 1), ((iArraySize / 7) + ((iArraySize % 7) ? 1 : 0)));
	
	for(new i = iStartLoop; i < iEndLoop; ++i)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], i, aMods);

		if(IsOptionSelected(id, (g_Admin_VoteCurrentPage[id] * 7) + j))
		{
			iKeys |= (1<<j);
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^t^t\r%d.\d %s%s\y [ X ]^n", ++j, aMods[ModName], (g_iCurrentMod == i) ? " (ACTUAL)" : "");
		}
		else if(IsFullSelected(id))
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
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\d8. Tenés que seleccionar al menos 2 modos (%d máx)^n", g_GlobalConfigs[AdminMaxOptionsInMenu]);
	
	if(iEndLoop != iArraySize)
	{
		iKeys |= MENU_KEY_9;
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r9.\w Siguiente^n\r0.\w %s^n", iStartLoop ? "Volver" : "Salir");
	}
	else
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r0.\w %s", iStartLoop ? "Volver" : "Salir");
	
	if(g_Admin_VoteSelectedOptions[id])
	{
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n^n\yModos seleccionados\w (%d%s)", g_Admin_VoteSelectedOptions[id], IsFullSelected(id) ? " Max" : "");
		
		//for(j = 0; j < g_Admin_VoteSelectedOptions[id]; ++j)
		//{
		//	ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_VoteOptions[id][j], aMods);
		//	iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\w%s^n", aMods[ModName]);
		//}
	}
	
	show_menu(id, iKeys, sMenu, -1, "SelectVoteMod_AdminMenu");
	return PLUGIN_HANDLED;
}

public menu_SelectVoteMod(const id, const key)
{
	CHECK_CONNECTED(id)

	switch(key)
	{
		case 7:
		{
			CHECK_ADMINVOTEINPROGRESS(id)

			AdminCmd_ResetGlobalData();
			g_Admin_VoteUserId = id;

			set_pcvar_float(g_pCvar_amx_last_voting, get_gametime() + 15.0 + 2.0 + float(g_bCvar_amx_vote_delay_mm));

			g_Admin_VoteStartInMod[id] = true;

			ShowMenu_StartCustomVotemod();
			return PLUGIN_HANDLED;
		}
		case 8: ++g_Admin_VoteCurrentPage[id];
		case 9: --g_Admin_VoteCurrentPage[id];
		default:
		{
			new iValue = ((g_Admin_VoteCurrentPage[id] * 7) + key);
			(IsOptionSelected(id, iValue)) ? DeleteOption(id, iValue) : PushOption(id, iValue);
		}
	}
	
	ShowMenu_Votemod(id);
	return PLUGIN_HANDLED;
}

ShowMenu_StartCustomVotemod()
{
	new iMenu = menu_create(fmt("\yVotación de\w %n^n^n\yAhora elige el próximo modo\r:\y ¡VOTÁ AHORA!", g_Admin_VoteUserId), "menu_StartCustomVotemod");

	for(new i = 0, aMods[ArrayMods_e]; i < g_Admin_VoteSelectedOptions[g_Admin_VoteUserId]; ++i)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_VoteOptions[g_Admin_VoteUserId][i], aMods);
		menu_additem(iMenu, fmt("\w%s%s", aMods[ModName], likely(i == (g_Admin_VoteSelectedOptions[g_Admin_VoteUserId]-1)) ? "^n" : ""));
	}

	menu_additem(iMenu, "\ySeleccionar al azar");

	menu_setprop(iMenu, MPROP_PERPAGE, 0);
	menu_setprop(iMenu, MPROP_EXIT, MEXIT_NEVER);

	UTIL_MenuDisplayAll(iMenu, _, 15);

	remove_task(g_Admin_VoteUserId + TASK_VOTE_ADMIN);
	set_task(15.1, "OnTask_CountVotes_Mod", g_Admin_VoteUserId + TASK_VOTE_ADMIN);
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_blue, "%s^3 %n^1 comenzó una votación para el cambio de modo!", g_GlobalConfigs[ChatPrefix], g_Admin_VoteUserId);
}

public menu_StartCustomVotemod(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)
	CHECK_TIMEOUT_NEWMENU(id, menuid, item)

	if(IsPlayer(g_Admin_VoteUserId) && task_exists(g_Admin_VoteUserId + TASK_VOTE_ADMIN))
	{
		new iModId = item;
		new bool:bRandom = false;

		new aMods[ArrayMods_e];

		if(iModId < g_Admin_VoteSelectedOptions[g_Admin_VoteUserId])
			ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_VoteOptions[g_Admin_VoteUserId][iModId], aMods);
		else
		{
			iModId = random(g_Admin_VoteSelectedOptions[g_Admin_VoteUserId]);
			ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_VoteOptions[g_Admin_VoteUserId][iModId], aMods);

			bRandom = true;
		}

		++g_Admin_iVoteCountId[iModId];
		++g_Admin_VoteCountTotal;

		g_Admin_VoteId[id] = iModId+1;

		client_print_color(id, print_team_default, "%s^1 Has votado por la opción^4 #%d^1 (^4%s^1)!%s", 
			g_GlobalConfigs[ChatPrefix], (iModId + 1), aMods[ModName], (bRandom) ? "^3 [ELECCIÓN ALEATORIA]" : "");

		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por la opcion #%d (%s)%s", id, (iModId + 1), aMods[ModName], (bRandom) ? " [ELECCIÓN ALEATORIA]" : "");
	}

	menu_destroy(menuid);
	return PLUGIN_HANDLED;
}

public OnTask_CountVotes_Mod(const taskid)
{
	new admin = (taskid - TASK_VOTE_ADMIN);

	CHECK_CONNECTED(admin)
	
	new aMods[ArrayMods_e];
	new iWinner = 0;
	new iResult = -1;

	ResetAdminVoteMenuAll();

	client_print(0, print_console, "Resultados de la votacion:");

	for(new i = 0; i < g_Admin_VoteSelectedOptions[admin]; ++i)
	{
		if(g_Admin_iVoteCountId[i] > iResult)
		{
			iResult = g_Admin_iVoteCountId[i];
			iWinner = i;
		}

		ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_VoteOptions[admin][i], aMods);
		client_print(0, print_console, "Modo: %s - Votos: %d - Porcentaje: %d%%", aMods[ModName], g_Admin_iVoteCountId[i], UTIL_GetPercent(g_Admin_iVoteCountId[i], g_Admin_VoteCountTotal));
	}
	
	if(iResult)
	{
		new j = 0;
		new iModsIds[MAX_ADMIN_VOTEOPTIONS];

		iWinner = g_Admin_VoteOptions[admin][iWinner];

		// Chequeo si hubo 2 o más mapas con los mismos votos
		for(new i = 0; i < g_Admin_VoteSelectedOptions[admin]; ++i)
		{
			if(g_Admin_iVoteCountId[i] == iResult)
				iModsIds[j++] = g_Admin_VoteOptions[admin][i];
		}

		if(j > 1) // Hubo más de 1 ganador (otra vez)
			iWinner = iModsIds[random(j)];

		ArrayGetArray(g_GlobalConfigs[Mods], iWinner, aMods);

		client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el modo ganador es^3 %s^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", 
			g_GlobalConfigs[ChatPrefix], aMods[ModName], iResult, g_Admin_VoteCountTotal, (g_Admin_VoteCountTotal != 1) ? "s" : "", UTIL_GetPercent(iResult, g_Admin_VoteCountTotal));

		g_Admin_SelectedMod[admin] = iWinner;
		{
			new aMods[ArrayMods_e];
			ArrayGetArray(g_GlobalConfigs[Mods], iWinner, aMods);

			new iArraySizeMaps = ArraySize(aMods[Maps]);
			new iMaxMaps = min(g_GlobalConfigs[MapsInMenu], iArraySizeMaps);

			set_pcvar_float(g_pCvar_amx_last_voting, get_gametime() + 15.0 + 2.0 + float(g_bCvar_amx_vote_delay_mm));

			g_Admin_VoteSelectedOptions[admin] = 0;
			arrayset(g_Admin_VoteOptions[admin], 0, MAX_ADMIN_VOTEOPTIONS);

			for(new iMapId; g_Admin_VoteSelectedOptions[admin] < iMaxMaps; ++g_Admin_VoteSelectedOptions[admin])
			{
				do {
					iMapId = random(iArraySizeMaps);
				} while(IsOptionSelected(admin, iMapId)); // La opcion está en el menú

				g_Admin_VoteOptions[admin][g_Admin_VoteSelectedOptions[admin]] = iMapId;
			}

			AdminCmd_ResetGlobalData();
			g_Admin_VoteUserId = admin;

			ShowMenu_StartCustomVotemap();
		}
	}
	else
	{
		AdminCmd_ResetUserData(admin);
		AdminCmd_ResetGlobalData();

		client_print_color(0, print_team_default, "%s^1 La votación para el cambio de modo no tuvo exito!", g_GlobalConfigs[ChatPrefix]);
	}

	return PLUGIN_HANDLED;
}

ShowMenu_Votemap(const id)
{
	CHECK_CONNECTED(id)
	CHECK_ADMIN_ACCESS(id, g_GlobalConfigs[AdminFlags_VoteMenu])

	if(g_Admin_VoteCurrentPage[id] < 0)
		return PLUGIN_HANDLED;

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);

	new iArraySize = ArraySize(aMods[Maps]);
	if(iArraySize < 2)
	{
		client_print_color(id, print_team_default, "%s^1 No hay mapas disponibles para seleccionar!", g_GlobalConfigs[ChatPrefix]);
		return PLUGIN_HANDLED;
	}

	new sMenu[MAX_MENU_LENGTH];
	new iStartLoop = g_Admin_VoteCurrentPage[id] * 7;
	new iEndLoop = clamp(((g_Admin_VoteCurrentPage[id] + 1) * 7), 0, iArraySize);
	new iKeys = MENU_KEY_0;
	new j = 0;

	new iLen = formatex(sMenu, charsmax(sMenu), "\yVOTACIÓN DE MAPAS^n\dLista de mapas del modo [ %s ]\R\y%d/%d^n^n", 
		aMods[ModName], (g_Admin_VoteCurrentPage[id] + 1), ((iArraySize / 7) + ((iArraySize % 7) ? 1 : 0)));
	
	for(new i = iStartLoop; i < iEndLoop; ++i)
	{
		if(IsOptionSelected(id, (g_Admin_VoteCurrentPage[id] * 7) + j))
		{
			iKeys |= (1<<j);
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^t^t\r%d.\d %a\y [ X ]^n", ++j, ArrayGetStringHandle(aMods[Maps], i));
		}
		else if(IsFullSelected(id))
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
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\d8. Tenés que seleccionar al menos 2 mapas (%d máx)^n", g_GlobalConfigs[AdminMaxOptionsInMenu]);
	
	if(iEndLoop != iArraySize)
	{
		iKeys |= MENU_KEY_9;
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r9.\w Siguiente^n\r0.\w %s^n", iStartLoop ? "Volver" : "Salir");
	}
	else
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r0.\w %s", iStartLoop ? "Volver" : "Salir");
	
	if(g_Admin_VoteSelectedOptions[id])
	{
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n^n\yMapas seleccionados\w (%d%s)", g_Admin_VoteSelectedOptions[id], IsFullSelected(id) ? " Max" : "");
		
		//for(j = 0; j < g_Admin_VoteSelectedOptions[id]; ++j)
		//	iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\w%a^n", ArrayGetStringHandle(aMods[Maps], g_Admin_VoteOptions[id][j]));
	}
	
	show_menu(id, iKeys, sMenu, -1, "SelectVoteMap_AdminMenu");
	return PLUGIN_HANDLED;
}

public menu_SelectVoteMap(const id, const key)
{
	CHECK_CONNECTED(id)

	switch(key)
	{
		case 7:
		{
			CHECK_ADMINVOTEINPROGRESS(id)

			AdminCmd_ResetGlobalData();
			g_Admin_VoteUserId = id;

			set_pcvar_float(g_pCvar_amx_last_voting, get_gametime() + 15.0 + 2.0 + float(g_bCvar_amx_vote_delay_mm));

			g_Admin_VoteStartInMod[id] = false;

			ShowMenu_StartCustomVotemap();
			return PLUGIN_HANDLED;
		}
		case 8: ++g_Admin_VoteCurrentPage[id];
		case 9: --g_Admin_VoteCurrentPage[id];
		default:
		{
			new iValue = ((g_Admin_VoteCurrentPage[id] * 7) + key);
			(IsOptionSelected(id, iValue)) ? DeleteOption(id, iValue) : PushOption(id, iValue);
		}
	}
	
	ShowMenu_Votemap(id);
	return PLUGIN_HANDLED;
}

ShowMenu_StartCustomVotemap()
{
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[g_Admin_VoteUserId], aMods);

	new iMenu = menu_create(fmt("\yVotación de\w %n^n\yModo\r:\w %s^n^n\yAhora elige el próximo mapa\r:\y ¡VOTÁ AHORA!", g_Admin_VoteUserId, aMods[ModName]), "menu_StartCustomVotemap");

	for(new i = 0; i < g_Admin_VoteSelectedOptions[g_Admin_VoteUserId]; ++i)
	{
		menu_additem(iMenu, fmt("\w%a%s", 
			ArrayGetStringHandle(aMods[Maps], g_Admin_VoteOptions[g_Admin_VoteUserId][i]), likely(i == (g_Admin_VoteSelectedOptions[g_Admin_VoteUserId]-1)) ? "^n" : ""));
	}

	menu_additem(iMenu, "\ySeleccionar al azar");

	menu_setprop(iMenu, MPROP_PERPAGE, 0);
	menu_setprop(iMenu, MPROP_EXIT, MEXIT_NEVER);

	UTIL_MenuDisplayAll(iMenu, _, 15);

	remove_task(g_Admin_VoteUserId + TASK_VOTE_ADMIN);
	set_task(15.1, "OnTask_AdminCountVotes_Map", g_Admin_VoteUserId + TASK_VOTE_ADMIN);
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_blue, "%s^3 %n^1 comenzó una votación para el cambio de mapa!", g_GlobalConfigs[ChatPrefix], g_Admin_VoteUserId);
}

public menu_StartCustomVotemap(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)
	CHECK_TIMEOUT_NEWMENU(id, menuid, item)

	if(IsPlayer(g_Admin_VoteUserId) && task_exists(g_Admin_VoteUserId + TASK_VOTE_ADMIN))
	{
		new iMapId = item;
		new bool:bRandom = false;

		new szMap[MAX_MAPNAME_LENGTH];

		new aMods[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);

		if(iMapId < g_Admin_VoteSelectedOptions[id])
			ArrayGetString(aMods[Maps], g_Admin_VoteOptions[g_Admin_VoteUserId][iMapId], szMap, MAX_MAPNAME_LENGTH-1);
		else
		{
			iMapId = random(g_Admin_VoteSelectedOptions[id]);
			ArrayGetString(aMods[Maps], g_Admin_VoteOptions[g_Admin_VoteUserId][iMapId], szMap, MAX_MAPNAME_LENGTH-1);

			bRandom = true;
		}

		++g_Admin_iVoteCountId[iMapId];
		++g_Admin_VoteCountTotal;

		g_Admin_VoteId[id] = iMapId+1;

		client_print_color(id, print_team_default, "%s^1 Has votado por la opción^4 #%d^1 (^4%s^1)!%s", 
			g_GlobalConfigs[ChatPrefix], (iMapId + 1), szMap, (bRandom) ? "^3 [ELECCIÓN ALEATORIA]" : "");

		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%n ha votado por la opcion #%d (%s)%s", id, (iMapId + 1), szMap, (bRandom) ? " [ELECCIÓN ALEATORIA]" : "");
	}

	menu_destroy(menuid);
	return PLUGIN_HANDLED;
}

public OnTask_AdminCountVotes_Map(const taskid)
{
	new admin = (taskid - TASK_VOTE_ADMIN);

	CHECK_CONNECTED(admin)

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[admin], aMods);

	new iWinner = 0;
	new iResult = -1;

	ResetAdminVoteMenuAll();

	client_print(0, print_console, "Resultados de la votacion:");

	for(new i = 0; i < g_Admin_VoteSelectedOptions[admin]; ++i)
	{
		if(g_Admin_iVoteCountId[i] > iResult)
		{
			iResult = g_Admin_iVoteCountId[i];
			iWinner = i;
		}

		client_print(0, print_console, "Mapa: %a - Votos: %d - Porcentaje: %d%%",
			ArrayGetStringHandle(aMods[Maps], g_Admin_VoteOptions[admin][i]), g_Admin_iVoteCountId[i], UTIL_GetPercent(g_Admin_iVoteCountId[i], g_Admin_VoteCountTotal));
	}

	if(iResult)
	{
		new j = 0;
		new iMapsIds[MAX_ADMIN_VOTEOPTIONS];
		new szMap[MAX_MAPNAME_LENGTH];

		iWinner = g_Admin_VoteOptions[admin][iWinner];

		// Chequeo si hubo 2 o más mapas con los mismos votos
		for(new i = 0; i < g_Admin_VoteSelectedOptions[admin]; ++i)
		{
			if(g_Admin_iVoteCountId[i] == iResult)
				iMapsIds[j++] = g_Admin_VoteOptions[admin][i];
		}

		if(j > 1) // Hubo más de 1 ganador (otra vez)
			iWinner = iMapsIds[random(j)];

		ArrayGetString(aMods[Maps], iWinner, szMap, MAX_MAPNAME_LENGTH-1);

		client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el mapa ganador es^3 %s^1 con^4 %d^1 /^4 %d^1 voto%s (%d%%)!", 
			g_GlobalConfigs[ChatPrefix], szMap, iResult, g_Admin_VoteCountTotal, (g_Admin_VoteCountTotal != 1) ? "s" : "", UTIL_GetPercent(iResult, g_Admin_VoteCountTotal));

		g_Admin_SelectedMap[admin] = iWinner+1;
		ShowMenu_VoteAdminConfirm(admin, aMods[ModName], szMap, g_Admin_VoteStartInMod[admin]);
	}
	else
	{
		AdminCmd_ResetUserData(admin);
		client_print_color(0, print_team_default, "%s^1 La votación para el cambio de mapa no tuvo exito!", g_GlobalConfigs[ChatPrefix]);
	}

	AdminCmd_ResetGlobalData();
	return PLUGIN_HANDLED;
}

ShowMenu_VoteAdminConfirm(const id, const szMod[], const szMap[], const bool:bModWinner)
{
	CHECK_CONNECTED(id)

	new iMenu = menu_create(fmt("\y¡VOTACIÓN FINALIZADA!^n^n\wModo%s\r:\y %s^n\wMapa ganador\r:\y %s", likely(bModWinner == true) ? " ganador" : "", szMod, szMap), "menu_VoteAdminConfirm");

	menu_additem(iMenu, "\yConfirmar y cambiar ahora");
	menu_additem(iMenu, "Rechazar votación");

	menu_setprop(iMenu, MPROP_EXIT, MEXIT_NEVER);

	menu_display(id, iMenu);
	return PLUGIN_HANDLED;
}

public menu_VoteAdminConfirm(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)

	switch(item)
	{
		case 0: OnFunc_ChangeModAndMap(id);
		case 1: client_print_color(0, print_team_blue, "%s^3 %n^1 ha rechazado la votación!", g_GlobalConfigs[ChatPrefix], id);
	}

	menu_destroy(menuid);
	return PLUGIN_HANDLED;
}

bool:IsOptionSelected(const id, const value)
{
	for(new i = 0; i < g_Admin_VoteSelectedOptions[id]; ++i)
	{
		if(g_Admin_VoteOptions[id][i] == value)
			return true;
	}
	
	return false;
}

bool:IsFullSelected(const id)
{
	return bool:likely(g_Admin_VoteSelectedOptions[id] == g_GlobalConfigs[AdminMaxOptionsInMenu]);
}

bool:PushOption(const id, const value)
{
	if(IsFullSelected(id))
		return false;

	g_Admin_VoteOptions[id][g_Admin_VoteSelectedOptions[id]++] = value;
	return true;
}

bool:DeleteOption(const id, const value)
{
	new iPos = -1;
	for(new i = 0; (i < g_Admin_VoteSelectedOptions[id]) && likely(iPos == -1); ++i)
	{
		if(g_Admin_VoteOptions[id][i] == value)
			iPos = i;
	}

	if(unlikely(iPos == -1))
	{
		for(new i = iPos; i < g_Admin_VoteSelectedOptions[id]-1; ++i)
		{
			g_Admin_VoteOptions[id][i] = g_Admin_VoteOptions[id][i+1];
		}

		--g_Admin_VoteSelectedOptions[id];
		return true;
	}

	return false;
}

bool:ResetAdminVoteMenuAll()
{
	for(new i = 1; i < MAX_USERS; ++i) if(GetPlayerBit(g_bConnected, i) && likely(g_Admin_VoteId[i] == -1))
	{
		menu_cancel(i);
		reset_menu(i);
	}

	return true;
}