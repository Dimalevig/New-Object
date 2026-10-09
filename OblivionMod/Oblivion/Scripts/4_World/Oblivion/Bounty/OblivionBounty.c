// Баунті: KillsForBounty вбивств поспіль без смерті -> сервер оголошує полювання з квадратом на мапі.
// Працює лише в момент смерті гравця, серія зберігається в пам'яті сервера за Steam ID.
class OblivionBounty
{
	protected static ref map<string, int> s_Streaks = new map<string, int>();

	static void OnPlayerKilled(PlayerBase victim, PlayerBase killer)
	{
		OblivionBountySettings s = OblivionSettings.Get().Bounty;
		if (!s.Enabled || !victim)
			return;

		string victimUid = victim.OblivionGetUid();
		int victimStreak = 0;
		if (victimUid != "")
		{
			victimStreak = s_Streaks.Get(victimUid);
			s_Streaks.Remove(victimUid);
		}
		bool victimHadBounty = victimStreak >= s.KillsForBounty;

		if (!killer)
		{
			if (victimHadBounty)
				Announce(s, "Баунті згоріло", victim.OblivionGetName() + " загинув сам. Серія: " + victimStreak + ".");
			return;
		}

		string killerUid = killer.OblivionGetUid();
		if (killerUid == "")
			return;

		if (victimHadBounty)
		{
			Announce(s, "Голову знято", killer.OblivionGetName() + " вбив " + victim.OblivionGetName() + " (серія " + victimStreak + ").");
			GiveRewards(killer, s.RewardItems);
		}

		int streak = s_Streaks.Get(killerUid) + 1;
		s_Streaks.Set(killerUid, streak);

		string where = "Квадрат " + Grid(killer.GetPosition(), s.GridMeters) + ".";
		if (streak == s.KillsForBounty)
			Announce(s, "Полювання!", "За гравцем " + killer.OblivionGetName() + " полює сервер: " + streak + " вбивств поспіль. " + where);
		else if (streak > s.KillsForBounty && s.AnnounceEveryKill)
			Announce(s, "Ціль знову вбила", killer.OblivionGetName() + ": " + streak + " вбивств поспіль. " + where);
	}

	// Координати як в iZurvive: сотні метрів, 3 цифри (напр. "040 110"), округлені до GridMeters.
	static string Grid(vector pos, int gridMeters)
	{
		int size = Math.Max(100, gridMeters);
		int x = Math.Floor(pos[0] / size) * size / 100;
		int z = Math.Floor(pos[2] / size) * size / 100;
		return x.ToStringLen(3) + " " + z.ToStringLen(3);
	}

	protected static void Announce(OblivionBountySettings s, string title, string text)
	{
		// identity = null -> повідомлення всім гравцям.
		NotificationSystem.SendNotificationToPlayerIdentityExtended(null, s.NotifySeconds, title, text);
	}

	protected static void GiveRewards(PlayerBase player, array<string> items)
	{
		foreach (string type : items)
		{
			if (!player.GetInventory().CreateInInventory(type))
				GetGame().CreateObjectEx(type, player.GetPosition(), ECE_PLACE_ON_SURFACE);
		}
	}
}
