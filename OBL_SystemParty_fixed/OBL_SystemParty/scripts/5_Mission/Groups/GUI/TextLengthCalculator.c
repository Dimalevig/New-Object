class OBLTextLengthCalculator {

	private TextWidget widget;
	
	static ref OBLTextLengthCalculator g_OBLTextLengthCalculator;
	
	static OBLTextLengthCalculator Get() {
		if (!g_OBLTextLengthCalculator)
			g_OBLTextLengthCalculator = new OBLTextLengthCalculator();
		return g_OBLTextLengthCalculator;
	}
	
	static void Delete() {
		if (g_OBLTextLengthCalculator)
			delete g_OBLTextLengthCalculator;
	}
	
	void OBLTextLengthCalculator() {
		widget = TextWidget.Cast(GetGame().GetWorkspace().CreateWidgets("OBL_SystemParty/gui/layouts/textLengthTester.layout", null));
	}
	
	void ~OBLTextLengthCalculator() {
		if (widget) {
			widget.Unlink();
		}
	}
	
	float GetTextLength(string text, int size) {
		widget.SetTextExactSize(size);
		widget.SetText(text);
		widget.Update();
		float width, height;
		widget.GetScreenSize(width, height);
		int screenWidth, screenHeight;
		GetScreenSize(screenWidth, screenHeight);
		width /= screenWidth;
		//OBLLogger.Debug("Text Length of " + text + " and Size " + size + " is: " + width);
		return width;
	}
}