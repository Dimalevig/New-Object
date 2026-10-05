class CfgPatches
{
	class CustomPalisade
	{
		units[] = {"CP_PalisadeKit","CP_PalisadeWall","CP_PalisadeWallModel"};
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
		version = "0.2.1";
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
	class FenceKit;
	class Fence;
	class HouseNoDestruct;

	// Palisade marking kit. Temporary: reuses the vanilla FenceKit model.
	class CP_PalisadeKit: FenceKit
	{
		scope = 2;
		displayName = "$STR_CP_PALISADEKIT_NAME";
		descriptionShort = "$STR_CP_PALISADEKIT_DESC";
	};

	// Temporary: reuses the vanilla Fence model and Construction config
	// until the mod has its own log-palisade model.
	class CP_PalisadeWall: Fence
	{
		scope = 2;
		displayName = "$STR_CP_PALISADEWALL_NAME";
		descriptionShort = "$STR_CP_PALISADEWALL_DESC";
	};

	// TEST: the mod's own log palisade model with vanilla DayZ textures,
	// as a static object (same base class as the official DayZ-Samples building).
	// Construction stages will move onto this model in stages 4-5.
	class CP_PalisadeWallModel: HouseNoDestruct
	{
		scope = 2;
		displayName = "$STR_CP_PALISADEWALL_NAME";
		descriptionShort = "$STR_CP_PALISADEWALL_DESC";
		model = "\CustomPalisade\Data\Models\cp_palisade_wall.p3d";

		// driven by SetAnimationPhase(); 0 = visible (model.cfg hide animations)
		class AnimationSources
		{
			class base
			{
				source = "user";
				animPeriod = 0.01;
				initPhase = 0;
			};
			class wall_down: base {};
			class wall_up: base {};
			class spikes: base {};
		};
	};
};
