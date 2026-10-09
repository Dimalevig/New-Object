// Усі зміни CarScript — в одному файлі: різні modded-шари не бачать методів один одного.
modded class CarScript
{
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

	protected void OblivionDestroyWheels()
	{
		for (int i = 0; i < GetInventory().AttachmentCount(); i++)
		{
			CarWheel wheel = CarWheel.Cast(GetInventory().GetAttachmentFromIndex(i));
			if (wheel)
				wheel.SetHealth("", "", 0);
		}
	}

	// Корпус машини гасить вибух, тому тим, хто всередині, шкода наноситься напряму.
	protected void OblivionHurtCrew(OblivionCarTrapsSettings s)
	{
		for (int i = 0; i < CrewSize(); i++)
		{
			PlayerBase player = PlayerBase.Cast(CrewMember(i));
			if (!player || !player.IsAlive())
				continue;

			if (s.CrewHealthDamage > 0)
				player.AddHealth("", "Health", -s.CrewHealthDamage);
			if (s.KnockOutCrew && player.IsAlive())
				player.SetHealth("", "Shock", 0); // шок 0 = непритомність, як у ванілі
		}
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

		OblivionCarTrapsSettings s = OblivionSettings.Get().CarTraps;
		if (s.DestroyEngine)
			SetHealth("Engine", "", 0);
		if (s.DestroyWheels)
			OblivionDestroyWheels();
		OblivionHurtCrew(s);

		// Справжня граната біля двигуна і її власне спрацювання, як у ванілі:
		// вибух з її типом, ефектом і шкодою, потім граната сама себе видаляє.
		EntityAI grenade = EntityAI.Cast(GetGame().CreateObjectEx(type, GetEnginePointPosWS(), ECE_NONE));
		Grenade_Base live = Grenade_Base.Cast(grenade);
		if (live)
			live.ActivateImmediate();
		else if (grenade)
			grenade.SetHealth("", "", 0);
	}
}
