#if defined _mm_recent_mods_maps_included_
	#endinput
#endif
#define _mm_recent_mods_maps_included_

Recent_LoadRecentModsMaps()
{
	LoadRecentMods();
	LoadRecentMaps();
}

Recent_SaveRecentModsMaps()
{
	SaveRecentMods();
	SaveRecentMaps();
}

LoadRecentMods()
{
	if(!g_GlobalConfigs[MaxRecentMods])
		return;
	
	ArrayClear(g_GlobalConfigs[RecentMods]);
	
	new szFileName[PLATFORM_MAX_PATH];
	new iLen = get_configsdir(szFileName, charsmax(szFileName));
	formatex(szFileName[iLen], charsmax(szFileName) - iLen, "/%s/%s", MM_CONFIG_FOLDER, MM_RECENT_MODS_FILENAME);

	if(!file_exists(szFileName))
		return;

	new JSON:jRecentModsFile = json_parse(szFileName, true);

	if(jRecentModsFile == Invalid_JSON)
		log_to_file(MM_LOG_FILENAME, "[MULTIMOD] %L [%s]", LANG_SERVER, "MM_INVALID_JSON_FILE", szFileName);
	else
	{
		new aRecents[ArrayRecentMods_e];
		copy(aRecents[RECENT_MOD_NAME], MAX_MODNAME_LENGTH-1, g_szCurrentMod);
		aRecents[RECENT_MOD_SYSTIME] = get_systime();

		ArrayPushArray(g_GlobalConfigs[RecentMods], aRecents);

		if(json_is_object(jRecentModsFile) && json_object_has_value(jRecentModsFile, "recent_mods", JSONObject))
		{
			new JSON:jRecentMods = json_object_get_value(jRecentModsFile, "recent_mods");

			for(new i = 0, iModId, iMaxLoop = json_object_get_count(jRecentMods); i < iMaxLoop; ++i)
			{
				json_object_get_name(jRecentMods, i, aRecents[RECENT_MOD_NAME], MAX_MODNAME_LENGTH-1);
				aRecents[RECENT_MOD_SYSTIME] = json_object_get_number(jRecentMods, aRecents[RECENT_MOD_NAME]);

				if((iModId = UTIL_GetModId(aRecents[RECENT_MOD_NAME])) != -1)
				{
					if(likely(g_iCurrentMod == iModId))
						continue;
					
					ArrayPushArray(g_GlobalConfigs[RecentMods], aRecents);
				}
			}

			json_free(jRecentMods);
		}
		
		json_free(jRecentModsFile);
	}
}

LoadRecentMaps()
{
	if(!g_GlobalConfigs[MaxRecentMaps])
		return;

	ArrayClear(g_GlobalConfigs[RecentMaps]);

	new szFileName[PLATFORM_MAX_PATH];
	new iLen = get_configsdir(szFileName, charsmax(szFileName));
	formatex(szFileName[iLen], charsmax(szFileName) - iLen, "/%s/%s", MM_CONFIG_FOLDER, MM_RECENT_MAPS_FILENAME);

	if(!file_exists(szFileName))
		return;

	new JSON:jRecentMapsFile = json_parse(szFileName, true);

	if(jRecentMapsFile == Invalid_JSON)
		log_to_file(MM_LOG_FILENAME, "[MULTIMOD] %L [%s]", LANG_SERVER, "MM_INVALID_JSON_FILE", szFileName);
	else
	{
		new aRecents[ArrayRecentMaps_e];
		copy(aRecents[RECENT_MOD_NAME], MAX_MODNAME_LENGTH-1, g_szCurrentMod);
		copy(aRecents[RECENT_MAP_NAME], MAX_MAPNAME_LENGTH-1, g_szCurrentMap);
		aRecents[RECENT_MAP_COUNT] = g_GlobalConfigs[MaxRecentMaps];
		aRecents[RECENT_MAP_SYSTIME] = get_systime();

		ArrayPushArray(g_GlobalConfigs[RecentMaps], aRecents);

		if(json_is_object(jRecentMapsFile) && json_object_has_value(jRecentMapsFile, "recent_maps", JSONObject))
		{
			new JSON:jRecentMaps = json_object_get_value(jRecentMapsFile, "recent_maps");

			for(new i = 0, j, iModId, iMapNum,
				szModName[MAX_MODNAME_LENGTH],
				szMapName[MAX_MAPNAME_LENGTH],
				JSON:jObject, iObjectCount,
				JSON:jArrayMapData,
				iMaxLoop = json_object_get_count(jRecentMaps); i < iMaxLoop; ++i)
			{
				json_object_get_name(jRecentMaps, i, szModName, MAX_MODNAME_LENGTH-1);

				if((iModId = UTIL_GetModId(szModName)) != -1)
				{
					jObject = json_object_get_value(jRecentMaps, szModName);

					for(j = 0, iObjectCount = json_object_get_count(jObject); j < iObjectCount; ++j)
					{
						json_object_get_name(jObject, j, szMapName, MAX_MAPNAME_LENGTH-1);
						jArrayMapData = json_object_get_value(jObject, szMapName);
						
						if(likely(g_iCurrentMod == iModId) && equali(g_szCurrentMap, szMapName))
							continue;
						
						iMapNum = json_array_get_number(jArrayMapData, 0) - (likely(g_iCurrentMod == iModId) ? 1 : 0);

						if(iMapNum > 0)
						{
							copy(aRecents[RECENT_MOD_NAME], MAX_MODNAME_LENGTH-1, szModName);
							copy(aRecents[RECENT_MAP_NAME], MAX_MAPNAME_LENGTH-1, szMapName);
							aRecents[RECENT_MAP_COUNT] = iMapNum;
							aRecents[RECENT_MAP_SYSTIME] = json_array_get_number(jArrayMapData, 1);

							ArrayPushArray(g_GlobalConfigs[RecentMaps], aRecents);
						}

						json_free(jArrayMapData);
					}

					json_free(jObject);
				}
			}

			json_free(jRecentMaps);
		}

		json_free(jRecentMapsFile);
	}
}

SaveRecentMods()
{
	if(!g_GlobalConfigs[MaxRecentMods])
		return;

	new szFileName[PLATFORM_MAX_PATH];
	new iLen = get_configsdir(szFileName, charsmax(szFileName));
	formatex(szFileName[iLen], charsmax(szFileName) - iLen, "/%s/%s", MM_CONFIG_FOLDER, MM_RECENT_MODS_FILENAME);

	new JSON:root_value = json_init_object();

	new iMaxLoop = min(g_GlobalConfigs[MaxRecentMods], ArraySize(g_GlobalConfigs[RecentMods]));
	for(new i = 0, aRecents[ArrayRecentMods_e]; i < iMaxLoop; ++i)
	{
		ArrayGetArray(g_GlobalConfigs[RecentMods], i, aRecents);
		json_object_set_number(root_value, fmt("recent_mods.%s", aRecents[RECENT_MOD_NAME]), aRecents[RECENT_MOD_SYSTIME], true);
	}

	json_serial_to_file(root_value, szFileName, true);
	json_free(root_value);
}

SaveRecentMaps()
{
	if(!g_GlobalConfigs[MaxRecentMaps])
		return;
	
	new szFileName[PLATFORM_MAX_PATH];
	new iLen = get_configsdir(szFileName, charsmax(szFileName));
	format(szFileName[iLen], charsmax(szFileName) - iLen, "/%s/%s", MM_CONFIG_FOLDER, MM_RECENT_MAPS_FILENAME);

	new JSON:root_value = json_init_object();
	new JSON:array = json_init_array();

	new iMaxLoop = ArraySize(g_GlobalConfigs[RecentMaps]);
	for(new i = 0, aRecents[ArrayRecentMaps_e]; i < iMaxLoop; ++i)
	{
		ArrayGetArray(g_GlobalConfigs[RecentMaps], i, aRecents);
		
		array = json_init_array();
		json_array_append_number(array, aRecents[RECENT_MAP_COUNT]);
		json_array_append_number(array, aRecents[RECENT_MAP_SYSTIME]);

		json_object_set_value(root_value, fmt("recent_maps.%s.%s", aRecents[RECENT_MOD_NAME], aRecents[RECENT_MAP_NAME]), array, true);
		json_free(array);
	}

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
	for(new i = 0, aRecents[ArrayRecentMods_e]; i < iMaxLoop; ++i)
	{
		ArrayGetArray(g_GlobalConfigs[RecentMods], i, aRecents);
		
		if(equali(szModName, aRecents[RECENT_MOD_NAME]))
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