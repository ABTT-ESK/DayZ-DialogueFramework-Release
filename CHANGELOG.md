# Changelog

All notable changes to Dialogue Framework.

Version numbers follow [semantic versioning](https://semver.org/): the
middle number changes when features are added, the last when only fixes are.

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
