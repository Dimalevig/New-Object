// Логер системи груп.
//  Error / Warn / Info — пишуться завжди (коротко, з префіксом [OBL]).
//  Debug — лише коли на сервері в MainConfig.json стоїть "enableDebugLog": 1.
//  Перед дорогими повідомленнями перевіряйте OBLLogger.IsDebug(), щоб не збирати рядок даремно.
class OBLLogger {

	private static bool debugEnabled = false;
	// захист від засмічення логу: не більше WARN_LIMIT попереджень за WARN_WINDOW_MS
	private static const int WARN_LIMIT = 60;
	private static const int WARN_WINDOW_MS = 60000;
	private static int warnWindowStart = 0;
	private static int warnCount = 0;
	private static int warnSuppressed = 0;

	static int Init() {
		SetupLogger();
		Print("[OBL] Логер запущено. Налагодження: " + debugEnabled);
		return 1;
	}

	private static void SetupLogger() {
		debugEnabled = false;
		if (GetGame() && GetGame().IsServer())
			debugEnabled = OBLPartyMainConfig.Get().enableDebugLog;
	}

	static bool IsDebug() {
		return debugEnabled;
	}

	// другий параметр лишився для сумісності зі старими викликами — ігнорується
	static void Debug(string message, bool unused = false) {
		if (!debugEnabled)
			return;
		Print("[OBL][DEBUG] " + message);
	}

	static void Info(string message) {
		Print("[OBL][INFO] " + message);
	}

	static void Warn(string message) {
		int now = 0;
		if (GetGame())
			now = GetGame().GetTime();
		if (now - warnWindowStart > WARN_WINDOW_MS) {
			if (warnSuppressed > 0)
				Print("[OBL][WARN] ... ще " + warnSuppressed + " попереджень пропущено (захист від спаму)");
			warnWindowStart = now;
			warnCount = 0;
			warnSuppressed = 0;
		}
		if (warnCount >= WARN_LIMIT) {
			warnSuppressed++;
			return;
		}
		warnCount++;
		Print("[OBL][WARN] " + message);
	}

	static void Error(string message) {
		Print("[OBL][ERROR] " + message);
	}
}
