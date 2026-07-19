# Dialogue Framework

An RPG-style branching dialogue window for DayZ servers running
[DayZ Expansion](https://github.com/salutesh/DayZ-Expansion-Scripts).

Replaces Expansion's stock quest menu with a proper conversation: the NPC
greets you, you pick what to say, conversations branch, and quests are
offered, accepted and handed in through dialogue instead of a list.

**Everything is configured with plain JSON files in your server profile
folder.** No scripting, and no repacking for anything except voice audio.

## Features

- Branching dialogue trees, one folder per NPC
- Traders can talk too — conversation first, then the shop
- Write your own small talk, lore and quest wording — nothing is generic
- Per-quest accept / decline / turn-in wording, so every quest reads in its
  own voice
- Responses can be locked behind quest completion
- Live quest list per NPC, always reflecting real quest state
- Reward picker with item previews for quests that let players choose
- Window position, size and colours configurable per server
- Optional voice lines, with a talking animation on AI NPCs

Source and releases: [github.com/ABTT-ESK/DayZ-DialogueFramework-Release](https://github.com/ABTT-ESK/DayZ-DialogueFramework-Release)

## Requirements

- [DayZ Expansion](https://github.com/salutesh/DayZ-Expansion-Scripts)
  (Core + Quests)
- Community Framework (CF)

## Installation

1. Add the mod to your server and to your client-side mod list.
2. Start the server once. It creates `$profile:\DialogFramework\` with a
   working example and a field reference.
3. Edit those files, restart, and check `Dialogues\LoadLog.txt`.

## Configuration

Everything lives in your server profile folder:

```
$profile:\DialogFramework\
  MenuConfig.json              # Window position, size, colours
  Dialogues\
    README.txt                 # Field reference, written for you
    LoadLog.txt                # What loaded, and what was wrong with it
    NPC_9999\Dialogue.json     # Working example
    NPC_<id>\*.json            # One folder per quest NPC
    Trader_<name>\*.json       # One folder per trader definition
    Shared\*.json              # Trees shared by several NPCs
  QuestText\
    Example.json               # Per-quest button wording
```

**Check `LoadLog.txt` after every restart.** It reports duplicate node IDs,
missing root nodes and broken links. A bad file never stops the server — that
NPC just falls back to no custom dialogue until you fix it.

Dialogue and appearance changes need a **server restart and a full client
restart** (not just a reconnect), because both are sent to players when they
connect.

## Documentation

| Guide | What's in it |
|---|---|
| [Config reference](docs/CONFIG_REFERENCE.md) | Every field of every config file, annotated. **Start here** |
| [Dialogue trees](docs/DIALOGUE_TREE_GUIDE.md) | Writing conversations: branching, quest gating, quest wording |
| [Window appearance](docs/MENU_CONFIG_GUIDE.md) | Position presets, colours, custom layouts |
| [Voice lines](#voice-lines) | Audio format, naming, publishing your voice pack |
| [Developer notes](docs/DEV_NOTES.md) | Only if you're modifying the mod itself |
| [Changelog](CHANGELOG.md) | What changed in each release |
| [Roadmap](ROADMAP.md) | Planned features and known limitations |

Ready-to-use files are in [`examples/`](examples/).

## Voice lines

Voice audio ships as a **separate mod you publish yourself**, so you never
repack or re-sign this one, and you stay on the upstream version for
updates.

[`voice-pack-template.zip`](voice-pack-template.zip) in this repo is a
complete, working skeleton. Unzip it somewhere outside this folder, then:

1. Drop your `.ogg` files into `data/audio/dialogues/`
2. Run `generate_soundsets.bat` — it rebuilds `config.cpp` from whatever
   files it finds, so you never hand-write config syntax. **The filename
   becomes the voice line ID.**
3. Edit `mod.cpp` with your own name
4. Pack it, sign it with your own key, and publish it as your own Workshop
   item

Full instructions, audio format requirements and the signing walkthrough are
in the template's own `README.md`.

The mod works fine with no voice pack at all — lines are simply silent, and
the client log lists every sound set you haven't recorded yet.

### Why a separate mod

Sound sets are read from packed addons at game start, and the audio has to
reach each player's machine, so custom voice lines must ship in a mod
clients download. Keeping it separate means your audio never forces a
rebuild of the scripts.

## Building from source

One Addon Builder pass. Point `SourceDir` at the repository root — the
`$PBOPREFIX$` there resolves packed paths to `DialogueFramework/Scripts/...`
and `DialogueFramework/GUI/...`, which is what every `files[]` entry and
layout path is written against.

The voice pack template builds the same way from its own folder once
unzipped, using its own prefix.

## Credits

Built on [DayZ Expansion](https://github.com/salutesh/DayZ-Expansion-Scripts)'
quest system. The NPC talking animation technique is credited to
[ZenExpansionAudioAI](https://github.com/ZenarchistCode/ZenExpansionAudioAI),
which was used as a reference for the underlying engine calls — this mod
implements it independently and does not require it.

## Repacking

**Repacking into a server mod pack is allowed.** 

Please:

- Keep the `LICENSE` file in your repack
- Credit Dialogue Framework in your server or mod pack description
- Don't present it as your own work

One thing to be aware of rather than a rule: **support covers the official
Workshop version only.** A repack is a frozen copy — when a fix ships,
subscribers get it automatically and your repack doesn't. If you hit a bug,
reproduce it on the current official version before reporting it, otherwise
there's a good chance it's already fixed.

The same applies to the voice pack template, which is meant to be
republished under your own name.

## Licensing

**This mod is MIT licensed** — see [`LICENSE`](LICENSE). Every line in this
repository is original work. Fork it, modify it, repack it, ship it on your
own server; attribution is appreciated but not required.

**DayZ Expansion is licensed separately and is not included here.** It's
distributed by the DayZ Expansion Mod Team under
[CC BY-NC-ND 4.0](https://creativecommons.org/licenses/by-nc-nd/4.0/), and
this mod contains none of their code. It extends their classes and calls
their API at runtime, the way any plugin extends the thing it plugs into,
and requires Expansion to be installed separately.

Two things to keep in mind if you fork this:

- **Don't copy Expansion source into your fork.** Their NoDerivatives terms
  cover redistributing their material. Referencing their API is fine;
  pasting their code into your repository is not.
- **Don't republish Expansion itself.** Their team requires a licence for
  that, and bundling their PBOs alongside yours would need their
  permission.

The same applies to the voice pack template: MIT, and the `.ogg` files you
add are yours.

None of the above is legal advice. If you intend to charge for something
built on this — Expansion's licence is NonCommercial — talk to the Expansion
team directly rather than relying on a README.
