#if defined _mm_recent_mods_maps_included_
	#endinput
#endif
#define _mm_recent_mods_maps_included_

Recent_LoadRecentModsMaps()
{
	ArrayClear(g_GlobalConfigs[RecentMods]);
	ArrayClear(g_GlobalConfigs[RecentMaps]);

	if(!g_GlobalConfigs[MaxRecentMods] && !g_GlobalConfigs[MaxRecentMaps])
		return;

	new szFileName[PLATFORM_MAX_PATH];
	get_configsdir(szFileName, charsmax(szFileName));
	add(szFileName, charsmax(szFileName), "/multimod_manager/recent_mods_maps.json");

	if(!file_exists(szFileName))
		return;

	new JSON:jRecentModsMaps = json_parse(szFileName, true);

	if(jRecentModsMaps == Invalid_JSON)
	{
		abort(AMX_ERR_GENERAL, "Error al analizar el archivo [%s]", szFileName);
		return;
	}

	if(g_GlobalConfigs[MaxRecentMods] && json_is_object(jRecentModsMaps) && json_object_has_value(jRecentModsMaps, "recent_mods", JSONArray))
	{
		new JSON:jRecentMods = json_object_get_value(jRecentModsMaps, "recent_mods");

		for(new i = 0, szModName[MAX_MODNAME_LENGTH], iMaxLoop = json_array_get_count(jRecentMods); i < iMaxLoop; ++i)
		{
			json_array_get_string(jRecentMods, i, szModName, MAX_MODNAME_LENGTH-1);

			if(UTIL_GetModId(szModName) > -1)
				ArrayPushString(g_GlobalConfigs[RecentMods], szModName);
		}

		json_free(jRecentMods);
	}

	if(g_GlobalConfigs[MaxRecentMaps] && json_is_object(jRecentModsMaps) && json_object_has_value(jRecentModsMaps, "recent_maps", JSONObject))
	{
		new JSON:jRecentMaps = json_object_get_value(jRecentModsMaps, "recent_maps");

		for(new i = 0, j, iModId, iMapNum,
			szModName[MAX_MODNAME_LENGTH],
			szMapName[MAX_MAPNAME_LENGTH],
			JSON:jObject, iObjectCount,
			aRecents[ArrayRecentMaps_e],
			iMaxLoop = json_object_get_count(jRecentMaps); i < iMaxLoop; ++i)
		{
			json_object_get_name(jRecentMaps, i, szModName, MAX_MODNAME_LENGTH-1);

			if((iModId = UTIL_GetModId(szModName)) > -1)
			{
				jObject = json_object_get_value(jRecentMaps, szModName);

				for(j = 0, iObjectCount = json_object_get_count(jObject); j < iObjectCount; ++j)
				{
					json_object_get_name(jObject, j, szMapName, MAX_MAPNAME_LENGTH-1);
					iMapNum = json_object_get_number(jObject, szMapName) - (likely(g_iCurrentMod == iModId) ? 1 : 0);

					if(iMapNum > 0)
					{
						copy(aRecents[RECENT_MOD_NAME], MAX_MODNAME_LENGTH-1, szModName);
						copy(aRecents[RECENT_MAP_NAME], MAX_MAPNAME_LENGTH-1, szMapName);
						aRecents[RECENT_MAP_COUNT] = iMapNum;

						ArrayPushArray(g_GlobalConfigs[RecentMaps], aRecents);
					}
				}

				json_free(jObject);
			}
		}

		json_free(jRecentMaps);
	}

	json_free(jRecentModsMaps);
}

Recent_SaveRecentModsMaps(const iMod, const szMapName[MAX_MAPNAME_LENGTH])
{
	if(!g_GlobalConfigs[MaxRecentMods] && !g_GlobalConfigs[MaxRecentMaps])
		return;

	new JSON:root_value = json_init_object();

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iMod, aMods);

	if(g_GlobalConfigs[MaxRecentMods])
	{
		new JSON:array = json_init_array();
		json_array_append_string(array, aMods[ModName]);

		new iMaxLoop = min(g_GlobalConfigs[MaxRecentMods], ArraySize(g_GlobalConfigs[RecentMods]));
		for(new i = 0, szModName[MAX_MODNAME_LENGTH]; i < iMaxLoop; ++i)
		{
			ArrayGetString(g_GlobalConfigs[RecentMods], i, szModName, MAX_MODNAME_LENGTH-1);
			json_array_append_string(array, szModName);
		}

		json_object_set_value(root_value, "recent_mods", array);
		json_free(array);
	}

	if(g_GlobalConfigs[MaxRecentMaps])
	{
		new iMaxLoop = ArraySize(g_GlobalConfigs[RecentMaps]);
		for(new i = 0, aRecents[ArrayRecentMaps_e]; i < iMaxLoop; ++i)
		{
			ArrayGetArray(g_GlobalConfigs[RecentMaps], i, aRecents);
			json_object_set_number(root_value, fmt("recent_maps.%s.%s", aRecents[RECENT_MOD_NAME], aRecents[RECENT_MAP_NAME]), aRecents[RECENT_MAP_COUNT], true);
		}

		json_object_set_number(root_value, fmt("recent_maps.%s.%s", aMods[ModName], szMapName), g_GlobalConfigs[MaxRecentMods], true);
	}

	new szFileName[PLATFORM_MAX_PATH];
	get_configsdir(szFileName, charsmax(szFileName));
	add(szFileName, charsmax(szFileName), "/multimod_manager/recent_mods_maps.json");

	json_serial_to_file(root_value, szFileName, true);
	json_free(root_value);
}

Recent_CountRecentMods()
{
	return ArraySize(g_GlobalConfigs[RecentMods]);
}

bool:Recent_IsRecentMod(const szModName[])
{
	new iMaxLoop = ArraySize(g_GlobalConfigs[RecentMods]);
	for(new i = 0, szGetModName[MAX_MODNAME_LENGTH]; i < iMaxLoop; ++i)
	{
		ArrayGetString(g_GlobalConfigs[RecentMods], i, szGetModName, MAX_MODNAME_LENGTH-1);
		
		if(equali(szModName, szGetModName))
			return true;
	}

	return false;
}

Recent_CountRecentMaps(const iMod)
{
	new iCount = 0;
	new iMaxLoop = ArraySize(g_GlobalConfigs[RecentMaps]);
	for(new i = 0, aRecents[ArrayRecentMaps_e]; i < iMaxLoop; ++i)
	{
		ArrayGetArray(g_GlobalConfigs[RecentMaps], i, aRecents);

		if(likely(iMod == UTIL_GetModId(aRecents[RECENT_MOD_NAME])))
			++iCount;
	}
	
	return iCount;
}

bool:Recent_IsRecentMap(const iMod, const szMapName[])
{
	new iMaxLoop = ArraySize(g_GlobalConfigs[RecentMaps]);
	for(new i = 0, aRecents[ArrayRecentMaps_e]; i < iMaxLoop; ++i)
	{
		ArrayGetArray(g_GlobalConfigs[RecentMaps], i, aRecents);

		if(likely(iMod == UTIL_GetModId(aRecents[RECENT_MOD_NAME])) && equali(aRecents[RECENT_MAP_NAME], szMapName))
			return true;
	}

	return false;
}