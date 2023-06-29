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
			client_print_color(id, print_team_default, "%s^1 La nominación de modos se encuentra deshabilitada!", g_GlobalConfigs[ChatPrefix]);
			return PLUGIN_HANDLED;
		}

		ShowMenu_NominateMod(id);
		return PLUGIN_HANDLED;
	}
	else if(!g_bSelectedNextMap && !g_bVoteMapHasStarted && !g_bSelectedNextMap)
	{
		if(!g_GlobalConfigs[Nom_Maps_Enabled])
		{
			client_print_color(id, print_team_default, "%s^1 La nominación de mapas se encuentra deshabilitada!", g_GlobalConfigs[ChatPrefix]);
			return PLUGIN_HANDLED;
		}

		ShowMenu_NominateMap(id);
		return PLUGIN_HANDLED;
	}

	client_print_color(id, print_team_default, "%s^1 Nominaciones no disponibles en este momento!", g_GlobalConfigs[ChatPrefix]);
	return PLUGIN_HANDLED;
}

ShowMenu_NominateMod(const id, menupage=0)
{
	CHECK_CONNECTED(id)

	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);

	if(likely((iArraySizeMods - 1 - UTIL_GetOffModsCount() - Recent_CountRecentMods()) <= g_GlobalConfigs[ModsInMenu]))
	{
		client_print_color(id, print_team_default, "%s^1 No hay suficientes modos para nominar en este momento!", g_GlobalConfigs[ChatPrefix]);
		return PLUGIN_HANDLED;
	}

	new iMenu = menu_create("\yNominar próximo modo:\R", "menu_NominateMod");

	for(new iModId = 0, aData[ArrayMods_e]; iModId < iArraySizeMods; ++iModId)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], iModId, aData);

		if(g_iCurrentMod == iModId)
			menu_additem(iMenu, fmt("%s\y (ACTUAL)", aData[ModName]), .callback=menu_makecallback("UTIL_DisableItem"));
		else if(UTIL_IsModOff(iModId))
			menu_additem(iMenu, fmt("%s\r (BLOQUEADO)", aData[ModName]), .callback=menu_makecallback("UTIL_DisableItem"));
		else if(Recent_IsRecentMod(aData[ModName]))
			menu_additem(iMenu, fmt("%s\r (RECIENTE)", aData[ModName]), .callback=menu_makecallback("UTIL_DisableItem"));
		else
			menu_additem(iMenu, fmt("%s%s", aData[ModName], IsNominated(iModId) ? ((iModId == (g_SVM_MyNominate[id]-1)) ? "\y [TU NOMINASTE]" : "\y [NOMINADO]"): ""));
	}

	menu_setprop(iMenu, MPROP_NEXTNAME, "Siguiente");
	menu_setprop(iMenu, MPROP_BACKNAME, "Atrás");
	menu_setprop(iMenu, MPROP_EXITNAME, "Salir");

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
			client_print_color(0, print_team_red, "%s^3 %n^1 ha eliminado de las nominaciones el modo:^4 %s", g_GlobalConfigs[ChatPrefix], id, aData[ModName]);
		}
		else
			client_print_color(id, print_team_blue, "%s^1 El modo^4 %s^1 ya fue nominado!", g_GlobalConfigs[ChatPrefix], aData[ModName]);
	}
	else
	{
		if(g_SVM_MyNominate[id])
			client_print_color(id, print_team_default, "%s^1 No podes nominar más de un modo a la vez!", g_GlobalConfigs[ChatPrefix]);
		else
		{
			ArrayPushCell(g_Array_Nominations, iModId);

			g_SVM_MyNominate[id] = iModId+1;
			client_print_color(0, print_team_blue, "%s^3 %n^1 ha nominado el modo:^4 %s", g_GlobalConfigs[ChatPrefix], id, aData[ModName]);
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

	if(likely((iArraySizeMaps - Recent_CountRecentMaps(g_iNextSelectMod)) <= g_GlobalConfigs[MapsInMenu]))
	{
		client_print_color(id, print_team_default, "%s^1 No hay suficientes mapas para nominar en este momento!", g_GlobalConfigs[ChatPrefix]);
		return PLUGIN_HANDLED;
	}

	new iMenu = menu_create("\yNominar próximo mapa:\R", "menu_NominateMap");

	for(new iMapId = 0, szMapName[MAX_MAPNAME_LENGTH]; iMapId < iArraySizeMaps; ++iMapId)
	{
		ArrayGetString(aMods[Maps], iMapId, szMapName, MAX_MAPNAME_LENGTH-1);

		if(Recent_IsRecentMap(g_iNextSelectMod, szMapName))
			menu_additem(iMenu, fmt("%s\r (RECIENTE)", szMapName), .callback=menu_makecallback("UTIL_DisableItem"));
		else
			menu_additem(iMenu, fmt("%s%s", szMapName, IsNominated(iMapId) ? ((iMapId == (g_SVM_MyNominate[id]-1)) ? "\y [TU NOMINASTE]" : "\y (NOMINADO)"): ""));
	}

	menu_setprop(iMenu, MPROP_NEXTNAME, "Siguiente");
	menu_setprop(iMenu, MPROP_BACKNAME, "Atrás");
	menu_setprop(iMenu, MPROP_EXITNAME, "Salir");

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
			client_print_color(0, print_team_red, "%s^3 %n^1 ha eliminado de las nominaciones el mapa:^4 %a", g_GlobalConfigs[ChatPrefix], id, ArrayGetStringHandle(aMods[Maps], iMapId));
		}
		else
			client_print_color(id, print_team_blue, "%s^1 El mapa^4 %a^1 ya fue nominado!", g_GlobalConfigs[ChatPrefix], ArrayGetStringHandle(aMods[Maps], iMapId));
	}
	else
	{
		if(g_SVM_MyNominate[id])
			client_print_color(id, print_team_default, "%s^1 No podes nominar más de un mapa a la vez!", g_GlobalConfigs[ChatPrefix]);
		else
		{
			ArrayPushCell(g_Array_Nominations, iMapId);

			g_SVM_MyNominate[id] = iMapId+1;
			client_print_color(0, print_team_blue, "%s^3 %n^1 ha nominado el mapa:^4 %a", g_GlobalConfigs[ChatPrefix], id, ArrayGetStringHandle(aMods[Maps], iMapId));
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