#ifdef EXPANSIONMODAI
class DialogueFW_FactionSlotBase : eAIFaction
{
	int DFW_Slot()
	{
		return -1;
	}

	override string GetName()
	{
		DialogueFW_FactionDef def = DialogueFW_FactionRegistry.GetDef(DFW_Slot());
		if (def)
			return def.Name;
		return super.GetName();
	}

	override string GetDisplayName()
	{
		return GetName();
	}

	override string GetDefaultLoadout()
	{
		DialogueFW_FactionDef def = DialogueFW_FactionRegistry.GetDef(DFW_Slot());
		if (def && def.Loadout != "")
			return def.Loadout;
		return super.GetDefaultLoadout();
	}

	override bool IsGuard()
	{
		DialogueFW_FactionDef def = DialogueFW_FactionRegistry.GetDef(DFW_Slot());
		if (def)
			return def.PlayerStance == "GUARD";
		return false;
	}

	override bool IsObserver()
	{
		return DialogueFW_FactionRegistry.GetDef(DFW_Slot()) == null;
	}

	override bool IsFriendly(notnull eAIFaction other)
	{
		DialogueFW_FactionDef def = DialogueFW_FactionRegistry.GetDef(DFW_Slot());
		if (!def)
			return false;
		if (other.GetName() == def.Name)
			return true;
		return DialogueFW_FactionRegistry.IsFriendlyName(def, other.GetName());
	}

	override bool IsFriendlyEntity(EntityAI other, DayZPlayer factionMember = null)
	{
		DialogueFW_FactionDef def = DialogueFW_FactionRegistry.GetDef(DFW_Slot());
		if (!def)
			return false;
		if (def.PlayerStance != "FRIENDLY")
			return false;

		eAIBase ai;
		if (Class.CastTo(ai, other))
			return false;
		PlayerBase player;
		if (Class.CastTo(player, other))
			return true;
		return false;
	}
}

[eAIRegisterFaction(eAIFactionDialogueFW0)]
class eAIFactionDialogueFW0 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 0; } }
[eAIRegisterFaction(eAIFactionDialogueFW1)]
class eAIFactionDialogueFW1 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 1; } }
[eAIRegisterFaction(eAIFactionDialogueFW2)]
class eAIFactionDialogueFW2 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 2; } }
[eAIRegisterFaction(eAIFactionDialogueFW3)]
class eAIFactionDialogueFW3 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 3; } }
[eAIRegisterFaction(eAIFactionDialogueFW4)]
class eAIFactionDialogueFW4 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 4; } }
[eAIRegisterFaction(eAIFactionDialogueFW5)]
class eAIFactionDialogueFW5 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 5; } }
[eAIRegisterFaction(eAIFactionDialogueFW6)]
class eAIFactionDialogueFW6 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 6; } }
[eAIRegisterFaction(eAIFactionDialogueFW7)]
class eAIFactionDialogueFW7 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 7; } }
[eAIRegisterFaction(eAIFactionDialogueFW8)]
class eAIFactionDialogueFW8 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 8; } }
[eAIRegisterFaction(eAIFactionDialogueFW9)]
class eAIFactionDialogueFW9 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 9; } }
[eAIRegisterFaction(eAIFactionDialogueFW10)]
class eAIFactionDialogueFW10 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 10; } }
[eAIRegisterFaction(eAIFactionDialogueFW11)]
class eAIFactionDialogueFW11 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 11; } }
[eAIRegisterFaction(eAIFactionDialogueFW12)]
class eAIFactionDialogueFW12 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 12; } }
[eAIRegisterFaction(eAIFactionDialogueFW13)]
class eAIFactionDialogueFW13 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 13; } }
[eAIRegisterFaction(eAIFactionDialogueFW14)]
class eAIFactionDialogueFW14 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 14; } }
[eAIRegisterFaction(eAIFactionDialogueFW15)]
class eAIFactionDialogueFW15 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 15; } }
[eAIRegisterFaction(eAIFactionDialogueFW16)]
class eAIFactionDialogueFW16 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 16; } }
[eAIRegisterFaction(eAIFactionDialogueFW17)]
class eAIFactionDialogueFW17 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 17; } }
[eAIRegisterFaction(eAIFactionDialogueFW18)]
class eAIFactionDialogueFW18 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 18; } }
[eAIRegisterFaction(eAIFactionDialogueFW19)]
class eAIFactionDialogueFW19 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 19; } }
[eAIRegisterFaction(eAIFactionDialogueFW20)]
class eAIFactionDialogueFW20 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 20; } }
[eAIRegisterFaction(eAIFactionDialogueFW21)]
class eAIFactionDialogueFW21 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 21; } }
[eAIRegisterFaction(eAIFactionDialogueFW22)]
class eAIFactionDialogueFW22 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 22; } }
[eAIRegisterFaction(eAIFactionDialogueFW23)]
class eAIFactionDialogueFW23 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 23; } }
[eAIRegisterFaction(eAIFactionDialogueFW24)]
class eAIFactionDialogueFW24 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 24; } }
[eAIRegisterFaction(eAIFactionDialogueFW25)]
class eAIFactionDialogueFW25 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 25; } }
[eAIRegisterFaction(eAIFactionDialogueFW26)]
class eAIFactionDialogueFW26 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 26; } }
[eAIRegisterFaction(eAIFactionDialogueFW27)]
class eAIFactionDialogueFW27 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 27; } }
[eAIRegisterFaction(eAIFactionDialogueFW28)]
class eAIFactionDialogueFW28 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 28; } }
[eAIRegisterFaction(eAIFactionDialogueFW29)]
class eAIFactionDialogueFW29 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 29; } }
[eAIRegisterFaction(eAIFactionDialogueFW30)]
class eAIFactionDialogueFW30 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 30; } }
[eAIRegisterFaction(eAIFactionDialogueFW31)]
class eAIFactionDialogueFW31 : DialogueFW_FactionSlotBase { override int DFW_Slot() { return 31; } }
#endif
