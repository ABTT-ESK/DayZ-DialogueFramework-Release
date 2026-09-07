# Developer notes

Only relevant if you're modifying the mod itself. Server owners don't need
any of this — see [`CONFIG_REFERENCE.md`](CONFIG_REFERENCE.md) instead.

## Layout

| Scope | Contains |
|---|---|
| `3_Game/Dialogue/` | Data model, menu config, RPC ids, sound accessor, translation overlay, client settings |
| `4_World/Dialogue/` | Config loading, server→client sync, NPC talk animation |
| `5_Mission/Dialogue/GUI/` | The dialogue window |

Trees, per-quest text and menu config are all read from the server's profile
folder and pushed to clients on connect. The client can't read the server's
profile folder, which is why all three need syncing.

## Screen names

There are eleven screens in the window, listed in [`SCREENS.md`](SCREENS.md),
each named after the config field that controls its wording (`TurnInTexts`
belongs to the quest turn-in screen, `NoQuestsTexts` to the no-quests screen).

**That file is a three-way contract**: the mod's `LogScreen()` calls,
DialogueForge's tab headings, and DialogueForge's live-preview titles all use
the same names. Three different sets of names were in use before 1.3.0, which
made it impossible to tell someone where to look. Adding a screen means adding
it to `SCREENS.md` and calling `LogScreen("...")` when it opens — the client
log then reads `[DialogueFramework] [SCREEN] Quest turn-in screen`.

## Gotchas worth knowing before you change something

**JSON loading skips constructors.** Field initialisers never run for data
loaded from disk, so an omitted field arrives as `0`/`""`/`null`, not its
declared default. Every class has a `Sanitize()` that re-applies defaults,
cascading tree → node → response. Call it after every load and every RPC
receive. Getting this wrong once caused every dialogue button to silently
disappear (omitted `RequiredQuestID` became `0`, gating every response
behind a quest that can't exist).

**EnforceScript will not parse an expression split across lines.** A `Print()`
whose string concatenation wrapped onto a second line starting with `+` failed
the whole Mission module with `Expected ',' or ')'` — the server would not
start. Build long strings up over several single-line statements instead.
`preflight.py` now flags a wrapped `+` inside an unclosed bracket as well as
`||`, `&&` and `?`, so this fails at your desk rather than on the server.

**A static `ref` container has to be built at its declaration**, not lazily
inside the accessor. `if (!s_All) s_All = new array<string>` threw
"NULL pointer to instance" when first touched from `MissionServer.OnInit`;
`static ref array<string> s_All = new array<string>` does not. Expansion
declares its statics the same way. Also assign a method's return value to a
local before `foreach` over it.

**Never close or rebuild the window from inside its own click handler.**
Doing so destroys the object while its method is still on the stack.
Everything that closes the window or swaps the widget tree is deferred one
frame with `CallLater`.

**The window needs a strong reference.** It's held by
`DialogueWindowLauncher`; without that it gets collected before rendering a
frame.

**Preview entities are real objects.** Reward thumbnails create local
entities (`ECE_LOCAL|ECE_NOLIFETIME`). Unlinking the widget doesn't free
them, and they must be deleted only *after* every `ItemPreviewWidget` that
referenced them is unlinked — deleting an entity still bound to a live
preview widget leaks its slot in the engine's limited preview pool, and once
that pool is exhausted later previews render blank. They're released on
`OnHide` (the reliable close hook covering the X, quest turn-in, and the game
closing the menu), not left to the destructor, because the script object can
outlive the closed menu by a frame or two.

**Content is populated in `OnShow()`, not `Init()`.** Doing tree navigation
or NPC lookups during menu construction leaves a half-built menu if anything
throws, which wedges the interaction key entirely.

**Client `Print()` goes to the client log,** never the server RPT. Anything
in the GUI classes logs client-side.

**EnforceScript has no ternary operator**, and variables are function-scoped
— declaring the same name in two sibling blocks of one function is a compile
error.

**Packed paths start with the PBO prefix.** Addon Builder's `SourceDir` is
the mod root and `$PBOPREFIX$` matches the folder name, so `files[]` entries
and `CreateWidgets()` paths read `DialogueFramework/Scripts/...` and
`DialogueFramework/GUI/...`.

**A class must be modded from its own script module.** `modded class X`
only compiles in the module where `X` is declared — `ExpansionQuestMenu` and
`ExpansionMarketMenu` are 5_Mission, `ExpansionNPCBase` and `eAIBase` are
4_World, `EffectSound` is 3_Game. Modding across modules fails with
"Unknown type".

**`requiredAddons[]` controls compile order, not just presence.** An
Expansion module you reference must be listed there or your scripts can be
compiled first, and its classes resolve as unknown types. `#ifdef` guards do
not fix this — the define can be visible while the class isn't yet.

**`string.Length()` and `Substring()` count BYTES, not characters.** DayZ
provides `LengthUtf8()` and `SubstringUtf8()` precisely because of this, and
Expansion's chat wrapping uses them. Anything that measures *displayed* text
must use the UTF-8 variants: `SetSpeakerLine` and `ResponseLinesNeeded` divide a
length by a per-line **character** budget, so measuring bytes made every
Cyrillic line count double and every CJK line treble -- speaker areas reserved
twice the height they needed and response buttons ran the font-shrink loop down
to the minimum for no reason. `ShortenForTile` is worse: a byte `Substring` can
cut a multi-byte character in half and leave a broken glyph. Path and key
handling (`DialogueLocPath.Normalize`) is ASCII and can stay on the byte calls.

**The game's JSON reader stops at 1023 bytes per string** (a 1024 buffer minus
its terminator). Measured in game with the `NPC_196` test tree: lines authored
at 1223, 1711, 2008 and 2269 bytes all arrived as exactly 1023 bytes / 586
characters. It is *not* the RPC -- `CheckLineLength` runs server-side after
`JsonFileLoader` and already sees 1023, so the text is lost before the mod ever
touches it.

That has a consequence worth keeping straight: **the mod cannot warn that a
line is too long**, because the original length is unrecoverable by the time it
can look. `LINE_BYTES_LIMIT` (1023) only lets it spot the *signature* -- a line
sitting exactly on the limit was almost certainly cut -- and report the damage.
DialogueForge reads the JSON itself, so it is the only place that can warn in
time; its `LINE_BYTES_LIMIT` / `LINE_BYTES_CLOSE` must stay in step with this.

**EnforceScript aliases string temporaries.** Two freshly-returned strings
used in one expression can end up as the same value. Passing two accessor
calls straight into one formatting call made every settings row read
`server's choice: server's choice`. Assign each to its own local first.
`preflight.py` cannot catch this — it is a runtime aliasing bug, not a syntax
error.

**Fonts can't be changed from script.** DayZ only reads them from `.layout`
files, which is why font choice is a pre-built layout set rather than a
runtime setting.

**Most layout files are generated — don't edit them.** Only these four are
hand-written:

```
dialogue_menu.layout
dialogue_response_button.layout
dialogue_reward_button.layout
dialogue_reward_display.layout
```

Everything ending `_light`, `_large` or `_compact` is produced from those by
`tools/gen_layout_variants.py` and will be overwritten. After changing a
master, regenerate from the repository root:

```
python tools/gen_layout_variants.py
```

The generated files are committed, so server owners never run this — it's
only needed when the masters change. Adding a style means one entry in the
script's `STYLES` table.

**Texture format comes from the filename suffix.** Bohemia's texture tools
read the output format from the file name: `_co` = no alpha, `_ca` = keep the
alpha channel. UI icons must be named `icon_*_ca`, or they convert to opaque
textures and render as solid blocks. UI textures are `.edds` (`.paa` also
loads); if you change the format, change the files in `GUI/images` to match.

**A `ScrollWidget` only scrolls when its content is taller than its
viewport.** Leaving the text widget at viewport height clips a long line
instead of scrolling it, so the speaker-line widget is sized from the text.
When estimating wrapped line count, lean generous — overestimating adds
harmless blank space at the bottom, underestimating clips text.

**EnforceScript float→int conversion is a silent build-breaker.** Where an
integer count matters, count in a loop rather than dividing floats.

## Quest turn-in — `NeedAnyCollection`

A collection objective with `NeedAnyCollection` set is rejected server-side
when handed in with `objItemIndex -1` (the "Quest turn-In failed / Something
went wrong" toast, with nothing in the logs). The window resolves a real
collection index before turning in, mirroring the stock menu (the first
`NeedAnyCollection` objective it finds drives the index):

- No `NeedAnyCollection` objective → index stays `-1`, turn in now.
- One collection defined → index `0` (auto-pick, works even before client
  objective data catches up).
- Exactly one satisfied collection → that index (auto-pick).
- More than one satisfied → show the item picker, then continue the turn-in
  from `ExecutePendingObjItemSelection`.

## Quest actions on a response

`ACCEPT_QUEST`, `OFFER_QUEST` and `TURN_IN_QUEST` all read the response's
`QuestID` (`-1` = "whichever quest the live quest-detail step is on").
`OFFER_QUEST` builds the quest's own offer screen from any node;
`ACCEPT_QUEST` hands the quest over with no offer screen; `TURN_IN_QUEST`
runs the hand-in.

`TurnInQuestByID()` is the mirror of the accept-by-id path. It resolves the
config, reads the player's state through `GetPlayerQuestState()`
(`GetClientQuestData()`, `NONE` when the client has no quest data yet), and
requires exactly `ExpansionQuestState.CAN_TURNIN` before setting
`m_ActiveQuestID` and falling into the existing `TurnInActiveQuest()` — the
objective-item picker, the reward picker and the turn-in RPC all read
`m_ActiveQuestID`, so the by-id path reuses every one of them rather than
duplicating the flow. Expansion validates a turn-in on quest **state**, not on
proximity to the turn-in NPC (the turn-in ID only drives a completion emote),
so a by-id hand-in from any character is accepted server-side.

**`SHOW_QUEST_LIST` is guarded on `m_NPCID`.** A trader or friendly-AI
conversation opens with an NPC ID of `-1`. Expansion's
`QuestDisplayConditions` only filters by giver/turn-in NPC when the id is
`> -1`, so passing `-1` through `GetAvailableQuestsForNPC()` returned **every
quest the player was eligible for, server-wide, in no order**.
`GetAvailableQuestsForNPC()` and `QuestBelongsToThisNPC()` now both return
empty/false when `m_NPCID <= 0` and log why. Note this is the opposite of the
deliberate `-1` passed *into* `QuestDisplayConditions` below — there it means
"skip the giver rule for one named quest", here it would mean "match no NPC
and therefore all of them".

`CanPlayerTakeQuest()` guards both. It calls Expansion's own
`QuestDisplayConditions(quest, player, questData, -1, false)` — **passing `-1`
as the NPC id is deliberate**, and is the only rule skipped: it drops
"is this NPC the giver", which is exactly what these actions exist to bypass.
Every other rule still applies (already completed, on cooldown, prerequisites
unmet, achievement quests), plus an explicit `STARTED` / `CAN_TURNIN` check in
front, since handing an in-progress quest over again would reset progress.
The offer screen drops its accept button when the check fails; `AcceptQuest`
refuses and logs why.

Before 1.3.0 `ACCEPT_QUEST` silently closed the window anywhere outside the
quest-detail step. It now logs the reason instead of failing quietly — if you
add another action, follow the same shape.

`RefuseTurnIn(questID, state)` owns every refusal, and **the message follows
the state**: `COMPLETED` -> "You've already handed that one in.", `STARTED` ->
"You haven't finished that yet.", anything else -> "You haven't taken that one
on.". The first version used the STARTED wording for all three, which told a
player who had just completed a quest that they had not finished it -- it reads
as a mod bug rather than as an answer. If you add a state, add its wording here;
a wrong-but-plausible message costs more than a vague one. Each string is
assigned to its own local first: **EnforceScript aliases string temporaries**,
so building the key and the fallback inline would let them clobber each other.

**Player-facing failures go through `NotifyPlayer(key, fallback, isError)`.**
`isError = true` means a config mistake and is suppressed by
`MenuConfig.ShowErrorNotifications`; `isError = false` means ordinary gating
("You can't take that quest right now") and is always shown. The detailed
reason always goes to the log regardless. Don't `Print()` a failure without
also calling it — a silently closing window is the bug this replaced.

`HideAfterQuestID` is the mirror of `RequiredQuestID` and is checked in
`PassesGating` alongside it. Both on one response gives it a window: appears
after one quest, gone after another. Setting them to the *same* quest makes an
option that can never appear, which `DialogueManager.LogIssue` flags at load
and DialogueForge's checker flags at edit time.

## Stage resolution

The active tree is resolved once per window and cached. A player can't
complete a quest mid-conversation (turn-in closes the window), so the active
tree is stable for the window's life. `m_StageNodes` / `m_StageRootNodeID`
drive every node lookup and every back-to-conversation, keeping the whole
conversation within the current stage. The base tree is the default; the
highest completed `Stage` replaces it wholesale. An empty stage is ignored so
a half-authored tree can't open the conversation on nothing.

**Back-to-conversation buttons use out-of-range user IDs.** They're numbered
past `m_CurrentQuests.Count()`, which `OnClick` reads as "return to root".

## Text placeholders (`%1`)

All player-facing prose runs through `DialogueFW_FormatText` — one call in
`SetSpeakerLine` (every spoken line) and one in `CreateResponseButton` (every
response, quest title and back button). It wraps the text in CF's
`StringLocaliser(text, playerName).Format()`, exactly what Expansion's own quest
menu does, so `%1` resolves to the player's name and `#STR_...` keys localise.
Add new player-facing text through those two chokepoints and it's covered for
free; don't format the same string twice.

## Packing

`stringtable.csv` sits at the PBO root and **has to actually be packed**. At
least one common packing tool filters `.csv` out silently: the scripts still
compile, the mod still loads, and every `#STR_DIALOGUEFW_*` lookup quietly
returns its own key. Add `*.csv` to your tool's copy/include list — Expansion
ships `stringtable.csv` inside its own PBOs, so it is a supported thing to do.

Run `python tools/verify_pbo.py <built.pbo>` after packing. It reads the PBO's
header table and fails if the stringtable, the layouts or the config are
missing. `tools/preflight.py` cannot catch this — it only sees source.

`UIText(key, fallback)` in the window exists for the same reason: if the table
is absent the engine hands the key back, so it falls back to the English
wording and logs the cause once rather than showing a player a raw key.

## Response button sizing

The option label is a `MultilineTextWidgetClass` with `wrap 1` — a plain
`TextWidgetClass` is single-line and silently clips, which was the original
bug. The engine does the wrapping; `SizeResponseButton` only decides the font
size and the button's height.

There is no verified engine call for measuring rendered text, so line count is
estimated from character count against `RESPONSE_CHAR_RATIO`, the same approach
`SetSpeakerLine` has always used for the NPC's line. `RESPONSE_WRAP_SAFETY`
biases the estimate toward *more* lines on purpose: word wrapping always breaks
earlier than a raw character count suggests, and an over-tall button is
cosmetic while an under-tall one clips text — which is the bug being fixed.

Width comes from the panel (`m_DialoguePanel.GetScreenSize` × `m_ScrollW`),
never from the button itself: a freshly created widget hasn't been laid out
yet and reports zero. If the panel isn't measured either, sizing is skipped
and the layout defaults stand.

`RESPONSE_FONT_PX_LARGE` / `_COMPACT` must match what `gen_layout_variants.py`
bakes into the style variants (`scale` 1.2 and 0.85 of 18 → 22 and 15). Setting
the size from script overrides the layout value, so if those drift, picking a
font style would stop changing anything.

`ResponseList` is a `WrapSpacerWidget`, which lays its children out once. A
button resized after that point is drawn overlapping the next one, so
`RefreshResponseList()` has to run after **every** screen finishes building its
buttons — miss it on a new screen and that screen's long options will overlap.

Line count is deliberately **not** capped after the shrink loop. If text still
needs more than `RESPONSE_GROW_LINES` at the minimum font, the button just
keeps growing. Truncation is the failure this code exists to prevent.

## Localization

Two separate systems, deliberately. [`LANGUAGES.md`](LANGUAGES.md) is the
server-owner side of this; what follows is the implementation.

**The mod's own wording** is a normal DayZ `stringtable.csv` at the addon root,
read through `Widget.TranslateString("#STR_DIALOGUEFW_...")` (wrapped as
`UIText()` in the window). Add a key there and a column value for all 14
languages; the engine picks the column. Layout-baked captions — the confirm
panel's Confirm/Cancel — are set from script in `ApplyConfirmButtonLabels()`
rather than in the four `.layout` variants, so the generator doesn't have to
know about languages.

**Server-owner dialogue** can't use the stringtable (owners can't edit the
PBO), so it has its own overlay system in `3_Game/Dialogue/DialogueLocalization.c`:

- `DialogueLocFile` is the on-disk shape, `DialogueLocBundle` is one language in
  memory and over the wire, and `DialogueLoc` is the client-side resolver.
- Lookups are a flat `map<string,string>` keyed `F|<treefile>|<key>`,
  `I|<treeid>|<key>` or `Q|<questid>|<key>`. File match wins; tree ID is the
  fallback, which is why a tree's `LocKey` (its path under `Dialogues\`,
  lowercased, forward slashes) is stamped on at load and shipped over RPC.
- **`DialogueLocKeys` is a contract with DialogueForge.** The Python side builds
  the identical strings in `loc_tree_entries` / `loc_quest_entries`. Change the
  format on one side only and translations silently stop matching — nothing
  errors, the text just stays in the source language.
- Indexes are into the **authored** arrays, not the filtered ones the player
  sees. `GetVisibleResponses` therefore walks `node.Responses` by index and
  hands back a parallel array of resolved strings; `PickSpeakerLine` reports
  which index it picked. Any new place that renders owner text has to do the
  same or it will translate the wrong line.
- Synthesised nodes (quest detail, the no-quests step, the language picker)
  carry text that was already resolved when it was built, so `IsAuthoredNode()`
  keeps them out of the tree lookup — otherwise their negative IDs would collide
  with real keys.

**Who picks the language.** The client detects its own via a probe key,
`#STR_DIALOGUEFW_LANGUAGE_ID`, whose value is the language name in each column
— there is no engine call for this. A player override is stored client-side in
`$profile:\DialogFramework\ClientSettings.json` alongside the rest of their
settings, so it follows the player across servers rather than being
per-character state. `ClientLanguage.txt` was the 1.3.0-development location
and is still read once and folded into the JSON, so an early tester doesn't
lose their choice; nothing writes it any more.

**Wire flow.** The connect payload carries the list of languages the server has,
nothing more. The client then asks for the one it wants over
`RPC_RequestDialogueLoc` (a server RPC, so no player entity is needed) and gets
one `RPC_SyncDialogueLoc` back. That keeps the payload at exactly one language
however many the owner ships, and makes the in-game language switch a re-request
rather than a reconnect. Downloaded bundles are cached per language in
`DialogueLoc`, so switching back is instant and silent.

## Player settings screen

`DialogueClientSettings` (3_Game) is a plain JSON-serialisable singleton saved
to `$profile:\DialogFramework\ClientSettings.json`. It is **client-only** and
never crosses the RPC boundary — the server neither reads nor validates it, so
nothing here can be a gating or trust decision.

Four values, each with a sentinel meaning "use whatever the server sent":
`Language` (`""`), `Position` (`""`), `TextScale` (`TEXT_SCALE_SERVER`) and
`Icons` (`ICONS_SERVER`). Anything reading a menu-config value the player can
override has to consult the settings first and fall back to `m_MenuConfig`, not
the other way round. `HasAnyOverride()` drives whether the reset row appears.

The screen is built by `ShowSettingsScreen()` as a synthesised node, so it goes
through `IsAuthoredNode()` and stays out of the translation lookup (its own
wording is stringtable text). Rows cycle on click and rebuild the screen in
place.

`SettingsButtonWanted()` only needs a menu config; `LanguageRowWanted()`
additionally needs `ShowLanguageButton` and at least one language in
`DialogueLoc.ServerLanguages()`, so the row is absent on a server with no
translations rather than showing a list of one.

Switching language re-requests the bundle over the network, so the redraw is
deferred `350ms` (`CallLater(ShowSettingsScreen, 350)`) — redrawing
immediately shows the screen in the *old* language for a frame. That delay is
the only reason the row doesn't rebuild synchronously like the others.

## RPC ordering

The sync module writes trees, then menu config, then per-quest text, then the
player's variables, then the list of translation languages, in one RPC. Reads
must match that order exactly — a mismatch desyncs the stream and corrupts
everything after it. If you add a field, add it to `OnSend` and `OnRecieve` in
the same position. `tools/preflight.py` counts the writes and reads in each
class and fails the build if they disagree.

## Config versioning

`MenuConfig.json` and `QuestText\*.json` self-upgrade: on load, `Sanitize()`
fills every missing field with its default, then `UpgradeFromOlderVersion()`
stamps the current version and reports whether the file should be rewritten —
it changes nothing else. To add a field: give it a default, add it to
`Sanitize()`, and bump `CURRENT_VERSION`. The file is backed up before a
rewrite. Dialogue trees deliberately do **not** self-upgrade (see
[`UPDATING.md`](UPDATING.md)).

QuestText versions: 1 added `QuestListTexts` and the `NoQuests*` fields; 2
added the per-screen `*BackTexts` fields.

MenuConfig versions: 1 added `WindowBorderThickness` and
`VisitedResponseOpacity`; 2 added `FontStyle`; 3 added `ShowResponseIcons`; 4
added `ShowLanguageButton`; 5 added `ScaleTextWithPanel`; 6 added
`ShowErrorNotifications`. `CURRENT_VERSION` is **6**.

`Localization\*.json` files are written by DialogueForge, not upgraded in place
— they only ever hold keys and text, so there is nothing to migrate.

## Faction AI dialogue & recruitment

Requires the Expansion AI module. `DayZExpansion_AI_Scripts` is in
`requiredAddons` (a hard dependency now — references to `eAIBase`, `eAIGroup`
and `ActionConstructor` need it in compile order, not just an `#ifdef`). The
AI code is still wrapped in `#ifdef EXPANSIONMODAI` so it reads clearly.

**Trigger — crossing the module boundary.** Arbitrary AI have no menu to
intercept (unlike traders/quest NPCs), so `DialogueFW_ActionTalkToAI`
(4_World, registered via `modded ActionConstructor`) opens the window. A
4_World action can't reference the 5_Mission `DialogueWindowMenu`, so it
stashes the request (tree + AI entity + name) in `DialogueAISession` (4_World)
and calls `EnterScriptedMenu(MENU_DIALOGUEFW_AI)`; the 5_Mission
`modded MissionBase.CreateScriptedMenu` reads the session and builds the
window. Same pattern Expansion uses for the code-lock UI.

**Matching is ID-only.** `DialogueManager.GetTreeForAIPatrol(patrolID, subID)`
returns the tree whose `AIPatrolID` matches (a specific `AIPatrolSubID` beats a
patrol-wide `AIPatrolSubID 0`). There is no faction/class/position fallback —
enemy factions shoot on sight, so talkable AI are always ones spawned through
the mod's own patrol file and tagged with an ID.

**Talkable patrols & the ID lock.** `DialogueFW_AIPatrolSpawner.SpawnAll()`
(called from `MissionServer.OnInit`, server-only) reads
`$profile:\DialogFramework\AIPatrol\AIPatrols.json` into
`DialogueFW_PatrolConfig` (`: ExpansionAIPatrol` + `DialogueID`) and spawns each
via `eAIDynamicPatrolT<DialogueFW_Patrol>.CreateEx` after
`eAIDynamicPatrol.InitSettings()` (so `-1` fields fall back to Expansion's
globals). `DialogueFW_Patrol` overrides `SetupAI` to stamp each unit with the
config's `DialogueID` + a running sub-ID (1..N, reset in the overridden
`Spawn()`). Those two ints live on `eAIBase` (`m_DialogueFW_PatrolID` /
`m_DialogueFW_PatrolSubID`), registered as net-sync ints in a modded
`eAIBase.Init()` and pushed with `SetSynchDirty()`, so the client-side
`ActionCondition` can resolve the exact tree. A patrol must NOT also be in
Expansion's own `AIPatrolSettings.json` or it double-spawns; recommend
`Persist: 0` on talkable patrols to avoid persistence-index entanglement.
Trees carry `AIPatrolID` (0 = unused) and `AIPatrolSubID` (0 = any unit); the
loader also treats `AIPatrolID > 0` as a valid AI key so an ID-only tree loads.

**Recruit.** The `RECRUIT_AI` response sends `CLIENT_REQUEST_RECRUIT_AI` on
the AI entity (carrying the response's `RequiredQuestID`). The server handler
in `eAIBase.OnRPC` re-validates everything — never trust the client — mirroring
`ExpansionActionRecruitAI`: alive, the AI's group leader is AI (not another
player's follower), not passive/invincible, `!PlayerIsEnemy`, and the settings
`CanRecruitFriendly`/`CanRecruitGuards`/`MaxRecruitableAI`
(`GetExpansionSettings().GetAI(false)`). The optional quest gate uses
`ExpansionQuestModule.GetModuleInstance().HasCompletedQuest(id, uid)`. Recruit
itself is `SetGroup(eAIGroup.GetGroupByLeader(player, true, faction), false)`.

## Dialogue variables

Per-player integer variables, persistent + server-authoritative, global across
all NPCs. `DialogueVarOp {Name, Op, Value}` appears as `RequiredVars` (gate) on
responses / speaker lines / stages, and `SetVars` (apply) on responses. Op
tokens are readable words: set ops `INCREASE` / `DECREASE` / `SET`; condition
ops `AT_LEAST` / `AT_MOST` / `MORE_THAN` / `BELOW` / `EQUALS` / `NOT_EQUAL`
(uppercased in `Sanitize`). `DialogueVarOpList.Evaluate` / `.Apply` / `.Compare`
are the shared logic;
`DialoguePlayerState` (parallel `Names`/`Values` arrays, JSON-safe) holds a
player's vars.

- **Store** (`DialogueVars`, 4_World): server caches `map<uid, DialoguePlayerState>`,
  loads/saves `$profile:\DialogFramework\PlayerState\<uid>.json`. Client keeps
  its own `m_ClientState`.
- **Sync**: the connect RPC (`SendTreesToClient`) appends the player's state;
  after a change the server calls `DialogueFrameworkSyncModule.DialogueFW_SendVars`
  (client RPC `RPC_SyncPlayerVars`).
- **Set**: choosing a response with `SetVars` sends `CLIENT_APPLY_VARS` on the
  player entity (raw `ScriptRPC`); `modded PlayerBase.OnRPC` (server) applies +
  persists + re-syncs. The window also applies optimistically to the client
  store so same-conversation gating sees the change without waiting.
- **Gate**: `VarGatePasses` (client, synced store) gates responses and speaker
  lines alongside `QuestGatePasses`.
- **Stages**: `ResolveStage` picks the highest-`Priority` active stage; a stage
  is active if its `RequiredQuestID` (if any) is completed AND its `RequiredVars`
  (if any) pass. Quest-only stages default Priority 0 → highest quest still wins
  (backward compatible). Stage selection stays cached per window; a var set
  mid-conversation only swaps the tree on the next open.

**Per-character reputation.** `DialogueTree.ReputationVar` names the variable
that is that character's reputation (independent per tree = per character).
`ReputationTiers` (`{Threshold, Label}`) drive the marker; `DialogueRepTierList.LabelFor`
picks the highest threshold at/below the value. The window shows it via
`ApplySpeakerName()` (appends the tier label or `Reputation: N` to the speaker
name — reused so no layout change), recomputed on each node render so live
`SetVars` changes show immediately.

**Anti-farm (`MaxUses`).** A response carries `MaxUses` (0 = unlimited) and a
Forge-generated `UsesKey`. `PassesGating` hides it once the player's `UsesKey`
count reaches `MaxUses`; `ApplySetVars` appends an `INCREASE UsesKey 1` op to the
set it applies + sends, so the count rides the same persisted var store. Purely
built on the variable system — no separate tracking.

Trust note: variable sets originate from the client (server applies them). Fine
for PvE/story; not hardened against client-script tampering. A hardening pass
would re-derive `SetVars` server-side from the tree by tree/node/response id.

## AI aggro reset

`DialogueAggro` (4_World) loads `AISettings.json` (`DialogueAISettings`) once at
mission init. A `GO_HOSTILE` choice calls `group.AddTarget(...)` then
`DialogueAggro.RegisterAggro`, which increments the player's per-faction and
per-patrol anger counts in the persisted `DialogueVars` store (keys
`__aggro_f_<Faction>` / `__aggro_p_<PatrolID>`).

Reset is self-contained per AI — no registry or player enumeration. In the
modded `eAIBase.CommandHandler`, patrol AI (`m_DialogueFW_PatrolID > 0`) run a
throttled `DialogueFW_CheckAggroReset` (`CheckInterval`): for each player in
`GetTargets()`, a dead target clears on `ResetOnDeath`; an alive target clears
(`eAI_RemoveTarget`) only if the player has a dialogue-aggro count for this
faction/patrol (`AnyAggroCount > 0` — so ordinary combat is left alone), isn't
`IsPermanent`, and `PlayerIsCalm` (weapon stowed / left area / surrender).
`IsPermanent` compares the count(s) to the *effective* threshold/mode per
`PersistenceMode`. Per-patrol overrides: `DialogueFW_PatrolConfig` carries
`PersistentAggroThreshold` (-1 = global) and `PersistenceMode` ("" = global);
the spawner calls `DialogueAggro.RegisterPatrolOverride(DialogueID, …)` and
`EffectiveThreshold`/`EffectiveMode(patrolID)` resolve override-else-global.

Surrender uses `GetEmoteManager().Expansion_GetCurrentGesture() ==
EmoteConstants.ID_EMOTE_SURRENDER` (id 61). One follow-up remains:
re-aggro-on-sight across respawns (needs verified player enumeration).

## Custom factions

Expansion factions are compiled `eAIFaction<Name>` classes registered with
`[eAIRegisterFaction(...)]`; `eAIFaction.Create(name)` resolves them by class
name. So a faction can't be born from JSON at runtime — we pre-compile a pool of
32 blank, registered slot classes `eAIFactionDialogueFW0..31`
(`DialogueFactionSlots.c`, 4_World — needs `eAIBase` to tell AI from real
players, so it can't sit in 3_Game with `eAIFaction`).

`DialogueFW_FactionRegistry` (3_Game) loads `Factions\Factions.json` server-side
at `MissionServer.OnInit` (before `SpawnAll`), keeping `s_Defs` (index = slot)
and a lowercased name→slot map. Each slot class overrides `DFW_Slot()`; the base
`DialogueFW_FactionSlotBase` reads the def for that slot for `GetName`,
`GetDefaultLoadout`, `IsGuard` (stance GUARD), `IsFriendly(faction)` (self or in
`FriendlyFactions`), and `IsFriendlyEntity(player)` (true only for stance
FRIENDLY, and only for real players — AI go through `IsFriendly`). Unconfigured
slots report `IsObserver() == true` so they stay out of the `RANDOM` faction pool
and are harmless.

The spawner remaps: if a patrol's `Faction` matches a registry name, it rewrites
`cfg.Faction` to the slot suffix (`DialogueFW<idx>`) before `CreateEx`, so
Expansion's own `Create` picks up our slot class. Player-stance mapping was
verified against `eAIBase.PlayerIsEnemy` / `eAIPlayerTargetInformation`: FRIENDLY
→ `IsFriendlyEntity` true; GUARD → `IsGuard` true (tolerant until you raise a
weapon); HOSTILE → both false. Registry is server-only (`DialogueFW_FactionRegistry.Load()` runs in
`DialogueMissionServerInit`, and the JSON lives in the profile). On the client
`GetDef()` is therefore null and `eAIFaction.GetName()` falls back to
`Type().ToString()` minus its first 10 characters -- `eAIFactionDialogueFW0`
becomes `DialogueFW0` -- so `DialogueFW_SpeakerName` suppresses it.

**The practical consequence: a talkable AI has a BLANK speaker name.** There is
nothing to fall back to -- `DialogueTree` carries no speaker-name field, so the
window title is empty for every AI conversation. Quest NPCs and traders are
unaffected (they pass a real name in). Giving an AI a name would mean adding a
name to the tree and a Forge field for it; until then, write the AI's name into
its opening line if players need to know who they are talking to.

## Credits

The NPC talk animation approach (play sound client-side, send the clip
length to the server, drive `HumanCommandAdditives.SetTalking()` from
`CommandHandler`) follows the technique used in
[ZenExpansionAudioAI](https://github.com/ZenarchistCode/ZenExpansionAudioAI).
Implemented independently here in its own namespace so this mod has no
dependency on it.


## Pre-flight

`python tools/preflight.py` from the repo root, before every build. You cannot
compile EnforceScript outside the game, so this is the only safety net.

It checks brace balance, `#ifdef`/`#endif` pairing, wrapped expressions,
ternaries, `OnSend`/`OnRecieve` field-count symmetry, and — the one that
matters most after refactoring — **every method called is actually defined**.
Deleting or moving a block and leaving a call behind is a compile error that
only shows up when the server starts.

It only sees this mod's own files, so anything inherited from
`UIScriptedMenu`, a CF module base, or the engine has to be added to
`KNOWN_EXTERNAL` at the top of the script. If it reports an undefined call for
something that plainly exists, that is what to check first.
