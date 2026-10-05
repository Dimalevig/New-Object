// Custom Palisade - logging helper (3_Game).
// Writes to the script log (script_*.log) via the vanilla Print().

class CP_Log
{
	static void Info(string message)
	{
		Print(CP_LOG_PREFIX + "INFO: " + message);
	}

	static void Warning(string message)
	{
		Print(CP_LOG_PREFIX + "WARNING: " + message);
	}

	static void Error(string message)
	{
		Print(CP_LOG_PREFIX + "ERROR: " + message);
	}
}
