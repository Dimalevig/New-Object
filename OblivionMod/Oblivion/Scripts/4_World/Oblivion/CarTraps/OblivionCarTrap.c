// Граната під капотом вибухає при запуску двигуна. Діє до рестарту (не зберігається в базу).
modded class CarScript
{
	protected string m_OblivionMine; // клас гранати, "" = не замінована

	bool OblivionIsMined()
	{
		return m_OblivionMine != "";
	}

	void OblivionSetMine(string grenadeType)
	{
		m_OblivionMine = grenadeType;
	}

	// Знімає міну і повертає клас гранати ("" — міни не було).
	string OblivionTakeMine()
	{
		string type = m_OblivionMine;
		m_OblivionMine = "";
		return type;
	}

	// Капот відкритий або знятий. Слот капота шукаємо за назвою — працює для різних машин.
	bool OblivionIsHoodOpen()
	{
		for (int i = 0; i < GetInventory().AttachmentCount(); i++)
		{
			EntityAI attachment = GetInventory().GetAttachmentFromIndex(i);
			InventoryLocation location = new InventoryLocation();
			if (!attachment || !attachment.GetInventory().GetCurrentInventoryLocation(location))
				continue;

			string slot = InventorySlots.GetSlotName(location.GetSlot());
			if (slot.Contains("Hood"))
				return GetCarDoorsState(slot) != CarDoorState.DOORS_CLOSED;
		}
		return true; // капота немає
	}

	override void OnEngineStart()
	{
		super.OnEngineStart();

		if (!GetGame().IsServer() || m_OblivionMine == "")
			return;

		string type = m_OblivionMine;
		m_OblivionMine = "";

		EntityAI grenade = EntityAI.Cast(GetGame().CreateObjectEx(type, GetPosition() + "0 0.5 0", ECE_NONE));
		if (!grenade)
			return;

		grenade.Explode(DamageType.EXPLOSION);
		// Видаляємо трохи пізніше, щоб клієнти встигли отримати ефект вибуху.
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GetGame().ObjectDelete, 1000, false, grenade);
	}
}
