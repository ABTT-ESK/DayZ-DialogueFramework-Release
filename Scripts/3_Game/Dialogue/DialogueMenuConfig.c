class DialogueMenuPosition
{
	static const string TOP_LEFT      = "TOP_LEFT";
	static const string TOP_CENTER    = "TOP_CENTER";
	static const string TOP_RIGHT     = "TOP_RIGHT";
	static const string CENTER_LEFT   = "CENTER_LEFT";
	static const string CENTER        = "CENTER";
	static const string CENTER_RIGHT  = "CENTER_RIGHT";
	static const string BOTTOM_LEFT   = "BOTTOM_LEFT";
	static const string BOTTOM_CENTER = "BOTTOM_CENTER";
	static const string BOTTOM_RIGHT  = "BOTTOM_RIGHT";

	protected static ref array<string> s_All = new array<string>;

	static array<string> All()
	{
		if (s_All.Count() == 0)
		{
			s_All.Insert(TOP_LEFT);
			s_All.Insert(TOP_CENTER);
			s_All.Insert(TOP_RIGHT);
			s_All.Insert(CENTER_LEFT);
			s_All.Insert(CENTER);
			s_All.Insert(CENTER_RIGHT);
			s_All.Insert(BOTTOM_LEFT);
			s_All.Insert(BOTTOM_CENTER);
			s_All.Insert(BOTTOM_RIGHT);
		}

		return s_All;
	}

	static bool IsKnown(string position)
	{
		return All().Find(position) != -1;
	}

	//! "BOTTOM_CENTER" reads badly on a button. This does not go through the
	//! stringtable on purpose -- it has to stay readable even on a client
	//! where the mod's stringtable failed to load.
	static string Label(string position)
	{
		if (position == TOP_LEFT) return "Top left";
		if (position == TOP_CENTER) return "Top centre";
		if (position == TOP_RIGHT) return "Top right";
		if (position == CENTER_LEFT) return "Middle left";
		if (position == CENTER) return "Middle";
		if (position == CENTER_RIGHT) return "Middle right";
		if (position == BOTTOM_LEFT) return "Bottom left";
		if (position == BOTTOM_CENTER) return "Bottom centre";
		if (position == BOTTOM_RIGHT) return "Bottom right";

		return position;
	}
}

//! The typefaces the window can wear. Each one is a set of pre-baked layouts
//! under GUI/layouts -- DayZ has SetFontSize but no SetFont, so a typeface can
//! only come from a .layout file. tools/gen_layout_variants.py writes them and
//! its FONTS table is the other half of this list; change one, change both.
class DialogueMenuFont
{
	static const string DEFAULT = "DEFAULT";
	static const string LIGHT   = "LIGHT";
	static const string BLACK   = "BLACK";
	static const string METRON  = "METRON";
	static const string SERIF   = "SERIF";
	static const string ETELKA  = "ETELKA";
	//! Ours rather than the game's -- each is a .fnt + .edds in GUI/fonts.
	static const string BLACKOPS   = "BLACKOPS";
	static const string INTER      = "INTER";
	static const string GARAMOND   = "GARAMOND";
	static const string NOTOSERIF  = "NOTOSERIF";
	static const string CONDENSED  = "CONDENSED";
	static const string ZILLA      = "ZILLA";
	static const string TYPEWRITER = "TYPEWRITER";

	protected static ref array<string> s_All = new array<string>;

	static array<string> All()
	{
		if (s_All.Count() == 0)
		{
			s_All.Insert(DEFAULT);
			s_All.Insert(LIGHT);
			s_All.Insert(BLACK);
			s_All.Insert(METRON);
			s_All.Insert(SERIF);
			s_All.Insert(ETELKA);
			s_All.Insert(BLACKOPS);
			s_All.Insert(INTER);
			s_All.Insert(GARAMOND);
			s_All.Insert(NOTOSERIF);
			s_All.Insert(CONDENSED);
			s_All.Insert(ZILLA);
			s_All.Insert(TYPEWRITER);
		}

		return s_All;
	}

	static bool IsKnown(string value)
	{
		return All().Find(value) != -1;
	}
}

class DialogueMenuTextSize
{
	static const string NORMAL  = "NORMAL";
	static const string LARGE   = "LARGE";
	static const string COMPACT = "COMPACT";

	protected static ref array<string> s_All = new array<string>;

	static array<string> All()
	{
		if (s_All.Count() == 0)
		{
			s_All.Insert(NORMAL);
			s_All.Insert(LARGE);
			s_All.Insert(COMPACT);
		}

		return s_All;
	}

	static bool IsKnown(string value)
	{
		return All().Find(value) != -1;
	}
}

class DialogueMenuConfig
{
	static const int CURRENT_VERSION = 11;
	int ConfigVersion = 0;

	//! What a scroll speed is allowed to be, wherever it is set -- this file
	//! or a player's own settings.
	static const float SCROLL_SPEED_MIN = 0.25;
	static const float SCROLL_SPEED_MAX = 4.0;

	string Position = DialogueMenuPosition.BOTTOM_CENTER;

	float PanelWidth = 0.6;
	float PanelHeight = 0.52;

	float OffsetX = 0.0;
	float OffsetY = 0.0;

	float EdgeMargin = 0.03;

	ref array<int> BackgroundColor;
	ref array<int> ResponseBackgroundColor;
	ref array<int> HoverBorderColor;
	ref array<int> SpeakerNameColor;
	ref array<int> SpeakerTextColor;
	ref array<int> ResponseTextColor;
	ref array<int> RewardSelectedColor;
	ref array<int> WindowBorderColor;

	int WindowBorderThickness = 2;

	float VisitedResponseOpacity = 0.4;

	//! Kept so a config written before 1.6.0, or hand-edited the old way,
	//! still works. Sanitize folds it into Font and TextSize below.
	string FontStyle = "DEFAULT";

	string Font = "DEFAULT";

	string TextSize = "NORMAL";

	bool ShowResponseIcons = false;

	bool ShowLanguageButton = true;

	bool ScaleTextWithPanel = false;

	bool ShowErrorNotifications = true;

	//! How far the mouse wheel moves a long speech. 1.0 is the built-in pace;
	//! 2.0 covers twice as much ground per notch, 0.5 half. Players can set
	//! their own in the window's settings screen, and theirs wins.
	float ScrollSpeed = 1.0;

	//! A short pop-up naming who a choice pleased or annoyed, and by how
	//! much. Off leaves reputation changing silently, as it did before.
	bool ShowReputationNotifications = true;

	//! What the standing page in Expansion's book calls itself: the name on
	//! its tab, and the heading on the page. Left empty, both fall back to
	//! the mod's own wording in the player's language -- so a server that
	//! sets these is choosing English for everyone, on purpose.
	string BookTabName = "";
	string BookPageTitle = "";

	//! The three column headings on that page. Same rule as the two above:
	//! empty means the mod's own wording in the player's language.
	string BookColumnName = "";
	string BookColumnStatus = "";
	string BookColumnReputation = "";

	string LayoutOverride = "";

	void DialogueMenuConfig()
	{
		ConfigVersion = CURRENT_VERSION;

		BackgroundColor = new array<int>;
		ResponseBackgroundColor = new array<int>;
		HoverBorderColor = new array<int>;
		SpeakerNameColor = new array<int>;
		SpeakerTextColor = new array<int>;
		ResponseTextColor = new array<int>;
		RewardSelectedColor = new array<int>;
		WindowBorderColor = new array<int>;

		ApplyDefaultColors();
	}

	void ApplyDefaultColors()
	{
		SetIfEmpty(BackgroundColor, 230, 0, 0, 0);
		SetIfEmpty(ResponseBackgroundColor, 200, 0, 0, 0);
		SetIfEmpty(HoverBorderColor, 255, 255, 215, 0);
		SetIfEmpty(SpeakerNameColor, 255, 255, 255, 255);
		SetIfEmpty(SpeakerTextColor, 255, 255, 255, 255);
		SetIfEmpty(ResponseTextColor, 255, 220, 220, 220);
		SetIfEmpty(RewardSelectedColor, 230, 90, 70, 20);
		SetIfEmpty(WindowBorderColor, 255, 255, 255, 255);
	}

	protected void SetIfEmpty(array<int> target, int a, int r, int g, int b)
	{
		if (!target || target.Count() >= 4)
			return;

		target.Clear();
		target.Insert(a);
		target.Insert(r);
		target.Insert(g);
		target.Insert(b);
	}

	void Sanitize()
	{
		if (!BackgroundColor)
			BackgroundColor = new array<int>;
		if (!ResponseBackgroundColor)
			ResponseBackgroundColor = new array<int>;
		if (!HoverBorderColor)
			HoverBorderColor = new array<int>;
		if (!SpeakerNameColor)
			SpeakerNameColor = new array<int>;
		if (!SpeakerTextColor)
			SpeakerTextColor = new array<int>;
		if (!ResponseTextColor)
			ResponseTextColor = new array<int>;
		if (!RewardSelectedColor)
			RewardSelectedColor = new array<int>;
		if (!WindowBorderColor)
			WindowBorderColor = new array<int>;

		ApplyDefaultColors();

		if (Position == "")
			Position = DialogueMenuPosition.BOTTOM_CENTER;

		if (PanelWidth <= 0.05 || PanelWidth > 1.0)
			PanelWidth = 0.6;
		if (PanelHeight <= 0.05 || PanelHeight > 1.0)
			PanelHeight = 0.52;
		if (EdgeMargin < 0 || EdgeMargin > 0.4)
			EdgeMargin = 0.03;

		if (VisitedResponseOpacity < 0.0 || VisitedResponseOpacity > 1.0)
			VisitedResponseOpacity = 0.4;

		if (WindowBorderThickness < 0 || WindowBorderThickness > 20)
			WindowBorderThickness = 2;

		if (ScrollSpeed < SCROLL_SPEED_MIN || ScrollSpeed > SCROLL_SPEED_MAX)
			ScrollSpeed = 1.0;

		FontStyle.ToUpper();
		if (FontStyle != "DEFAULT" && FontStyle != "LIGHT" && FontStyle != "LARGE" && FontStyle != "COMPACT")
			FontStyle = "DEFAULT";

		Font.ToUpper();
		if (!DialogueMenuFont.IsKnown(Font))
			Font = DialogueMenuFont.DEFAULT;

		TextSize.ToUpper();
		if (!DialogueMenuTextSize.IsKnown(TextSize))
			TextSize = DialogueMenuTextSize.NORMAL;

		ApplyLegacyFontStyle();
	}

	//! An owner who edits the old FontStyle field on an already-upgraded file
	//! would otherwise see nothing happen: the upgrade only runs once. So
	//! FontStyle still wins whenever the two new fields are untouched, which
	//! is also what makes a guide written for 1.6.0 keep working.
	protected void ApplyLegacyFontStyle()
	{
		if (FontStyle == "DEFAULT")
			return;

		if (Font != DialogueMenuFont.DEFAULT)
			return;

		if (TextSize != DialogueMenuTextSize.NORMAL)
			return;

		if (FontStyle == "LIGHT")
			Font = DialogueMenuFont.LIGHT;
		else if (FontStyle == "LARGE")
			TextSize = DialogueMenuTextSize.LARGE;
		else if (FontStyle == "COMPACT")
			TextSize = DialogueMenuTextSize.COMPACT;
	}

	int GetColor(array<int> source)
	{
		if (!source || source.Count() < 4)
			return ARGB(255, 255, 255, 255);

		return ARGB(source[0], source[1], source[2], source[3]);
	}

	bool UpgradeFromOlderVersion()
	{
		if (ConfigVersion >= CURRENT_VERSION)
			return false;

		if (ConfigVersion < 1)
		{
			WindowBorderThickness = 2;
			VisitedResponseOpacity = 0.4;
		}

		if (ConfigVersion < 2)
		{
			FontStyle = "DEFAULT";
		}

		if (ConfigVersion < 3)
		{
			ShowResponseIcons = false;
		}

		if (ConfigVersion < 4)
		{
			ShowLanguageButton = true;
		}

		if (ConfigVersion < 5)
		{
			ScaleTextWithPanel = false;
		}

		if (ConfigVersion < 6)
		{
			ShowErrorNotifications = true;
		}

		if (ConfigVersion < 7)
		{
			ScrollSpeed = 1.0;
		}

		if (ConfigVersion < 8)
		{
			ShowReputationNotifications = true;
		}

		if (ConfigVersion < 9)
		{
			BookTabName = "";
			BookPageTitle = "";
		}

		if (ConfigVersion < 10)
		{
			BookColumnName = "";
			BookColumnStatus = "";
			BookColumnReputation = "";
		}

		if (ConfigVersion < 11)
		{
			//! FontStyle used to mean a typeface and a size at once. Split it
			//! so both halves can be picked independently from here on.
			Font = DialogueMenuFont.DEFAULT;
			TextSize = DialogueMenuTextSize.NORMAL;
			ApplyLegacyFontStyle();
		}

		ConfigVersion = CURRENT_VERSION;
		return true;
	}

	//! Which set of pre-baked layouts to load. The names match what
	//! tools/gen_layout_variants.py writes: the plain master for Metron Book
	//! at normal size, then "_<font>", "_<size>", or both in that order.
	string GetLayoutSuffix()
	{
		string tail = "";

		if (Font != DialogueMenuFont.DEFAULT)
		{
			string fontPart = Font;
			fontPart.ToLower();
			tail = tail + "_" + fontPart;
		}

		if (TextSize != DialogueMenuTextSize.NORMAL)
		{
			string sizePart = TextSize;
			sizePart.ToLower();
			tail = tail + "_" + sizePart;
		}

		return tail;
	}

	int GetFadedColor(array<int> source, float opacity)
	{
		if (!source || source.Count() < 4)
			return ARGB(255, 255, 255, 255);

		int alpha = (int)(source[0] * opacity);
		if (alpha < 0)
			alpha = 0;
		if (alpha > 255)
			alpha = 255;

		return ARGB(alpha, source[1], source[2], source[3]);
	}

	void GetResolvedPosition(out float x, out float y)
	{
		float centerX = (1.0 - PanelWidth) / 2.0;
		float centerY = (1.0 - PanelHeight) / 2.0;
		float rightX = 1.0 - PanelWidth - EdgeMargin;
		float bottomY = 1.0 - PanelHeight - EdgeMargin;

		x = centerX;
		y = centerY;

		if (Position == DialogueMenuPosition.TOP_LEFT)
		{
			x = EdgeMargin;
			y = EdgeMargin;
		}
		else if (Position == DialogueMenuPosition.TOP_CENTER)
		{
			x = centerX;
			y = EdgeMargin;
		}
		else if (Position == DialogueMenuPosition.TOP_RIGHT)
		{
			x = rightX;
			y = EdgeMargin;
		}
		else if (Position == DialogueMenuPosition.CENTER_LEFT)
		{
			x = EdgeMargin;
			y = centerY;
		}
		else if (Position == DialogueMenuPosition.CENTER_RIGHT)
		{
			x = rightX;
			y = centerY;
		}
		else if (Position == DialogueMenuPosition.BOTTOM_LEFT)
		{
			x = EdgeMargin;
			y = bottomY;
		}
		else if (Position == DialogueMenuPosition.BOTTOM_CENTER)
		{
			x = centerX;
			y = bottomY;
		}
		else if (Position == DialogueMenuPosition.BOTTOM_RIGHT)
		{
			x = rightX;
			y = bottomY;
		}

		x = x + OffsetX;
		y = y + OffsetY;

		//! Keep it on screen whatever the offsets say. A window pushed off the
		//! edge cannot be clicked, and a player who cannot reach its close
		//! button has no way out of the conversation.
		x = ClampToScreen(x, PanelWidth);
		y = ClampToScreen(y, PanelHeight);
	}

	protected float ClampToScreen(float start, float size)
	{
		float most = 1.0 - size;
		if (most < 0)
			most = 0;

		if (start < 0)
			return 0;

		if (start > most)
			return most;

		return start;
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(Position);
		rpc.Write(PanelWidth);
		rpc.Write(PanelHeight);
		rpc.Write(OffsetX);
		rpc.Write(OffsetY);
		rpc.Write(EdgeMargin);
		rpc.Write(LayoutOverride);
		rpc.Write(FontStyle);
		rpc.Write(Font);
		rpc.Write(TextSize);
		rpc.Write(WindowBorderThickness);
		rpc.Write(VisitedResponseOpacity);
		rpc.Write(ShowResponseIcons);
		rpc.Write(ShowLanguageButton);
		rpc.Write(ScaleTextWithPanel);
		rpc.Write(ShowErrorNotifications);
		rpc.Write(ScrollSpeed);
		rpc.Write(ShowReputationNotifications);
		rpc.Write(BookTabName);
		rpc.Write(BookPageTitle);
		rpc.Write(BookColumnName);
		rpc.Write(BookColumnStatus);
		rpc.Write(BookColumnReputation);

		WriteColor(rpc, BackgroundColor);
		WriteColor(rpc, ResponseBackgroundColor);
		WriteColor(rpc, HoverBorderColor);
		WriteColor(rpc, SpeakerNameColor);
		WriteColor(rpc, SpeakerTextColor);
		WriteColor(rpc, ResponseTextColor);
		WriteColor(rpc, RewardSelectedColor);
		WriteColor(rpc, WindowBorderColor);
	}

	protected void WriteColor(ScriptRPC rpc, array<int> source)
	{
		for (int i = 0; i < 4; i++)
			rpc.Write(source[i]);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(Position)) return false;
		if (!ctx.Read(PanelWidth)) return false;
		if (!ctx.Read(PanelHeight)) return false;
		if (!ctx.Read(OffsetX)) return false;
		if (!ctx.Read(OffsetY)) return false;
		if (!ctx.Read(EdgeMargin)) return false;
		if (!ctx.Read(LayoutOverride)) return false;
		if (!ctx.Read(FontStyle)) return false;
		if (!ctx.Read(Font)) return false;
		if (!ctx.Read(TextSize)) return false;
		if (!ctx.Read(WindowBorderThickness)) return false;
		if (!ctx.Read(VisitedResponseOpacity)) return false;
		if (!ctx.Read(ShowResponseIcons)) return false;
		if (!ctx.Read(ShowLanguageButton)) return false;
		if (!ctx.Read(ScaleTextWithPanel)) return false;
		if (!ctx.Read(ShowErrorNotifications)) return false;
		if (!ctx.Read(ScrollSpeed)) return false;
		if (!ctx.Read(ShowReputationNotifications)) return false;
		if (!ctx.Read(BookTabName)) return false;
		if (!ctx.Read(BookPageTitle)) return false;
		if (!ctx.Read(BookColumnName)) return false;
		if (!ctx.Read(BookColumnStatus)) return false;
		if (!ctx.Read(BookColumnReputation)) return false;

		if (!ReadColor(ctx, BackgroundColor)) return false;
		if (!ReadColor(ctx, ResponseBackgroundColor)) return false;
		if (!ReadColor(ctx, HoverBorderColor)) return false;
		if (!ReadColor(ctx, SpeakerNameColor)) return false;
		if (!ReadColor(ctx, SpeakerTextColor)) return false;
		if (!ReadColor(ctx, ResponseTextColor)) return false;
		if (!ReadColor(ctx, RewardSelectedColor)) return false;
		if (!ReadColor(ctx, WindowBorderColor)) return false;

		return true;
	}

	protected bool ReadColor(ParamsReadContext ctx, array<int> target)
	{
		if (!target)
			return false;

		target.Clear();
		for (int i = 0; i < 4; i++)
		{
			int component;
			if (!ctx.Read(component))
				return false;
			target.Insert(component);
		}
		return true;
	}
}
