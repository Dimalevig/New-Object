class OBLLogger {    
    
    private static const int LOG_FATAL = 1;
	private static const int LOG_ERROR = 2;
	private static const int LOG_ADMIN = 3;
	private static const int LOG_INFO = 4;
	private static const int LOG_DEBUG = 5;
	private static const int LOG_VERBOSE = 6;
	private static const int LOG_SPAM = 7;

    private static bool logToScriptlog;
	private static bool disableLogger = true;

	static int Init() {
		Print("[Init] --- OBLLogger[DEBUG] ---");
		SetupLogger();
		return 1;
	}

	private static void SetupLogger() {
		disableLogger = OBLPartyMainConfig.Get().disableLoggerDebug;
		if (GetGame() && GetGame().IsClient()) {
			disableLogger = true;
		}
	}

    static void Debug(string message, bool disable_ = true) {
        if (disableLogger || disable_) {
			return;
		}
        logToScriptlog = !disableLogger;
		Log(message, LOG_DEBUG, disableLogger);
	}

    private static string Log(string message, int logLevel, bool disable) {
		if (disable) {
			return "";
		}

		string logLevelStr;
		if (logLevel == LOG_FATAL) {
			logLevelStr = "FATAL";
		} else if (logLevel == LOG_ERROR) {
			logLevelStr = "ERROR";
		} else if (logLevel == LOG_ADMIN) {
			logLevelStr = "ADMIN";
		} else if (logLevel == LOG_INFO) {
			logLevelStr = "INFO ";
		} else if (logLevel == LOG_DEBUG) {
			logLevelStr = "DEBUG";
		} else if (logLevel == LOG_VERBOSE) {
			logLevelStr = "VERB ";
		} else if (logLevel == LOG_SPAM) {
			logLevelStr = "SPAM ";
		} else {
			logLevelStr = "Unknown";
		}
		string finalMessage = " [" + logLevelStr + "]" + message;
		if (logToScriptlog) {
			Print("" + finalMessage);
		}
		return finalMessage;
	}
}