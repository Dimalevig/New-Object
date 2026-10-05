class CfgPatches
{
	class CustomPalisade
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts"
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
		version = "0.1.0";
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
