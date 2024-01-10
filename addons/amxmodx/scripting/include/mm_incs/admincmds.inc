#if defined _mm_admincmds_included_
	#endinput
#endif
#define _mm_admincmds_included_

AdminCmd_Init()
{
	register_concmd("mm_force_votemod", "ConsoleCommand_ForceVoteMod", g_GlobalConfigs[AdminFlags_ForceVoteMod]);

	register_clcmd("amx_multimod", "ClientCommand__AdminMultimod", g_GlobalConfigs[AdminFlags_Menu]);
	register_clcmd("amx_manage_mm", "ClientCommand__ManageMM", g_GlobalConfigs[AdminFlags_ManageMods]);

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
	if(IsPlayer(g_Admin_VoteUserId) && (g_Admin_VoteUserId == id) && task_exists(id + TASK_VOTE_ADMIN))
	{
		remove_task(id + TASK_VOTE_ADMIN);
		client_print_color(0, print_team_blue, "%s^1 %L %L!", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_VOTE_DISCONNECTED", id);

		ResetAdminVoteMenuAll();
		menu_destroy(g_Menu_AdminCmd);
	}
	
	if(g_Admin_VoteUserId == id)
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

public ConsoleCommand_ForceVoteMod(const id, const level, const cid)
{
	if(!cmd_access(id, level, cid, false))
		return PLUGIN_HANDLED;
	
	if(CanForceVoteNextMod())
	{
		ExecuteForward(g_Forward_AdminForceVotemod, g_Forward_RetVal, id);

		if(g_Forward_RetVal == PLUGIN_CONTINUE)
		{
			StartVoteNextMod();

			client_print_color(0, id, "%s^1 %L %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_FORCE_VOTEMODE", id);
			return PLUGIN_HANDLED;
		}
	}
	
	console_print(id, "%L", LANG_PLAYER, "MM_ADMIN_CANT_FORCE_VOTEMODE");
	return PLUGIN_HANDLED;
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

public ClientCommand__ManageMM(const id, const level, const cid)
{
	CHECK_CONNECTED(id)

	if(!cmd_access(id, level, cid, 1))
		return PLUGIN_HANDLED;
	
	ShowMenu_ManageMultiMod(id);
	return PLUGIN_HANDLED;
}

ShowMenu_AdminMultimod(const id)
{
	CHECK_CONNECTED(id)

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iCurrentMod, aMods);

	new iMenu = menu_create(fmt("\y%L^n^n\w- %L:\y %s^n\w- %L: \y%s\y", 
		LANG_PLAYER, "MULTIMOD_MANAGER", LANG_PLAYER, "MM_CURRENT_MODE", aMods[ModName], LANG_PLAYER, "MM_CURRENT_MAP", g_szCurrentMap), "menu_AdminMultimod");

	ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);
	menu_additem(iMenu, fmt("%L", LANG_PLAYER, "MM_ADMIN_SELECT_NEXTMOD", aMods[ModName]));

	if(g_Admin_SelectedMap[id])
		menu_additem(iMenu, fmt("%L", LANG_PLAYER, "MM_ADMIN_SELECT_NEXTMAP", ArrayGetStringHandle(aMods[Maps], g_Admin_SelectedMap[id]-1)));
	else
		menu_additem(iMenu, fmt("%L", LANG_PLAYER, "MM_ADMIN_SELECT_NEXTMAP_UNKNOWN"));

	if(g_Admin_SelectedMap[id])
		menu_additem(iMenu, fmt("%L^n", LANG_PLAYER, "MM_ADMIN_CHANGE_TO_MODE_AND_MAP", aMods[ModName],  ArrayGetStringHandle(aMods[Maps], g_Admin_SelectedMap[id]-1)));
	else
		menu_additem(iMenu, fmt("%L^n", LANG_PLAYER, "MM_ADMIN_CHANGE_TO_MODE_AND_MAP_UNKNOWN", aMods[ModName]), .callback=menu_makecallback("UTIL_DisableItem"));

	menu_additem(iMenu, fmt("%L", LANG_PLAYER, "MM_ADMIN_START_VOTEMODE"));
	menu_additem(iMenu, fmt("%L", LANG_PLAYER, "MM_ADMIN_START_VOTEMODE", aMods[ModName]));

	menu_setprop(iMenu, MPROP_EXITNAME, fmt("%L", LANG_PLAYER, "MM_EXIT"));

	menu_display(id, iMenu, 0);
	return PLUGIN_HANDLED;
}

public menu_AdminMultimod(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)
	CHECK_EXIT_NEWMENU(id, menuid, item)

	switch(item)
	{
		case 0: ShowMenu_SelectMod(id);
		case 1: ShowMenu_SelectMap(id);
		case 2: OnFunc_ChangeModAndMap(id);
		case 3:
		{
			arrayset(g_Admin_VoteOptions[id], 0, MAX_ADMIN_VOTEOPTIONS);
			g_Admin_VoteSelectedOptions[id] = g_Admin_VoteCurrentPage[id] = 0;
			ShowMenu_Votemod(id);
		}
		case 4:
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

	new iMenu = menu_create(fmt("%L", LANG_PLAYER, "MM_ADMIN_SELECT_MODE"), "menu_SelectMod");

	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);
	new aMods[ArrayMods_e];

	for(new iModId = 0; iModId < iArraySizeMods; ++iModId)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

		if(UTIL_IsModOff(iModId))
			menu_additem(iMenu, fmt("%s\r (%L)", aMods[ModName], LANG_PLAYER, "MM_LOCKED"), .callback=menu_makecallback("UTIL_DisableItem"));
		else
			menu_additem(iMenu, fmt("%s%s", aMods[ModName], (g_Admin_SelectedMod[id] == iModId) ? fmt("\y (%L)", LANG_PLAYER, "MM_SELECTED") : ((g_iCurrentMod == iModId) ? fmt("\y (%L)", LANG_PLAYER, "MM_CURRENT") : "")));
	}

	menu_setprop(iMenu, MPROP_NEXTNAME, fmt("%L", LANG_PLAYER, "MM_MORE"));
	menu_setprop(iMenu, MPROP_BACKNAME, fmt("%L", LANG_PLAYER, "MM_BACK"));
	menu_setprop(iMenu, MPROP_EXITNAME, fmt("%L", LANG_PLAYER, "MM_EXIT"));

	menu_display(id, iMenu, 0);
	return PLUGIN_HANDLED;
}

public menu_SelectMod(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)

	if(item == MENU_EXIT)
	{
		menu_destroy(menuid);

		ShowMenu_AdminMultimod(id);
		return PLUGIN_HANDLED;
	}

	new iModId = item;
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	g_Admin_SelectedMod[id] = iModId;
	g_Admin_SelectedMap[id] = 0;
	client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_VOTEMOD_MODE_SELECTED", aMods[ModName]);
	client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_VOTEMOD_MODE_SELECTED_INFO");

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
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_VOTEMAP_NO_MAPS");
		return PLUGIN_HANDLED;
	}

	new iMenu = menu_create(fmt("%L", LANG_PLAYER, "MM_ADMIN_VOTEMAP_M_TITLE_SELECT_MAP", aMods[ModName]), "menu_SelectMap");

	for(new iMapId = 0; iMapId < iArraySize; ++iMapId)
		menu_additem(iMenu, fmt("%a%s", ArrayGetStringHandle(aMods[Maps], iMapId), (g_Admin_SelectedMap[id] == iMapId+1) ? fmt("\y (%L)", LANG_PLAYER, "MM_SELECTED") : ""));

	menu_setprop(iMenu, MPROP_NEXTNAME, fmt("%L", LANG_PLAYER, "MM_MORE"));
	menu_setprop(iMenu, MPROP_BACKNAME, fmt("%L", LANG_PLAYER, "MM_BACK"));
	menu_setprop(iMenu, MPROP_EXITNAME, fmt("%L", LANG_PLAYER, "MM_EXIT"));

	menu_display(id, iMenu, 0);
	return PLUGIN_HANDLED;
}

public menu_SelectMap(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)

	if(item == MENU_EXIT)
	{
		menu_destroy(menuid);

		ShowMenu_AdminMultimod(id);
		return PLUGIN_HANDLED;
	}

	new iMapId = item;
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[id], aMods);

	g_Admin_SelectedMap[id] = iMapId+1;
	client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_VOTEMAP_MAP_SELECTED", ArrayGetStringHandle(aMods[Maps], iMapId), aMods[ModName]);
	client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_VOTEMAP_MAP_SELECTED_INFO");

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
		
		set_task(2.0, "OnTask_ChangeMap", _, szNextMap, MAX_MAPNAME_LENGTH);
		
		client_print_color(0, print_team_blue, "%s %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_CHANGE_MOD_MAP", id, aMods[ModName], szNextMap);
	}

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
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_VOTEMAP_NO_MODS");
		return PLUGIN_HANDLED;
	}

	new sMenu[MAX_MENU_LENGTH];
	new aMods[ArrayMods_e];
	new iStartLoop = g_Admin_VoteCurrentPage[id] * 7;
	new iEndLoop = clamp(((g_Admin_VoteCurrentPage[id] + 1) * 7), 0, iArraySize);
	new iKeys = MENU_KEY_0;
	new j = 0;

	new iLen = formatex(sMenu, charsmax(sMenu), "%L^n^n", LANG_PLAYER, "MM_ADMIN_VOTEMOD_M_TITLE_VOTE", (g_Admin_VoteCurrentPage[id] + 1), ((iArraySize / 7) + ((iArraySize % 7) ? 1 : 0)));
	
	for(new i = iStartLoop, bool:bBlocked; i < iEndLoop; ++i)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], i, aMods);

		bBlocked = UTIL_IsModOff(i);

		if(IsOptionSelected(id, (g_Admin_VoteCurrentPage[id] * 7) + j))
		{
			iKeys |= (1<<j);
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^t^t\r%d.\d %s%s\y [ X ]^n", ++j, aMods[ModName], (g_iCurrentMod == i) ? fmt(" (%L)", LANG_PLAYER, "MM_CURRENT") : "");
		}
		else if(IsFullSelected(id) || bBlocked)
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\d%d. %s%s%s^n", ++j, aMods[ModName], (g_iCurrentMod == i) ? fmt(" (%L)", LANG_PLAYER, "MM_CURRENT") : "", bBlocked ? fmt("\r (%L)", LANG_PLAYER, "MM_LOCKED") : "");
		else
		{
			iKeys |= (1<<j);
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %s%s^n", ++j, aMods[ModName], (g_iCurrentMod == i) ? fmt("\y (%L)", LANG_PLAYER, "MM_CURRENT") : "");
		}
	}
	
	if(g_Admin_VoteSelectedOptions[id] > 1)
	{
		iKeys |= MENU_KEY_8;
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r8.\w %L^n", LANG_PLAYER, "MM_ADMIN_VOTEMOD_START_VOTE");
	}
	else
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\d8. %L^n", LANG_PLAYER, "MM_ADMIN_VOTEMOD_MIN_MODES", g_GlobalConfigs[AdminMaxOptionsInMenu]);
	
	if(iEndLoop != iArraySize)
	{
		iKeys |= MENU_KEY_9;
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r9.\w %L^n\r0.\w %s^n", LANG_PLAYER, "MM_MORE", LANG_PLAYER, iStartLoop ? "MM_BACK" : "MM_EXIT");
	}
	else
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r0.\w %s", LANG_PLAYER, iStartLoop ? "MM_BACK" : "MM_EXIT");
	
	if(g_Admin_VoteSelectedOptions[id])
	{
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n^n%L", LANG_PLAYER, "MM_ADMIN_VOTEMOD_SELECTED_MODES", g_Admin_VoteSelectedOptions[id], IsFullSelected(id) ? fmt(" %L", LANG_PLAYER, "MM_MAX") : "");
		
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
		case 9:
		{
			if(--g_Admin_VoteCurrentPage[id] < 0)
			{
				ShowMenu_AdminMultimod(id);
				return PLUGIN_HANDLED;
			}
		}
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
	g_Menu_AdminCmd = menu_create(fmt("\y%L^n^n\y%L:", LANG_PLAYER, "MM_ADMIN_VOTEMOD_ADMIN_NAME", g_Admin_VoteUserId, LANG_PLAYER, "MM_SELECT_NEXT_MODE"), "menu_StartCustomVotemod");

	for(new i = 0, aMods[ArrayMods_e]; i < g_Admin_VoteSelectedOptions[g_Admin_VoteUserId]; ++i)
	{
		if(g_iCurrentMod == g_Admin_VoteOptions[g_Admin_VoteUserId][i])
		{
			menu_additem(g_Menu_AdminCmd, fmt("%L%s", LANG_PLAYER, "MM_M_EXTEND_CURRENT_MODE", g_szCurrentMod, g_bCvar_amx_extendmap_step, (i == (g_Admin_VoteSelectedOptions[g_Admin_VoteUserId]-1)) ? "^n" : ""));
			continue;
		}

		ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_VoteOptions[g_Admin_VoteUserId][i], aMods);
		menu_additem(g_Menu_AdminCmd, fmt("\w%s%s", aMods[ModName], (i == (g_Admin_VoteSelectedOptions[g_Admin_VoteUserId]-1)) ? "^n" : ""));
	}

	menu_additem(g_Menu_AdminCmd, fmt("\y%L", LANG_PLAYER, "MM_SELECT_VOTE_RANDOM_ITEM"));

	menu_setprop(g_Menu_AdminCmd, MPROP_PERPAGE, 0);
	menu_setprop(g_Menu_AdminCmd, MPROP_EXIT, MEXIT_NEVER);

	UTIL_MenuDisplayAll(g_Menu_AdminCmd, _, 15);

	remove_task(g_Admin_VoteUserId + TASK_VOTE_ADMIN);
	set_task(15.1, "OnTask_CountVotes_Mod", g_Admin_VoteUserId + TASK_VOTE_ADMIN);
	
	if(g_bCvar_amx_multimod_voice)
		client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);

	client_print_color(0, print_team_blue, "%s %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_VOTEMOD_START_VOTE_SAY", g_Admin_VoteUserId);
}

public menu_StartCustomVotemod(const id, const menuid, const item)
{
	CHECK_CONNECTED(id)
	CHECK_MENU_EXIT(item)
	CHECK_MENU_TIMEOUT(item)

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

		client_print_color(id, print_team_default, "%s^1 %L%s", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_VOTED_FOR", aMods[ModName], (bRandom) ? fmt("^3 [%s]", LANG_PLAYER, "MM_RANDOM") : "");

		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%L%s", LANG_PLAYER, "MM_PLAYER_VOTED_FOR", id, aMods[ModName], (bRandom) ? fmt("[%s]", LANG_PLAYER, "MM_RANDOM") : "");
	}

	return PLUGIN_HANDLED;
}

public OnTask_CountVotes_Mod(const taskid)
{
	new admin = (taskid - TASK_VOTE_ADMIN);

	CHECK_CONNECTED(admin)
	
	new aMods[ArrayMods_e];
	new iWinner = 0;
	new iResult = 0;

	ResetAdminVoteMenuAll();
	menu_destroy(g_Menu_AdminCmd);

	client_print(0, print_console, "%L:", LANG_PLAYER, "MM_VOTE_RESULT");

	for(new i = 0; i < g_Admin_VoteSelectedOptions[admin]; ++i)
	{
		if(g_Admin_iVoteCountId[i] > iResult)
		{
			iResult = g_Admin_iVoteCountId[i];
			iWinner = i;
		}

		ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_VoteOptions[admin][i], aMods);
		client_print(0, print_console, "%L: %s - %L: %d - %L: %d%%", LANG_PLAYER, "MM_MODE", aMods[ModName], LANG_PLAYER, "MM_VOTES", g_Admin_iVoteCountId[i], LANG_PLAYER, "MM_PERCENT", UTIL_GetPercent(g_Admin_iVoteCountId[i], g_Admin_VoteCountTotal));
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
		
		// Ganador, extender el modo
		if(iWinner == g_iCurrentMod)
		{
			ExtendTimeleft(g_bCvar_amx_extendmap_step);
			client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MOD_CHO_FIN_EXT", g_bCvar_amx_extendmap_step, iResult, g_Admin_VoteCountTotal, UTIL_GetPercent(iResult, g_Admin_VoteCountTotal));
			
			AdminCmd_ResetUserData(admin);
			AdminCmd_ResetGlobalData();

			return PLUGIN_HANDLED;
		}

		ArrayGetArray(g_GlobalConfigs[Mods], iWinner, aMods);

		client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MOD_CHO_FIN_NEXT", aMods[ModName], iResult, g_Admin_VoteCountTotal, UTIL_GetPercent(iResult, g_Admin_VoteCountTotal));

		g_Admin_SelectedMod[admin] = iWinner;
		{
			new aMods[ArrayMods_e];
			ArrayGetArray(g_GlobalConfigs[Mods], iWinner, aMods);

			g_Admin_VoteSelectedOptions[admin] = 0;
			arrayset(g_Admin_VoteOptions[admin], 0, MAX_ADMIN_VOTEOPTIONS);

			new iArraySizeMaps = ArraySize(aMods[Maps]);

			if(ArraySize(aMods[Maps]) < 2)
			{
				new szMap[MAX_MAPNAME_LENGTH];
				ArrayGetString(aMods[Maps], 0, szMap, MAX_MAPNAME_LENGTH-1);

				AdminCmd_ResetGlobalData();
				g_Admin_VoteUserId = admin;
				
				g_Admin_SelectedMap[admin] = 1;
				ShowMenu_VoteAdminConfirm(admin, aMods[ModName], szMap, g_Admin_VoteStartInMod[admin]);

				client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MOD_CHO_ONLY_MAP", aMods[ModName], szMap);
				return PLUGIN_HANDLED;
			}

			new iMaxMaps = min(g_GlobalConfigs[MapsInMenu], iArraySizeMaps);

			set_pcvar_float(g_pCvar_amx_last_voting, get_gametime() + 15.0 + 2.0 + float(g_bCvar_amx_vote_delay_mm));

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

		client_print_color(0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_CHO_NO_RESULT");
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
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_VOTEMAP_NO_MAPS");
		return PLUGIN_HANDLED;
	}

	new sMenu[MAX_MENU_LENGTH];
	new iStartLoop = g_Admin_VoteCurrentPage[id] * 7;
	new iEndLoop = clamp(((g_Admin_VoteCurrentPage[id] + 1) * 7), 0, iArraySize);
	new iKeys = MENU_KEY_0;
	new j = 0;

	new iLen = formatex(sMenu, charsmax(sMenu), "%L^n^n", LANG_PLAYER, "MM_ADMIN_VOTEMAP_M_TITLE_VOTE", aMods[ModName], (g_Admin_VoteCurrentPage[id] + 1), ((iArraySize / 7) + ((iArraySize % 7) ? 1 : 0)));
	
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
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r8.\w %L^n", LANG_PLAYER, "MM_ADMIN_VOTEMAP_START_VOTE");
	}
	else
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\d8. %L^n", LANG_PLAYER, "MM_ADMIN_VOTEMAP_MIN_MAPS", g_GlobalConfigs[AdminMaxOptionsInMenu]);
	
	if(iEndLoop != iArraySize)
	{
		iKeys |= MENU_KEY_9;
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r9.\w %L^n\r0.\w %s^n", LANG_PLAYER, "MM_MORE", LANG_PLAYER, iStartLoop ? "MM_BACK" : "MM_EXIT");
	}
	else
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r0.\w %s", LANG_PLAYER, iStartLoop ? "MM_BACK" : "MM_EXIT");
	
	if(g_Admin_VoteSelectedOptions[id])
	{
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n^n%L", LANG_PLAYER, "MM_ADMIN_VOTEMOD_SELECTED_MODES", g_Admin_VoteSelectedOptions[id], IsFullSelected(id) ? fmt(" %L", LANG_PLAYER, "MM_MAX") : "");
		
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
		case 9:
		{
			if(--g_Admin_VoteCurrentPage[id] < 0)
			{
				ShowMenu_AdminMultimod(id);
				return PLUGIN_HANDLED;
			}
		}
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

	g_Menu_AdminCmd = menu_create(fmt("\y%L^n^n\y%L:", LANG_PLAYER, "MM_ADMIN_VOTEMAP_ADMIN_NAME", g_Admin_VoteUserId, LANG_PLAYER, "MM_SELECT_NEXT_MAP"), "menu_StartCustomVotemap");

	for(new i = 0; i < g_Admin_VoteSelectedOptions[g_Admin_VoteUserId]; ++i)
	{
		menu_additem(g_Menu_AdminCmd, fmt("\w%a%s", 
			ArrayGetStringHandle(aMods[Maps], g_Admin_VoteOptions[g_Admin_VoteUserId][i]), (i == (g_Admin_VoteSelectedOptions[g_Admin_VoteUserId]-1)) ? "^n" : ""));
	}

	menu_additem(g_Menu_AdminCmd, fmt("\y%L", LANG_PLAYER, "MM_SELECT_VOTE_RANDOM_ITEM"));

	menu_setprop(g_Menu_AdminCmd, MPROP_PERPAGE, 0);
	menu_setprop(g_Menu_AdminCmd, MPROP_EXIT, MEXIT_NEVER);

	UTIL_MenuDisplayAll(g_Menu_AdminCmd, _, 15);

	remove_task(g_Admin_VoteUserId + TASK_VOTE_ADMIN);
	set_task(15.1, "OnTask_AdminCountVotes_Map", g_Admin_VoteUserId + TASK_VOTE_ADMIN);
	
	if(g_bCvar_amx_multimod_voice)
		client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
		
	client_print_color(0, print_team_blue, "%s %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_VOTEMAP_START_VOTE_SAY", g_Admin_VoteUserId);
}

public menu_StartCustomVotemap(const id, const menuid, const item)
{
	CHECK_CONNECTED(id)
	CHECK_MENU_EXIT(item)
	CHECK_MENU_TIMEOUT(item)

	if(IsPlayer(g_Admin_VoteUserId) && task_exists(g_Admin_VoteUserId + TASK_VOTE_ADMIN))
	{
		new iMapId = item;
		new bool:bRandom = false;

		new szMap[MAX_MAPNAME_LENGTH];

		new aMods[ArrayMods_e];
		ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[g_Admin_VoteUserId], aMods);

		if(iMapId < g_Admin_VoteSelectedOptions[g_Admin_VoteUserId])
			ArrayGetString(aMods[Maps], g_Admin_VoteOptions[g_Admin_VoteUserId][iMapId], szMap, MAX_MAPNAME_LENGTH-1);
		else
		{
			iMapId = random(g_Admin_VoteSelectedOptions[g_Admin_VoteUserId]);
			ArrayGetString(aMods[Maps], g_Admin_VoteOptions[g_Admin_VoteUserId][iMapId], szMap, MAX_MAPNAME_LENGTH-1);

			bRandom = true;
		}

		++g_Admin_iVoteCountId[iMapId];
		++g_Admin_VoteCountTotal;

		g_Admin_VoteId[id] = iMapId+1;

		client_print_color(id, print_team_default, "%s^1 %L%s", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_VOTED_FOR", szMap, (bRandom) ? fmt("^3 [%s]", LANG_PLAYER, "MM_RANDOM") : "");

		if(g_bCvar_amx_vote_answers)
			client_print(0, print_console, "%L%s", LANG_PLAYER, "MM_PLAYER_VOTED_FOR", id, szMap, (bRandom) ? fmt("[%s]", LANG_PLAYER, "MM_RANDOM") : "");
	}

	return PLUGIN_HANDLED;
}

public OnTask_AdminCountVotes_Map(const taskid)
{
	new admin = (taskid - TASK_VOTE_ADMIN);

	CHECK_CONNECTED(admin)

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_Admin_SelectedMod[admin], aMods);

	new iWinner = 0;
	new iResult = 0;

	ResetAdminVoteMenuAll();
	menu_destroy(g_Menu_AdminCmd);

	client_print(0, print_console, "%L:", LANG_PLAYER, "MM_VOTE_RESULT");

	for(new i = 0; i < g_Admin_VoteSelectedOptions[admin]; ++i)
	{
		if(g_Admin_iVoteCountId[i] > iResult)
		{
			iResult = g_Admin_iVoteCountId[i];
			iWinner = i;
		}

		client_print(0, print_console, "%L: %s - %L: %d - %L: %d%%", LANG_PLAYER, "MM_MAP", ArrayGetStringHandle(aMods[Maps], g_Admin_VoteOptions[admin][i]), LANG_PLAYER, "MM_VOTES", g_Admin_iVoteCountId[i], LANG_PLAYER, "MM_PERCENT", UTIL_GetPercent(g_Admin_iVoteCountId[i], g_Admin_VoteCountTotal));
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

		client_print_color(0, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_MAP_CHO_FIN_NEXT", szMap, iResult, g_Admin_VoteCountTotal, UTIL_GetPercent(iResult, g_Admin_VoteCountTotal));

		g_Admin_SelectedMap[admin] = iWinner+1;
		ShowMenu_VoteAdminConfirm(admin, aMods[ModName], szMap, g_Admin_VoteStartInMod[admin]);
	}
	else
	{
		AdminCmd_ResetUserData(admin);
		client_print_color(0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_CHO_NO_RESULT");
	}

	AdminCmd_ResetGlobalData();
	return PLUGIN_HANDLED;
}

ShowMenu_VoteAdminConfirm(const id, const szMod[], const szMap[], const bool:bModWinner)
{
	CHECK_CONNECTED(id)

	new iMenu = menu_create(fmt("\y%L^n^n\y-\w %L\r:\y %s^n\y-\w %L\r:\y %s", LANG_PLAYER, "MM_VOTING_END", LANG_PLAYER, bModWinner ? "MM_MOD_WIN" : "MM_MODE", szMod, LANG_PLAYER, "MM_MAP_WIN", szMap), "menu_VoteAdminConfirm");

	menu_additem(iMenu, fmt("%L", LANG_PLAYER, "MM_ADMIN_VOTE_CONFIRM"));
	menu_additem(iMenu, fmt("%L", LANG_PLAYER, "MM_ADMIN_VOTE_REF"));

	menu_setprop(iMenu, MPROP_EXIT, MEXIT_NEVER);

	menu_display(id, iMenu);
	return PLUGIN_HANDLED;
}

public menu_VoteAdminConfirm(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)
	CHECK_EXIT_NEWMENU(id, menuid, item)

	switch(item)
	{
		case 0: OnFunc_ChangeModAndMap(id);
		case 1: client_print_color(0, print_team_blue, "%s %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_RES_REF", id);
	}

	menu_destroy(menuid);
	return PLUGIN_HANDLED;
}

ShowMenu_ManageMultiMod(const id)
{
	CHECK_CONNECTED(id)
	CHECK_ADMIN_ACCESS(id, g_GlobalConfigs[AdminFlags_ManageMods])

	new iMenu = menu_create(fmt("%L", LANG_PLAYER, "MM_ADMIN_MANAGE_MM_M_TITLE"), "menu_ManageMultiMod");

	menu_additem(iMenu, fmt("%L^n", LANG_PLAYER, "MM_ADMIN_MANAGE_MODS"));

	menu_additem(iMenu, fmt("%L", LANG_PLAYER, "MM_ADMIN_DELETE_MODS"));
	menu_additem(iMenu, fmt("%L^n", LANG_PLAYER, "MM_ADMIN_DELETE_MAPS", g_szCurrentMod));

	menu_additem(iMenu, fmt("%L", LANG_PLAYER, "MM_ADMIN_DELETE_ALL_MODS"));
	menu_additem(iMenu, fmt("%L", LANG_PLAYER, "MM_ADMIN_DELETE_ALL_MAPS", g_szCurrentMod));

	menu_setprop(iMenu, MPROP_EXITNAME, fmt("%L", LANG_PLAYER, "MM_EXIT"));
	
	menu_display(id, iMenu);
	return PLUGIN_HANDLED;
}

public menu_ManageMultiMod(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)
	CHECK_EXIT_NEWMENU(id, menuid, item)

	switch(item)
	{
		case 0: ShowMenu_ManageMods(id);
		case 1: {
			if(!g_GlobalConfigs[MaxRecentMods])
			{
				client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_CANT_DELETE_RECENT_MODS");
				return PLUGIN_HANDLED;
			}

			ShowMenu_DeleteRecentMods(id);
		}
		case 2: {
			if(!g_GlobalConfigs[MaxRecentMaps])
			{
				client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_CANT_DELETE_RECENT_MAPS");
				return PLUGIN_HANDLED;
			}

			ShowMenu_DeleteRecentMaps(id);
		}
		case 3:
		{
			if(!g_GlobalConfigs[MaxRecentMods])
			{
				client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_CANT_DELETE_RECENT_MODS");
				return PLUGIN_HANDLED;
			}

			DeleteAllRecentMods(id);
		}
		case 4:
		{
			if(!g_GlobalConfigs[MaxRecentMaps])
			{
				client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_CANT_DELETE_RECENT_MAPS");
				return PLUGIN_HANDLED;
			}

			DeleteAllRecentMaps(id);
		}
	}

	menu_destroy(menuid);
	return PLUGIN_HANDLED;
}

ShowMenu_ManageMods(const id, menupage=0)
{
	CHECK_CONNECTED(id)
	CHECK_ADMIN_ACCESS(id, g_GlobalConfigs[AdminFlags_ManageMods])

	new iMenu = menu_create(fmt("%L", LANG_PLAYER, "MM_ADMIN_M_TITLE_MANAGE_MODS"), "menu_ManageMods");

	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);
	new aMods[ArrayMods_e];

	for(new iModId = 0; iModId < iArraySizeMods; ++iModId)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);
		menu_additem(iMenu, fmt("%s%s", aMods[ModName], aMods[Enabled] ? "" : fmt("\r (%L)", LANG_PLAYER, "MM_LOCKED")));
	}

	menu_setprop(iMenu, MPROP_NEXTNAME, fmt("%L", LANG_PLAYER, "MM_MORE"));
	menu_setprop(iMenu, MPROP_BACKNAME, fmt("%L", LANG_PLAYER, "MM_BACK"));
	menu_setprop(iMenu, MPROP_EXITNAME, fmt("%L", LANG_PLAYER, "MM_EXIT"));

	menu_display(id, iMenu, min(menupage, menu_pages(iMenu) - 1));
	return PLUGIN_HANDLED;
}

public menu_ManageMods(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)

	if(item == MENU_EXIT)
	{
		menu_destroy(menuid);

		ShowMenu_ManageMultiMod(id);
		return PLUGIN_HANDLED;
	}

	new iNothing, iMenuPage;
	player_menu_info(id, iNothing, iNothing, iMenuPage);

	new iModId = item;
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	if(iModId == 0)
	{
		menu_destroy(menuid);

		client_print_color(id, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_CANT_BLOCK_MAIN_MODE", aMods[ModName]);
		ShowMenu_ManageMods(id, iMenuPage);
		return PLUGIN_HANDLED;
	}

	if(g_iCurrentMod == iModId)
	{
		menu_destroy(menuid);

		client_print_color(id, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_CANT_BLOCK_CURRENT_MODE", aMods[ModName]);
		ShowMenu_ManageMods(id, iMenuPage);
		return PLUGIN_HANDLED;
	}

	new bool:bTemp = aMods[Enabled];
	bTemp = !bTemp;
	aMods[Enabled] = bTemp;
	
	client_print_color(id, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, aMods[Enabled] ? "MM_ADMIN_MODE_ENABLED" : "MM_ADMIN_MODE_DISABLED", aMods[ModName]);

	ArraySetArray(g_GlobalConfigs[Mods], iModId, aMods);
	MultiMod_SaveOffMods();

	menu_destroy(menuid);
	ShowMenu_ManageMods(id, iMenuPage);
	return PLUGIN_HANDLED;
}

ShowMenu_DeleteRecentMods(const id, menupage=0)
{
	CHECK_CONNECTED(id)
	CHECK_ADMIN_ACCESS(id, g_GlobalConfigs[AdminFlags_ManageMods])

	new iArraySizeMods = Recent_CountRecentMods();

	if(iArraySizeMods == 0)
	{
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NO_RECENT_MODES_PLAYED");
		return PLUGIN_HANDLED;
	}

	new iMenu = menu_create(fmt("%L", LANG_PLAYER, "MM_ADMIN_DELETE_MODS"), "menu_DeleteRecentMods");

	for(new iModId = 0, aRecents[ArrayRecentMods_e]; iModId < iArraySizeMods; ++iModId)
	{
		ArrayGetArray(g_GlobalConfigs[RecentMods], iModId, aRecents);
		menu_additem(iMenu, aRecents[RECENT_MOD_NAME]);
	}
	
	menu_setprop(iMenu, MPROP_NEXTNAME, fmt("%L", LANG_PLAYER, "MM_MORE"));
	menu_setprop(iMenu, MPROP_BACKNAME, fmt("%L", LANG_PLAYER, "MM_BACK"));
	menu_setprop(iMenu, MPROP_EXITNAME, fmt("%L", LANG_PLAYER, "MM_EXIT"));

	menu_display(id, iMenu, min(menupage, menu_pages(iMenu) - 1));
	return PLUGIN_HANDLED;
}

public menu_DeleteRecentMods(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)

	if(item == MENU_EXIT)
	{
		menu_destroy(menuid);

		ShowMenu_ManageMultiMod(id);
		return PLUGIN_HANDLED;
	}

	new iMenuPage, iNothing, szItemName[MAX_MODNAME_LENGTH], iModId = item;
	player_menu_info(id, iNothing, iNothing, iMenuPage);

	menu_item_getinfo(menuid, item, iNothing, _, _, szItemName, MAX_MODNAME_LENGTH-1);
	menu_destroy(menuid);

	if(iModId < ArraySize(g_GlobalConfigs[RecentMods]))
	{
		new aRecents[ArrayRecentMods_e];
		ArrayGetArray(g_GlobalConfigs[RecentMods], iModId, aRecents);

		if(equal(aRecents[RECENT_MOD_NAME], szItemName))
		{
			ArrayDeleteItem(g_GlobalConfigs[RecentMods], iModId);
			
			client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_RECENT_MOD_DELETED", szItemName);

			SaveRecentMods();
		}
		else
			client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_RECENT_MOD_NOT_DELETED", szItemName);
	}
	else
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_RECENT_MOD_NOT_DELETED", szItemName);
		
	ShowMenu_DeleteRecentMods(id, iMenuPage);
	return PLUGIN_HANDLED;
}

ShowMenu_DeleteRecentMaps(const id, menupage=0)
{
	CHECK_CONNECTED(id)
	CHECK_ADMIN_ACCESS(id, g_GlobalConfigs[AdminFlags_ManageMods])

	if(Recent_CountRecentMaps(g_iCurrentMod) < 1)
	{
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NO_RECENT_MAPS_PLAYED");
		return PLUGIN_HANDLED;
	}

	new iMenu = menu_create(fmt("%L", LANG_PLAYER, "MM_ADMIN_DELETE_MAPS", g_szCurrentMod), "menu_DeleteRecentMaps");
	new iArraySizeMaps = ArraySize(g_GlobalConfigs[RecentMaps]);

	for(new iMapId = 0, aRecents[ArrayRecentMaps_e]; iMapId < iArraySizeMaps; ++iMapId)
	{
		ArrayGetArray(g_GlobalConfigs[RecentMaps], iMapId, aRecents);

		if(g_iCurrentMod != UTIL_GetModId(aRecents[RECENT_MOD_NAME]))
			continue;

		menu_additem(iMenu, aRecents[RECENT_MAP_NAME], fmt("%d", iMapId));
	}
	
	menu_setprop(iMenu, MPROP_NEXTNAME, fmt("%L", LANG_PLAYER, "MM_MORE"));
	menu_setprop(iMenu, MPROP_BACKNAME, fmt("%L", LANG_PLAYER, "MM_BACK"));
	menu_setprop(iMenu, MPROP_EXITNAME, fmt("%L", LANG_PLAYER, "MM_EXIT"));

	menu_display(id, iMenu, min(menupage, menu_pages(iMenu) - 1));
	return PLUGIN_HANDLED;
}

public menu_DeleteRecentMaps(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)

	if(item == MENU_EXIT)
	{
		menu_destroy(menuid);

		ShowMenu_ManageMultiMod(id);
		return PLUGIN_HANDLED;
	}

	new iMenuPage, iNothing, szItemIndex[11], szItemName[MAX_MODNAME_LENGTH];
	player_menu_info(id, iNothing, iNothing, iMenuPage);

	menu_item_getinfo(menuid, item, iNothing, szItemIndex, charsmax(szItemIndex), szItemName, MAX_MODNAME_LENGTH-1);
	menu_destroy(menuid);

	new iMapId = str_to_num(szItemIndex);

	if(iMapId < ArraySize(g_GlobalConfigs[RecentMaps]))
	{
		new aRecents[ArrayRecentMaps_e];
		ArrayGetArray(g_GlobalConfigs[RecentMaps], iMapId, aRecents);

		if((g_iCurrentMod == UTIL_GetModId(aRecents[RECENT_MOD_NAME])) && equal(aRecents[RECENT_MAP_NAME], szItemName))
		{
			ArrayDeleteItem(g_GlobalConfigs[RecentMaps], iMapId);
			
			client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_RECENT_MAP_DELETED", szItemName, g_szCurrentMod);

			SaveRecentMaps();
		}
		else
			client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_RECENT_MAP_NOT_DELETED", szItemName, g_szCurrentMod);
	}
	else
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_RECENT_MAP_NOT_DELETED", szItemName, g_szCurrentMod);
		
	ShowMenu_DeleteRecentMaps(id, iMenuPage);
	return PLUGIN_HANDLED;
}

DeleteAllRecentMods(const id)
{
	CHECK_CONNECTED(id)
	CHECK_ADMIN_ACCESS(id, g_GlobalConfigs[AdminFlags_ManageMods])

	new iArraySizeMods = Recent_CountRecentMods();

	if(iArraySizeMods == 0)
	{
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NO_RECENT_MODES_PLAYED");

		ShowMenu_ManageMultiMod(id);
		return PLUGIN_HANDLED;
	}

	ArrayClear(g_GlobalConfigs[RecentMods]);
	
	client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_RECENT_MOD_DELETED_ALL");

	SaveRecentMaps();

	ShowMenu_ManageMultiMod(id);
	return PLUGIN_HANDLED;
}

DeleteAllRecentMaps(const id)
{
	CHECK_CONNECTED(id)
	CHECK_ADMIN_ACCESS(id, g_GlobalConfigs[AdminFlags_ManageMods])

	if(Recent_CountRecentMaps(g_iCurrentMod) < 1)
	{
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NO_RECENT_MAPS_PLAYED");

		ShowMenu_ManageMultiMod(id);
		return PLUGIN_HANDLED;
	}

	new iArraySizeMaps = ArraySize(g_GlobalConfigs[RecentMaps]);

	for(new iMapId = 0, aRecents[ArrayRecentMaps_e]; iMapId < iArraySizeMaps; ++iMapId)
	{
		ArrayGetArray(g_GlobalConfigs[RecentMaps], iMapId, aRecents);

		if(g_iCurrentMod != UTIL_GetModId(aRecents[RECENT_MOD_NAME]))
			continue;

		ArrayDeleteItem(g_GlobalConfigs[RecentMaps], iMapId);
		--iMapId;

		iArraySizeMaps = ArraySize(g_GlobalConfigs[RecentMaps]);
	}

	client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ADMIN_RECENT_MAP_DELETED_ALL", g_szCurrentMod);

	SaveRecentMaps();

	ShowMenu_ManageMultiMod(id);
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
	return bool:(g_Admin_VoteSelectedOptions[id] == g_GlobalConfigs[AdminMaxOptionsInMenu]);
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
	for(new i = 0; (i < g_Admin_VoteSelectedOptions[id]) && (iPos == -1); ++i)
	{
		if(g_Admin_VoteOptions[id][i] == value)
			iPos = i;
	}

	if(iPos != -1)
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
	for(new i = 1; i < MAX_USERS; ++i) if(GetPlayerBit(g_bConnected, i) && (g_Admin_VoteId[i] == -1))
	{
		menu_cancel(i);
		reset_menu(i);
	}

	return true;
}