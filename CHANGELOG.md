# Changelog

All notable changes to Dialogue Framework.

Version numbers follow [semantic versioning](https://semver.org/): the
middle number changes when features are added, the last when only fixes are.

---

## [1.6.0]

### Added
- **Long lines.** A spoken line or translation can be any length. The game
  cuts a line at about 1000 characters (fewer in Russian or Chinese), so
  longer ones are stored in pieces the mod joins back together. DialogueForge
  1.6.0 does the splitting for you. Options and quest wording still need to
  stay short.
- **Reputation updates.**
  - **Icons on the reputation marker.** Each rank can show a face or a thumb
    after the NPC's name: `"Icon": "HAPPY"`, `"NEUTRAL"`, `"ANGRY"`,
    `"THUMBUP"`, `"THUMBSIDE"` or `"THUMBDOWN"`. Icons take the name's
    colour. A rank can show an icon, a word, both or neither.
  - **A standing page in Expansion's book** showing where the player stands
    with every character: name, rank and number. It costs the server nothing
    and hides itself on servers with no reputations set up.
  - **Your own wording for the book page:** `"BookTabName"`, `"BookPageTitle"`,
    `"BookColumnName"`, `"BookColumnStatus"` and `"BookColumnReputation"`.
    Leave one empty and it uses the mod's translated default.
  - **Set `"ReputationMax"`** on a character and the book page shows
    *10 / 100* instead of *10*. Display only.
  - **A pop-up naming who a choice pleased or annoyed**, like *Yefim +5* or
    *Smitty −2*. Set the server default with
    `"ShowReputationNotifications"`; players can switch it under
    **Settings**.
  - **Reputation for finishing a quest.** Add `"RepOnComplete"` to a quest's
    `QuestText` entry: `INCREASE`, `DECREASE` or `SET` on any reputation,
    applied when the quest is handed in.
- **Fonts.** `"Font"` picks from thirteen typefaces and `"TextSize"` picks
  `NORMAL`, `LARGE` or `COMPACT`. Any pairing works, with no repacking.
  - **DayZ's own six:** `DEFAULT`, `LIGHT`, `BLACK`, `METRON`, `SERIF` and
    `ETELKA`. They cover every language the mod ships.
  - **Seven new:** `INTER`, `GARAMOND` and `NOTOSERIF` cover Russian;
    `CONDENSED`, `ZILLA`, `TYPEWRITER` and `BLACKOPS` don't. All are
    open-licence, and their licences ship with them.
  - The old `"FontStyle"` still works and converts itself on the first start.
- **Scroll speed.** `"ScrollSpeed"` in `MenuConfig.json`, from 0.25 to 4.0.
  Players can set their own under **Settings**, and theirs wins.
- **The settings button is a cog** instead of an asterisk.

### Fixed
- **A conversation window could open by itself and lock the player's
  controls** (reported on 1.5.0). It now only opens when the player talks to
  someone, and only one at a time.
- **Controls can't be left switched off.** If a window closes unexpectedly,
  they come back within half a second.
- **Escape closes the conversation** from any screen.
- **Long speeches can be read to the end.** The last lines no longer sit out
  of reach.
- **The mouse wheel scrolls speeches smoothly.**
- **No more gap between the NPC's line and the options.** The options now
  sit right under what the NPC says.
- **The player's text size applies to the whole window**, not just the
  options.
- **The reputation icon no longer overlaps the name** in wider fonts.

---

## [1.5.0]

### Added
- **An option can be shown only while a quest is in a particular state.** Pick a
  quest and one of *not started yet*, *in progress*, *ready to hand in* or
  *completed*, and the option only appears then. The obvious use is a hand-in
  button that stays hidden until the player has actually finished the job,
  rather than sitting there through the whole conversation — but it also covers
  an NPC who only asks how you are getting on while you are actually on it.
- **Player-to-player traders can hold conversations.** P2P traders never used
  the ordinary market menu, so they were invisible to the mod. They now open a
  conversation the same way any other trader does, and `OPEN_TRADER` sends the
  player through to the P2P market. Attach one with `P2PTraderIDs` — the id
  from `expansion\p2pmarket\P2PTrader_<n>.json`, which is unique per trader.
  - Like an ordinary trader they have no quest-giver identity, so
    `SHOW_QUEST_LIST` shows nothing there; give quests with `OFFER_QUEST` and
    take them back with `TURN_IN_QUEST`.
- **A worked example of a trader who gives quests.**
  [`examples/TraderQuestChain`](examples/TraderQuestChain) is one trader who
  talks, keeps the shop open and hands out two quests in a row, ready to copy
  onto a server. [`docs/TRADER_QUEST_CHAIN.md`](docs/TRADER_QUEST_CHAIN.md)
  explains every field, how to build the same thing in DialogueForge, how to
  place a new trader, and how to move an existing Expansion setup over —
  including the two quest settings that catch everyone out: an empty
  `QuestGiverIDs` starts the quest for every player at login, and an empty
  `QuestTurnInIDs` hands it in without the trader.

---

## [1.4.0]

### Added

- **An option can now hand a finished quest back in.** `TURN_IN_QUEST` takes a
  quest in the response's `QuestID`, the same way `OFFER_QUEST` and
  `ACCEPT_QUEST` already did, so the hand-in can happen at any character
  instead of only inside the quest list the mod builds itself. If the quest
  gives a choice of reward, the reward picker opens as usual. Left without a
  `QuestID` it behaves exactly as before.
- **A trader can now run a whole quest, start to finish.** Traders have no
  quest-giver identity, so they can never show a quest list — but they can
  give a quest with `OFFER_QUEST` and take it back with `TURN_IN_QUEST`, one
  option per quest. Add `RequiredQuestID` and `HideAfterQuestID` and the
  options swap over on their own as the player gets further in.
- **A refused hand-in now says which reason it is** — hasn't taken the quest,
  hasn't finished it, or has already handed it in. Nothing changes in any of
  those cases, and all three messages are translated into all 14 languages.

### Fixed

- **Long lines were mis-measured in every language that isn't plain English.**
  The game counts text in bytes, and a Russian letter costs two of them while a
  Chinese one costs three — but the window was sizing text as though every byte
  were a letter. Speaker areas reserved roughly double the height they needed in
  Russian and treble in Chinese, response buttons shrank their font to the
  minimum for no reason, and a long item name on a reward tile could be cut in
  the middle of a character and show a broken glyph. All three now measure
  characters.
- **Long lines are cut off at 1023 bytes, and nothing used to tell you.** The
  game reads at most 1023 bytes of any one line and silently discards the rest.
  Because that is a limit in bytes rather than letters, it lands at roughly 1000
  English characters but only 500 Russian or 340 Chinese — so a line that was
  fine when you wrote it can lose its ending once translated. DialogueForge now
  counts the bytes as you type and warns before you cross it, and `LoadLog.txt`
  names any line that arrives already cut.
- **Fixed a crash when a conversation closed.** Ending a conversation through a
  farewell option could throw a null-pointer error in the client log as the
  window tore down. Harmless to play through, but it is gone.
- **`SHOW_QUEST_LIST` on a trader or a talkable AI showed every quest on the
  server.** Neither has a quest-giver ID to narrow the list against, so the
  player was offered every quest they were eligible for, in no order. It now
  shows nothing there and writes the reason to the server log, pointing at the
  quest actions to use instead. Quest NPCs are unaffected.

---

## [1.3.0]

### Added

- **Your dialogue can now be written in more than one language.** Each player
  reads the conversation in their own DayZ language automatically, and can pick
  a different one from the player settings screen.
  Translations live in `$profile:\DialogFramework\Localization\<language>\`,
  one folder per language, and your tree files are never touched — a
  translation is a separate overlay listing only the lines you've translated.
  Anything you haven't translated (or a language you don't ship) falls back to
  the wording in the tree, so a half-finished language is safe to go live with.
  Supported folder names: `english`, `czech`, `german`, `russian`, `polish`,
  `hungarian`, `italian`, `spanish`, `french`, `chinese`, `japanese`,
  `portuguese`, `chinesesimp`.
- **DialogueForge has a Translations tab** that writes those files for you —
  pick a language, work down the list of every line in the open tree or quest
  wording file, and save. It keeps the keys straight so you never hand-edit
  them.
- **The mod's own wording is translated into all 14 languages** — `Reward:`,
  `Turn in:`, `Confirm` / `Cancel`, the reputation marker, and the built-in
  fallback response lines (`I'll take it.`, `Not interested.`, and the rest).
  This needs nothing from you: it follows each player's game language.
- **A player settings screen**, reached from a small button in the corner of
  the conversation window next to the close button. Everything on it belongs
  to that player, is saved on their own machine, and follows them to any
  server running the mod. Each row cycles when clicked:
  - **Language** — read the conversation in a language other than the one
    they play the game in. Only appears if your server has translations.
  - **Window position** — move the window to any of the nine spots if yours
    covers something they'd rather see. Your size and colours are kept.
  - **Text size** — 80% to 150% of whatever your settings produce, for
    players whose screen makes the default hard to read.
  - **Button icons** — show or hide the hint icons regardless of your setting.
  - **Reset to the server's settings** — appears once they've changed
    anything, and puts everything back.
- **`ShowLanguageButton` in `MenuConfig.json`** (default on) controls whether
  the Language row appears on that screen. The rest of the settings are always
  available. Language only ever shows when the server has translations.
- **A response can now offer or hand over a specific quest.** Two ways, both
  set with the response's new `QuestID` (the "Quest to use" picker in
  DialogueForge):
  - **`OFFER_QUEST`** opens that quest's own offer screen -- its description,
    what it needs, what it pays, and accept/decline -- from any option you
    write. This is the one to reach for.
  - **`ACCEPT_QUEST`** now takes a quest too, and hands it straight over with
    no offer screen.

  Previously `ACCEPT_QUEST` only did anything inside the live quest-detail
  step the mod builds itself; anywhere else it silently closed the window.
  It now says so in the log instead of failing quietly. (Reported on the
  Workshop -- thank you.)
- **Players are told on screen when an option doesn't work**, instead of the
  window just closing. A short pop-up in Expansion's own toast style says
  either *"You can't take that quest right now"* (normal gating, always
  shown) or *"That option isn't set up correctly"* (a config mistake). The
  detailed reason still goes to the log. Turn the second kind off with
  `"ShowErrorNotifications": false` in `MenuConfig.json` if you'd rather
  players never saw it.
- **Neither quest action can hand out a quest the player shouldn't have.**
  `OFFER_QUEST` and `ACCEPT_QUEST` check the same rules Expansion applies to
  its own quest list before starting anything — already completed, already in
  progress, on cooldown, prerequisites unmet, achievement quests. The offer
  screen drops its accept button when the player isn't eligible, and
  `ACCEPT_QUEST` refuses and says why in the log. The one rule deliberately
  skipped is "is this NPC the giver", since pointing a player at another
  character's quest is what these actions are for.
- **`HideAfterQuestID` on a response**, the mirror of `RequiredQuestID`: the
  option disappears once that quest is completed. Lets a line retire itself,
  so "go and see Mikhail about the mill" stops being offered once the mill
  is dealt with. Both can be used on the same option to give it a window:
  appears after one quest, gone after another.
- **A quest flow report** in DialogueForge, on the Server files tab. Writes
  `QuestFlow.txt` listing every quest your conversations mention, by quest
  and by conversation, so you don't have to remember which option shows
  after 102 and hides after 105. It also flags the mistakes that are
  invisible in game: an option that shows and hides on the same quest and so
  can never appear, an `OFFER_QUEST` with no quest picked, and any quest id
  that isn't in your quest folder. The same checks run in "Check ALL config
  files" and in the server's own `LoadLog.txt`.
- **`ScaleTextWithPanel` in `MenuConfig.json`** makes response text follow your
  panel width, so a scaled-up menu gets proportionally bigger text and a
  compact one still shows every option in full, just smaller.
  **It ships switched off** so nothing about your current menu changes when you
  update — turn it on in `MenuConfig.json`, or tick "Option text scales with
  panel size" on DialogueForge's Menu appearance tab, if you want it.

- **The screens now have agreed names**, listed in
  [`docs/SCREENS.md`](docs/SCREENS.md). There are eleven, and each is named
  after the config field that controls its wording -- `TurnInTexts` belongs to
  the quest turn-in screen, `NoQuestsTexts` to the no-quests screen. Three
  different sets of names were in use before (the quest wording tab, the live
  preview and the logs all disagreed), which made it hard to tell anyone where
  to look. DialogueForge's tab headings and preview titles now use these names,
  and the client log announces each screen as it opens:
  `[DialogueFramework] [SCREEN] Quest turn-in screen`

### Fixed

- **Options that grew onto a second line drew on top of the option below.**
  The list only laid itself out once, at the old fixed height, so a button
  that grew afterwards overlapped its neighbour. The list is now re-flowed
  after it is built.
- **The mod's own wording falls back to English if `stringtable.csv` is
  missing from the built PBO**, instead of showing raw keys like
  `STR_DIALOGUEFW_HEAD_REWARD` to players. It says so once in the client log
  so the packing mistake is obvious. (Automatic language detection still
  cannot work without the table — the player settings screen is unaffected
  and still switches languages by hand.)
- **Long response options are no longer cut off.** Response buttons were a
  single line of text in a fixed-height button, so anything wider than the
  button was clipped mid-sentence — while the same text on the NPC's own line
  wrapped fine. Options now wrap and the button grows to fit, up to three
  lines; past that the text shrinks instead, down to a readable floor. Nothing
  is ever truncated, and no scrollbars appear inside a button. Short options
  look exactly as they did.
  - There was never a character limit to raise: it was a *pixel width* limit,
    which is why it moved around with your panel width and font style.

### Changed

- `MenuConfig.json` is version 6. Existing files are upgraded in place on the
  next server start with the new fields added and everything else left alone.
- `dialogue_response_button.layout` now uses a `MultilineTextWidgetClass` for
  the option label. **If you ship a custom `LayoutOverride`**, copy that change
  across or your options will keep being cut off — a plain `TextWidgetClass`
  cannot wrap.

---

## [1.2.1]

### Fixed
- **Quest item previews now show "Turn in:" and "Reward:" labels** so players can
  tell what a quest is asking for versus what it pays out. (The labels existed but
  were collapsed to zero height by a sizing bug and never showed — now fixed, and
  the reward strip was previously unlabelled entirely.)
- **`%1` now fills in the player's name throughout the window.** Placeholders
  like `%1` in NPC lines, response buttons, greetings, and Expansion quest
  titles/descriptions are replaced with the player's name (and localised
  `#STR_...` text is resolved), the same way Expansion's own quest menu does it —
  previously they showed up as a raw `%1`.

---

## [1.2.0]

### Added
- **Custom AI factions.** Make your own factions instead of being limited to
  Expansion's built-in ones. Each faction has a name, a loadout, how it treats
  players (walk up and talk / defends itself when you raise a weapon / hostile on
  sight), and a list of other factions it won't fight. Up to 32, all built in
  DialogueForge's new Factions tab, then assigned to your talkable patrols.
  (Making a faction friendly with one of Expansion's *built-in* factions only
  works where that built-in's own rules allow it — an Expansion limitation.)
- **AI can calm back down after a conversation turns them hostile.** When a
  choice makes a patrol turn on the player, they can stand down again — when the
  player dies, puts their weapon away, puts their hands up, or gets far enough
  away (each one optional, with a distance you set). You can also let certain
  patrols hold a grudge: after enough bad encounters a player becomes permanently
  hostile to them — remembered by the whole faction, just that patrol, or either.
  Set a server-wide default and override it per patrol, so one patrol never
  forgives after two bad runs while another takes six and a third never holds a
  grudge at all. Only AI angered through dialogue are affected; normal AI combat
  is untouched. All set in DialogueForge's Global AI settings and AI patrols tabs.
  (Re-attacking a marked player on sight across respawns is coming in a follow-up.)
- **Reputation & story flags that follow the player everywhere.** Choices can
  raise, lower or set hidden values that stick to the player and are shared
  across every NPC — so what you say to one character changes how others treat
  you. Use them to unlock or hide individual responses, change a greeting, or
  even swap in a whole different conversation. It's all done in DialogueForge by
  picking characters and factions from dropdowns — no codes to type. Standing is
  saved per player and survives relogs.
- **Per-character reputation, shown in the window.** Give a character its own
  reputation, so a player can be trusted by one character and disliked by
  another. When set, the dialogue window shows the player's standing next to the
  character's name — a word you choose (Hostile / Wary / Friendly / Trusted…) or
  a number — updating live as choices change it.
- **Stop reputation farming.** Any response can be limited to a set number of
  uses per player — once, or any number you like. After that it disappears, so
  players can't grind standing by spamming the same choice.
- **Talk to Expansion AI, and recruit them.** Spawn talkable patrols through the
  mod (set up in DialogueForge's AI patrols tab); each patrol carries an ID so a
  conversation locks onto the exact unit — two guards in one patrol can say
  completely different things. Walk up and a **Talk** prompt opens a full
  branching conversation, voice lines and all. A response can recruit the AI into
  the player's group (respecting Expansion's own recruit settings, and optionally
  locked behind a completed quest), and the AI holds still while you talk and
  resumes its patrol afterwards. Another response can turn the whole patrol
  hostile — for encounters that can go sideways. **This adds the Expansion AI
  module as a requirement.**
- **Multiple conversations per NPC, unlocked by quests.** A character can have
  several complete conversations, each opening once the player has finished a
  particular quest — so an NPC's dialogue grows with the story instead of piling
  every option onto one screen. Built as clean, separate conversations in
  DialogueForge; NPCs that use only one behave exactly as before.

---

## [1.1.1]

Shout out to "Too Often Played" for the bug reports that helped push most fixes this round.

### Added
- **Per-screen "back to the conversation" buttons.** Previously the only way
  out of a quest screen was the X or an answer that ends the chat, and the
  tree's back-button wording was read only on the "nothing available" step.
  Every quest screen — the quest list, the offer, in-progress and turn-in
  screens, and the no-quests step — now shows its own back button that returns
  to the greeting without ending the conversation. Each screen has its own
  wording so an NPC can say something different on each (`QuestListBackTexts`,
  `OfferBackTexts`, `InProgressBackTexts`, `TurnInBackTexts`, and the existing
  `NoQuestsBackTexts`). Set NPC-wide defaults on the Quest talk tab, or override
  per quest on the Quest wording tab; resolution is per-quest → NPC default →
  none. The cooldown screen shares the offer wording. QuestText files add these
  fields automatically on the next server start (file version 2). All optional:
  a screen with nothing set shows no back button, exactly as before.
- **Quests with a "hand in any one of these" objective could not be turned in
  through dialogue.** Handing a quest in from the dialogue window always told
  Expansion the collection index was `-1`. For a collection objective with
  `NeedAnyCollection` set, the server rejects that index and aborts the
  turn-in, which surfaced only as an Expansion "Quest turn-In failed /
  Something went wrong" toast with nothing in the logs. The same quests turned
  in fine through the stock menu, which resolves the index for you, so the
  fault looked like it belonged to Expansion rather than to an active NPC
  dialogue file. The dialogue window now resolves a real collection index
  before turning in: it picks the only option automatically when a
  `NeedAnyCollection` objective defines a single collection or the player has
  satisfied exactly one, and shows an item picker — the same preview tiles as
  the reward picker — when more than one collection is satisfied, chaining into
  the reward picker afterwards if the quest also lets the player choose a
  reward. Quests without a `NeedAnyCollection` objective are unaffected
- **Item previews went blank after reopening the dialogue.** Closing the window
  — with the X, by handing a quest in, or by the game closing it when the quest
  log opened — left the preview items alive a moment longer than the menu, so
  their slots in the engine's limited preview pool stayed taken and the next
  quest you looked at showed blank pictures. The previews are now released the
  instant the window hides, and only after their widgets are gone, so they no
  longer pile up.

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
