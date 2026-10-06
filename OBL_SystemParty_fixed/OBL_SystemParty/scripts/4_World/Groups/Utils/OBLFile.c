class OBLFile<Class T> {

    static void SaveToJson(string subPath, T data, bool onlyServer = false) {
        if (OBLLogger.IsDebug())
        	OBLLogger.Debug("init SaveJson: " + subPath);
        OBLFilePlus.DebugJson();

        if (onlyServer && GetGame().IsClient()) {
            if (OBLLogger.IsDebug())
            	OBLLogger.Debug("Error: SaveJson, not authorized for client: " + subPath);
            return;
        }

        string basePath = OBLPartyConstants.SAVE_PREFIX;
        if (data == null) {
            if (OBLLogger.IsDebug())
            	OBLLogger.Debug("Error: Ao salvar em : " + subPath);
            return;
        }

        JsonFileLoader<T>.JsonSaveFile(basePath + subPath, data);
    }

    static bool LoadFromJson(string subPath, out T data, bool onlyServer = false) {
        if (OBLLogger.IsDebug())
        	OBLLogger.Debug("SaveToJson: " + subPath);
        OBLFilePlus.DebugJson();

        if (onlyServer && GetGame().IsClient()) {
            if (OBLLogger.IsDebug())
            	OBLLogger.Debug("Error: LoadJson, not authorized for client: " + subPath);
            return false;
        }

        string basePath = OBLPartyConstants.SAVE_PREFIX;
        if (!FileExist(basePath + subPath)) {
            if (OBLLogger.IsDebug())
            	OBLLogger.Debug("Arquivo não existe: " + subPath);
            return false;
        }

        JsonFileLoader<T>.JsonLoadFile(basePath + subPath, data);
        return true;
    }
}

class OBLFilePlus {
    private static bool IsClient = false;

    static int Init() {
		IsClient = true;
		return 1;
	}

    static bool IsClient() {
        return IsClient;
    }

    static bool JsonExist(string subPath) {
        string basePath = OBLPartyConstants.SAVE_PREFIX;
        return FileExist(basePath + subPath);
    }

    static void DeleteJson(string subPath) {
        string basePath = OBLPartyConstants.SAVE_PREFIX;
        if (FileExist(basePath + subPath)) {
            DeleteFile(basePath + subPath);
            if (OBLLogger.IsDebug())
            	OBLLogger.Debug("Arquivo deletado: " + subPath);
        } else {
            if (OBLLogger.IsDebug())
            	OBLLogger.Debug("Arquivo não deletado: " + subPath);
        }
    }

    static void DebugJson() {
        if (GetGame().IsServer()) {
            OBLLogger.Debug("IsServer");
        }
        if (GetGame().IsClient()) {
            OBLLogger.Debug("IsClient");
        }
    }
}