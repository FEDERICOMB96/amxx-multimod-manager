#include <amxmodx>
#include <multimod_manager_natives>

public plugin_init()
{
	register_srvcmd("mm_test", "SrvCmd_Test");
}

public SrvCmd_Test()
{
	server_print("SrvCmd_Test called");
	server_print("--------------------------------------------------");

	new iModsCount = mm_get_mods_count();
	server_print("Mods count: %d", iModsCount);
	server_print("--------------------------------------------------");

	new bool:bModEnabled;
	new sModName[32];
	new sModTag[32];
	new ChangeMap_e:iModChangeMapType;
	new Array:aModMaps;
	new Array:aModCvars;
	new Array:aModPlugins;

	for(new i = 0, j, iMaxLoop; i < iModsCount; ++i)
	{
		bModEnabled = mm_is_mod_enabled(i);
		mm_get_mod_name(i, sModName, sizeof(sModName));
		mm_get_mod_tag(i, sModTag, sizeof(sModTag));
		iModChangeMapType = mm_get_mod_changemap_type(i);
		aModMaps = mm_get_mod_maps(i);
		aModCvars = mm_get_mod_cvars(i);
		aModPlugins = mm_get_mod_plugins(i);
		
		server_print("Mod #%d: %s (Tag: %s)", i, sModName, sModTag);
		server_print("Mod enabled: %s", bModEnabled ? "Yes" : "No");
		server_print("Change Map Type: %d", iModChangeMapType);

		server_print("> Maps count: %d", ArraySize(aModMaps));
		for(j = 0, iMaxLoop = (ArraySize(aModMaps) - 1); j <= iMaxLoop; ++j)
		{
			server_print(" * Map #%d: %a", j + 1, ArrayGetStringHandle(aModMaps, j));
		}
		
		server_print("> Cvars count: %d", ArraySize(aModCvars));
		for(j = 0, iMaxLoop = (ArraySize(aModCvars) - 1); j <= iMaxLoop; ++j)
		{
			server_print(" * Cvar #%d: %a", j + 1, ArrayGetStringHandle(aModCvars, j));
		}

		server_print("> Plugins count: %d", ArraySize(aModPlugins));
		for(j = 0, iMaxLoop = (ArraySize(aModPlugins) - 1); j <= iMaxLoop; ++j)
		{
			server_print(" * Plugin #%d: %a", j + 1, ArrayGetStringHandle(aModPlugins, j));
		}
		
		server_print("--------------------------------------------------");

		ArrayDestroy(aModMaps);
		ArrayDestroy(aModCvars);
		ArrayDestroy(aModPlugins);
	}

	new iCurrentMod = mm_get_currentmod_id();
	mm_get_mod_name(iCurrentMod, sModName, sizeof(sModName));

	server_print("Current mod: %s", sModName);
	server_print("--------------------------------------------------");

	new iNextMod = mm_get_nextmod_id();

	if(iNextMod != -1)
	{
		mm_get_mod_name(iNextMod, sModName, sizeof(sModName));
		server_print("Next mod: %s", sModName);
		server_print("--------------------------------------------------");
	}
	else
	{
		server_print("Next mod: Isn't chosen yet");
		server_print("--------------------------------------------------");
	}
	
	server_print("SrvCmd_Test finished");
}

public multimod_start_votemod(const bool:bSecondVote)
{
	server_print("multimod_start_votemod forward called");
	server_print("--------------------------------------------------");

	(bSecondVote) ? server_print("Second vote started") : server_print("First vote started");
	
	server_print("--------------------------------------------------");
}

public multimod_end_votemod(const bool:bSecondVote)
{
	server_print("multimod_end_votemod forward called");
	server_print("--------------------------------------------------");

	(bSecondVote) ? server_print("Second vote ended") : server_print("First vote ended");
	
	server_print("--------------------------------------------------");
}

public multimod_start_votemap(const bool:bSecondVote)
{
	server_print("multimod_start_votemap forward called");
	server_print("--------------------------------------------------");

	(bSecondVote) ? server_print("Second vote started") : server_print("First vote started");
	
	server_print("--------------------------------------------------");
}

public multimod_end_votemap(const bool:bSecondVote)
{
	server_print("multimod_end_votemap forward called");
	server_print("--------------------------------------------------");

	(bSecondVote) ? server_print("Second vote ended") : server_print("First vote ended");
	
	server_print("--------------------------------------------------");
}

public multimod_admin_force_votemod(const iAdminId)
{
	server_print("multimod_admin_force_votemod forward called");
	server_print("--------------------------------------------------");

	server_print("Admin #%d (Name: %n) forced vote", iAdminId, iAdminId);
	
	server_print("--------------------------------------------------");

	return PLUGIN_CONTINUE;
}