// DayZ of Our Lives: every new character spawns as a role - police officer, doctor,
// firefighter, executive... - in the matching vanilla outfit, with a few things that fit
// the job. The roles are data, in $profile:DayZOfOurLives/Roles.json:
//
// {
//   "roles": [
//     {
//       "name": "Police officer",
//       "weight": 1,
//       "clothing": ["PoliceCap", "PoliceJacket|PoliceJacketOrel", ...],
//       "items": [
//         {"type": "Flashlight", "hands": true, "attachments": ["Battery9V"], "cargo": []},
//         ...
//       ]
//     }
//   ]
// }
//
// "A|B|C" anywhere a class name is expected picks one of them at random. The server writes
// the default roles there on its first start; edit the file and restart to change them.

class DOOL_RoleItem
{
	string type;
	bool hands;                  // spawn in the player's hands instead of the inventory
	ref array<string> attachments = new array<string>();
	ref array<string> cargo = new array<string>();
}

class DOOL_Role
{
	string name;
	float weight = 1;
	ref array<string> clothing = new array<string>();
	ref array<ref DOOL_RoleItem> items = new array<ref DOOL_RoleItem>();
}

class DOOL_RoleSet
{
	ref array<ref DOOL_Role> roles = new array<ref DOOL_Role>();
}

class DOOL_Roles
{
	static const string DIR = "$profile:DayZOfOurLives";
	static const string FILE = "$profile:DayZOfOurLives/Roles.json";
	static const string DEFAULTS = "DayZOfOurLives/Roles.json"; // packed in the mod

	protected static ref DOOL_RoleSet s_Roles;

	// Loads the roles, writing the defaults out first if the server has none yet.
	static int Init()
	{
		string error;
		if (!FileExist(FILE))
		{
			DOOL_RoleSet defaults;
			if (!JsonFileLoader<DOOL_RoleSet>.LoadFile(DEFAULTS, defaults, error))
			{
				Print("[DayZOfOurLives] cannot read the default roles: " + error);
				s_Roles = new DOOL_RoleSet();
				return 0;
			}
			MakeDirectory(DIR);
			if (JsonFileLoader<DOOL_RoleSet>.SaveFile(FILE, defaults, error))
				Print("[DayZOfOurLives] wrote the default roles to " + FILE);
			else
				Print("[DayZOfOurLives] cannot write " + FILE + ": " + error);
			s_Roles = defaults;
			return s_Roles.roles.Count();
		}
		DOOL_RoleSet loaded;
		if (!JsonFileLoader<DOOL_RoleSet>.LoadFile(FILE, loaded, error))
		{
			Print("[DayZOfOurLives] " + FILE + ": " + error + " - spawning players the vanilla way");
			s_Roles = new DOOL_RoleSet();
			return 0;
		}
		s_Roles = loaded;
		return s_Roles.roles.Count();
	}

	static array<ref DOOL_Role> All()
	{
		if (!s_Roles)
			s_Roles = new DOOL_RoleSet();
		return s_Roles.roles;
	}

	// weighted random choice
	static DOOL_Role Pick()
	{
		array<ref DOOL_Role> roles = All();
		float total = 0;
		for (int i = 0; i < roles.Count(); i++)
			total += roles[i].weight;
		if (total <= 0)
			return null;

		float r = Math.RandomFloat(0, total);
		for (int j = 0; j < roles.Count(); j++)
		{
			r -= roles[j].weight;
			if (r <= 0)
				return roles[j];
		}
		return roles[roles.Count() - 1];
	}

	// "A|B|C" -> one of them
	static string Choose(string spec)
	{
		array<string> options = new array<string>();
		spec.Split("|", options);
		if (options.Count() == 0)
			return spec;
		string choice = options.GetRandomElement();
		choice.Trim();
		return choice;
	}

	static bool IsKnown(string spec)
	{
		array<string> options = new array<string>();
		spec.Split("|", options);
		for (int i = 0; i < options.Count(); i++)
		{
			string option = options[i];
			option.Trim();
			if (!GetGame().ConfigIsExisting("CfgVehicles " + option))
				return false;
		}
		return true;
	}

	// "You are a Police officer" / "You are an Executive" / "You are an NBC specialist"
	static string Title(DOOL_Role role)
	{
		array<string> words = new array<string>();
		role.name.Split(" ", words);
		string first = words[0];
		string upper = first;
		upper.ToUpper();
		string letter = first.Substring(0, 1);
		letter.ToUpper();
		// a vowel, or an abbreviation read letter by letter whose name starts with a vowel sound
		bool vowel = "AEIOU".IndexOf(letter) >= 0;
		if (first.Length() > 1 && first == upper)
			vowel = "AEFHILMNORSX".IndexOf(letter) >= 0;
		if (vowel)
			return "You are an " + role.name;
		return "You are a " + role.name;
	}

	// Dresses a freshly created, naked player as the role.
	static void Equip(PlayerBase player, DOOL_Role role)
	{
		for (int i = 0; i < role.clothing.Count(); i++)
		{
			string type = Choose(role.clothing[i]);
			if (!player.GetInventory().CreateInInventory(type))
				Print("[DayZOfOurLives] " + role.name + ": could not wear " + type);
		}

		for (int j = 0; j < role.items.Count(); j++)
		{
			DOOL_RoleItem spec = role.items[j];
			string itemType = Choose(spec.type);
			EntityAI item;
			if (spec.hands)
				item = player.GetHumanInventory().CreateInHands(itemType);
			else
				item = player.GetInventory().CreateInInventory(itemType);
			if (!item)
			{
				Print("[DayZOfOurLives] " + role.name + ": no room for " + itemType);
				continue;
			}
			Fill(item, spec);
		}
	}

	static void Fill(EntityAI item, DOOL_RoleItem spec)
	{
		for (int i = 0; i < spec.attachments.Count(); i++)
		{
			string a = Choose(spec.attachments[i]);
			if (!item.GetInventory().CreateAttachment(a))
				Print("[DayZOfOurLives] " + item.GetType() + ": could not attach " + a);
		}
		for (int j = 0; j < spec.cargo.Count(); j++)
		{
			string c = Choose(spec.cargo[j]);
			if (!item.GetInventory().CreateEntityInCargo(c))
				Print("[DayZOfOurLives] " + item.GetType() + ": no room for " + c);
		}
	}

	// Check for server owners editing the roles: every class name exists, and every item
	// takes its attachments and cargo. Items are spawned at pos and deleted again. Returns
	// the number of problems; each one is logged.
	static int Validate(vector pos)
	{
		int problems = 0;
		array<ref DOOL_Role> roles = All();
		for (int r = 0; r < roles.Count(); r++)
		{
			DOOL_Role role = roles[r];
			for (int c = 0; c < role.clothing.Count(); c++)
			{
				if (!IsKnown(role.clothing[c]))
				{
					Print("[DayZOfOurLives] " + role.name + ": unknown clothing " + role.clothing[c]);
					problems++;
				}
			}
			for (int i = 0; i < role.items.Count(); i++)
			{
				DOOL_RoleItem spec = role.items[i];
				if (!IsKnown(spec.type))
				{
					Print("[DayZOfOurLives] " + role.name + ": unknown item " + spec.type);
					problems++;
					continue;
				}
				int k;
				for (k = 0; k < spec.attachments.Count(); k++)
				{
					if (!IsKnown(spec.attachments[k]))
					{
						Print("[DayZOfOurLives] " + role.name + ": unknown attachment " + spec.attachments[k]);
						problems++;
					}
				}
				for (k = 0; k < spec.cargo.Count(); k++)
				{
					if (!IsKnown(spec.cargo[k]))
					{
						Print("[DayZOfOurLives] " + role.name + ": unknown cargo " + spec.cargo[k]);
						problems++;
					}
				}

				// try it for real
				EntityAI item = EntityAI.Cast(GetGame().CreateObjectEx(Choose(spec.type), pos, ECE_PLACE_ON_SURFACE));
				if (!item)
				{
					Print("[DayZOfOurLives] " + role.name + ": could not create " + spec.type);
					problems++;
					continue;
				}
				for (k = 0; k < spec.attachments.Count(); k++)
				{
					if (!item.GetInventory().CreateAttachment(Choose(spec.attachments[k])))
					{
						Print("[DayZOfOurLives] " + role.name + ": " + item.GetType() + " does not take attachment " + spec.attachments[k]);
						problems++;
					}
				}
				for (k = 0; k < spec.cargo.Count(); k++)
				{
					if (!item.GetInventory().CreateEntityInCargo(Choose(spec.cargo[k])))
					{
						Print("[DayZOfOurLives] " + role.name + ": " + item.GetType() + " has no room for " + spec.cargo[k]);
						problems++;
					}
				}
				GetGame().ObjectDelete(item);
			}
		}
		return problems;
	}
}
