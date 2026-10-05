class PalisadeKit extends KitBase
{
	override bool PlacementCanBeRotated()
	{
		return false;
	}

	override bool DoPlacingHeightCheck()
	{
		return true;
	}

	override float HeightCheckOverride()
	{
		return 3.2;
	}

	//================================================================
	// DISASSEMBLE (rope detached from kit)
	//================================================================
	override void DisassembleKit(ItemBase item)
	{
		if (!IsIgnoredByConstruction())
		{
			ItemBase stick = ItemBase.Cast(GetGame().CreateObjectEx("WoodenStick", GetPosition(), ECE_PLACE_ON_SURFACE));
			MiscGameplayFunctions.TransferItemProperties(this, stick);
			stick.SetQuantity(4);

			Rope rope = Rope.Cast(item);
			CreateRope(rope);
		}
	}

	//================================================================
	// ADVANCED PLACEMENT
	//================================================================
	override void OnPlacementComplete(Man player, vector position = "0 0 0", vector orientation = "0 0 0")
	{
		super.OnPlacementComplete(player, position, orientation);

		if (GetGame().IsServer())
		{
			Palisade palisade = Palisade.Cast(GetGame().CreateObjectEx("Palisade", GetPosition(), ECE_PLACE_ON_SURFACE));
			palisade.SetPosition(position);
			palisade.SetOrientation(orientation);

			//make the kit invisible, so it can be destroyed from deploy UA when action ends
			HideAllSelections();

			SetIsDeploySound(true);
		}
	}

	override string GetDeploySoundset()
	{
		return "putDown_FenceKit_SoundSet";
	}

	override string GetLoopDeploySoundset()
	{
		return "BarbedWire_Deploy_loop_SoundSet";
	}

	override void SetActions()
	{
		super.SetActions();

		AddAction(ActionTogglePlaceObject);
		AddAction(ActionDeployObject);
	}
}
