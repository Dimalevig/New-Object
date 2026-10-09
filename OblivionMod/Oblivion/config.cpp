class CfgPatches
{
	class Oblivion_Scripts
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Weapons_Optics"};
	};
};

class CfgMods
{
	class Oblivion
	{
		dir = "Oblivion";
		name = "Oblivion";
		credits = "Oblivion Server";
		author = "Oblivion";
		version = "0.7.0";
		type = "mod";
		hideName = 1;
		hidePicture = 1;
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"Oblivion/Scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"Oblivion/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"Oblivion/Scripts/5_Mission"};
			};
		};
	};
};

class CfgVehicles
{
	class ItemOptics;

	// Мисливський приціл кріпиться і на Мосіна (слот прицілу ПУ).
	class HuntingOptic: ItemOptics
	{
		inventorySlot[] = {"weaponOpticsHunting", "weaponOpticsMosin"};
	};
};
