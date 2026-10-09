// Нагорода за час у грі: кожні RewardMinutes живої гри — випадковий предмет із LootPool.
// Один таймер на сервер раз на хвилину. Накопичений час зберігається в playtime.json
// раз на SAVE_EVERY_TICKS хвилин і при вимкненні сервера — рестарт його не скидає.
class OblivionPlaytimeEntry
{
	string Uid;
	int    Seconds;
}

class OblivionPlaytimeData
{
	ref array<ref OblivionPlaytimeEntry> Players = new array<ref OblivionPlaytimeEntry>();
}

class OblivionPlaytimeRewards
{
	protected const int TICK_SECONDS     = 60;
	protected const int SAVE_EVERY_TICKS = 5;

	protected ref map<string, int> m_Seconds = new map<string, int>(); // Steam ID -> накопичені секунди
	protected int  m_TicksSinceSave;
	protected bool m_Dirty;
	protected bool m_Started;

	void Start()
	{
		OblivionPlaytimeRewardsSettings s = OblivionSettings.Get().PlaytimeRewards;
		if (!s.Enabled || s.LootPool.Count() == 0 || s.RewardMinutes <= 0)
			return;

		Load();
		m_Started = true;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Tick, TICK_SECONDS * 1000, true);
	}

	protected void Load()
	{
		if (!FileExist(OBLIVION_PLAYTIME_FILE))
			return;

		OblivionPlaytimeData data = new OblivionPlaytimeData();
		JsonFileLoader<OblivionPlaytimeData>.JsonLoadFile(OBLIVION_PLAYTIME_FILE, data);
		if (!data.Players)
			return;

		foreach (OblivionPlaytimeEntry entry : data.Players)
			m_Seconds.Set(entry.Uid, entry.Seconds);
	}

	void Save()
	{
		if (!m_Started || !m_Dirty)
			return;

		OblivionPlaytimeData data = new OblivionPlaytimeData();
		foreach (string uid, int seconds : m_Seconds)
		{
			OblivionPlaytimeEntry entry = new OblivionPlaytimeEntry();
			entry.Uid     = uid;
			entry.Seconds = seconds;
			data.Players.Insert(entry);
		}

		JsonFileLoader<OblivionPlaytimeData>.JsonSaveFile(OBLIVION_PLAYTIME_FILE, data);
		m_Dirty = false;
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
			m_Dirty = true;
		}

		m_TicksSinceSave++;
		if (m_TicksSinceSave >= SAVE_EVERY_TICKS)
		{
			m_TicksSinceSave = 0;
			Save();
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
