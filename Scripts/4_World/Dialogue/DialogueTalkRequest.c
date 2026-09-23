#ifdef EXPANSIONMODQUESTS
//! Which character the player asked to talk to, and when they asked.
//!
//! Expansion's "Talk to ..." action runs on the server, and the server then
//! tells this client to open its quest window -- which the mod turns into a
//! conversation (see DialogueMenu.c). Nothing in that round trip says whether
//! the player still wants one. Press the key twice before the first window
//! appears and the second answer comes back exactly like the first, long after
//! the player has closed the conversation and walked away: a window opens by
//! itself in front of them.
//!
//! That is the half of the 1.5.0 report 1.6.0 did not reach. 1.6.0 stopped old
//! quest windows answering; this stops new ones arriving too late.
//!
//! So the machine the player is sitting at writes down each request as it is
//! made, and the conversation it opens spends the note. A second answer for
//! the same press finds nothing waiting and is ignored.
class DialogueTalkRequest
{
	//! Long enough for any round trip a server can produce, short enough that
	//! a note cannot sit here from a conversation minutes ago.
	static const float LIFETIME_SECONDS = 10.0;

	protected static int s_NPCID = -1;
	protected static float s_Asked;

	static void Note(int npcID)
	{
		//! Every key but Escape is taken while a conversation is on screen, so
		//! a request running now was made before that window opened -- and its
		//! answer will land after the player has closed it. The same holds for
		//! any other menu: someone reading the pause screen is not standing in
		//! front of a character asking to talk.
		UIManager uiManager = GetGame().GetUIManager();
		if (uiManager && uiManager.GetMenu())
		{
			Print("[DialogueFramework] [DIAG] A talk request for NPC ID=" + npcID + " ran with a menu already on screen -- not noted, so its answer cannot open a window later.");
			return;
		}

		s_NPCID = npcID;
		s_Asked = GetGame().GetTickTime();

		Print("[DialogueFramework] [DIAG] The player asked to talk to NPC ID=" + npcID + ".");
	}

	//! True once, and only for the conversation the player actually asked for.
	static bool Take(int npcID)
	{
		if (s_NPCID != npcID)
			return false;

		float age = GetGame().GetTickTime() - s_Asked;
		if (age > LIFETIME_SECONDS)
			return false;

		Forget();
		return true;
	}

	static void Forget()
	{
		s_NPCID = -1;
		s_Asked = 0;
	}
}

modded class ExpansionActionOpenQuestMenu
{
	//! OnExecute is the moment the action goes through, on both sides of the
	//! wire: the server's half of it is where Expansion sends the request to
	//! open the quest window (OnExecuteServer, a few lines further down its own
	//! class), so this note is always written before that answer can arrive.
	//!
	//! OnExecuteClient would do on a normal client -- it is what this mod's own
	//! P2P trader action uses -- but it never runs where the player and the
	//! server are the same machine, which is how some owners test. OnExecute
	//! covers both; a dedicated server has no window to open, so it stops here.
	override void OnExecute(ActionData action_data)
	{
		super.OnExecute(action_data);

		if (GetGame().IsDedicatedServer())
			return;

		if (!action_data || !action_data.m_Target)
			return;

		Object targetObject = action_data.m_Target.GetParentOrObject();
		if (!targetObject)
			return;

		int npcID = -1;

		//! The same three kinds Expansion itself reads the ID from when it
		//! sends the request (ExpansionQuestModule.RequestOpenQuestMenu).
		ExpansionQuestNPCBase npc = ExpansionQuestNPCBase.Cast(targetObject);
		if (npc)
			npcID = npc.GetQuestNPCID();

		ExpansionQuestStaticObject npcObject = ExpansionQuestStaticObject.Cast(targetObject);
		if (npcObject)
			npcID = npcObject.GetQuestNPCID();

	#ifdef EXPANSIONMODAI
		ExpansionQuestNPCAIBase npcAI = ExpansionQuestNPCAIBase.Cast(targetObject);
		if (npcAI)
			npcID = npcAI.GetQuestNPCID();
	#endif

		if (npcID == -1)
			return;

		DialogueTalkRequest.Note(npcID);
	}
}
#endif
