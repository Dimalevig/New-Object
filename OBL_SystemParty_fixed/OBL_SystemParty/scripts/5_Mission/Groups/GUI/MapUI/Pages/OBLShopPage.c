// "Магазин" (Shop / Warehouse) tab.
// Lists items the player bought on the web-store and routed to the warehouse
// (purchases.is_delivered = 2). Claiming flips it to is_delivered = 0 so the
// existing MyWebStore mod spawns it in-game.
class OBLShopPage : OBLPartyPage {

	TextWidget        txt_title;
	TextWidget        lbl_hint;
	TextListboxWidget list_items;
	ButtonWidget      btn_refresh;
	ButtonWidget      btn_claim;
	TextWidget        txt_status;

	// відбиток поточного вмісту списку — щоб не перебудовувати його щотіку
	// (інакше виділення товару злітає одразу після кліку)
	string m_listSignature = "\x01";

	override bool InitPage(OBLPartyUI parentUI) {
		return super.InitPage(parentUI, 5, 0, "Магазин", false);
	}

	override bool IsAvailable() {
		return OBLPartyMainConfig.Get().enableShop;
	}

	override void InitMainWidget() {
		OBLLogger.Debug("InitMainWidget ShopPage");
		txt_title   = TextWidget.Cast(rootWidget.FindAnyWidget("txt_title"));
		lbl_hint    = TextWidget.Cast(rootWidget.FindAnyWidget("lbl_hint"));
		list_items  = TextListboxWidget.Cast(rootWidget.FindAnyWidget("list_items"));
		btn_refresh = ButtonWidget.Cast(rootWidget.FindAnyWidget("btn_refresh"));
		btn_claim   = ButtonWidget.Cast(rootWidget.FindAnyWidget("btn_claim"));
		txt_status  = TextWidget.Cast(rootWidget.FindAnyWidget("txt_status"));
	}

	override void OnShow() {
		super.OnShow();
		// прибираємо попереднє повідомлення, щоб не висіло старе про помилку
		OBLShopClient.Get().lastMessage = "";
		m_listSignature = "\x01"; // примусово перебудувати список при відкритті
		// склад більше не завантажується автоматично — лише кнопкою «Оновити»
		RefreshList();
	}

	override void OnUpdateSlow() {
		RefreshList();
	}

	void SetStatus(string msg) {
		if (txt_status)
			txt_status.SetText(msg);
	}

	void RefreshList() {
		OBLShopClient client = OBLShopClient.Get();
		client.CheckLoadingTimeout();

		// статус оновлюємо завжди
		if (client.items.Count() > 0) {
			// товари є — прибираємо будь-який статус (інструкція в підзаголовку)
			SetStatus("");
		} else if (client.loading) {
			SetStatus("Завантаження складу...");
		} else if (client.lastMessage != "") {
			SetStatus(client.lastMessage);
		} else if (client.gotResponse) {
			SetStatus("Склад порожній.");
		} else {
			SetStatus("Натисніть «Оновити», щоб завантажити склад.");
		}

		if (!list_items)
			return;

		// будуємо відбиток вмісту; якщо не змінився — НЕ чіпаємо список,
		// щоб зберегти виділення гравця
		string sig = "";
		foreach (OBLShopItem it : client.items) {
			sig = sig + it.purchaseId.ToString() + ":" + it.qty.ToString() + ";";
		}
		if (sig == m_listSignature)
			return;
		m_listSignature = sig;

		list_items.ClearItems();
		foreach (OBLShopItem item : client.items) {
			string label = " " + item.itemName;
			if (item.qty > 1)
				label = label + " x" + item.qty;
			int row = list_items.AddItem(label, new Param1<int>(item.purchaseId), 0);
			list_items.SetItemColor(row, 0, ARGB(255, 180, 220, 255));
		}
	}

	override bool OnClick(Widget w) {
		if (w == btn_refresh) {
			if (OBLShopClient.Get().loading)
				return true;
			SetStatus("Завантаження складу...");
			OBLShopClient.Get().RequestList();
			return true;
		} else if (w == btn_claim) {
			ClaimSelected();
			return true;
		}
		return false;
	}

	void ClaimSelected() {
		if (!list_items)
			return;
		int row = list_items.GetSelectedRow();
		if (row < 0) {
			SetStatus("Виберіть предмет зі списку.");
			return;
		}
		Param1<int> data;
		list_items.GetItemData(row, 0, data);
		if (!data)
			return;
		SetStatus("Видача предмета...");
		OBLShopClient.Get().RequestClaim(data.param1);
	}
}
