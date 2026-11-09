//! One hidden copy of each layout the conversation window builds from, held
//! for the whole session.
//!
//! With the speaker portrait on, the game began answering a request for the
//! window's own layout with the response-button layout -- the one loaded
//! straight after it in every conversation -- and went on doing so until it
//! was restarted. A logging build caught it: CreateWidgets on
//! dialogue_menu_inter.layout handed back a DialogueResponseButton, for a
//! path that had built the window correctly three times that session. The
//! game's record of the path was outliving the layout it pointed to, and the
//! next layout loaded took its place.
//!
//! A copy that is never closed means the game never lets go of the layout, so
//! nothing can take its place. Each copy is made once, hidden, and only kept
//! if it really is the layout it was asked for. Nothing searches the screen
//! for these widget names -- the window only looks inside its own layout --
//! so a hidden copy is never mistaken for the real thing.
class DialogueLayoutPins
{
	protected static ref array<string> s_Paths;
	protected static ref array<Widget> s_Roots;

	//! Hold path for the rest of the session, if it is not held already.
	static void Pin(string path, string expectedRoot)
	{
		if (path == "")
			return;

		if (!s_Paths)
		{
			s_Paths = new array<string>;
			s_Roots = new array<Widget>;
		}

		if (s_Paths.Find(path) != -1)
			return;

		Widget root = GetGame().GetWorkspace().CreateWidgets(path);
		if (!root)
		{
			Print("[DialogueFramework] [UI] [WARN] Could not hold " + path + " -- the game would not build it.");
			return;
		}

		string rootName = root.GetName();
		if (rootName != expectedRoot)
		{
			//! Already the wrong layout -- holding it would hold the damage.
			Print("[DialogueFramework] [UI] [WARN] " + path + " came back as " + rootName + ", not " + expectedRoot + " -- not held.");
			root.Unlink();
			return;
		}

		root.Show(false);
		s_Paths.Insert(path);
		s_Roots.Insert(root);
		Print("[DialogueFramework] [UI] Holding " + path + " for the session.");
	}
}
