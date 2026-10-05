// Custom Palisade - palisade marking kit "Rozmitka chastokolu" (4_World).
//
// Stage 2: the kit exists as an item (spawn, pick up, carry, drop).
// It reuses the vanilla FenceKit model, rope attachment and disassembly
// (detaching the rope gives back sticks + rope).
// Placement is intentionally disabled until stage 3 (Kit -> Placement).

class CP_PalisadeKit extends FenceKit
{
	override void SetActions()
	{
		super.SetActions();

		// enabled in stage 3
		RemoveAction(ActionTogglePlaceObject);
		RemoveAction(ActionDeployObject);
	}

	// Admin/debug "spawn special": only the kit itself, no vanilla fence materials.
	override void OnDebugSpawn()
	{
	}
}
