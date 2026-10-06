#ifdef BS_HackedCrate
modded class HackedCrate_Base {

    void HackedCrate_Base() {
        markerUID = 0;
    }

    int markerUID = 0;

    override void DeferredEEInitLogic() {
        super.DeferredEEInitLogic();
        CrateSettings currentSettings = GetCrateSettings();
        if (currentSettings.UseAdvancedGroupsMapMarker) {
            HCCreateMapMarker();
        }
    }

    void HCCreateMapMarker() {
        CrateSettings currentSettings = GetCrateSettings(); 
        if (!currentSettings) {
            return;
        }

        if (!currentSettings.UseAdvancedGroupsMapMarker) {
            return;
        }

        if (!currentSettings.MarkerEnable2D && !currentSettings.MarkerEnable3D) {
            return;
        }

        if (markerUID != 0) {
            return;
        }

        string markerName = GetDisplayName();
        vector markerPos = GetPosition();
        string iconPath = "BS_HackedCrate/gui/iconcrate.paa";
        int color = BSHC_ParseColorFromString(currentSettings.AdvancedGroupsMarkerColor); 

        if (OBLStaticMarkerManager) {
            OBLServerMarker serverMarker = OBLStaticMarkerManager.Get().AddTempServerMarker(markerName, markerPos, iconPath, color);
            if (serverMarker) {
                markerUID = serverMarker.uid;
            } 
        }
    } 
        

    override void BSHC_UpdateAdvancedGroupsMarkerText(string newText) {
        super.BSHC_UpdateAdvancedGroupsMarkerText(newText);

        if (markerUID != 0) {
            CrateSettings currentSettings = GetCrateSettings();
            if (!currentSettings || !currentSettings.UseAdvancedGroupsMapMarker) return;

            HCRemoveMapMarker();

            string iconPath = "BS_HackedCrate/gui/iconcrate.paa";
            int color = BSHC_ParseColorFromString(currentSettings.AdvancedGroupsMarkerColor);

            if (OBLStaticMarkerManager) {
                OBLServerMarker newServerMarker = OBLStaticMarkerManager.Get().AddTempServerMarker(newText, GetPosition(), iconPath, color);
                if (newServerMarker) {
                    markerUID = newServerMarker.uid;
                }
            }
        }
    }

    override void EEDelete(EntityAI parent) {
        super.EEDelete(parent);

        HCRemoveMapMarker();
    }

    void HCRemoveMapMarker() {
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