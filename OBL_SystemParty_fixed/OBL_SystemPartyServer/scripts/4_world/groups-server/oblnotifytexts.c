// OBL FIX: single source of truth for notification texts.
// The member-management flow exists twice on purpose:
//   groupmanager.c -> GROUP_ADMIN_* RPCs (admin panel)
//   oblparty.c     -> KICK / PROMOTE / DEMOTE RPCs (group page)
// Both are live, so neither can be deleted. Instead every duplicated
// string now comes from here, so a text is edited in ONE place.
class OBLNotifyTexts {

	static string PlayerNotFound() {
		return "Гравця не знайдено!";
	}

	static string GroupFull() {
		return "Група заповнена!";
	}

	static string AlreadyInGroup() {
		return "Гравець вже перебуває у групі!";
	}

	static string RankLowerNotFound() {
		return "Нижчий ранг не знайдено!";
	}

	static string RankHigherNotFound() {
		return "Вищий ранг не знайдено!";
	}

	static string RankLeaderNotFound() {
		return "Ранг лідера не знайдено!";
	}

	// ---- kick ----
	static string KickedSender(string memberName) {
		return "Гравця " + memberName + " вигнано з групи!";
	}

	static string KickedTarget(string groupName) {
		return "Вас вигнано з групи " + groupName + "!";
	}

	// ---- demote ----
	static string DemotedSender(string memberName, string rankName) {
		return "Гравця " + memberName + " понижено до рангу " + rankName + " у групі!";
	}

	static string DemotedTarget(string rankName) {
		return "Вас понижено до рангу " + rankName + " у групі!";
	}

	// ---- promote ----
	static string PromotedSender(string memberName, string rankName) {
		return "Гравця " + memberName + " підвищено до рангу " + rankName + " у групі!";
	}

	static string PromotedTarget(string rankName) {
		return "Вас підвищено до рангу " + rankName + " у групі!";
	}

	// OBL FIX: true when the actor is acting on themselves. Used to skip the
	// second banner so one player never gets two popups for one action.
	static bool IsSelfAction(PlayerIdentity sender, string targetSteamid) {
		if (!sender)
			return false;
		return sender.GetPlainId() == targetSteamid;
	}
}
