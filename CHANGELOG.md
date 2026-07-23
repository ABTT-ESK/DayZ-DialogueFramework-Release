# Changelog

All notable changes to Dialogue Framework.

Version numbers follow [semantic versioning](https://semver.org/): the
middle number changes when features are added, the last when only fixes are.

---

## [1.1.0]

Existing configs keep working untouched. Every new setting is optional and
falls back to the wording and appearance 1.0.0 had.

### Added

**Wording that follows the player's progress**
- A quest can carry its own `QuestListTexts` and `NoQuestsTexts` in
  `QuestText\*.json` — what the NPC says over their quest list, and what they
  say when they have nothing available. The mod uses the highest-numbered
  quest of that NPC's the player has **completed**, so an NPC greets a
  newcomer one way and a veteran another with no extra setup
- The obvious use for the "nothing left" line is pointing players at whoever
  hands out the next quest in a chain, which the mod had no other way to
  express
- `NoQuestsBackTexts` and `NoQuestsLeaveTexts` decide what the player can do
  from that step — carry on talking, or leave
- Matching tree-level `QuestListTexts`, `NoQuestsTexts`, `NoQuestsBackTexts`,
  `NoQuestsLeaveTexts` and `NoQuestsVoiceLineIDs` as the per-NPC fallback
- All of these are arrays with **one entry picked at random per visit**, so a
  handful of phrasings keeps a busy NPC from sounding scripted without
  writing wording for every one of their quests

**Presentation**
- Four built-in font and text-size presets — `DEFAULT`, `LIGHT`, `LARGE` and
  `COMPACT` — selectable from `MenuConfig.json` with no repacking. DayZ reads
  fonts only from layout files and has no runtime font API, so each style is a
  pre-built set of layouts that ships with the mod
- Optional hint icons on response buttons via `"ShowResponseIcons": true`: an
  exit door for anything that closes the menu, a shopping cart for opening the
  market, a speech bubble for anything that keeps the conversation going. They
  tint to `ResponseTextColor` and are off by default

**Quests**
- Item previews throughout quest detail, not just for rewards — items the
  quest **gives** you, items it **needs** from you, and items it **pays out**
  each get their own tile
- Daily and weekly quest cooldowns are now visible and enforced in dialogue.
  A quest still on cooldown shows its remaining time beside its name in the
  list, and its accept option is suppressed with the NPC saying when to come
  back, rather than letting players accept something that will be rejected

**Config handling**
- `QuestText\*.json` now upgrades itself the way `MenuConfig.json` already
  did. On the first start after an update, fields that didn't exist yet are
  written in at their defaults and everything already there is kept. The file
  is copied to `YourFile.json.v<old>.bak` first, and if that backup can't be
  made the file is left untouched and `LoadLog.txt` explains why
- New [Updating guide](docs/UPDATING.md): what updates itself, why dialogue
  trees deliberately don't, and the two ways to bring a tree up to date — open
  and save it in DialogueForge, or add the keys by hand. Leaving a tree alone
  is a valid choice; it keeps working, just without the new options

- **[DialogueForge](https://github.com/ABTT-ESK/DialogueForge)**, a free
  Windows editor for these config files, released alongside this version. It
  builds conversations visually with a branch map, previews the in-game menu
  in your own colours as you type, picks quests and NPCs by name from your
  Expansion configs, and checks every file for broken links. Entirely
  optional — the mod cannot tell whether a config was written by hand or by
  the editor

### Changed
- Layout files are now generated per font style by
  `tools/gen_layout_variants.py`. Sixteen layouts ship, but only four are
  hand-written; anything ending `_light`, `_large` or `_compact` is generated
  and committed, so server owners never run the script
- Repacking into server mod packs is now **explicitly allowed**, with the
  conditions written down in the README rather than left to guesswork
- Voice audio ships as `voice-pack-template.zip`, so server owners publish
  their own audio without repacking or re-signing this mod
- Documentation reorganised around what a server owner actually needs, with
  contributor-only material moved into `DEV_NOTES.md`

### Fixed
- A long spoken line was cut off mid-sentence on smaller panels. The NPC's
  line now scrolls, so the whole thing is readable however the window is
  sized, and it starts at the top on every new line rather than carrying the
  previous one's scroll position over
- Rewards and required items shared one strip headed "Reward", so a splint
  the player had to go and find was presented as a payout. They are now two
  separate groups with their own headings — **Required** above what the quest
  wants from you, **Reward** above what it pays out — and the required
  heading says "Given to you" when the quest hands you the items instead.
  The two groups sit side by side, required on the left and reward on the
  right, so each gets the panel's full height rather than sharing it
- Item tile text was cut off and the tiles hugged the left edge. The tile's
  contents were positioned in fixed pixels inside a tile whose height depends
  on your panel size, so the name and amount rows fell outside it. The tile
  is now proportional throughout, and both groups are laid out from the
  panel's real pixel size, and item names wrap onto a second line rather than
  running out of the sides. Tiles now grow to fill the space available instead
  of sitting tiny in an empty panel, wrap onto more rows when they don't fit
  across, and shrink rather than clip when space is tight. The response list takes
  whatever height is left, so changing `PanelWidth` or `PanelHeight` can no
  longer crop a preview
- Response icons rendered as solid blocks, cropped to the corner of the
  texture. The layout's `ImageWidgetClass` was missing `mode blend`,
  `"src alpha" 1` and `stretch 1` — without the first two the icon draws
  opaque, and without the third a 128x128 texture drawn into a 20x20 widget
  renders at native size and is clipped
- Response icons rendered as solid black blocks. Two causes: the shipped
  source art had black RGB beneath its transparent pixels, and the files were
  not named with the `_ca` suffix that tells Bohemia's texture tools to keep
  an alpha channel. Both fixed — the art is now white beneath the
  transparency, and the icons ship as `icon_*_ca.png`
- Response icons never appeared on clients even with `ShowResponseIcons`
  enabled. The menu is client-side and the setting was missing from the menu
  config's RPC, so the server read it correctly but never sent it and the
  client fell back to `false`. Added to both halves, and a one-line
  `[ICONS]` diagnostic now reports on the client what it received
- **The "no quests available" step was a dead end.** When an NPC had nothing
  to offer, its single button was created while the menu was still in
  quest-list mode, so every click hit a bounds check against an empty quest
  array and was silently swallowed — closing the window was the only way out.
  That step is now rendered as an ordinary dialogue node, so its buttons
  behave like any other. Servers that configure none of the new wording still
  get a working **Back** button, so it can never strand a player again

---

## [1.0.0] — Initial release

RPG-style branching dialogue for DayZ servers running DayZ Expansion.
Replaces the stock quest menu with proper conversation, and puts a
conversation in front of traders before the shop opens: NPCs greet you, you
choose what to say, conversations branch, and quests are offered, accepted
and handed in through dialogue instead of a list.

Everything is configured with plain JSON in the server profile folder. No
scripting, and no repacking for anything except voice audio.

### Conversations
- Branching dialogue trees authored entirely in JSON, one folder per NPC,
  with shared trees for NPCs that speak alike
- Responses lockable behind quest completion, for gating story topics until
  players have earned them
- Scrollable response lists with hover highlighting
- Options already chosen during a conversation are dimmed, fading your own
  text colour rather than switching to grey, so custom palettes hold

### Quests
- Live quest list per NPC, driven by real Expansion quest state
- State-aware quest detail: not started, in progress, and ready to hand in
  each read differently
- Per-quest accept, decline and turn-in wording, so every quest reads in its
  own voice rather than sharing generic lines
- Reward previews on every quest that pays out, visible before committing
  and again at turn-in
- Reward picker with 3D item previews for quests that let players choose:
  click to highlight, double-click to confirm

### Traders
- Traders hold a conversation before opening the shop, with trading always
  one click away
- Matched on world position, entity class name or trader definition name,
  with a configurable number of keys required to agree — outposts share
  trader definitions, entity classes get reused, and AI traders drift off
  their spawn points, so no single key is dependable alone
- Works with traders spawned at runtime, such as outposts unlocked by quests

### Presentation
- Window position, size and every colour configurable per server, with nine
  screen position presets plus free offsets
- Thin configurable border around the window
- `LayoutOverride` for servers wanting their own fonts and layout
- Config files carry a version and are upgraded in place when the mod adds
  new settings, keeping everything the owner had already set

### Voice
- Optional voice lines with an NPC talking animation synced to clip length
- Shipped as a separate voice pack mod so server owners publish their own
  audio without repacking or re-signing this one
- Startup audit listing every voice line with no matching sound set, so you
  always know what still needs recording
