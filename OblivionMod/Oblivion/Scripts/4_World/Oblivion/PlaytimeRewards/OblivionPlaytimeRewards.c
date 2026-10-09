// Нагорода за час у грі: кожні RewardMinutes живої гри — нагорода «чекає», гравцю приходить сповіщення,
// і він забирає її командою /reward. Предмети випадкові з LootPool, зброя відкидається.
// Один таймер на сервер раз на хвилину. Накопичений час зберігається в playtime.json
// раз на SAVE_EVERY_TICKS хвилин і при вимкненні сервера — рестарт його не скидає.
class OblivionPlaytimeEntry
{
	string Uid;
	int    Seconds;
	int    Pending; // нагород, які ще не забрали
}

class OblivionPlaytimeData
{
	ref array<ref OblivionPlaytimeEntry> Players = new array<ref OblivionPlaytimeEntry>();
}

class OblivionPlaytimeRewards
{
	protected const int TICK_SECONDS     = 60;
	protected const int SAVE_EVERY_TICKS = 5;

	protected static OblivionPlaytimeRewards s_Instance;

	protected ref map<string, int> m_Seconds = new map<string, int>(); // Steam ID -> накопичені секунди
	protected ref map<string, int> m_Pending = new map<string, int>(); // Steam ID -> незабрані нагороди
	protected int  m_TicksSinceSave;
	protected bool m_Dirty;
	protected bool m_Started;

	void Start()
	{
		OblivionPlaytimeRewardsSettings s = OblivionSettings.Get().PlaytimeRewards;
		if (!s.Enabled || s.LootPool.Count() == 0 || s.RewardMinutes <= 0)
			return;

		Load();
		m_Started  = true;
		s_Instance = this;
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
		{
			m_Seconds.Set(entry.Uid, entry.Seconds);
			if (entry.Pending > 0)
				m_Pending.Set(entry.Uid, entry.Pending);
		}
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
			entry.Pending = m_Pending.Get(uid);
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
		string uid = player.OblivionGetUid();
		int pending = m_Pending.Get(uid) + 1;
		m_Pending.Set(uid, pending);

		OblivionNotify.Player(player, "Нагорода готова", "Введи в чат /reward, щоб забрати. Нагород чекає: " + pending + ".", s.NotifySeconds);
	}

	// Нагадування при вході на сервер.
	static void OnConnect(PlayerBase player)
	{
		if (!s_Instance || !player)
			return;

		int pending = s_Instance.m_Pending.Get(player.OblivionGetUid());
		if (pending > 0)
			OblivionNotify.Player(player, "Нагорода чекає", "Нагород за час у грі: " + pending + ". Введи в чат /reward.", OblivionSettings.Get().PlaytimeRewards.NotifySeconds);
	}

	// /reward — видати всі незабрані нагороди.
	static void HandleCommand(PlayerBase player)
	{
		OblivionPlaytimeRewardsSettings s = OblivionSettings.Get().PlaytimeRewards;
		if (!s_Instance)
		{
			OblivionNotify.Player(player, "Нагорода", "Нагороди за час у грі вимкнені.", s.NotifySeconds);
			return;
		}
		s_Instance.Claim(player, s);
	}

	protected void Claim(PlayerBase player, OblivionPlaytimeRewardsSettings s)
	{
		string uid = player.OblivionGetUid();
		int pending = m_Pending.Get(uid);
		if (pending <= 0)
		{
			int left = Math.Ceil((s.RewardMinutes * 60 - m_Seconds.Get(uid)) / 60.0);
			OblivionNotify.Player(player, "Нагорода", "Поки нічого немає. Наступна — через " + left + " хв гри.", s.NotifySeconds);
			return;
		}
		if (!player.IsAlive())
			return;

		array<string> pool = new array<string>();
		foreach (string type : s.LootPool)
		{
			if (!GetGame().IsKindOf(type, "Weapon_Base"))
				pool.Insert(type);
		}
		if (pool.Count() == 0)
			return;

		int count = pending * Math.Max(1, s.ItemsPerReward);
		for (int i = 0; i < count; i++)
			OblivionNotify.SpawnItem(player, pool.GetRandomElement(), player.GetPosition());

		m_Pending.Remove(uid);
		m_Dirty = true;
		Save();

		OblivionNotify.Player(player, "Нагороду отримано", "Предметів: " + count + ". Що не влізло в інвентар — під ногами.", s.NotifySeconds);
	}
}
