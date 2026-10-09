// Нагорода за час у грі: кожні RewardMinutes живої гри — випадковий предмет із LootPool.
// Один таймер на сервер раз на хвилину; час рахується в пам'яті до рестарту.
class OblivionPlaytimeRewards
{
	protected const int TICK_SECONDS = 60;
	protected ref map<string, int> m_Seconds = new map<string, int>(); // Steam ID -> накопичені секунди

	void Start()
	{
		OblivionPlaytimeRewardsSettings s = OblivionSettings.Get().PlaytimeRewards;
		if (!s.Enabled || s.LootPool.Count() == 0 || s.RewardMinutes <= 0)
			return;

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Tick, TICK_SECONDS * 1000, true);
	}

	void Tick()
	{
		OblivionPlaytimeRewardsSettings s = OblivionSettings.Get().PlaytimeRewards;
		int needed = s.RewardMinutes * 60;

		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);

		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.IsAlive() || !player.GetIdentity())
				continue;

			string uid = player.OblivionGetUid();
			int seconds = m_Seconds.Get(uid) + TICK_SECONDS;
			if (seconds >= needed)
			{
				seconds -= needed;
				Reward(player, s);
			}
			m_Seconds.Set(uid, seconds);
		}
	}

	protected void Reward(PlayerBase player, OblivionPlaytimeRewardsSettings s)
	{
		array<string> items = new array<string>();
		for (int i = 0; i < Math.Max(1, s.ItemsPerReward); i++)
			items.Insert(s.LootPool.GetRandomElement());

		OblivionNotify.GiveItems(player, items);

		int minutes = s.RewardMinutes;
		OblivionNotify.Player(player, "Нагорода за час у грі", minutes + " хв у грі — у тебе в інвентарі (або під ногами) подарунок.", s.NotifySeconds);
	}
}
