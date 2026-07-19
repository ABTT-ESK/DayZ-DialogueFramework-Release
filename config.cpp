class CfgPatches
{
	class DialogueFramework
	{
		units[] = {};
		weapons[] = {};
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts",
			"DayZExpansion_Core_Scripts",
			"DayZExpansion_Quests_Scripts",
			"DayZExpansion_Market_Scripts"
		};
	};
};

class CfgMods
{
	class DialogueFramework
	{
		dir = "DialogueFramework";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "Dialogue Framework";
		credits = "ABTT ESK";
		author = "ABTT ESK";
		authorID = "0";
		version = "0.1.0";
		extra = 0;
		type = "mod";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"DialogueFramework/Scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"DialogueFramework/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"DialogueFramework/Scripts/5_Mission"};
			};
		};
	};
};
