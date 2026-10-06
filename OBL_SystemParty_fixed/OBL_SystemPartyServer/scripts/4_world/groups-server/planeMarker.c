#ifdef CDS_PlaneEvent
modded class CDS_PlaneCrash {
    int markerUID = 0;

    override void EEInit() {
        super.EEInit();
        // OBL FIX: EEInit also runs on clients, markers are server-only
        if (GetGame() && GetGame().IsServer())
            CreateMarkerEntity(this);
    }

    void CreateMarkerEntity(EntityAI obj) {
        vector pos   = obj.GetPosition();
        string label = "Авіакатастрофа";
        string icon  = "CDS_PlaneCrash\\PlaneEvent\\Icon\\PlaneCrash.paa";
        int color    = ARGB(255, 255, 255, 255);

        OBLServerMarker serverMarker = OBLStaticMarkerManager.Get().AddTempServerMarker(label, pos, icon, color);
        if (serverMarker) {
            markerUID = serverMarker.uid;
        }
    }

    void ~CDS_PlaneCrash() {
        CGame game = GetGame();
        if (!game)
            return;
        
        if (!game.IsServer())
            return;

        if (markerUID != 0) {
            OBLServerMarker marker = OBLStaticMarkerManager.Get().FindTempMarker(markerUID);
            if (marker) {
                OBLStaticMarkerManager.Get().RemoveServerMarker(marker);
            }
            markerUID = 0;
        }
    }
}
#endif
