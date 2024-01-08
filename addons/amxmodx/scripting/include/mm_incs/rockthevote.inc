#if defined _mm_rtv_included_
	#endinput
#endif
#define _mm_rtv_included_

RockTheVote_Init()
{
	UTIL_RegisterClientCommandAll("rtv", "ClientCommand_RockTheVote");
	UTIL_RegisterClientCommandAll("rockthevote", "ClientCommand_RockTheVote");

	g_iSystimeRtv = get_systime();

	RockTheVote_ResetAllData();
}

RockTheVote_ResetAllData()
{
	g_bVoteInProgress = false;
	
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

	if(!g_GlobalConfigs[RTV_Enabled])
	{
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_RTV_NOT_ENABLED");
		return PLUGIN_HANDLED;
	}

	new iCurrentTime = get_systime();
	if(g_GlobalConfigs[RTV_Cooldown] && (0 < ((g_iSystimeRtv + g_GlobalConfigs[RTV_Cooldown]) - iCurrentTime) < g_GlobalConfigs[RTV_Cooldown]))
	{
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_RTV_COOLDOWN", ((g_iSystimeRtv + g_GlobalConfigs[RTV_Cooldown]) - iCurrentTime));
		return PLUGIN_HANDLED;
	}

	if(g_bVoteRtvResult)
	{
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_RTV_ALREADY_MOD_CHOOSED");
		return PLUGIN_HANDLED;
	}
	
	if(g_bIsVotingRtv || g_bVoteModHasStarted || g_bSVM_ModSecondRound || g_bVoteMapHasStarted || g_bSVM_MapSecondRound || g_bVoteInProgress)
	{
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_ALREADY_VOTING");
		return PLUGIN_HANDLED;
	}

	new iPlayersNum = get_playersnum_ex(GetPlayers_ExcludeBots);

	if(g_GlobalConfigs[RTV_MinPlayers] && (iPlayersNum < g_GlobalConfigs[RTV_MinPlayers]))
	{
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_RTV_MIN_PLAYERS", g_GlobalConfigs[RTV_MinPlayers]);
		return PLUGIN_HANDLED;
	}
	
	new iPercent = (iPlayersNum * g_GlobalConfigs[RTV_Percentage]) / 100;
	
	if(g_HasVoteRTV[id])
	{
		new iResult = (iPercent - g_iVoteCountRtv);
		
		client_print_color(id, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_RTV_ALREADY_VOTED");
		
		client_print_color(0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_RTV_NEED_MIN_VOTES", iResult);
		return PLUGIN_HANDLED;
	}

	g_HasVoteRTV[id] = 1;

	++g_iVoteCountRtv;
	
	client_print_color(0, print_team_blue, "%s %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_RTV_PLAYER_VOTED", id, g_iVoteCountRtv, iPercent);
	
	if(g_iVoteCountRtv >= iPercent)
	{
		g_bIsVotingRtv = true;
		
		if(g_bCvar_amx_multimod_voice)
			client_cmd(0, "spk ^"get red(e80) ninety(s45) to check(e20) use bay(s18) mass(e42) cap(s50)^"");
		
		client_print_color(0, print_team_default, "%s^1 %L", g_GlobalConfigs[ChatPrefix], LANG_PLAYER, "MM_RTV_VOTING_START");

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