class CfgPatches
{
	class PalisadeMod
	{
		units[] = {"PalisadeKit", "Palisade"};
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
	class PalisadeMod
	{
		dir = "PalisadeMod";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "PalisadeMod";
		credits = "";
		author = "";
		authorID = "0";
		version = "1.0";
		extra = 0;
		type = "mod";
		dependencies[] = {"World"};

		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"PalisadeMod/scripts/4_World"};
			};
		};
	};
};

class CfgVehicles
{
	class Inventory_Base;
	class KitBase;
	class BaseBuildingBase;

	//================================================================
	// KIT
	//================================================================
	class PalisadeKit: KitBase
	{
		scope = 2;
		displayName = "$STR_CfgVehicles_PalisadeKit0";
		descriptionShort = "$STR_CfgVehicles_PalisadeKit1";
		// placeholder: vanilla fence kit model (pile of sticks + rope)
		model = "\DZ\gear\camping\fence_kit.p3d";
		rotationFlags = 17;
		itemSize[] = {1, 5};
		weight = 280;
		itemBehaviour = 1;
		attachments[] = {"Rope"};
		projectionTypename = "PalisadeKitPlacing";

		class AnimationSources
		{
			class AnimSourceShown
			{
				source = "user";
				animPeriod = 0.01;
				initPhase = 0;
			};
			class AnimSourceHidden
			{
				source = "user";
				animPeriod = 0.01;
				initPhase = 1;
			};
			class Inventory: AnimSourceShown {};
			class Placing: AnimSourceHidden {};
		};
	};

	// Hologram shown while placing the kit
	class PalisadeKitPlacing: PalisadeKit
	{
		scope = 1;
		model = "\PalisadeMod\data\palisade_placing.p3d";
		storageCategory = 10;
		hiddenSelections[] = {"placing"};
		hiddenSelectionsTextures[] = {"\PalisadeMod\data\palisade_co.paa"};
		hiddenSelectionsMaterials[] = {"\PalisadeMod\data\palisade.rvmat"};
		hologramMaterial = "tent_medium";
		hologramMaterialPath = "dz\gear\camping\data";
		alignHologramToTerain = 0;
		slopeTolerance = 0.3;
		yawPitchRollLimit[] = {10, 10, 10};

		class AnimationSources
		{
			class AnimSourceShown
			{
				source = "user";
				animPeriod = 0.01;
				initPhase = 0;
			};
			class AnimSourceHidden
			{
				source = "user";
				animPeriod = 0.01;
				initPhase = 1;
			};
			class Inventory: AnimSourceHidden {};
			class Placing: AnimSourceShown {};
		};
	};

	//================================================================
	// PALISADE
	//================================================================
	class Palisade: BaseBuildingBase
	{
		scope = 2;
		displayName = "$STR_CfgVehicles_Palisade0";
		descriptionShort = "$STR_CfgVehicles_Palisade1";
		model = "\PalisadeMod\data\palisade.p3d";
		bounding = "BSphere";
		overrideDrawArea = "3.0";
		forceFarBubble = "true";
		handheld = "false";
		lootCategory = "Crafted";
		carveNavmesh = 1;
		weight = 10000;
		itemSize[] = {2, 3};
		physLayer = "item_large";
		createProxyPhysicsOnInit = "false";
		rotationFlags = 2;

		attachments[] =
		{
			"Material_WoodenLogs",
			"Material_WoodenPlanks",
			"Material_Nails",
			"Material_MetalWire"
		};

		class GUIInventoryAttachmentsProps
		{
			class Base
			{
				name = "$STR_CfgVehicles_Palisade_Att_Category_Base";
				description = "";
				attachmentSlots[] = {"Material_WoodenLogs"};
				icon = "cat_bb_base";
				selection = "base";
			};
			class Attachments
			{
				name = "$STR_CfgVehicles_Palisade_Att_Category_Materials";
				description = "";
				attachmentSlots[] = {"Material_WoodenPlanks", "Material_Nails", "Material_MetalWire"};
				icon = "cat_bb_attachments";
				selection = "wall_down";
			};
		};

		class AnimationSources
		{
			class AnimSourceShown
			{
				source = "user";
				animPeriod = 0.01;
				initPhase = 0;
			};
			class AnimSourceHidden
			{
				source = "user";
				animPeriod = 0.01;
				initPhase = 1;
			};
			// one source per construction part (name == part name)
			class base: AnimSourceHidden {};
			class wall_down: AnimSourceHidden {};
			class wall_up: AnimSourceHidden {};
			class spikes: AnimSourceHidden {};
		};

		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 1000;
				};
			};
			class GlobalArmor
			{
				class Projectile { class Health { damage = 0; }; class Blood { damage = 0; }; class Shock { damage = 0; }; };
				class FragGrenade { class Health { damage = 0; }; class Blood { damage = 0; }; class Shock { damage = 0; }; };
			};
			class DamageZones
			{
				// zone name must match construction part name - when the zone is ruined the part is destroyed
				class base
				{
					class Health
					{
						hitpoints = 6000;
						transferToGlobalCoef = 0;
					};
					class ArmorType
					{
						class Projectile { class Health { damage = 0; }; class Blood { damage = 0; }; class Shock { damage = 0; }; };
						class Melee { class Health { damage = 0.25; }; class Blood { damage = 0; }; class Shock { damage = 0; }; };
						class FragGrenade { class Health { damage = 2; }; class Blood { damage = 0; }; class Shock { damage = 0; }; };
					};
					componentNames[] = {"base"};
					fatalInjuryCoef = -1;
				};
				class wall_down: base
				{
					class Health
					{
						hitpoints = 15000;
						transferToGlobalCoef = 0;
					};
					componentNames[] = {"wall_down"};
				};
				class wall_up: wall_down
				{
					componentNames[] = {"wall_up"};
				};
				class spikes: base
				{
					class Health
					{
						hitpoints = 4000;
						transferToGlobalCoef = 0;
					};
					componentNames[] = {"spikes"};
				};
			};
		};

		//------------------------------------------------------------
		// build_action_type / dismantle_action_type are bit flags that
		// must match the tool used (same values as vanilla Fence):
		//   4 = Shovel / dig tools
		//   2 = Hammer / Hatchet / wood tools
		// material_type: 1 = logs, 2 = wood (planks), 4 = metal, 5 = wire
		//------------------------------------------------------------
		class Construction
		{
			class palisade
			{
				// 1) dug in corner posts - two logs
				class base
				{
					name = "$STR_CfgVehicles_Palisade_Base0";
					is_base = 1;
					id = 1;
					required_parts[] = {};
					conflicted_parts[] = {};
					collision_data[] = {};
					build_action_type = 4;
					dismantle_action_type = 4;
					material_type = 1;
					class Materials
					{
						class Material1
						{
							type = "WoodenLog";
							slot_name = "Material_WoodenLogs";
							quantity = 2;
							lockable = 1;
						};
					};
				};

				// 2) half of the logs + two lower board rows (inner side)
				class wall_down
				{
					name = "$STR_CfgVehicles_Palisade_WallDown0";
					id = 2;
					required_parts[] = {"base"};
					conflicted_parts[] = {};
					collision_data[] = {"wall_down_min", "wall_down_max"};
					build_action_type = 2;
					dismantle_action_type = 2;
					material_type = 1;
					class Materials
					{
						class Material1
						{
							type = "WoodenLog";
							slot_name = "Material_WoodenLogs";
							quantity = 2;
						};
						class Material2
						{
							type = "WoodenPlank";
							slot_name = "Material_WoodenPlanks";
							quantity = 4;
						};
						class Material3
						{
							type = "Nails";
							slot_name = "Material_Nails";
							quantity = 10;
						};
					};
				};

				// 3) other half of the logs + top board row + outer pole, tied with metal wire
				class wall_up
				{
					name = "$STR_CfgVehicles_Palisade_WallUp0";
					id = 3;
					required_parts[] = {"wall_down"};
					conflicted_parts[] = {};
					collision_data[] = {"wall_up_min", "wall_up_max"};
					build_action_type = 2;
					dismantle_action_type = 2;
					material_type = 1;
					class Materials
					{
						class Material1
						{
							type = "WoodenLog";
							slot_name = "Material_WoodenLogs";
							quantity = 2;
						};
						class Material2
						{
							type = "WoodenPlank";
							slot_name = "Material_WoodenPlanks";
							quantity = 2;
						};
						class Material3
						{
							type = "MetalWire";
							slot_name = "Material_MetalWire";
							quantity = -1;
							lockable = 1;
						};
					};
				};

				// 4) sharpened tops
				class spikes
				{
					name = "$STR_CfgVehicles_Palisade_Spikes0";
					id = 4;
					required_parts[] = {"wall_up"};
					conflicted_parts[] = {};
					collision_data[] = {};
					build_action_type = 2;
					dismantle_action_type = 2;
					material_type = 2;
					class Materials
					{
						class Material1
						{
							type = "Nails";
							slot_name = "Material_Nails";
							quantity = 10;
						};
					};
				};
			};
		};
	};
};
