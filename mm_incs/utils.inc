#if defined _mm_utils_included_
	#endinput
#endif
#define _mm_utils_included_

stock UTIL_GetCurrentMod(const szFilePath[PLATFORM_MAX_PATH], szMod[], const iModLen, szMap[], const iMapLen)
{
	if(file_exists(szFilePath))
	{
		new iFile = fopen(szFilePath, "r");

		if(iFile)
		{
			new szLine[PLATFORM_MAX_PATH];

			// Mod name
			if(fgets(iFile, szLine, charsmax(szLine)))
			{
				trim(szLine);
				replace_string(szLine, charsmax(szLine), ";Mod:", "");
				
				copy(szMod, iModLen, szLine);

				// Default Map
				if(fgets(iFile, szLine, charsmax(szLine)))
				{
					trim(szLine);
					replace_string(szLine, charsmax(szLine), ";Map:", "");
					
					copy(szMap, iMapLen, szLine);
				}
			}

			fclose(iFile);
		}
	}
}

stock ChangeMap_e:UTIL_GetChangeMapType(const iMod)
{
	if(g_GlobalConfigs[Mods] == Invalid_Array || ArraySize(g_GlobalConfigs[Mods]) < 1)
		return CHANGEMAP_END_OF_ROUND;

	new aData[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iMod, aData);

	return aData[ChangeMapType];
}

stock UTIL_RegisterClientCommandAll(const command[], function[], flags = -1, const info[] = "", FlagManager = -1, bool:info_ml = false)
{
	new sFormatCommand[PLATFORM_MAX_PATH];

	formatex(sFormatCommand, charsmax(sFormatCommand), "say %s", command);
	register_clcmd(sFormatCommand, function, flags, info, FlagManager, info_ml);
	
	formatex(sFormatCommand, charsmax(sFormatCommand), "say /%s", command);
	register_clcmd(sFormatCommand, function, flags, info, FlagManager, info_ml);

	formatex(sFormatCommand, charsmax(sFormatCommand), "say !%s", command);
	register_clcmd(sFormatCommand, function, flags, info, FlagManager, info_ml);
	
	formatex(sFormatCommand, charsmax(sFormatCommand), "say .%s", command);
	register_clcmd(sFormatCommand, function, flags, info, FlagManager, info_ml);

	formatex(sFormatCommand, charsmax(sFormatCommand), "say_team %s", command);
	register_clcmd(sFormatCommand, function, flags, info, FlagManager, info_ml);
	
	formatex(sFormatCommand, charsmax(sFormatCommand), "say_team /%s", command);
	register_clcmd(sFormatCommand, function, flags, info, FlagManager, info_ml);

	formatex(sFormatCommand, charsmax(sFormatCommand), "say_team !%s", command);
	register_clcmd(sFormatCommand, function, flags, info, FlagManager, info_ml);
	
	formatex(sFormatCommand, charsmax(sFormatCommand), "say_team .%s", command);
	register_clcmd(sFormatCommand, function, flags, info, FlagManager, info_ml);
}

public UTIL_GetPercent(const iVotes, const iTotal)
{
	return (iTotal < 1) ? 0 : ((iVotes * 100) / iTotal);
}

public UTIL_DisableItem(const id, const menuid, const item)
{
	return ITEM_DISABLED;
}

stock UTIL_MenuDisplayAll(menu, page=0, time=-1)
{
	for(new i = 1; i < MAX_USERS; ++i)
	{
		if(GetPlayerBit(g_bConnected, i))
			menu_display(i, menu, page, time);
	}

	return 1;
}

stock UTIL_GetOffModsCount()
{
	new i = 1;
	new j = 0;
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);
	new aMods[ArrayMods_e];

	while(i < iArraySizeMods)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], i, aMods);

		if(unlikely(aMods[Enabled] == true))
			++j;

		++i;
	}

	return j;
}

stock bool:UTIL_IsModOff(const iMod)
{
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);
	if(likely(iMod < 0) || likely(iMod >= iArraySizeMods))
		return true;

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iMod, aMods);

	return bool:unlikely(aMods[Enabled] == true);
}

stock UTIL_GetModId(const szModName[])
{
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);
	for(new i = 0, aMods[ArrayMods_e]; i < iArraySizeMods; ++i)
	{
		ArrayGetArray(g_GlobalConfigs[Mods], i, aMods);

		if(equali(aMods[ModName], szModName))
			return i;
	}

	return -1;
}