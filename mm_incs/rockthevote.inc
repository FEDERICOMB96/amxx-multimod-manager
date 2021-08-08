#if defined _mm_rtv_included_
	#endinput
#endif
#define _mm_rtv_included_

RockTheVote_Init()
{
	RockTheVote_ResetAllData();

	UTIL_RegisterClientCommandAll("rtv", "ClientCommand_RockTheVote");
	UTIL_RegisterClientCommandAll("rockthevote", "ClientCommand_RockTheVote");

	g_iSystimeRtv = get_systime();
}

RockTheVote_ResetAllData()
{
	g_iVoteCountRtv = 0;

	g_bIsRockTheVote = false;
	g_bVoteRtvResult = false;
	g_bIsVotingRtv = false;

	arrayset(g_HasVoteRTV, 0, MAX_USERS);

	remove_task(TASK_RTV);
}

RockTheVote_ClientPutInServer(const id)
{
	g_HasVoteRTV[id] = 0;
}

RockTheVote_ClientDisconnected(const id)
{
	if(g_HasVoteRTV[id])
		g_iVoteCountRtv = max(0, (g_iVoteCountRtv - 1));

	g_HasVoteRTV[id] = 0;
}

public ClientCommand_RockTheVote(const id)
{
	if(!GetPlayerBit(g_bConnected, id))
		return PLUGIN_HANDLED;

	if(!g_GlobalConfigs[GlobalConfig_RTV_Enabled])
	{
		client_print_color(id, print_team_default, "%s^1 La opción^3 Rock The Vote^1 se encuentra deshabilitada!", g_GlobalConfigs[GlobalConfig_ChatPrefix]);
		return PLUGIN_HANDLED;
	}

	new iCurrentTime = get_systime();
	if(g_GlobalConfigs[GlobalConfig_RTV_Cooldown] && (0 < ((g_iSystimeRtv + g_GlobalConfigs[GlobalConfig_RTV_Cooldown]) - iCurrentTime) < g_GlobalConfigs[GlobalConfig_RTV_Cooldown]))
	{
		client_print_color(id, print_team_default, "%s^1 Tenés que esperar^4 %d segundos^1 para utilizar^3 Rock The Vote!", g_GlobalConfigs[GlobalConfig_ChatPrefix], ((g_iSystimeRtv + g_GlobalConfigs[GlobalConfig_RTV_Cooldown]) - iCurrentTime));
		return PLUGIN_HANDLED;
	}

	if(g_bVoteRtvResult)
	{
		client_print_color(id, print_team_default, "%s^1 Ya se realizó una votación para el cambio de modo!", g_GlobalConfigs[GlobalConfig_ChatPrefix]);
		return PLUGIN_HANDLED;
	}
	
	if(g_bIsVotingRtv || g_bVoteModHasStarted || g_bSVM_ModSecondRound || g_bVoteMapHasStarted || g_bSVM_MapSecondRound)
	{
		client_print_color(id, print_team_default, "%s^1 Ya existe una votación en curso!", g_GlobalConfigs[GlobalConfig_ChatPrefix]);
		return PLUGIN_HANDLED;
	}

	new iPlayersNum = get_playersnum();

	if(g_GlobalConfigs[GlobalConfig_RTV_MinPlayers] && (iPlayersNum < g_GlobalConfigs[GlobalConfig_RTV_MinPlayers]))
	{
		client_print_color(id, print_team_default, "%s^1 Se necesita al menos^4 %d jugador%s^1 en la partida para utilizar^3 Rock The Vote!", 
			g_GlobalConfigs[GlobalConfig_ChatPrefix], g_GlobalConfigs[GlobalConfig_RTV_MinPlayers], (g_GlobalConfigs[GlobalConfig_RTV_MinPlayers] != 1) ? "es" : "");
		return PLUGIN_HANDLED;
	}
	
	new iPercent = (iPlayersNum * g_GlobalConfigs[GlobalConfig_RTV_Percentage]) / 100;
	
	if(g_HasVoteRTV[id])
	{
		new iResult = (iPercent - g_iVoteCountRtv);
		
		client_print_color(id, print_team_default, "%s^1 Ya diste tu voto para iniciar una votación para el cambio de modo!", g_GlobalConfigs[GlobalConfig_ChatPrefix]);
		
		client_print_color(0, print_team_default, "%s^1 Se necesita%s^4 %d voto%s más^1 para iniciar una votación para el cambio de modo!", g_GlobalConfigs[GlobalConfig_ChatPrefix], (iResult != 1) ? "n" : "", iResult, (iResult != 1) ? "s" : "");
		return PLUGIN_HANDLED;
	}

	g_HasVoteRTV[id] = 1;

	++g_iVoteCountRtv;
	
	client_print_color(0, print_team_blue, "%s^3 %n^1 votó para iniciar una votación para cambiar el mapa! Votos emitidos^4 %d^1 /^4 %d", g_GlobalConfigs[GlobalConfig_ChatPrefix], id, g_iVoteCountRtv, iPercent);
	
	if(g_iVoteCountRtv >= iPercent)
	{
		g_bIsVotingRtv = true;
		
		client_cmd(0, "spk ^"get red(e80) ninety(s45) to check(e20) use bay(s18) mass(e42) cap(s50)^"");
		
		client_print_color(0, print_team_default, "%s^1 RockTheVote finalizado, en^4 10 segundos ^1comenzará una nueva votación para el cambio de mapa!", g_GlobalConfigs[GlobalConfig_ChatPrefix]);

		SetAlertStartNextVote(5.0, 5);
		
		remove_task(TASK_RTV);
		set_task(10.0, "OnTask_StartRockTheVote", TASK_RTV);
	}

	return PLUGIN_HANDLED;
}

public OnTask_StartRockTheVote()
{
	g_bIsRockTheVote = true;
	g_bVoteRtvResult = false;
	
	remove_task(TASK_VOTEMAP);
	remove_task(TASK_VOTEMOD);
	remove_task(TASK_SHOWTIME);
	
	OnTask_VoteNextMod();
}