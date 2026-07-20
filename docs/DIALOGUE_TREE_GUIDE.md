# Dialogue Tree Authoring Guide

This is the reference for building your own dialogue trees — no code
editing required, just JSON files dropped into a folder.

## Where files go

```
$profile:\DialogFramework\Dialogues\NPC_<id>\*.json    <- one NPC
$profile:\DialogFramework\Dialogues\Shared\*.json      <- many NPCs at once
```

**`NPC_<id>` folders** (the common case): dedicated to exactly one NPC.
Name the folder after that NPC's Quest NPC ID — the same `"ID"` field from
your `QuestNPC_X.json` file. Leave `NPCIDs` empty in the JSON itself; it's
inferred from the folder name automatically. Copy/rename this folder to
add a new NPC — there's nothing else to configure.

**`Shared` folder**: for one tree deliberately reused by *multiple* NPCs
(e.g. every generic bandit trader talks the same way). `NPCIDs` must be
set explicitly here, listing every NPC ID that should use this tree.

Restart the server after any change — files are loaded once at startup.

**After every restart, check `LoadLog.txt`** in this same folder. It lists
exactly what loaded, and — critically — flags anything broken: a file that
failed to parse, an ID conflict between two files, a dead-end reference,
etc. Nothing here ever stops the server from starting; a broken file just
means that one NPC falls back to no custom dialogue (or a dangling branch
in it doesn't work) until you fix it.

## File shape

```json
{
  "ID": 1,
  "NPCIDs": [],
  "RootNodeID": 1,
  "GreetingVoiceLineIDs": ["Trader_Greeting_1", "Trader_Greeting_2"],
  "FarewellVoiceLineIDs": ["Trader_Farewell_1", "Trader_Farewell_2"],
  "Nodes": [
    {
      "ID": 1,
      "Type": "STANDARD",
      "SpeakerText": "Well, look who wandered in.",
      "VoiceLineIDs": ["Trader_Root_1"],
      "Responses": [
        { "Text": "I'm looking for work.", "NextNodeID": -1, "RequiredQuestID": -1, "ActionType": "SHOW_QUEST_LIST" },
        { "Text": "Nevermind.", "NextNodeID": -1, "RequiredQuestID": -1, "ActionType": "END_CONVERSATION" },
        { "Text": "What's new around here?", "NextNodeID": 2, "RequiredQuestID": -1, "ActionType": "NONE" },
        { "Text": "Seen any trouble lately?", "NextNodeID": 3, "RequiredQuestID": -1, "ActionType": "NONE" }
      ]
    }
  ]
}
```

## `DialogueTree` fields

| Field | Type | Meaning |
|---|---|---|
| `ID` | int | Your own label for this tree, used only in logs — doesn't need to be unique across files, doesn't need to match anything |
| `NPCIDs` | int array | Which NPC(s) use this tree. Leave empty in a `NPC_<id>` folder (inferred from the folder). Required, non-empty, in `Shared\` |
| `RootNodeID` | int | Which node's `ID` is shown first when the conversation opens |
| `GreetingVoiceLineIDs` | string array | One picked at random each time the conversation opens. Empty array = no greeting audio |
| `FarewellVoiceLineIDs` | string array | One picked at random when the conversation ends via `END_CONVERSATION` |
| `Nodes` | node array | The actual conversation content — see below |

## `DialogueNode` fields

| Field | Type | Meaning |
|---|---|---|
| `ID` | int | Referenced by `Responses[].NextNodeID` elsewhere in **this same tree**. Must be unique within the tree — reusing an ID across nodes silently breaks navigation to whichever one isn't found first |
| `Type` | string | Almost always `"STANDARD"` — see [Node types](#node-types) below |
| `SpeakerText` | string | The line shown for this node |
| `VoiceLineIDs` | string array | One picked at random when this node is shown |
| `Responses` | response array | The player's options at this node |

> **Write every response field explicitly.** Leaving a field out doesn't
> reliably give you its default, and an omitted `RequiredQuestID` can hide
> every option on a node. Copy a full example and edit it.
>
> Node IDs start at **1**.

## `DialogueResponse` fields

| Field | Type | Meaning |
|---|---|---|
| `Text` | string | Button text shown to the player |
| `NextNodeID` | int | Which node to go to next. Only used when `ActionType` is `"NONE"` (or omitted). `-1` (or omitted) ends the conversation |
| `RequiredQuestID` | int | Optional gating — only show this response if that quest ID is `COMPLETED` for the player. `-1` (default) = no gating |
| `ActionType` | string | See [Action types](#action-types) below |

## Locking dialogue behind quests

Set `RequiredQuestID` on any response to hide it until that quest is
**`COMPLETED`** for that player. It works on any response anywhere — root
small-talk, a branch deep in a tree, or an option that opens the quest list.

```json
{ "Text": "Any news from the outpost?", "NextNodeID": 5, "RequiredQuestID": 42,
  "ActionType": "NONE" }
```

That option simply doesn't exist for the player until quest 42 is done —
no greyed-out button, no hint that it's there.

Notes worth knowing:

- Gating is checked **before** random pooling, so a gated response in a
  it simply doesn't appear.
- Only `COMPLETED` unlocks. A quest that's merely accepted or turned in but
  not finished still hides the response.
- If gating hides *every* response on a node, the player gets a line of
  dialogue with no buttons and only the X to escape. Always leave at least
  one ungated response on any node that can be reached.

## Per-quest wording

Quest wording is **per quest**, not per NPC. On a server with a lot of
quests, a handful of shared lines gets obvious fast, so every quest gets its
own accept / decline / turn-in wording.

Put per-quest wording in `$profile:\DialogFramework\QuestText\*.json`:

```json
{
  "Quests": [
    {
      "QuestID": 101,
      "AcceptTexts": ["I'll get your wall built.", "Point me at the timber."],
      "DeclineTexts": ["I'm not a carpenter."],
      "TurnInTexts": ["Wall's up. Try knocking it down."],
      "NotYetTexts": ["Not finished yet."],
      "InProgressTexts": ["Still hauling timber."],
      "RewardSelectText": "Fair work deserves fair pay. Take one."
    }
  ]
}
```

Every array shows **all** its entries as separate buttons, so you can offer
several ways to say yes. Split across as many files as you like — they're
all loaded and merged.

Anything you leave out falls back to plain built-in wording, so partial
files are fine.

## Talking to traders

Traders can have dialogue too. Instead of an `NPC_<id>` folder, use the
trader definition name:

```
$profile:\DialogFramework\Dialogues\Trader_Weapons\Dialogue.json
```

The conversation opens **before** the market menu rather than replacing it —
Expansion's shop is untouched, it just waits until the player asks for it. A
response with
`"ActionType": "OPEN_TRADER"` closes the dialogue and opens the normal
market. Put that on more than one node so players are never more than one
click from trading.

```json
{ "Text": "Let's see what you've got.", "NextNodeID": -1,
  "RequiredQuestID": -1, "ActionType": "OPEN_TRADER" }
```

### Giving one specific trader its own dialogue

The folder name matches **every** trader of that type. To give a single
trader its own conversation, add one of the more specific keys to the file:

```json
{
  "TraderIDs": ["Weapons"],
  "TraderClassNames": [],
  "TraderPositions": ["1234.50 300.00 5678.90"],
  "TraderPositionRadius": 8.0
}
```

A tree can declare all three keys, and `TraderMinKeyMatches` sets how many
must agree:

```json
{
  "TraderIDs": ["Weapons"],
  "TraderClassNames": ["ExpansionTraderAIDenis"],
  "TraderPositions": ["1234.50 300.00 5678.90"],
  "TraderPositionRadius": 50.0,
  "TraderMinKeyMatches": 2
}
```

| Key | Matches |
|---|---|
| `TraderPositions` | A trader within `TraderPositionRadius` metres of a listed spot |
| `TraderClassNames` | A trader using that entity class |
| `TraderIDs` | A trader of that definition |

**Why two keys rather than one.** None of the three is dependable alone:

- Several outposts commonly share a trader definition, so `Weapons` matches
  in more than one place.
- One entity class gets reused for unrelated traders.
- AI traders get shoved off their spawn point, so position drifts.

Requiring two to agree handles all three. Set `TraderMinKeyMatches` to `1`
if you want a tree to catch every trader of a definition regardless of where
it stands, or to `3` to demand an exact match on all counts.

Among trees that qualify, the one with the most agreeing keys wins, and a
position match outranks a name match. **If a trader drifts outside its
radius, the nearest listed position still decides which outpost it belongs
to** — so a wandering trader gets its own dialogue rather than a neighbour's.

Traders spawned at runtime, e.g. after a quest unlocks an outpost, work the
same way: the lookup happens when a player opens the trader, not when it
spawns.

**Finding the values.** Whenever a player opens a trader, the client log
prints all three:

```
[DialogueFramework] [TRADER] Trader opened -- name='Weapons' class='eAI_SurvivorM_Denis' position='1234.5 300.0 5678.9'
```

Copy whichever fits. A following line tells you which key actually matched:

```
[DialogueFramework] [TRADER] Matched by position.
```

Traders with no match open the market directly, exactly as before.

Quest responses (`SHOW_QUEST_LIST` and the rest) don't apply to traders and
are ignored there.

## Action types

| Value | What it does |
|---|---|
| `"NONE"` (default) | Just go to `NextNodeID`. If `NextNodeID` is `-1`, ends the conversation instead |
| `"SHOW_QUEST_LIST"` | Opens a **live** list of whatever quests are actually available from this NPC right now (built from real Expansion quest data, not authored) |
| `"ACCEPT_QUEST"` | Only meaningful as a response inside the live quest-detail step (see below) — accepts the currently-viewed quest |
| `"DECLINE_QUEST"` | Ends the conversation without accepting |
| `"TURN_IN_QUEST"` | Only meaningful when the quest-detail step is showing a quest that's ready to turn in — completes it |
| `"END_CONVERSATION"` | Plays a random farewell line, then closes the window |

## Node types

| Value | Meaning |
|---|---|
| `"STANDARD"` | A normal node you author yourself. This is what you'll use for everything |
| `"QUEST_LIST"` / `"QUEST_DETAIL"` | Built live by the mod — you never author these. They exist purely as internal markers |

**You never need to write a `QUEST_LIST` or `QUEST_DETAIL` node by hand.**
A response with `ActionType: "SHOW_QUEST_LIST"` is all that's needed —
the mod builds the actual list (and, once a quest is picked, the detail
view with accept/decline/turn-in options, using your own
wording from `QuestText\*.json`) live from
Expansion's own quest data every time. This also means the list and the
options shown automatically reflect the *real* state of the quest for that
player: a quest not yet started shows accept/decline; a quest already
turned in to this NPC shows a turn-in prompt instead; a quest given by one
NPC but turned in to a different one correctly shows nothing at the first
NPC once started, and shows the turn-in prompt only at the correct one.


## Voice lines

A `VoiceLineIDs` entry like `"Trader_Greeting_1"` expects a matching
`CfgSoundSets` entry named `DialogueFW_Trader_Greeting_1_SoundSet`, defined
in the separate `DialogueFramework_SoundSets` addon (that's the *only*
addon you ever need to repack when adding audio — never this one). If no
matching SoundSet exists, that line just plays with no audio — never an
error, never breaks the conversation. This means you can write and test an
entire tree's branching logic before recording a single line of audio.

For quest-detail steps specifically, the mod looks for voice lines named
`Quest_<questID>_Start`, `Quest_<questID>_InProgress`, and
`Quest_<questID>_Complete` automatically — you don't declare these
anywhere, just add matching SoundSets if you want them voiced.

## Building large, deeply-branching trees

The system is built to scale to arbitrarily deep and wide trees without
requiring anything special:

- **Depth has no cost.** Navigating from node to node doesn't recurse or
  consume any extra resources per level — a conversation 50 nodes deep
  behaves identically to one 2 nodes deep.
- **Node IDs only need to be unique *within one tree/file*.** Two
  different NPCs' files can both use node ID `1`, `2`, `3`... with no
  conflict. Keeping IDs simple and sequential per file is fine and
  encouraged.
- **Loops are completely fine.** A response can point back to an earlier
  node (like the small-talk nodes looping back to root in the example) —
  build a hub-and-spoke conversation, a long linear one, or anything in
  between.
- **One tree can cover many NPCs** (`Shared\`) if they should all say the
  same things, or **one NPC can have a large, unique tree** — both are
  first-class, not workarounds.

**What to watch for as trees grow large** (all of this is now checked
automatically and reported in `LoadLog.txt`):
- A `NextNodeID` that doesn't match any node in the file (usually a typo)
- Two nodes accidentally sharing the same `ID`
- A `RootNodeID` that doesn't match any node at all

None of these crash the server or break other NPCs' trees — they're
reported so you can fix them, and the affected branch just won't be
reachable until you do.

**Practical tip:** if you're building something large, keep each node's
`Responses` list reasonably short (roughly 2–6 visible at once, using
even if you've authored a dozen options). The system will happily render
more, but a wall of buttons is a UX problem for the player, not a
technical one.
