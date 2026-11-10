class CfgPatches
{
	class DayZOfOurLives
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
	};
};

class CfgMods
{
	class DayZOfOurLives
	{
		dir = "DayZOfOurLives";
		name = "DayZ of Our Lives";
		type = "mod";
		dependencies[] = {"Mission"};

		class defs
		{
			class missionScriptModule
			{
				value = "";
				files[] = {"DayZOfOurLives/Scripts/5_Mission"};
			};
		};
	};
};
