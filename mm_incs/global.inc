#if defined _mm_global_included_
	#endinput
#endif
#define _mm_global_included_

/* ===========================================================================
*                 [ CORE ]
* ============================================================================ */

new g_bConnected;

new g_szCurrentMap[64];
new g_szCurrentMod[64];

new g_iCurrentMod = 0;
new g_iNoMoreTime = 0;
new g_iCountdownTime = 0;

new Float:g_RestoreTimelimit = 0.0;

new g_Hud_Vote = 0;
new g_Hud_Alert = 0;

new Array:g_Array_MapName;

new g_GlobalConfigs[GlobalConfigs_e];


/* ===========================================================================
*                 [ CVARS ]
* ============================================================================ */

// GameDll Cvars
new g_pCvar_mp_timelimit;
new g_bCvar_mp_winlimit;
new g_bCvar_mp_maxrounds;

// Plugin Cvars
new g_bCvar_amx_nextmap[64], g_pCvar_amx_nextmap;
new g_bCvar_amx_extendmap_max;
new g_bCvar_amx_extendmap_step;
new g_bCvar_amx_vote_answers;


/* ===========================================================================
*                 [ MOD CHOOSER ]
* ============================================================================ */

new g_iModVoteNum;
new g_iVoteModCount;
new g_iVoteModCountGlobal[MAX_SELECTMODS + 2];
new g_iSVM_ModInMenu[MAX_SELECTMODS + 1];
new g_iSVM_ModInMenu_SecondRound[MAX_SELECTMODS + 1];

new bool:g_bSVM_ModSecondRound;
new bool:g_bSelectedNextMod;
new bool:g_bVoteModHasStarted;

new g_VoteModId[MAX_USERS];


/* ===========================================================================
*                 [ MAP CHOOSER ]
* ============================================================================ */

new g_iMapVoteNum;
new g_iChangeMapTime;
new g_iVoteMapCount;
new g_iVoteMapCountGlobal[MAX_SELECTMAPS + 2];
new g_iSVM_MapInMenu[MAX_SELECTMAPS + 1];
new g_iSVM_MapInMenu_SecondRound[MAX_SELECTMAPS + 1];

new bool:g_bSVM_MapSecondRound;
new bool:g_bSelectedNextMap;
new bool:g_bVoteMapHasStarted;
new bool:g_bChangeMapOneMoreRound;

new g_VoteMapId[MAX_USERS];


/* ===========================================================================
*                 [ ROCK THE VOTE ]
* ============================================================================ */
new g_iVoteCountRtv;

new bool:g_bIsRockTheVote;
new bool:g_bVoteRtvResult;
new bool:g_bIsVotingRtv;

new g_iSystimeRtv;

new g_HasVoteRTV[MAX_USERS];