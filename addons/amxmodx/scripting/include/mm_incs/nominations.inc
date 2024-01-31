#if defined _mm_nominations_included_
	#endinput
#endif
#define _mm_nominations_included_

Nominations_Init()
{
	UTIL_RegisterClientCommandAll("nom", "ClientCommand_Nominate");
	
	Nominations_ResetAllData();
}

Nominations_ResetAllData()
{
	ArrayClear(g_Array_Nominations);

	arrayset(g_SVM_MyNominate, 0, MAX_USERS);
}

Nominations_ClientPutInServer(const id)
{
	g_SVM_MyNominate[id] = 0;
}

Nominations_ClientDisconnected(const id)
{
	if(g_SVM_MyNominate[id]) // Si se desconecta, eliminar su nominacion
		DeleteNominated(g_SVM_MyNominate[id]-1);

	g_SVM_MyNominate[id] = 0;
}

public ClientCommand_Nominate(const id)
{
	CHECK_CONNECTED(id)

	if(!g_bSelectedNextMod && !g_bVoteModHasStarted && !g_bSVM_ModSecondRound)
	{
		if(!g_GlobalConfigs[Nom_Mods_Enabled])
		{
			client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NOMS_MOD_NOT_ENABLED");
			return PLUGIN_HANDLED;
		}

		ShowMenu_NominateMod(id);
		return PLUGIN_HANDLED;
	}
	else if(!g_bSelectedNextMap && !g_bVoteMapHasStarted && !g_bSelectedNextMap)
	{
		if(!g_GlobalConfigs[Nom_Maps_Enabled])
		{
			client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NOMS_MAP_NOT_ENABLED");
			return PLUGIN_HANDLED;
		}

		ShowMenu_NominateMap(id);
		return PLUGIN_HANDLED;
	}

	client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NOMS_NOT_ALLOWED");
	return PLUGIN_HANDLED;
}

ShowMenu_NominateMod(const id, menupage=0)
{
	CHECK_CONNECTED(id)

	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);

	if((iArraySizeMods - 1 - UTIL_GetOffModsCount() - Recent_CountRecentMods()) <= g_GlobalConfigs[ModsInMenu])
	{
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NOMS_MODS_NOT_ENOUGH");
		return PLUGIN_HANDLED;
	}

	new iMenu = menu_create(fmt("\y%L:", LANG_PLAYER, "MM_NOMS_M_TITLE_NOMINATE_MODS"), "menu_NominateMod");

	for(new iModId = 0, aData[ArrayMods_e]; iModId < iArraySizeMods; ++iModId)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], iModId, aData);

		if(g_iCurrentMod == iModId)
			menu_additem(iMenu, fmt("%s\y (%L)", aData[ModName], LANG_PLAYER, "MM_CURRENT"), .callback=menu_makecallback("UTIL_DisableItem"));
		else if(UTIL_IsModOff(iModId))
			menu_additem(iMenu, fmt("%s\r (%L)", aData[ModName], LANG_PLAYER, "MM_LOCKED"), .callback=menu_makecallback("UTIL_DisableItem"));
		else if(Recent_IsRecentMod(aData[ModName]))
			menu_additem(iMenu, fmt("%s\r (%L)", aData[ModName], LANG_PLAYER, "MM_RECENT"), .callback=menu_makecallback("UTIL_DisableItem"));
		else if(IsNominated(iModId))
			menu_additem(iMenu, fmt("%s\y (%L)", aData[ModName], LANG_PLAYER, ((iModId == (g_SVM_MyNominate[id]-1)) ? "MM_YOU_NOMINATED" : "MM_NOMINATED")));
		else
			menu_additem(iMenu, fmt("%s", aData[ModName]));
	}

	menu_setprop(iMenu, MPROP_NEXTNAME, fmt("%L", LANG_PLAYER, "MM_MORE"));
	menu_setprop(iMenu, MPROP_BACKNAME, fmt("%L", LANG_PLAYER, "MM_BACK"));
	menu_setprop(iMenu, MPROP_EXITNAME, fmt("%L", LANG_PLAYER, "MM_EXIT"));

	menu_display(id, iMenu, min(menupage, menu_pages(iMenu) - 1));
	return PLUGIN_HANDLED;
}

public menu_NominateMod(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)
	CHECK_EXIT_NEWMENU(id, menuid, item)

	new iModId = item;
	new aData[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aData);

	if(IsNominated(iModId))
	{
		if(iModId == (g_SVM_MyNominate[id]-1))
		{
			DeleteNominated(g_SVM_MyNominate[id]-1);

			g_SVM_MyNominate[id] = 0;
			client_print_color(0, print_team_red, "%s %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NOMS_PLAYER_DEL_NOM_MOD", id, aData[ModName]);
		}
		else
			client_print_color(id, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NOMS_ALREADY_NOMINATED_MOD", aData[ModName]);
	}
	else
	{
		if(g_SVM_MyNominate[id])
			client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NOMS_ALREADY_NOMINATED_MOD_INFO1", aData[ModName]);
		else
		{
			ArrayPushCell(g_Array_Nominations, iModId);

			g_SVM_MyNominate[id] = iModId+1;
			client_print_color(0, print_team_blue, "%s %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NOMS_PLAYER_NOMINATED_MOD", id, aData[ModName]);
		}
	}

	new iNothing, iMenuPage;
	player_menu_info(id, iNothing, iNothing, iMenuPage);

	menu_destroy(menuid);
	ShowMenu_NominateMod(id, iMenuPage);
	return PLUGIN_HANDLED;
}

ShowMenu_NominateMap(const id, menupage=0)
{
	CHECK_CONNECTED(id)

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);

	new iArraySizeMaps = ArraySize(aMods[Maps]);

	if((iArraySizeMaps - Recent_CountRecentMaps(g_iNextSelectMod)) <= g_GlobalConfigs[MapsInMenu])
	{
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NOMS_MAPS_NOT_ENOUGH");
		return PLUGIN_HANDLED;
	}

	new iMenu = menu_create(fmt("\y%L:", LANG_PLAYER, "MM_NOMS_M_TITLE_NOMINATE_MAPS"), "menu_NominateMap");

	for(new iMapId = 0, szMapName[MAX_MAPNAME_LENGTH]; iMapId < iArraySizeMaps; ++iMapId)
	{
		ArrayGetString(aMods[Maps], iMapId, szMapName, MAX_MAPNAME_LENGTH-1);

		if(Recent_IsRecentMap(g_iNextSelectMod, szMapName))
			menu_additem(iMenu, fmt("%s\r (%L)", szMapName, LANG_PLAYER, "MM_RECENT"), .callback=menu_makecallback("UTIL_DisableItem"));
		else if(IsNominated(iMapId))
			menu_additem(iMenu, fmt("%s\y (%L)", szMapName, LANG_PLAYER, ((iMapId == (g_SVM_MyNominate[id]-1)) ? "MM_YOU_NOMINATED" : "MM_NOMINATED")));
		else
			menu_additem(iMenu, fmt("%s", szMapName));
	}

	menu_setprop(iMenu, MPROP_NEXTNAME, fmt("%L", LANG_PLAYER, "MM_MORE"));
	menu_setprop(iMenu, MPROP_BACKNAME, fmt("%L", LANG_PLAYER, "MM_BACK"));
	menu_setprop(iMenu, MPROP_EXITNAME, fmt("%L", LANG_PLAYER, "MM_EXIT"));

	menu_display(id, iMenu, min(menupage, menu_pages(iMenu) - 1));
	return PLUGIN_HANDLED;
}

public menu_NominateMap(const id, const menuid, const item)
{
	CHECK_CONNECTED_NEWMENU(id, menuid)
	CHECK_EXIT_NEWMENU(id, menuid, item)

	new iMapId = item;

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);

	if(IsNominated(iMapId))
	{
		if(iMapId == (g_SVM_MyNominate[id]-1))
		{
			DeleteNominated(g_SVM_MyNominate[id]-1);

			g_SVM_MyNominate[id] = 0;
			client_print_color(0, print_team_red, "%s %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NOMS_PLAYER_DEL_NOM_MAP", id, ArrayGetStringHandle(aMods[Maps], iMapId));
		}
		else
			client_print_color(id, print_team_blue, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NOMS_ALREADY_NOMINATED_MAP", ArrayGetStringHandle(aMods[Maps], iMapId));
	}
	else
	{
		if(g_SVM_MyNominate[id])
			client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NOMS_ALREADY_NOMINATED_MAP_INFO1");
		else
		{
			ArrayPushCell(g_Array_Nominations, iMapId);

			g_SVM_MyNominate[id] = iMapId+1;
			client_print_color(0, print_team_blue, "%s %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_NOMS_PLAYER_NOMINATED_MAP", id, ArrayGetStringHandle(aMods[Maps], iMapId));
		}
	}

	new iNothing, iMenuPage;
	player_menu_info(id, iNothing, iNothing, iMenuPage);

	menu_destroy(menuid);
	ShowMenu_NominateMap(id, iMenuPage);
	return PLUGIN_HANDLED;
}

bool:IsNominated(const iNom)
{
	new iArraySizeNoms = ArraySize(g_Array_Nominations);
	for(new i = 0; i < iArraySizeNoms; ++i)
	{
		if(ArrayGetCell(g_Array_Nominations, i) == iNom)
			return true;
	}

	return false;
}

bool:DeleteNominated(const iNom)
{
	new iArraySizeNoms = ArraySize(g_Array_Nominations);
	for(new i = 0; i < iArraySizeNoms; ++i)
	{
		if(ArrayGetCell(g_Array_Nominations, i) == iNom)
		{
			ArrayDeleteItem(g_Array_Nominations, i);
			return true;
		}
	}

	return false;
}