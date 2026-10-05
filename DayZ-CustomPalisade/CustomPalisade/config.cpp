class CfgPatches
{
	class CustomPalisade
	{
		units[] = {"CP_PalisadeWall"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts",
			"DZ_Gear_Camping"
		};
	};
};

class CfgMods
{
	class CustomPalisade
	{
		dir = "CustomPalisade";
		picture = "";
		action = "";
		hideName = 0;
		hidePicture = 1;
		name = "Custom Palisade";
		credits = "";
		author = "";
		authorID = "0";
		version = "0.1.1";
		extra = 0;
		type = "mod";

		dependencies[] =
		{
			"Game",
			"World",
			"Mission"
		};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] =
				{
					"CustomPalisade/Scripts/3_Game"
				};
			};

			class worldScriptModule
			{
				value = "";
				files[] =
				{
					"CustomPalisade/Scripts/4_World"
				};
			};

			class missionScriptModule
			{
				value = "";
				files[] =
				{
					"CustomPalisade/Scripts/5_Mission"
				};
			};
		};
	};
};

class CfgVehicles
{
	class Fence;

	// Temporary: reuses the vanilla Fence model and Construction config
	// until the mod has its own log-palisade model.
	class CP_PalisadeWall: Fence
	{
		scope = 2;
		displayName = "$STR_CP_PALISADEWALL_NAME";
		descriptionShort = "$STR_CP_PALISADEWALL_DESC";
	};
};
