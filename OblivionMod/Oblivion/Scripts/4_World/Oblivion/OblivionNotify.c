class OblivionNotify
{
	static void All(string title, string text, float seconds)
	{
		// identity = null -> повідомлення всім гравцям.
		NotificationSystem.SendNotificationToPlayerIdentityExtended(null, seconds, title, text);
	}

	static void ToPlayer(PlayerBase player, string title, string text, float seconds)
	{
		if (player && player.GetIdentity())
			NotificationSystem.SendNotificationToPlayerIdentityExtended(player.GetIdentity(), seconds, title, text);
	}

	static PlayerBase FindPlayerByName(string name)
	{
		string wanted = name;
		wanted.ToLower();

		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);

		PlayerBase partial;
		int partialCount;
		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.GetIdentity())
				continue;

			string nick = player.GetIdentity().GetName();
			nick.ToLower();
			if (nick == wanted)
				return player;
			if (nick.Contains(wanted))
			{
				partial = player;
				partialCount++;
			}
		}

		if (partialCount == 1)
			return partial;
		return null;
	}

	// Предмети в інвентар гравця, якщо немає місця — на землю. times — скільки разів видати весь список.
	static void GiveItems(PlayerBase player, array<string> items, int times = 1)
	{
		if (!player || !items)
			return;

		for (int i = 0; i < times; i++)
		{
			foreach (string type : items)
				SpawnItem(player, type, player.GetPosition());
		}
	}

	static void SpawnItem(EntityAI container, string type, vector fallbackPos)
	{
		if (container && container.GetInventory().CreateInInventory(type))
			return;
		GetGame().CreateObjectEx(type, fallbackPos, ECE_PLACE_ON_SURFACE);
	}
}
