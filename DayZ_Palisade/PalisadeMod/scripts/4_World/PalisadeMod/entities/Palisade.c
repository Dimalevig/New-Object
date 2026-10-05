class Palisade extends BaseBuildingBase
{
	const float MAX_ACTION_DETECTION_ANGLE_RAD 		= 1.3;		//1.3 RAD = ~75 DEG
	const float MAX_ACTION_DETECTION_DISTANCE 		= 2.0;		//meters

	void Palisade()
	{
	}

	override string GetConstructionKitType()
	{
		return "PalisadeKit";
	}

	override int GetMeleeTargetType()
	{
		return EMeleeTargetType.NONALIGNABLE;
	}

	//--- INVENTORY
	override bool CanPutInCargo(EntityAI parent)
	{
		return false;
	}

	override bool CanPutIntoHands(EntityAI parent)
	{
		return false;
	}

	//--- KIT
	override vector GetKitSpawnPosition()
	{
		if (MemoryPointExists("kit_spawn_position"))
		{
			vector position = GetMemoryPointPos("kit_spawn_position");
			return ModelToWorld(position);
		}

		return GetPosition();
	}

	//================================================================
	// ACTION CONDITIONS
	//================================================================
	// player can build/dismantle only from the front side (like vanilla Fence)
	override bool IsFacingPlayer(PlayerBase player, string selection)
	{
		vector palisade_pos = GetPosition();
		vector player_pos = player.GetPosition();
		vector ref_dir = GetDirection();

		vector palisade_player_dir = player_pos - palisade_pos;
		palisade_player_dir.Normalize();
		palisade_player_dir[1] = 0;

		ref_dir.Normalize();
		ref_dir[1] = 0;

		if (ref_dir.Length() != 0)
		{
			float angle = Math.Acos(palisade_player_dir * ref_dir);
			if (angle >= MAX_ACTION_DETECTION_ANGLE_RAD)
				return true;
		}

		return false;
	}

	override bool IsFacingCamera(string selection)
	{
		vector ref_dir = GetDirection();
		vector cam_dir = GetGame().GetCurrentCameraDirection();

		ref_dir[1] = 0;
		ref_dir.Normalize();

		cam_dir[1] = 0;
		cam_dir.Normalize();

		if (ref_dir.Length() != 0 && cam_dir.Length() != 0)
		{
			float angle = Math.Acos(cam_dir * ref_dir);
			if (angle >= MAX_ACTION_DETECTION_ANGLE_RAD)
				return true;
		}

		return false;
	}

	override bool HasProperDistance(string selection, PlayerBase player)
	{
		if (MemoryPointExists(selection))
		{
			vector selection_pos = ModelToWorld(GetMemoryPointPos(selection));
			float distance = vector.Distance(selection_pos, player.GetPosition());
			if (distance >= MAX_ACTION_DETECTION_DISTANCE)
				return false;
		}

		return true;
	}

	//================================================================
	// ACTIONS
	//================================================================
	override void SetActions()
	{
		super.SetActions();

		AddAction(ActionTogglePlaceObject);
		AddAction(ActionPlaceObject);
		AddAction(ActionFoldBaseBuildingObject);
	}
}
