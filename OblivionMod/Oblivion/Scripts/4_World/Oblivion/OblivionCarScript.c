// Усі зміни CarScript — в одному файлі: різні modded-шари не бачать методів один одного.
modded class CarScript
{
	// --- Інвентар машини зсередини: у русі й при закритому багажнику.

	static CarScript OblivionGetVehicleOf(PlayerBase player)
	{
		if (!player)
			return null;

		HumanCommandVehicle hcv = player.GetCommand_Vehicle();
		if (!hcv)
			return null;

		return CarScript.Cast(hcv.GetTransport());
	}

	protected bool OblivionCargoFromInside()
	{
		OblivionVehicleActionsSettings s = OblivionSettings.Get().VehicleActions;
		if (!s.Enabled || !s.CargoFromInside)
			return false;

		// Клієнт: чи сидить у цій машині сам гравець.
		if (GetGame().IsClient())
			return OblivionGetVehicleOf(PlayerBase.Cast(GetGame().GetPlayer())) == this;

		// Сервер: чи є в машині хоч хтось.
		for (int i = 0; i < CrewSize(); i++)
		{
			if (CrewMember(i))
				return true;
		}
		return false;
	}

	override bool CanDisplayCargo()
	{
		if (OblivionCargoFromInside())
			return true;
		return super.CanDisplayCargo();
	}

	override bool CanReceiveItemIntoCargo(EntityAI item)
	{
		if (OblivionCargoFromInside())
			return true;
		return super.CanReceiveItemIntoCargo(item);
	}

	override bool CanReleaseCargo(EntityAI cargo)
	{
		if (OblivionCargoFromInside())
			return true;
		return super.CanReleaseCargo(cargo);
	}

	// --- Мінування: граната під капотом вибухає при запуску двигуна. Діє до рестарту (не зберігається в базу).

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

	// Гравець біля двигуна: як у ванілі для заливки антифризу — відстань до точки радіатора.
	bool OblivionIsNearEngine(PlayerBase player)
	{
		if (!player)
			return false;
		return vector.Distance(GetCoolantPtcPosWS(), player.GetPosition()) < GetActionDistanceCoolant();
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

		// Тип вибуху: з налаштувань, інакше з конфігу гранати (там він заданий списком, тому читаємо масив).
		OblivionCarTrapsSettings s = OblivionSettings.Get().CarTraps;
		array<string> ammoTypes = new array<string>();
		if (s.ExplosionAmmo != "")
			ammoTypes.Insert(s.ExplosionAmmo);
		else
			GetGame().ConfigGetTextArray("CfgVehicles " + type + " ammoType", ammoTypes);

		if (ammoTypes.Count() == 0)
		{
			string single = GetGame().ConfigGetTextOut("CfgVehicles " + type + " ammoType");
			if (single != "")
				ammoTypes.Insert(single);
		}

		foreach (string ammo : ammoTypes)
			grenade.Explode(DamageType.EXPLOSION, ammo);

		if (s.DestroyEngine)
			SetHealth("Engine", "", 0);

		// Видаляємо трохи пізніше, щоб клієнти встигли отримати ефект вибуху.
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GetGame().ObjectDelete, 1000, false, grenade);
	}
}
