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
	register_clcmd(fmt("say %s", command), function, flags, info, FlagManager, info_ml);
	register_clcmd(fmt("say /%s", command), function, flags, info, FlagManager, info_ml);
	register_clcmd(fmt("say !%s", command), function, flags, info, FlagManager, info_ml);
	register_clcmd(fmt("say .%s", command), function, flags, info, FlagManager, info_ml);

	register_clcmd(fmt("say_team %s", command), function, flags, info, FlagManager, info_ml);
	register_clcmd(fmt("say_team /%s", command), function, flags, info, FlagManager, info_ml);
	register_clcmd(fmt("say_team !%s", command), function, flags, info, FlagManager, info_ml);
	register_clcmd(fmt("say_team .%s", command), function, flags, info, FlagManager, info_ml);
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

		if(!aMods[Enabled])
			++j;

		++i;
	}

	return j;
}

stock bool:UTIL_IsModOff(const iMod)
{
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);
	if((iMod < 0) || (iMod >= iArraySizeMods))
		return true;

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iMod, aMods);

	return bool:aMods[Enabled];
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

stock UTIL_GetTimeElapsed(const iSeconds, buffer[], const len)
{
	new const iSecondsMult[3] = {86400, 3600, 60};
	new const sWord[3][] = {"d", "h", "m"};
	
	new iData[3] = {0, 0, 0};
	new iRest = iSeconds;
	
	for(new i = 0; i < 3; ++i) {
		iData[i] = iRest / iSecondsMult[i];
		iRest = iRest % iSecondsMult[i];
		
		if(iData[i] > 0)
			return formatex(buffer, len, "%d%s", iData[i], sWord[i]);
	}
	
	return formatex(buffer, len, "0%s", sWord[2]);
}

stock UTIL_SimpleSortVotes(iPos[], iNum[], const iSize)
{
	for(new i = 0, j, iTemp; i < (iSize - 1); ++i)
	{
		for(j = (i + 1); j < iSize; ++j)
		{
			if(iNum[j] > iNum[i])
			{
				iTemp = iNum[j];
				iNum[j] = iNum[i];
				iNum[i] = iTemp;

				iTemp = iPos[j];
				iPos[j] = iPos[i];
				iPos[i] = iTemp;
			}
		}
	}
}