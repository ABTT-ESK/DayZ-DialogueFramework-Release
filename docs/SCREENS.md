# The screens, and what they're called

Dialogue Framework shows a player one of eleven screens. This page is the
list — what each one is called, how a player gets there, and which settings
change its wording.

Use these names when asking for help or reporting something. They're the same
names DialogueForge uses on its tabs and in its live preview, so "the quest
turn-in screen" means one specific thing to everybody.

## The naming rule

Every full-window screen is a **screen**. The one thing that appears *on top*
of a screen without replacing it is a **box**.

Where a screen's wording comes from a field in your config, **the screen is
named after that field**. `TurnInTexts` belongs to the quest turn-in screen;
`NoQuestsTexts` belongs to the no-quests screen. If you know one, you know the
other.

## The screens

| Screen | The player gets there by | You change its wording with |
|---|---|---|
| **Conversation screen** | Talking to an NPC, trader or talkable AI | The nodes and options you write |
| **Quest list screen** | An option with `SHOW_QUEST_LIST`, on a quest NPC | `QuestListTexts`, `QuestListBackTexts` |
| **No-quests screen** | Asking for work when there is none | `NoQuestsTexts`, `NoQuestsBackTexts`, `NoQuestsLeaveTexts` |
| **Quest offer screen** | Picking a quest they haven't started, or an option with `OFFER_QUEST` | `AcceptTexts`, `DeclineTexts`, `OfferBackTexts` |
| **Cooldown screen** | Picking a daily or weekly quest that isn't ready yet | `NotYetTexts`, `OfferBackTexts` |
| **Quest in-progress screen** | Picking a quest they've accepted but not finished | `InProgressTexts`, `InProgressBackTexts` |
| **Quest turn-in screen** | Picking a quest they've finished. An option with `TURN_IN_QUEST` skips straight past this to the hand-in itself | `TurnInTexts`, `NotYetTexts`, `TurnInBackTexts` |
| **Item choice screen** | Handing in a quest that accepts any one of several items, however the hand-in was reached | Nothing — built from the quest |
| **Reward choice screen** | Handing in a quest that lets them pick a reward | `RewardSelectText` |
| **Reward confirm box** | Double-clicking a reward | Nothing — built in, translated |
| **Player settings screen** | The settings button in the window corner | Nothing — it's the player's own |

## Worth knowing

**The cooldown screen is the offer screen wearing a different hat.** Same
screen, but the accept button is suppressed and a "come back in ..." line is
added. That's why it borrows `NotYetTexts` from the turn-in screen rather than
having its own field.

**Four screens can carry a back-to-conversation button**, and each has its own
field so you can word them differently: `QuestListBackTexts`,
`OfferBackTexts`, `InProgressBackTexts`, `TurnInBackTexts`. The no-quests
screen uses `NoQuestsBackTexts` and `NoQuestsLeaveTexts`.

**Per-quest wording beats tree-level wording.** Anything set for a specific
quest in `QuestText\*.json` wins over the same field on the NPC's own tree,
which in turn wins over the mod's built-in text. Nothing is ever blank.

**Only the conversation screen is yours to build.** The other ten are
assembled by the mod from live Expansion data — you change their wording, not
their structure.

## Where these names appear

- **DialogueForge → Quest wording** groups its fields under these names.
- **DialogueForge → Live preview** shows the screen name in its title bar, so
  you can see which one you're looking at.
- **This mod's logs** use them in diagnostics.

See [CONFIG_REFERENCE.md](CONFIG_REFERENCE.md) for the fields themselves and
[DIALOGUE_TREE_GUIDE.md](DIALOGUE_TREE_GUIDE.md) for writing the conversation
screen.
