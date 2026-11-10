// Hooks the roles into the vanilla character creation. Vanilla calls EquipCharacter for a
// new character unless the server uses spawn gear presets (spawnGearPresetFiles in
// cfggameplay.json), in which case the presets win and this mod does nothing. A mission
// that overrides EquipCharacter itself also bypasses it.
modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		int count = DOOL_Roles.Init();
		Print("[DayZOfOurLives] " + count + " roles loaded");
	}

	override void EquipCharacter(MenuDefaultCharacterData char_data)
	{
		DOOL_Role role = DOOL_Roles.Pick();
		if (!role)
		{
			super.EquipCharacter(char_data);
			return;
		}
		DOOL_Roles.Equip(m_player, role);
		NotificationSystem.SendNotificationToPlayerExtended(m_player, 10, DOOL_Roles.Title(role), "Check your pockets.");
		// the mission's own starting kit (vanilla: a bandage, a chemlight and some fruit)
		StartingEquipSetup(m_player, true);
	}
}
