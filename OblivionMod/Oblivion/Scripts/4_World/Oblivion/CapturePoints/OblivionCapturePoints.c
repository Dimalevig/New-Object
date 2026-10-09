// Захоплення точок: простояв у зоні CaptureSeconds — отримав ящик з лутом.
// Один таймер на всі точки, раз на CheckSeconds; якщо точок немає — таймер не запускається.
class OblivionCapturePointState
{
	PlayerBase Capturer;
	int        StartTime;
	int        CooldownUntil;
}

class OblivionCapturePoints
{
	protected ref array<ref OblivionCapturePointState> m_States = new array<ref OblivionCapturePointState>();

	void Start()
	{
		OblivionCapturePointsSettings s = OblivionSettings.Get().CapturePoints;
		if (!s.Enabled || s.Points.Count() == 0)
			return;

		for (int i = 0; i < s.Points.Count(); i++)
			m_States.Insert(new OblivionCapturePointState());

		int interval = Math.Max(1, s.CheckSeconds) * 1000;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Tick, interval, true);
	}

	protected static bool InZone(PlayerBase player, OblivionCapturePoint point)
	{
		vector d = player.GetPosition() - point.Position;
		d[1] = 0;
		return d.LengthSq() <= point.Radius * point.Radius;
	}

	void Tick()
	{
		OblivionCapturePointsSettings s = OblivionSettings.Get().CapturePoints;
		int now = GetGame().GetTime();

		array<Man> players;

		for (int i = 0; i < s.Points.Count(); i++)
		{
			OblivionCapturePoint point = s.Points[i];
			OblivionCapturePointState state = m_States[i];

			if (now < state.CooldownUntil)
				continue;

			if (state.Capturer && state.Capturer.IsAlive() && InZone(state.Capturer, point))
			{
				if (now - state.StartTime >= point.CaptureSeconds * 1000)
					Complete(point, state, now, s);
				continue;
			}

			if (state.Capturer)
			{
				state.Capturer = null;
				OblivionNotify.All("Захоплення зірвано", "Точка «" + point.Name + "» знову вільна.", s.NotifySeconds);
			}

			if (!players)
			{
				players = new array<Man>();
				GetGame().GetPlayers(players);
			}

			foreach (Man man : players)
			{
				PlayerBase player = PlayerBase.Cast(man);
				if (!player || !player.IsAlive() || !InZone(player, point))
					continue;

				state.Capturer  = player;
				state.StartTime = now;
				int minutes = Math.Ceil(point.CaptureSeconds / 60);
				OblivionNotify.All("Точку захоплюють", "«" + point.Name + "», квадрат " + OblivionBounty.Grid(point.Position, 100) + ". До захоплення " + minutes + " хв.", s.NotifySeconds);
				break;
			}
		}
	}

	protected void Complete(OblivionCapturePoint point, OblivionCapturePointState state, int now, OblivionCapturePointsSettings s)
	{
		EntityAI box = EntityAI.Cast(GetGame().CreateObjectEx(point.RewardContainer, point.Position, ECE_PLACE_ON_SURFACE));
		vector dropPos = point.Position;
		if (box)
			dropPos = box.GetPosition();

		foreach (string type : point.RewardItems)
			OblivionNotify.SpawnItem(box, type, dropPos);

		OblivionNotify.All("Точку захоплено", state.Capturer.OblivionGetName() + " захопив «" + point.Name + "». Ящик з лутом у центрі точки.", s.NotifySeconds);

		state.Capturer      = null;
		state.CooldownUntil = now + point.CooldownSeconds * 1000;
	}
}
