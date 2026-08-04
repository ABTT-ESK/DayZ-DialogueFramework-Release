# Dialogue Tree Authoring Guide

> **Building trees by hand is the hard way.**
> [DialogueForge](https://github.com/ABTT-ESK/DialogueForge) draws the whole
> conversation as a branch map, keeps node IDs consistent when you renumber,
> and flags options that lead nowhere. This guide explains what the fields
> mean either way.


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
| `QuestListTexts` | string array | What the NPC says above the live quest list. One picked at random. Empty = built-in wording |
| `NoQuestsTexts` | string array | What the NPC says when they have no quests available. One picked at random. Empty = built-in wording |
| `NoQuestsBackTexts` | string array | Buttons shown with it that return to `RootNodeID`. Every entry is its own button |
| `NoQuestsLeaveTexts` | string array | Buttons shown with it that end the conversation |
| `NoQuestsVoiceLineIDs` | string array | One picked at random on the no-quests step |
| `QuestListBackTexts` `OfferBackTexts` `InProgressBackTexts` `TurnInBackTexts` | string array | Back-to-conversation buttons for each quest screen. Empty = no button. Per-quest values in `QuestText\*.json` override these |
| `Stages` | stage array | Quest-locked alternate trees — see [Quest-locked trees](#quest-locked-trees) |
| `AIPatrolID` | int | Attaches this tree to Expansion AI you spawn through the mod's own patrol file — see [Talking to friendly AI](#talking-to-friendly-ai). `0` = not an AI tree |
| `AIPatrolSubID` | int | `0` = any unit in that patrol; a number = one specific unit |
| `Nodes` | node array | The actual conversation content — see below |

## `DialogueNode` fields

| Field | Type | Meaning |
|---|---|---|
| `ID` | int | Referenced by `Responses[].NextNodeID` elsewhere in **this same tree**. Must be unique within the tree — reusing an ID across nodes silently breaks navigation to whichever one isn't found first |
| `Type` | string | Almost always `"STANDARD"` — see [Node types](#node-types) below |
| `SpeakerText` | string | The line shown for this node |
| `VoiceLineIDs` | string array | One picked at random when this node is shown |
| `SpeakerLines` | line array | Optional extra candidate lines — see [Multiple lines per node](#multiple-lines-per-node). Empty = only `SpeakerText` is used |
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
| `MaxUses` | int | Anti-farm: max times a player may pick this option, ever. `0` (default) = unlimited. After the limit the option disappears — stops reputation-farming by spamming the same choice. In DialogueForge, just set the number; it manages the counter for you |

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

- Gating is checked **before** the response is shown, so a gated response
  simply doesn't appear until its quest is complete.
- Only `COMPLETED` unlocks. A quest that's merely accepted or turned in but
  not finished still hides the response.
- If gating hides *every* response on a node, the player gets a line of
  dialogue with no buttons and only the X to escape. Always leave at least
  one ungated response on any node that can be reached.

## Multiple lines per node

A node can carry extra candidate lines in `SpeakerLines` so a greeting varies
visit to visit, or a line only appears once a quest is done. When present, the
mod picks one at random from `SpeakerText` plus whichever `SpeakerLines` the
player qualifies for.

```json
{
  "ID": 1,
  "SpeakerText": "Well, look who wandered in.",
  "SpeakerLines": [
    { "Text": "Back again?", "RequiredQuestID": -1, "OverrideQuestID": -1, "VoiceLineIDs": [] },
    { "Text": "The hero returns. Didn't think you'd make it.", "RequiredQuestID": 42, "OverrideQuestID": -1, "VoiceLineIDs": [] }
  ],
  "Responses": [ ... ]
}
```

| Field | Type | Meaning |
|---|---|---|
| `Text` | string | The candidate line |
| `RequiredQuestID` | int | `-1` = always in the random pool. Otherwise the line joins the pool only once that quest is `COMPLETED` |
| `OverrideQuestID` | int | `-1` = normal random line. Otherwise, once that quest is `COMPLETED`, this line stops being random and becomes the NPC's fixed greeting (highest qualifying `OverrideQuestID` wins) |
| `VoiceLineIDs` | string array | Optional audio for this specific line, one picked at random |

`RequiredQuestID` and `OverrideQuestID` are independent — a line can be a random
extra, a locked reveal, a permanent post-quest greeting, or a combination.

## Quest-locked trees

A tree can carry any number of `Stages`, each a **complete tree of its own** —
its own nodes, its own root, its own greeting and branches. Once the player has
`COMPLETED` a stage's `RequiredQuestID`, that stage replaces the base tree for
them. The highest completed stage wins; with none completed, the base tree runs.

```json
{
  "ID": 1,
  "RootNodeID": 1,
  "Stages": [
    {
      "RequiredQuestID": 42,
      "RootNodeID": 1,
      "Nodes": [
        { "ID": 1, "Type": "STANDARD", "SpeakerText": "Different story now that the wall's up.", "Responses": [ ... ] }
      ]
    }
  ],
  "Nodes": [ ... ]
}
```

| Field | Type | Meaning |
|---|---|---|
| `RequiredQuestID` | int | The quest that unlocks this stage when `COMPLETED` |
| `RootNodeID` | int | First node shown, numbered within this stage |
| `Nodes` | node array | This stage's conversation, fully independent of the base tree and other stages |

Because each stage is a clean tree, node IDs restart per stage and a veteran's
options only show what's relevant to their point in the story. An empty `Stages`
array behaves exactly as a single base tree.

## Dialogue variables (attitude / mood / flags)

Variables are **per-player numbers that persist and are shared across every
NPC**. A choice can change one, and any NPC — this one or another — can react to
it. Use them for reputation, mood, "sided with the bandits" flags, anything.

They're stored server-side per player (`$profile\DialogFramework\PlayerState\`),
so they survive relogs and carry between characters in the story sense.

A variable op is `{ "Name": "...", "Op": "...", "Value": N }`.

**Set a variable when an option is chosen** — put `SetVars` on a response:

```json
{ "Text": "You can count on me.", "NextNodeID": 5, "ActionType": "NONE",
  "SetVars": [ { "Name": "rep_hana", "Op": "INCREASE", "Value": 1 } ] }
```
`Op` for setting: `INCREASE`, `DECREASE`, `SET` (assign an exact value).

**Show an option only if variables match** — put `RequiredVars` on a response
(works alongside `RequiredQuestID`). Also works on a `SpeakerLines` greeting, so
an NPC opens differently by mood:

```json
{ "Text": "[She trusts you] Take the back road, it's clear.", "NextNodeID": 8,
  "ActionType": "NONE",
  "RequiredVars": [ { "Name": "rep_hana", "Op": "AT_LEAST", "Value": 3 } ] }
```
`Op` for conditions: `AT_LEAST`, `AT_MOST`, `MORE_THAN`, `BELOW`, `EQUALS`,
`NOT_EQUAL`. All conditions in the list must pass. An unset variable reads
as `0`.

### Two ways an NPC can react

- **Inline** — gate individual responses and greeting lines (above). The
  conversation stays one tree; options appear and disappear by variable.
- **Whole separate tree** — a `Stage` can be selected by variables instead of a
  quest. Give the stage `RequiredVars` (and a `Priority`); when it matches, that
  entire tree replaces the base one, exactly like the quest-locked stages:

```json
"Stages": [
  { "RequiredVars": [ { "Name": "rep_hana", "Op": "AT_MOST", "Value": -3 } ],
    "Priority": 10, "RootNodeID": 1, "Nodes": [ ... ] }
]
```
Among matching stages the highest `Priority` wins (quest-locked stages default
to Priority 0, so existing behaviour is unchanged). A whole-tree swap takes
effect the **next** time the conversation opens; inline gating updates
immediately as you talk.

Notes:

- Variable **sets are applied server-side** and saved. On a story/PvE server
  this is the right tradeoff; it isn't hardened against a player editing their
  own client scripts to bump a value.
- Because variables are global per player, a choice with one NPC changes what
  **every** NPC will say — that's how characters "intersect."

> The `Op` codes above are only needed if you hand-edit JSON. **In DialogueForge
> you never see them** — a response has a *Reputation & story flags* section
> where you pick "Increase by / Decrease by / Set to" and "is at least / is more
> than / …" from dropdowns.

### Per-character reputation and the in-game marker

A single shared reputation gets awkward — a player might be trusted by a quest
giver but on thin ice with a trader. Give **each character its own reputation**
by pointing its tree at its own variable with `ReputationVar`:

```json
{ "ID": 1, "NPCIDs": [], "ReputationVar": "rep_hana",
  "ReputationTiers": [
    { "Threshold": -3, "Label": "Hostile" },
    { "Threshold": 0,  "Label": "Wary" },
    { "Threshold": 3,  "Label": "Friendly" },
    { "Threshold": 6,  "Label": "Trusted" }
  ],
  "RootNodeID": 1, "Nodes": [ ... ] }
```

Because each character's tree names a different variable, their reputations are
independent — `rep_hana` and `rep_weapons` move separately, and each character's
options and greetings gate on their own.

When `ReputationVar` is set, the **dialogue window shows where the player stands**
next to the speaker's name — the matching tier `Label` (highest `Threshold` at or
below the current value), or the raw number if you set no tiers. It updates as
choices change the value during the conversation.

Change it like any variable — a response `SetVars` on `rep_hana`. In
DialogueForge, set the reputation name and tiers on the *Who it's for* tab, and
use the **"+ change this character's reputation"** / **"+ require this
character's reputation"** buttons on a response to fill it in for you.

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

## Talking to friendly AI

> Requires the **Expansion AI** module. Dialogue Framework lists it as a
> required addon.

Regular AI have no fixed ID like a quest NPC, and enemy factions shoot on
sight — so talkable AI are ones **you spawn through the mod** and tag with an
ID. Walk up to a tagged AI and a **Talk** action opens the dialogue window.
The AI halts while you talk and resumes its patrol when the window closes.

A response can also carry `"ActionType": "GO_HOSTILE"` — the AI's whole patrol
turns on the player and attacks. Handy for a neutral encounter that can go
wrong.

### Spawning talkable patrols

Put an `AIPatrols.json` in `$profile\DialogFramework\AIPatrol\`. It uses the
**same per-patrol format as Expansion's `AIPatrolSettings.json`**, wrapped in
`{ "Patrols": [ ... ] }`, with one extra field per patrol: `"DialogueID"`. The
mod spawns those patrols itself and stamps every unit with that `DialogueID`
plus a sub-ID (`1..N`, in spawn order).

```jsonc
{
  "Patrols": [
    {
      "DialogueID": 1,          // <<<<< your ID for this patrol; a tree's AIPatrolID matches it
      "Persist": 0,             // <<<<< recommended, so IDs reassign cleanly each restart
      "Faction": "Guards",      // <<<<< an Expansion faction, or one of your own from Factions\Factions.json
      "Loadout": "ChernoGuardsLoadout",
      "NumberOfAI": 2,
      "Behaviour": "LOOP",
      "Speed": "WALK",
      "Chance": 1.0,
      "PersistentAggroThreshold": -1, // <<<<< per-patrol override of AISettings. -1 = use global, 0 = this patrol never goes permanently hostile, N = permanent after N times
      "PersistenceMode": "",          // <<<<< per-patrol override: "" = use global, or FACTION / PATROL / BOTH
      "Waypoints": [ [6493.68, 18.31, 2236.15], [6488.95, 18.31, 2238.59] ]
      // ...all the other normal AIPatrolSettings fields...
    }
  ]
}
```

Two rules:

- **Don't also list a talkable patrol in Expansion's `AIPatrolSettings.json`** —
  it would spawn twice. The `AIPatrol\AIPatrols.json` file owns it.
- Keep `"Persist": 0` on talkable patrols, so IDs are reassigned cleanly each
  restart and they don't tangle with Expansion's group persistence.

**Per-patrol permanent-hostility.** `AISettings.json` sets the server default,
but each patrol can override it with `PersistentAggroThreshold` and
`PersistenceMode` — so one patrol turns permanently hostile after 2 bad runs,
another after 6, and another (`0`) never does. Leave them at `-1` / `""` to use
the global setting.

### Attaching a tree

A dialogue tree locks onto tagged AI with:

| Field | Meaning |
|---|---|
| `AIPatrolID` | Matches any unit spawned from the patrol with this `DialogueID` |
| `AIPatrolSubID` | `0` = any unit in that patrol; a number = only that one unit (unit 1, unit 2, …) |

```json
{ "ID": 20, "AIPatrolID": 1, "AIPatrolSubID": 0, "RootNodeID": 1, "Nodes": [ ... ] }
```

So two units in the same patrol can hold entirely different conversations —
give one tree `AIPatrolSubID: 1` and another `AIPatrolSubID: 2`. The tree file
can live in any folder; matching is by `AIPatrolID`, not the folder name.

In **DialogueForge**, pick **"Talkable AI (Expansion)"** on the Dialogue tab and
set the Patrol DialogueID (and optional Sub-ID); it saves to `Dialogues\AI\`.
The `RECRUIT_AI` and `GO_HOSTILE` actions are in the response action dropdown.

### Recruiting through dialogue

A response with `"ActionType": "RECRUIT_AI"` recruits the AI into the player's
group, then closes the window. Recruiting is validated **server-side** and
respects Expansion's own AI settings — `CanRecruitFriendly`, `CanRecruitGuards`
and `MaxRecruitableAI` — so it behaves like the stock recruit action and can't
exceed a server's limits.

To lock recruiting behind quest progress, set a `RequiredQuestID` on the
`RECRUIT_AI` response. That hides the option until the quest is `COMPLETED`
(like any gated response) **and** is re-checked on the server before the
recruit goes through.

```json
{ "Text": "Watch my back out there.", "NextNodeID": -1,
  "RequiredQuestID": 12, "ActionType": "RECRUIT_AI" }
```

## Action types

| Value | What it does |
|---|---|
| `"NONE"` (default) | Just go to `NextNodeID`. If `NextNodeID` is `-1`, ends the conversation instead |
| `"SHOW_QUEST_LIST"` | Opens a **live** list of whatever quests are actually available from this NPC right now (built from real Expansion quest data, not authored) |
| `"ACCEPT_QUEST"` | Only meaningful as a response inside the live quest-detail step (see below) — accepts the currently-viewed quest |
| `"DECLINE_QUEST"` | Ends the conversation without accepting |
| `"TURN_IN_QUEST"` | Only meaningful when the quest-detail step is showing a quest that's ready to turn in — completes it |
| `"END_CONVERSATION"` | Plays a random farewell line, then closes the window |
| `"OPEN_TRADER"` | Trader trees only. Closes dialogue and opens the market |
| `"RECRUIT_AI"` | AI trees only. Recruits the AI into the player's group, then closes. See [Talking to friendly AI](#talking-to-friendly-ai) |
| `"GO_HOSTILE"` | AI trees only. The AI's whole patrol turns hostile and attacks the player, then closes. For conversations that can go sideways |

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


## When the NPC has nothing to offer

If a player opens the quest list and the NPC has nothing available, they get a
real dialogue step rather than a dead end — a spoken line plus whatever buttons
you configured.

Wording is resolved in this order:

1. `NoQuestsTexts` on the **highest-numbered quest of this NPC's that the player
   has completed** (set per quest in `QuestText\*.json`). A quest counts as
   this NPC's if they are its giver *or* its turn-in target.
2. This tree's `NoQuestsTexts`.
3. Plain built-in wording.

Because the highest completed ID wins, a quest chain advances the NPC's parting
line for free. Give quest 2 a `NoQuestsTexts` that points at the NPC who hands
out quest 8, and the moment quest 2 is turned in, that becomes what they say —
until the player completes something later in the chain.

`QuestListTexts` — the line above the quest list itself — resolves by exactly
the same rule, so a busy NPC doesn't greet a veteran the way they greet a
stranger. Every one of these is an array with one entry picked at random, so
even a single NPC with three phrasings and no per-quest wording reads
differently visit to visit.

Buttons work the same way: quest-level `NoQuestsBackTexts` /
`NoQuestsLeaveTexts` if set, otherwise the tree's. Back returns to
`RootNodeID`, Leave ends the conversation. **If you configure none of it, the
player still gets a plain Back button** — this step can never strand them with
only the X.

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
`Responses` list reasonably short (roughly 2–6 visible at once), even if
you've authored a dozen options. The system will happily render
more, but a wall of buttons is a UX problem for the player, not a
technical one.
