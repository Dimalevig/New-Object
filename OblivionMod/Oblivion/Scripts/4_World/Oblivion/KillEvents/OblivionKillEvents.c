// «Вбивця в розшуку» і «Помста». Працює лише в момент смерті гравця, дані в пам'яті до рестарту.
class OblivionKillEvents
{
	protected static ref map<string, int>    s_WantedUntil  = new map<string, int>();    // Steam ID -> до коли в розшуку
	protected static ref map<string, string> s_LastKillerOf = new map<string, string>(); // жертва -> хто її вбив
	protected static ref map<string, int>    s_LastKillTime = new map<string, int>();

	static void OnPlayerKilled(PlayerBase victim, PlayerBase killer)
	{
		if (!victim || !killer)
			return;

		string victimUid = victim.OblivionGetUid();
		string killerUid = killer.OblivionGetUid();
		if (victimUid == "" || killerUid == "")
			return;

		int now = GetGame().GetTime();
		CheckRevenge(victim, killer, victimUid, killerUid, now);
		CheckWanted(victim, killer, victimUid, killerUid, now);
	}

	protected static void CheckRevenge(PlayerBase victim, PlayerBase killer, string victimUid, string killerUid, int now)
	{
		OblivionRevengeSettings s = OblivionSettings.Get().Revenge;
		if (s.Enabled && s_LastKillerOf.Get(killerUid) == victimUid && now - s_LastKillTime.Get(killerUid) <= s.WindowMinutes * 60000)
		{
			s_LastKillerOf.Remove(killerUid);
			s_LastKillTime.Remove(killerUid);
			OblivionNotify.All("Помста!", killer.OblivionGetName() + " помстився " + victim.OblivionGetName() + ".", s.NotifySeconds);
			OblivionNotify.GiveItems(killer, s.RewardItems);
		}

		s_LastKillerOf.Set(victimUid, killerUid);
		s_LastKillTime.Set(victimUid, now);
	}

	protected static void CheckWanted(PlayerBase victim, PlayerBase killer, string victimUid, string killerUid, int now)
	{
		OblivionWantedSettings s = OblivionSettings.Get().Wanted;
		if (!s.Enabled)
			return;

		if (s_WantedUntil.Get(victimUid) > now)
		{
			s_WantedUntil.Remove(victimUid);
			OblivionNotify.All("Розшукуваного вбито", killer.OblivionGetName() + " вбив розшукуваного " + victim.OblivionGetName() + ".", s.NotifySeconds);
			OblivionNotify.GiveItems(killer, s.RewardItems, s.RewardMultiplier);
		}

		if (victim.OblivionIsFreshSpawn(s.FreshSpawnMinutes))
		{
			s_WantedUntil.Set(killerUid, now + s.WantedMinutes * 60000);
			int minutes = s.WantedMinutes;
			OblivionNotify.All("Вбивця в розшуку", killer.OblivionGetName() + " вбив новачка. У розшуку " + minutes + " хв — за його голову подвійна нагорода.", s.NotifySeconds);
		}
	}
}
