class CfgPatches
{
	class OBL_SystemParty
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]={
			"DZ_Scripts",
			"DZ_Data",
			"DZ_Gear_Navigation",
			"RPC_Scripts"
		};
	};
};
class CfgMods
{
	class OBL_SystemParty
	{
		dir="OBL_SystemParty";
		picture="";
		action="";
		hideName=0;
		hidePicture=1;
		name="Oblivion";
		author="Дreykwood";
		credits="Дreykwood";
		authorID="0";
		version="1.0";
		type="mod";
		inputs="OBL_SystemParty/inputsOBL.xml";
		dependencies[] = { "Game", "World", "Mission" };
		class defs
		{
			class gameScriptModule
			{
				value="";
				files[] = {"OBL_SystemParty/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value="";
				files[] = {"OBL_SystemParty/scripts/4_World"};
			};
			class missionScriptModule
			{
				value="";
				files[] = {"OBL_SystemParty/scripts/5_Mission"};
			};
		};
	};
};
