class OblivionNotify
{
	static void All(string title, string text, float seconds)
	{
		// identity = null -> повідомлення всім гравцям.
		NotificationSystem.SendNotificationToPlayerIdentityExtended(null, seconds, title, text);
	}

	// Особисте повідомлення: сповіщення + рядок у чаті (на випадок, якщо сповіщення не видно).
	static void ToPlayer(PlayerBase player, string title, string text, float seconds)
	{
		if (!player || !player.GetIdentity())
			return;

		NotificationSystem.SendNotificationToPlayerIdentityExtended(player.GetIdentity(), seconds, title, text);
		GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>(title + ": " + text), true, player.GetIdentity());

		// Чат мода пати (OBL_SystemPartyServer) замінює ванільний — відповідаємо і туди, викликом за назвою.
		if (HasPartyChat())
			GetGame().GameScript.CallFunctionParams(GetGame().GetMission(), "SendSimpleChatMessage", null, new Param3<PlayerIdentity, string, bool>(player.GetIdentity(), title + ": " + text, false));
	}

	protected static int s_PartyChat = -1;

	static bool HasPartyChat()
	{
		if (s_PartyChat == -1)
		{
			s_PartyChat = 0;
			if (GetGame().ConfigIsExisting("CfgPatches OBL_SystemPartyServer"))
				s_PartyChat = 1;
		}
		return s_PartyChat == 1;
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
