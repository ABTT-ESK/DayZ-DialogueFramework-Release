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
}

class DialogueMenuConfig
{
	static const int CURRENT_VERSION = 3;
	int ConfigVersion = 0;

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

	string FontStyle = "DEFAULT";

	//! Small hint icon on the right of each response button: an exit sign for
	//! anything that closes the menu, a cart for opening the market, a speech
	//! bubble for anything that keeps the conversation going. Off by default
	//! so existing servers look exactly as they did.
	bool ShowResponseIcons = false;

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

		FontStyle.ToUpper();
		if (FontStyle != "DEFAULT" && FontStyle != "LIGHT" && FontStyle != "LARGE" && FontStyle != "COMPACT")
			FontStyle = "DEFAULT";
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

		ConfigVersion = CURRENT_VERSION;
		return true;
	}

	//! Suffix appended to every built-in layout filename for this style.
	string GetLayoutSuffix()
	{
		if (FontStyle == "LIGHT")
			return "_light";
		if (FontStyle == "LARGE")
			return "_large";
		if (FontStyle == "COMPACT")
			return "_compact";

		return "";
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
		rpc.Write(WindowBorderThickness);
		rpc.Write(VisitedResponseOpacity);
		rpc.Write(ShowResponseIcons);

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
		if (!ctx.Read(WindowBorderThickness)) return false;
		if (!ctx.Read(VisitedResponseOpacity)) return false;
		if (!ctx.Read(ShowResponseIcons)) return false;

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
