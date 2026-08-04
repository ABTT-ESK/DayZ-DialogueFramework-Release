# Developer notes

Only relevant if you're modifying the mod itself. Server owners don't need
any of this — see [`CONFIG_REFERENCE.md`](CONFIG_REFERENCE.md) instead.

## Layout

| Scope | Contains |
|---|---|
| `3_Game/Dialogue/` | Data model, menu config, RPC ids, sound accessor |
| `4_World/Dialogue/` | Config loading, server→client sync, NPC talk animation |
| `5_Mission/Dialogue/GUI/` | The dialogue window |

Trees, per-quest text and menu config are all read from the server's profile
folder and pushed to clients on connect. The client can't read the server's
profile folder, which is why all three need syncing.

## Gotchas worth knowing before you change something

**JSON loading skips constructors.** Field initialisers never run for data
loaded from disk, so an omitted field arrives as `0`/`""`/`null`, not its
declared default. Every class has a `Sanitize()` that re-applies defaults,
cascading tree → node → response. Call it after every load and every RPC
receive. Getting this wrong once caused every dialogue button to silently
disappear (omitted `RequiredQuestID` became `0`, gating every response
behind a quest that can't exist).

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

**No multi-line expressions.** A statement wrapped across lines (a `||`
chain, say) is a syntax error. Keep expressions on one line or split them
into separate statements.

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

## RPC ordering

The sync module writes trees, then per-quest text, then menu config, in one
RPC. Reads must match that order exactly — a mismatch desyncs the stream and
corrupts everything after it. If you add a field, add it to `OnSend` and
`OnRecieve` in the same position.

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
weapon); HOSTILE → both false. Registry is server-only (JSON lives in the
profile); client `GetName()` would return the slot suffix, so
`DialogueFW_SpeakerName` suppresses any name starting with the slot prefix and
lets the tree's own speaker name show.

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
