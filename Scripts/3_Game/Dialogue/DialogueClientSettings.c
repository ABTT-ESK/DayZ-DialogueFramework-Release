//! Per-player preferences, saved on the player's OWN machine rather than the
//! server, so they follow the player from server to server. Everything here
//! is an override: an empty string or a zero means "use whatever the server
//! configured", which is what a fresh install has for all of them.

class DialogueClientSettings
{
	protected static const string SETTINGS_FOLDER = "$profile:\\DialogFramework\\";
	protected static const string SETTINGS_FILE = "$profile:\\DialogFramework\\ClientSettings.json";

	//! Where the old language-only override lived, before this file existed.
	protected static const string LEGACY_LANGUAGE_FILE = "$profile:\\DialogFramework\\ClientLanguage.txt";

	static const float TEXT_SCALE_SERVER = 0;

	static const int ICONS_SERVER = 0;
	static const int ICONS_ON = 1;
	static const int ICONS_OFF = 2;

	string Language = "";

	string Position = "";

	float TextScale = TEXT_SCALE_SERVER;

	int Icons = ICONS_SERVER;

	protected static ref DialogueClientSettings s_Instance;

	static DialogueClientSettings Get()
	{
		if (!s_Instance)
		{
			s_Instance = new DialogueClientSettings();
			s_Instance.Load();
		}

		return s_Instance;
	}

	void Sanitize()
	{
		Language = DialogueFWLanguages.Normalize(Language);
		if (Language != "" && !DialogueFWLanguages.IsKnown(Language))
			Language = "";

		if (Position != "" && !DialogueMenuPosition.IsKnown(Position))
			Position = "";

		if (TextScale != TEXT_SCALE_SERVER)
		{
			if (TextScale < 0.6)
				TextScale = 0.6;
			if (TextScale > 2.0)
				TextScale = 2.0;
		}

		if (Icons != ICONS_ON && Icons != ICONS_OFF)
			Icons = ICONS_SERVER;
	}

	void Load()
	{
		if (FileExist(SETTINGS_FILE))
		{
			JsonFileLoader<DialogueClientSettings>.JsonLoadFile(SETTINGS_FILE, this);
			Sanitize();
			Print("[DialogueFramework] [SETTINGS] Loaded player settings: language='" + Language + "' position='" + Position + "' textScale=" + TextScale + " icons=" + Icons);
			return;
		}

		//! Carry across whatever the language-only version of this saved.
		if (FileExist(LEGACY_LANGUAGE_FILE))
		{
			FileHandle handle = OpenFile(LEGACY_LANGUAGE_FILE, FileMode.READ);
			if (handle != 0)
			{
				string line = "";
				FGets(handle, line);
				CloseFile(handle);

				Language = DialogueFWLanguages.Normalize(line);
				Sanitize();
				Save();
				Print("[DialogueFramework] [SETTINGS] Moved the old language choice '" + Language + "' into ClientSettings.json.");
			}
		}
	}

	void Save()
	{
		Sanitize();

		if (!FileExist(SETTINGS_FOLDER))
			MakeDirectory(SETTINGS_FOLDER);

		JsonFileLoader<DialogueClientSettings>.JsonSaveFile(SETTINGS_FILE, this);
	}

	void ResetAll()
	{
		Language = "";
		Position = "";
		TextScale = TEXT_SCALE_SERVER;
		Icons = ICONS_SERVER;
		Save();
	}

	bool HasAnyOverride()
	{
		if (Language != "")
			return true;
		if (Position != "")
			return true;
		if (TextScale != TEXT_SCALE_SERVER)
			return true;
		if (Icons != ICONS_SERVER)
			return true;

		return false;
	}
}
