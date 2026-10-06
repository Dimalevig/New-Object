class OBLPartyCreatePage : OBLPartyPage {
	
	EditBoxWidget inputGroupName, inputGroupTag;
	ButtonWidget buttonCreateGroup;

	override bool InitPage(OBLPartyUI parentUI) {
		return super.InitPage(parentUI, 1, 0, "Група", false);
	}
	
	override bool IsAvailable() {
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		return pb && pb.GetOBLParty() == null;
	}
	
	override void StoreAllWidgetData(OBLDataSerializer data) {
		data.Write(new Param2<string, string>(inputGroupName.GetText(), inputGroupTag.GetText()));
	}
	
	override void RestoreAllWidgetData(OBLDataSerializer data) {
		Param2<string, string> inputParam = Param2<string, string>.Cast(data.Read());
		inputGroupName.SetText(inputParam.param1);
		inputGroupTag.SetText(inputParam.param2);
	}
	
	override bool OnClick(Widget w) {
		if (w == buttonCreateGroup) {
			string groupname = inputGroupName.GetText();
			string grouptag = inputGroupTag.GetText();
			SendCreateGroupRPC(groupname, grouptag);
			return true;
		}
		return false;
	}
	
	override bool OnTopButtonClicked(Widget w) {
		return super.OnTopButtonClicked(w);
	}
	
	override void InitMainWidget() {
		buttonCreateGroup = ButtonWidget.Cast(rootWidget.FindAnyWidget("btn_create"));
		inputGroupName = EditBoxWidget.Cast(rootWidget.FindAnyWidget("input_groupname"));
		inputGroupTag = EditBoxWidget.Cast(rootWidget.FindAnyWidget("input_groupnametag"));
		
		TextWidget groupCostWidget = TextWidget.Cast(rootWidget.FindAnyWidget("groupCreationInfo"));
		groupCostWidget.SetText("Введіть унікальну назву групи та короткий тег.");
	}
	
	void SendCreateGroupRPC(string name, string tag) {
		GetGame().RPCSingleParam(null, OBLPartyRPCs.GROUP_CREATE, new Param2<string, string>(name, tag), true);
	}
	
}
