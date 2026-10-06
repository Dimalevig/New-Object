// Слюсарна пилка + двері / кришка багажника / капот машини -> металеві пластини.
// Скільки пластин — залежить від деталі (settings.json -> MetalPlates).
class CraftOblivionMetalPlates extends RecipeBase
{
	override void Init()
	{
		m_Name = "#STR_OBLIVION_CRAFT_METAL_PLATES";
		m_IsInstaRecipe = false;
		m_AnimationLength = 2;
		m_Specialty = 0.02;

		// Пилка: будь-який стан, крім зіпсованої.
		m_MinDamageIngredient[0]   = -1;
		m_MaxDamageIngredient[0]   = 3;
		m_MinQuantityIngredient[0] = -1;
		m_MaxQuantityIngredient[0] = -1;

		// Деталь машини: будь-який стан, навіть зіпсована.
		m_MinDamageIngredient[1]   = -1;
		m_MaxDamageIngredient[1]   = -1;
		m_MinQuantityIngredient[1] = -1;
		m_MaxQuantityIngredient[1] = -1;

		InsertIngredient(0, "Hacksaw");
		m_IngredientAddHealth[0]      = 0; // знос пилки рахується в Do() з налаштувань
		m_IngredientSetHealth[0]      = -1;
		m_IngredientAddQuantity[0]    = 0;
		m_IngredientDestroy[0]        = false;
		m_IngredientUseSoftSkills[0]  = false;

		// CarDoor — базовий конфіг-клас для дверей, капотів і кришок багажника всіх машин.
		InsertIngredient(1, "CarDoor");
		m_IngredientAddHealth[1]      = 0;
		m_IngredientSetHealth[1]      = -1;
		m_IngredientAddQuantity[1]    = 0;
		m_IngredientDestroy[1]        = true;
		m_IngredientUseSoftSkills[1]  = false;
	}

	static int GetPlateCount(ItemBase part)
	{
		OblivionMetalPlatesSettings s = OblivionSettings.Get().MetalPlates;
		if (!s.Enabled || !part)
			return 0;

		string type = part.GetType();
		type.ToLower();

		if (type.Contains("hood"))
			return s.PlatesFromHood;
		if (type.Contains("trunk"))
			return s.PlatesFromTrunk;
		return s.PlatesFromDoor;
	}

	override bool CanDo(ItemBase ingredients[], PlayerBase player)
	{
		return GetPlateCount(ingredients[1]) > 0;
	}

	override void Do(ItemBase ingredients[], PlayerBase player, array<ItemBase> results, float specialty_weight)
	{
		int left = GetPlateCount(ingredients[1]);

		while (left > 0)
		{
			EntityAI spawned = player.GetInventory().CreateInInventory("MetalPlate");
			if (!spawned)
				spawned = EntityAI.Cast(GetGame().CreateObjectEx("MetalPlate", player.GetPosition(), ECE_PLACE_ON_SURFACE));

			ItemBase plate = ItemBase.Cast(spawned);
			if (!plate)
				break;

			int placed = 1;
			if (plate.HasQuantity() && plate.GetQuantityMax() > 1)
			{
				placed = Math.Min(left, plate.GetQuantityMax());
				plate.SetQuantity(placed);
			}
			left -= placed;
		}

		ItemBase hacksaw = ingredients[0];
		if (hacksaw)
			hacksaw.AddHealth("", "", -OblivionSettings.Get().MetalPlates.HacksawDamage);
	}
}
