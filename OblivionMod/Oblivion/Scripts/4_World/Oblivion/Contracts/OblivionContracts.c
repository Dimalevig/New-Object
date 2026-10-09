// Контракти: /contract <нік> — предмет у руках стає нагородою за вбивство цілі.
// Нагороду отримує той, хто вб'є ціль. Контракти зберігаються у contracts.json (запис лише при зміні).
class OblivionContract
{
	string TargetUid;
	string TargetName;
	string OrdererUid;
	string OrdererName;
	string ItemType;
	string ItemName;
	float  Quantity = -1;
	float  Health   = 1;
}

class OblivionContractsData
{
	ref array<ref OblivionContract> Contracts = new array<ref OblivionContract>();
}

class OblivionContracts
{
	protected static ref OblivionContractsData s_Data;

	protected static OblivionContractsData Data()
	{
		if (!s_Data)
		{
			s_Data = new OblivionContractsData();
			if (FileExist(OBLIVION_CONTRACTS_FILE))
				JsonFileLoader<OblivionContractsData>.JsonLoadFile(OBLIVION_CONTRACTS_FILE, s_Data);
			if (!s_Data.Contracts)
				s_Data.Contracts = new array<ref OblivionContract>();
		}
		return s_Data;
	}

	protected static void Save()
	{
		JsonFileLoader<OblivionContractsData>.JsonSaveFile(OBLIVION_CONTRACTS_FILE, Data());
	}

	static void HandleCommand(PlayerBase player, array<string> words)
	{
		OblivionContractsSettings s = OblivionSettings.Get().Contracts;
		if (!s.Enabled)
		{
			OblivionNotify.Player(player, "Контракти", "Контракти вимкнені.", s.NotifySeconds);
			return;
		}

		string cmd = words[0];
		cmd.ToLower();
		if (cmd == "/contracts" || cmd == "/контракти" || words.Count() < 2)
		{
			List(player, s);
			return;
		}

		string nick = words[1];
		for (int i = 2; i < words.Count(); i++)
			nick += " " + words[i];

		Order(player, nick, s);
	}

	protected static void Order(PlayerBase orderer, string nick, OblivionContractsSettings s)
	{
		PlayerBase target = OblivionNotify.FindPlayerByName(nick);
		if (!target)
		{
			OblivionNotify.Player(orderer, "Контракти", "Гравця «" + nick + "» немає онлайн або збігів кілька.", s.NotifySeconds);
			return;
		}
		if (target == orderer)
		{
			OblivionNotify.Player(orderer, "Контракти", "Не можна замовити самого себе.", s.NotifySeconds);
			return;
		}

		string ordererUid = orderer.OblivionGetUid();
		int active;
		foreach (OblivionContract existing : Data().Contracts)
		{
			if (existing.OrdererUid == ordererUid)
				active++;
		}
		if (active >= s.MaxActivePerPlayer)
		{
			OblivionNotify.Player(orderer, "Контракти", "У тебе вже " + active + " активних контрактів (максимум " + s.MaxActivePerPlayer + ").", s.NotifySeconds);
			return;
		}

		ItemBase item = orderer.GetItemInHands();
		if (!item || item.IsRuined())
		{
			OblivionNotify.Player(orderer, "Контракти", "Візьми в руки предмет-нагороду (не зіпсований).", s.NotifySeconds);
			return;
		}
		CargoBase cargo = item.GetInventory().GetCargo();
		if (item.GetInventory().AttachmentCount() > 0 || (cargo && cargo.GetItemCount() > 0))
		{
			OblivionNotify.Player(orderer, "Контракти", "Зніми з предмета все приладдя і вийми вміст.", s.NotifySeconds);
			return;
		}

		OblivionContract c = new OblivionContract();
		c.TargetUid   = target.OblivionGetUid();
		c.TargetName  = target.OblivionGetName();
		c.OrdererUid  = ordererUid;
		c.OrdererName = orderer.OblivionGetName();
		c.ItemType    = item.GetType();
		c.ItemName    = item.GetDisplayName();
		c.Health      = item.GetHealth01("", "");
		if (item.HasQuantity())
			c.Quantity = item.GetQuantity();

		GetGame().ObjectDelete(item);
		Data().Contracts.Insert(c);
		Save();

		OblivionNotify.All("Контракт", "На гравця " + c.TargetName + " замовлено контракт. Нагорода: " + c.ItemName + ".", s.NotifySeconds);
	}

	protected static void List(PlayerBase player, OblivionContractsSettings s)
	{
		// ціль -> кількість нагород
		map<string, int> perTarget = new map<string, int>();
		foreach (OblivionContract c : Data().Contracts)
			perTarget.Set(c.TargetName, perTarget.Get(c.TargetName) + 1);

		if (perTarget.Count() == 0)
		{
			OblivionNotify.Player(player, "Контракти", "Активних контрактів немає. Замовити: /contract <нік> з нагородою в руках.", s.NotifySeconds);
			return;
		}

		string text;
		int shown;
		foreach (string name, int count : perTarget)
		{
			if (shown == 5)
			{
				text += " …";
				break;
			}
			if (shown > 0)
				text += "; ";
			text += name + " (" + count + ")";
			shown++;
		}
		OblivionNotify.Player(player, "Контракти", text, s.NotifySeconds);
	}

	static void OnPlayerKilled(PlayerBase victim, PlayerBase killer)
	{
		OblivionContractsSettings s = OblivionSettings.Get().Contracts;
		if (!s.Enabled || !victim || !killer)
			return;

		string victimUid = victim.OblivionGetUid();
		if (victimUid == "")
			return;

		array<ref OblivionContract> contracts = Data().Contracts;
		int paid;
		for (int i = contracts.Count() - 1; i >= 0; i--)
		{
			OblivionContract c = contracts[i];
			if (c.TargetUid != victimUid)
				continue;

			GiveReward(killer, c);
			contracts.Remove(i);
			paid++;
		}

		if (paid == 0)
			return;

		Save();
		OblivionNotify.All("Контракт виконано", killer.OblivionGetName() + " вбив " + victim.OblivionGetName() + " і забрав нагород: " + paid + ".", s.NotifySeconds);
	}

	protected static void GiveReward(PlayerBase player, OblivionContract c)
	{
		EntityAI spawned = player.GetInventory().CreateInInventory(c.ItemType);
		if (!spawned)
			spawned = EntityAI.Cast(GetGame().CreateObjectEx(c.ItemType, player.GetPosition(), ECE_PLACE_ON_SURFACE));

		ItemBase item = ItemBase.Cast(spawned);
		if (!item)
			return;

		if (c.Quantity >= 0 && item.HasQuantity())
			item.SetQuantity(c.Quantity);
		item.SetHealth("", "", item.GetMaxHealth("", "") * c.Health);
	}
}
