#ifdef EXPANSIONMODBOOK
//! The mod's own mark, added to Expansion's icon set so the standing page's
//! tab can ask for it by name the way the book asks for every other one.
//!
//! This lives in 3_Game because that is where Expansion declares
//! ExpansionIcons, and a modded class has to be in the same script module as
//! the class it extends. Calling ExpansionIcons from 5_Mission is fine;
//! extending it there is not, and fails the whole Mission module with
//! "Unknown type".
class DialogueFW_BookIcon
{
	static const string NAME = "DialogueFW Standing";
	static const string PATH = "DialogueFramework\\GUI\\images\\icon_book_standing_ca.paa";
}

modded class ExpansionIcons
{
	override void Generate()
	{
		super.Generate();
		AddIcon(DialogueFW_BookIcon.NAME, DialogueFW_BookIcon.PATH);
	}
};
#endif
