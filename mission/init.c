// DayZ of Our Lives test mission: vanilla Chernarus with the mod loaded server-side. Players
// spawn on the Chernogorsk beach as one of the roles.
//
// Started with -missiontest (tools/test.sh dayzofourlives) it runs headless checks instead:
// the roles loaded, every class name and attachment is real, the weighted pick spreads over
// all of them, a test character can be dressed as each role; it logs [DOOLTest] lines and
// shuts the server down.
void main()
{
	Hive ce = CreateHive();
	if (ce)
		ce.InitOffline();
	GetGame().GetWorld().SetDate(2026, 9, 20, 10, 0);
}

class CustomMission: MissionServer
{
	static const vector TEST_SPOT = "6537 0 2440";

	override void OnInit()
	{
		super.OnInit();
		string value;
		if (GetGame().CommandlineGetParam("missiontest", value))
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Test, 5000, false);
	}

	override PlayerBase CreateCharacter(PlayerIdentity identity, vector pos, ParamsReadContext ctx, string characterName)
	{
		Entity playerEnt = GetGame().CreatePlayer(identity, characterName, TEST_SPOT, 0, "NONE");
		Class.CastTo(m_player, playerEnt);
		GetGame().SelectPlayer(identity, m_player);
		return m_player;
	}

	void Test()
	{
		array<ref DOOL_Role> roles = DOOL_Roles.All();
		Print("[DOOLTest] " + roles.Count() + " roles; file " + DOOL_Roles.FILE + " exists: " + FileExist(DOOL_Roles.FILE));

		int problems = DOOL_Roles.Validate(TEST_SPOT);
		Print("[DOOLTest] validated: " + problems + " problems");

		map<string, int> picks = new map<string, int>();
		for (int n = 0; n < 2000; n++)
		{
			DOOL_Role picked = DOOL_Roles.Pick();
			int seen = 0;
			picks.Find(picked.name, seen);
			picks.Set(picked.name, seen + 1);
		}
		Print("[DOOLTest] 2000 picks covered " + picks.Count() + " of " + roles.Count() + " roles");

		// dress a naked AI survivor as every role and count what it ends up wearing
		int dressed = 0;
		for (int r = 0; r < roles.Count(); r++)
		{
			vector at = TEST_SPOT + Vector(r * 3, 0, 20);
			at[1] = GetGame().SurfaceY(at[0], at[2]);
			PlayerBase body = PlayerBase.Cast(GetGame().CreateObject("SurvivorM_Mirek", at, false, true));
			if (!body)
			{
				Print("[DOOLTest] could not create a test survivor");
				break;
			}
			DOOL_Roles.Equip(body, roles[r]);
			int worn = body.GetInventory().AttachmentCount();
			array<EntityAI> all = new array<EntityAI>();
			body.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, all);
			EntityAI inHands = body.GetHumanInventory().GetEntityInHands();
			Print("[DOOLTest] " + DOOL_Roles.Title(roles[r]) + ": " + worn + " worn of " + roles[r].clothing.Count() + ", " + (all.Count() - 1) + " things in all" + ", in hands " + inHands);
			if (worn >= roles[r].clothing.Count())
				dressed++;
			GetGame().ObjectDelete(body);
		}
		Print("[DOOLTest] " + dressed + " of " + roles.Count() + " roles fully dressed");
		GetGame().RequestExit(0);
	}
};

Mission CreateCustomMission(string path)
{
	return new CustomMission();
}
