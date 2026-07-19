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
them — they're deleted explicitly in `ClearButtons()` and the destructor.

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
files. That's why `LayoutOverride` exists.

## RPC ordering

The sync module writes trees, then per-quest text, then menu config, in one
RPC. Reads must match that order exactly — a mismatch desyncs the stream and
corrupts everything after it. If you add a field, add it to `OnSend` and
`OnRecieve` in the same position.

## Credits

The NPC talk animation approach (play sound client-side, send the clip
length to the server, drive `HumanCommandAdditives.SetTalking()` from
`CommandHandler`) follows the technique used in
[ZenExpansionAudioAI](https://github.com/ZenarchistCode/ZenExpansionAudioAI).
Implemented independently here in its own namespace so this mod has no
dependency on it.
