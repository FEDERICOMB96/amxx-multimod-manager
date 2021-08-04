#if defined _mm_modchooser_included_
	#endinput
#endif
#define _mm_modchooser_included_

#include <amxmodx>
#include <multimod_manager/defines>
#include <multimod_manager/cvars>

#define MAX_SELECTMODS			5

new g_VoteModCount = 0;
new g_VoteModCountGlobal[MAX_SELECTMODS + 2];
new g_SVM_ModInMenu[MAX_SELECTMODS + 1];
new g_SVM_ModInMenu__FuckingFIX[MAX_SELECTMODS + 1];
new g_SVM_ModName[MAX_SELECTMODS + 1][64];
new g_SVM_ModSecondRound = 0;
new g_ModVoteNum = 0;
new g_SelectedNextMod = 0;
new g_VoteModHasStarted = 0;

new g_VoteModId[MAX_USERS];

ModChooser_Init()
{
	for(new i = 0; i < (MAX_SELECTMODS + 1); ++i)
	{
		g_SVM_ModInMenu[i] = -1;
		g_SVM_ModName[i][0] = EOS;
		g_SVM_ModInMenu__FuckingFIX[i] = -1;
	}

	register_menu("VoteMod_Menu", KEYSMENU, "menu__CountVoteMod");
	register_menu("VoteModFIX_Menu", KEYSMENU, "menu__CountVoteModFIX");
}

ModChooser_ResetAllData()
{
	for(new i = 0; i < (MAX_SELECTMODS + 2); ++i)
		g_VoteModCountGlobal[i] = 0;

	for(new i = 0; i < (MAX_SELECTMODS + 1); ++i)
	{
		g_SVM_ModInMenu[i] = -1;
		g_SVM_ModInMenu__FuckingFIX[i] = -1;
		g_SVM_ModName[i][0] = EOS;
	}

	g_VoteModCount = 0;
	g_SVM_ModSecondRound = 0;
	g_ModVoteNum = 0;
	g_NoMoreTime = 0;
	g_ShowTime = 10;
	g_VoteModHasStarted = 0;

	for(new i = 0; i < MAX_USERS; ++i)
	{
		RockTheVote_ResetUserData(i);
		g_VoteModId[i] = 0;
	}

	remove_task(TASK_VOTEMAP);
	remove_task(TASK_VOTEMOD);
	remove_task(TASK_TIMELEFT);
	remove_task(TASK_RTV);
	remove_task(TASK_SHOWTIME);
}

ModChooser_ClientPutInServer(const id)
{
	g_VoteModId[id] = 0;
}

ModChooser_ClientDisconnected(const id)
{
	// Si votó y se fue antes de terminar la votacion, su voto es removido!
	if(g_VoteModId[id] && g_VoteModHasStarted)
	{
		--g_VoteModCountGlobal[g_VoteModId[id]-1];
		--g_VoteModCount;

		g_VoteModId[id] = 0;
	}
}

public OnTaskVoteNextMod()
{
	new sMenu[400];
	new iMenuKeys;
	new iRandom;
	new j;
	new iLen;
	new iMaxMods;
	new iStart;
	new iOk;
	new iArraySizeMods = ArraySize(g_aModNames);

	iMaxMods = (iArraySizeMods > MAX_SELECTMODS) ? MAX_SELECTMODS : iArraySizeMods;
	iMenuKeys = (1<<iMaxMods + 1);
	
	iLen = formatex(sMenu, charsmax(sMenu), "\yElige el próximo modo: ¡VOTÁ AHORA!^n^n");
	j = 0;
	
	for(g_ModVoteNum = 0; g_ModVoteNum < iMaxMods; ++g_ModVoteNum)
	{
		if(g_SVM_ModInMenu[g_ModVoteNum] == -1)
		{
			iOk = 1;
			iStart = iRandom = random_num(0, (iArraySizeMods - 1));
			
			while(IsModInMenu(iRandom))
			{
				if(++iRandom >= iArraySizeMods)
					iRandom = 0;

				if(iRandom == iStart) // Fix infinite bucle 
				{
					iOk = 0;
					break;
				}
			}

			if(!iOk)
				continue;

			g_SVM_ModInMenu[g_ModVoteNum] = iRandom;
		}

		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %a^n", ++j, ArrayGetStringHandle(g_aModNames, g_SVM_ModInMenu[g_ModVoteNum]));
		
		iMenuKeys |= (1<<g_ModVoteNum);
	}
	
	for(j = 0; j < (MAX_SELECTMODS + 2); ++j)
		g_VoteModCountGlobal[j] = 0;
	
	g_VoteModCount = 0;
	g_VoteModHasStarted = 1;
	g_SVM_ModSecondRound = 0;

	if((get_pcvar_float(mp_timelimit) < Float:amx_extendmap_max) && !g_IsRtv)
	{
		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r%d.\w Extender el modo %dm^n", iMaxMods + 1, amx_extendmap_step);
		iMenuKeys |= (1<<iMaxMods);

		g_SVM_ModInMenu[g_ModVoteNum] = g_ModVoteNum;
	}

	iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n\r%d.\y Seleccionar al azar", ((iMaxMods + 2) > 9) ? 0 : (iMaxMods + 2));
	
	show_menu(0, iMenuKeys, sMenu, 15, "VoteMod_Menu");
	set_task(15.1, "OnTaskCheckVoteMod");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_default, "%sEs momento de elegir el próximo modo!", g_GlobalPrefix);
	
	client_print(0, print_center, "");
}

public menu__CountVoteMod(const id, const iKey)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new szMod[32];
	new iModId = iKey;
	
	if(iKey == MAX_SELECTMODS)
	{
		client_print_color(id, print_team_blue, "%s^1 Has elegido extender el modo^3 %d minutos más", g_GlobalPrefix, amx_extendmap_step);
		
		if(amx_vote_answers)
			client_print(0, print_console, "%n ha votado por: extender el modo %d minutos más", id, amx_extendmap_step);
	}
	else if(iKey < MAX_SELECTMODS)
	{
		ArrayGetString(g_aModNames, g_SVM_ModInMenu[iModId], szMod, 31);
		
		client_print_color(id, print_team_blue, "%s^1 Has elegido el modo^3 %s", g_GlobalPrefix, szMod);
		
		if(amx_vote_answers)
			client_print(0, print_console, "%n ha votado por el modo: %s", id, szMod);
	}
	else
	{
		iModId = random(g_ModVoteNum-1);

		ArrayGetString(g_aModNames, g_SVM_ModInMenu[iModId], szMod, 31);

		client_print_color(id, print_team_blue, "%sHas elegido el modo^3 %s^4 [ELECCIÓN ALEATORIA]", g_GlobalPrefix, szMod);
		
		if(amx_vote_answers)
			client_print(0, print_console, "%n ha votado por el modo: %s [ELECCIÓN ALEATORIA]", id, szMod);
	}
	
	++g_VoteModCountGlobal[iModId];
	++g_VoteModCount;

	g_VoteModId[id] = iModId+1;
}

public OnTaskCheckVoteMod()
{
	new iWinner = 0;
	new iFirst = 0;
	new iResult = 0;
	new i;
	new sMod[32];
	
	for(i = 0; i <= g_ModVoteNum; ++i)
	{
		if(!iFirst)
		{
			iWinner = i;
			iFirst = 1;
			iResult = g_VoteModCountGlobal[i];
		}

		if(g_VoteModCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_VoteModCountGlobal[i];
		}
	}

	client_print(0, print_console, "Resultados de la votacion:");
	
	for(i = 0; i < g_ModVoteNum; ++i)
	{
		ArrayGetString(g_aModNames, g_SVM_ModInMenu[i], sMod, 31);
		client_print(0, print_console, "Modo: %s - Votos: %d - Porcentaje: %d%%", sMod, g_VoteModCountGlobal[i], (g_VoteModCount > 0) ? ((g_VoteModCountGlobal[i] * 100) / g_VoteModCount) : 0);
	}
	
	client_print(0, print_console, "Extender modo: %dm - Votos: %d - Porcentaje: %d%%", amx_extendmap_step, g_VoteModCountGlobal[MAX_SELECTMODS], (g_VoteModCount > 0) ? ((g_VoteModCountGlobal[MAX_SELECTMODS] * 100) / g_VoteModCount) : 0);
	
	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;

		// Chequeo si hubo 2 o más mapas con los mismos votos
		for(i = 0; i <= g_ModVoteNum; ++i)
		{
			if(g_VoteModCountGlobal[i] == iResult)
			{
				(i == MAX_SELECTMODS)
					? (g_SVM_ModInMenu__FuckingFIX[j] = -1) // Opcion de extender..
					: (g_SVM_ModInMenu__FuckingFIX[j] = g_SVM_ModInMenu[i]);

				j++;
			}
		}

		// Hubo 2 o más..
		if(j > 1)
		{
			g_VoteModHasStarted = 0;
			g_SVM_ModSecondRound = 1;
			g_ShowTime = 5;
			g_ModVoteNum = j;

			client_print_color(0, print_team_blue, "%s^1 Hubo^4 %d mapas^1 con^4 %d^1 voto%s cada uno, siguiente votación en^3 5 segundos^1!", g_GlobalPrefix, j, iResult, (iResult != 1) ? "s" : "");

			client_cmd(0, "spk ^"run officer(e40) voltage(e30) accelerating(s70) is required^"");

			set_task(5.0, "OnTaskSpamStartVote", TASK_SHOWTIME);
			set_task(10.0, "OnTaskVoteNextMod__FIX");
		}

		// Solo 1 ganador
		else
		{
			// Ganador, extender el modo
			if(iWinner == MAX_SELECTMODS)
			{
				new Float:flExtendMod = ExtendActualMod();
				
				client_print_color(0, print_team_blue, "%s^1 El modo actual se extenderá^3 %d minutos más ^1con^4 %d ^1/^4 %d ^1voto%s (%d%%)!", g_GlobalPrefix, floatround(flExtendMod), iResult, g_VoteModCount, (g_VoteModCount != 1) ? "s" : "", (g_VoteModCount > 0) ? ((iResult * 100) / g_VoteModCount) : 0); // , (g_VoteModCount > 0) ? ((g_VoteModCountGlobal[MAX_SELECTMODS] * 100) / g_VoteModCount) : 0
				
				ModChooser_ResetAllData();

				flExtendMod -= 3;
				flExtendMod *= 60;
				
				set_task(flExtendMod, "OnTaskVoteNextMod", TASK_VOTEMOD);
				set_task(flExtendMod - 10.0, "OnTaskSpamStartVote", TASK_SHOWTIME);
				
				g_ShowTime = 10;
				return;
			}

			MultiMod_SetNextMod(g_SVM_ModInMenu[iWinner]);

			ArrayGetString(g_aModNames, g_SVM_ModInMenu[iWinner], sMod, 31);
			client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el modo ganador es ^3%s ^1con^4 %d ^1/^4 %d ^1voto%s (%d%%)!", g_GlobalPrefix, sMod, iResult, g_VoteModCount, (g_VoteModCount != 1) ? "s" : "", (g_VoteModCount > 0) ? ((iResult * 100) / g_VoteModCount) : 0); // , (g_VoteModCount > 0) ? ((g_VoteModCountGlobal[j] * 100) / g_VoteModCount) : 0
			
			ModChooser_ResetAllData();

			g_SelectedNextMod = 1;

			MapChooser_InitNextVoteMap();
		}
	}
	else
	{
		if(g_IsRtv)
		{
			g_IsRtv = 0;
			g_VoteRtvResult = 0;
			g_VoteCountRtv = 0;
			g_IsVotingRtv = 0;
			g_VoteModHasStarted = 0;
			
			ModChooser_ResetAllData();
			
			client_print_color(0, print_team_red, "%s^3La votación solicitada por los usuarios no tuvo exito!", g_GlobalPrefix);
			return;
		}

		iWinner = random(g_ModVoteNum+1);
		MultiMod_SetNextMod(g_SVM_ModInMenu[iWinner]);

		ArrayGetString(g_aModNames, g_SVM_ModInMenu[iWinner], sMod, 31);
		client_print_color(0, print_team_blue, "%s^1 Votación finalizada, el modo ganador es ^3%s ^1con^4 %d ^1/^4 %d ^1voto%s (%d%%)!", g_GlobalPrefix, sMod, iResult, g_VoteModCount, (g_VoteModCount != 1) ? "s" : "", (g_VoteModCount > 0) ? ((iResult * 100) / g_VoteModCount) : 0); // , (g_VoteModCount > 0) ? ((g_VoteModCountGlobal[j] * 100) / g_VoteModCount) : 0
		
		ModChooser_ResetAllData();

		g_SelectedNextMod = 1;

		MapChooser_InitNextVoteMap();
	}
}

public OnTaskVoteNextMod__FIX()
{
	new sMenu[400];
	new iMenuKeys;
	new iLen;
	new i;

	iLen = formatex(sMenu, charsmax(sMenu), "\yElige el próximo modo: ¡VOTÁ AHORA!^n^n");

	for(i = 0; i < g_ModVoteNum; ++i)
	{
		iMenuKeys |= (1<<i);

		if(g_SVM_ModInMenu__FuckingFIX[i] == -1)
		{
			iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w Extender el modo %dm^n", i+1, amx_extendmap_step);
			continue;
		}

		iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "\r%d.\w %a^n", i+1, ArrayGetStringHandle(g_aModNames, g_SVM_ModInMenu__FuckingFIX[i]));
	}

	iLen += formatex(sMenu[iLen], charsmax(sMenu) - iLen, "^n^n\d(SEGUNDA VOTACIÓN)");

	for(i = 0; i < (MAX_SELECTMODS + 2); ++i)
		g_VoteModCountGlobal[i] = 0;

	g_VoteModCount = 0;
	g_VoteModHasStarted = 1;
	g_SVM_ModSecondRound = 1;

	show_menu(0, iMenuKeys, sMenu, 15, "VoteModFIX_Menu");
	set_task(15.1, "OnTaskCheckVoteModFIX");
	
	client_cmd(0, "spk ^"%s^"", g_SOUND_GmanChoose[random_num(0, charsmax(g_SOUND_GmanChoose))]);
	client_print_color(0, print_team_default, "%sEs momento de elegir el próximo modo (SEGUNDA VOTACIÓN)!", g_GlobalPrefix);
	
	client_print(0, print_center, "");
}

public menu__CountVoteModFIX(const id, const iKey)
{
	if(!GetPlayerBit(g_bConnected, id))
		return;

	new sMod[32];
	new iModId = iKey;
	
	if(g_SVM_ModInMenu__FuckingFIX[iModId] == -1)
	{
		client_print_color(id, print_team_blue, "%s^1 Has elegido extender el modo^3 %d minutos más", g_GlobalPrefix, amx_extendmap_step);
		
		if(amx_vote_answers)
			client_print(0, print_console, "%n ha votado por: extender el modo %d minutos más", id, amx_extendmap_step);
	}
	else
	{
		ArrayGetString(g_aModNames, g_SVM_ModInMenu__FuckingFIX[iModId], sMod, 31);
		
		client_print_color(id, print_team_blue, "%s^1 Has elegido el modo^3 %s", g_GlobalPrefix, sMod);
		
		if(amx_vote_answers)
			client_print(0, print_console, "%n ha votado por el modo: %s", id, sMod);
	}
	
	++g_VoteModCountGlobal[iModId];
	++g_VoteModCount;

	g_VoteModId[id] = iModId+1;
}

public OnTaskCheckVoteModFIX()
{
	new iWinner = 0;
	new iFirst = 0;
	new iResult = 0;
	new i;
	new sMod[32];
	
	for(i = 0; i < g_ModVoteNum; ++i)
	{
		if(!iFirst)
		{
			iWinner = i;
			iFirst = 1;
			iResult = g_VoteModCountGlobal[i];
		}

		if(g_VoteModCountGlobal[i] > iResult)
		{
			iWinner = i;
			iResult = g_VoteModCountGlobal[i];
		}
	}

	client_print(0, print_console, "Resultados de la votacion:");
	
	for(i = 0; i < g_ModVoteNum; ++i)
	{
		if(g_SVM_ModInMenu__FuckingFIX[i] == -1)
		{
			client_print(0, print_console, "Extender mapa: %d' - Votos: %d - Porcentaje: %d%%", amx_extendmap_step, g_VoteModCountGlobal[i], (g_VoteModCount > 0) ? ((g_VoteModCountGlobal[i] * 100) / g_VoteModCount) : 0);
			continue;
		}

		ArrayGetString(g_aModNames, g_SVM_ModInMenu__FuckingFIX[i], sMod, 31);
		
		client_print(0, print_console, "Modo: %s - Votos: %d - Porcentaje: %d%%^n", sMod, g_VoteModCountGlobal[i], (g_VoteModCount > 0) ? ((g_VoteModCountGlobal[i] * 100) / g_VoteModCount) : 0);
	}

	// Hubo votos emitidos..
	if(iResult)
	{
		new j = 0;
		new iMapsId[MAX_SELECTMODS + 1];

		iWinner = g_SVM_ModInMenu__FuckingFIX[iWinner];

		// Chequeo si hubo 2 o más mapas con los mismos votos
		for(i = 0; i <= g_ModVoteNum; ++i)
		{
			if(g_VoteModCountGlobal[i] == iResult)
			{
				iMapsId[j] = g_SVM_ModInMenu__FuckingFIX[i];

				j++;
			}
		}

		// Hubo más de 1 ganador (otra vez)
		if(j > 1)
			iWinner = iMapsId[random_num(0, (j - 1))];

		// Ganador, extender el mapa
		if(iWinner == -1)
		{
			new Float:flExtendMod = ExtendActualMod();

			if(j > 1)
			{
				client_print_color(0, print_team_default, "%s^1 Ningún mapa superó^4 la mayoría de los votos^1 por segunda vez.", g_GlobalPrefix);
				client_print_color(0, print_team_blue, "%s^1 Entre los más votados, el mapa ganador al azar es:^3 Extender el mapa %dm.", g_GlobalPrefix, floatround(flExtendMod));
				client_print_color(0, print_team_default, "%s^1 El mapa actual se extenderá^3 %d minutos más", g_GlobalPrefix, floatround(flExtendMod));
			}
			else
				client_print_color(0, print_team_blue, "%s^1 El mapa actual se extenderá^3 %d minutos más ^1con^4 %d ^1/^4 %d ^1voto%s (%d%%)!", g_GlobalPrefix, floatround(flExtendMod), iResult, g_VoteModCount, (g_VoteModCount != 1) ? "s" : "", (g_VoteModCount > 0) ? ((iResult * 100) / g_VoteModCount) : 0);

			ModChooser_ResetAllData();

			flExtendMod -= 3;
			flExtendMod *= 60;
			
			set_task(flExtendMod, "OnTaskVoteNextMod", TASK_VOTEMOD);
			set_task(flExtendMod - 10.0, "OnTaskSpamStartVote", TASK_SHOWTIME);
			
			g_ShowTime = 10;
			return;
		}

		ArrayGetString(g_aModNames, iWinner, sMod, 31);
		
		if(j > 1)
		{
			client_print_color(0, print_team_default, "%sNingún mapa superó^4 la mayoría de los votos^1 por segunda vez.", g_GlobalPrefix);
			client_print_color(0, print_team_blue, "%sEntre los más votados, el mapa ganador al azar es:^3 %s", g_GlobalPrefix, sMod);
		}
		else
			client_print_color(0, print_team_blue, "%sVotación finalizada, el mapa ganador es ^3%s ^1con^4 %d ^1/^4 %d ^1voto%s (%d%%)!", g_GlobalPrefix, sMod, iResult, g_VoteModCount, (g_VoteModCount != 1) ? "s" : "", (g_VoteModCount > 0) ? ((iResult * 100) / g_VoteModCount) : 0);
		
		ModChooser_ResetAllData();

		g_SelectedNextMod = 1;

		MapChooser_InitNextVoteMap();
	}
	else
	{
		if(g_IsRtv)
		{
			g_IsRtv = 0;
			g_VoteRtvResult = 0;
			g_VoteCountRtv = 0;
			g_IsVotingRtv = 0;
			g_VoteModHasStarted = 0;
			
			ModChooser_ResetAllData();
			
			client_print_color(0, print_team_red, "%s^3 La votación solicitada por los usuarios no tuvo exito!", g_GlobalPrefix);
			return;
		}

		iWinner = random(g_ModVoteNum+1);
		MultiMod_SetNextMod(g_SVM_ModInMenu__FuckingFIX[iWinner]);

		ArrayGetString(g_aModNames, g_SVM_ModInMenu__FuckingFIX[iWinner], sMod, 31);
		client_print_color(0, print_team_blue, "%s^1 Ningún voto fue emitido, por lo tanto el mapa ganador al azar es:^3 %s", g_GlobalPrefix, sMod);

		ModChooser_ResetAllData();

		g_SelectedNextMod = 1;

		MapChooser_InitNextVoteMap();
	}
}

Float:ExtendActualMod()
{
	new Float:fExtendMod = Float:amx_extendmap_step;
	set_pcvar_float(mp_timelimit, (get_pcvar_float(mp_timelimit) + fExtendMod));

	return fExtendMod;
}

IsModInMenu(const i)
{
	for(new j = 0; j < (MAX_SELECTMODS + 1); ++j)
	{
		if(i == g_SVM_ModInMenu[j])
			return 1;
	}

	return 0;
}