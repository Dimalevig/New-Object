class OblivionNotify
{
	static void All(string title, string text, float seconds)
	{
		// identity = null -> повідомлення всім гравцям.
		NotificationSystem.SendNotificationToPlayerIdentityExtended(null, seconds, title, text);
	}

	static void Player(PlayerBase player, string title, string text, float seconds)
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

	static void SpawnItem(EntityAI container, string type, vector fallbackPos)
	{
		if (container && container.GetInventory().CreateInInventory(type))
			return;
		GetGame().CreateObjectEx(type, fallbackPos, ECE_PLACE_ON_SURFACE);
	}
}
