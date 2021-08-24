#if defined _mm_global_included_
	#endinput
#endif
#define _mm_global_included_

/* ===========================================================================
*                 [ CORE ]
* ============================================================================ */

new g_bConnected;

new g_szCurrentMap[MAX_MAPNAME_LENGTH];
new g_szCurrentMod[MAX_MODNAME_LENGTH];

new g_iCurrentMod;
new g_iNextSelectMod;
new g_iNoMoreTime;
new g_iCountdownTime;

new Float:g_RestoreTimelimit;

new bool:g_bVoteInProgress;

new g_Hud_Vote;
new g_Hud_Alert;

new Array:g_Array_Nominations;

new g_GlobalConfigs[GlobalConfigs_e];


/* ===========================================================================
*                 [ CVARS ]
* ============================================================================ */

// GameDll Cvars
new g_pCvar_mp_timelimit;
new g_bCvar_mp_winlimit;
new g_bCvar_mp_maxrounds;

// Plugin Cvars
new g_bCvar_amx_nextmap[MAX_MAPNAME_LENGTH], g_pCvar_amx_nextmap;
new g_bCvar_amx_extendmap_max;
new g_bCvar_amx_extendmap_step;
new g_bCvar_amx_vote_answers;
new g_bCvar_amx_last_voting, g_pCvar_amx_last_voting;
new g_bCvar_amx_vote_delay_mm;


/* ===========================================================================
*                 [ ADMIN COMMANDS ]
* ============================================================================ */

new g_Menu_AdminCmd;

new g_Admin_VoteUserId;
new g_Admin_VoteCountTotal;

new g_Admin_iVoteCountId[MAX_ADMIN_VOTEOPTIONS];

new g_Admin_VoteOptions[MAX_USERS][MAX_ADMIN_VOTEOPTIONS];
new g_Admin_VoteSelectedOptions[MAX_USERS];
new g_Admin_VoteCurrentPage[MAX_USERS];
new g_Admin_SelectedMod[MAX_USERS];
new g_Admin_SelectedMap[MAX_USERS];

new g_Admin_VoteId[MAX_USERS];

new bool:g_Admin_VoteStartInMod[MAX_USERS];


/* ===========================================================================
*                 [ MOD CHOOSER ]
* ============================================================================ */

new g_Menu_ModChooser;

new g_iModVoteNum;
new g_iVoteModCount;
new g_iVoteModCountGlobal[MAX_SELECTMODS];
new g_iSVM_ModInMenu[MAX_SELECTMODS];
new g_iSVM_ModInMenu_SecondRound[MAX_SELECTMODS];

new bool:g_bSVM_ModSecondRound;
new bool:g_bSelectedNextMod;
new bool:g_bVoteModHasStarted;

new g_VoteModId[MAX_USERS];


/* ===========================================================================
*                 [ MAP CHOOSER ]
* ============================================================================ */

new g_Menu_MapChooser;

new g_iMapVoteNum;
new g_iChangeMapTime;
new g_iVoteMapCount;
new g_iVoteMapCountGlobal[MAX_SELECTMAPS];
new g_iSVM_MapInMenu[MAX_SELECTMAPS];
new g_iSVM_MapInMenu_SecondRound[MAX_SELECTMAPS];

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


/* ===========================================================================
*                 [ NOMINATIONS ]
* ============================================================================ */

new g_SVM_MyNominate[MAX_USERS];