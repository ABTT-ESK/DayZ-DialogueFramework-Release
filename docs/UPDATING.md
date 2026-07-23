# Updating

**Nothing you have written will break, and nothing will be overwritten.**

When this mod adds new settings, your existing configs keep working exactly as
they did. The only thing you miss out on by doing nothing is the new options
themselves — the mod falls back to its built-in wording for anything you
haven't filled in.

You never have to start a config over because of an update.

## What happens on its own

Two of the three config types bring themselves up to date the first time the
server starts on a new version:

| File | Updates itself? |
|---|---|
| `MenuConfig.json` | **Yes** |
| `QuestText\*.json` | **Yes**, after making a backup |
| `Dialogues\...\*.json` (dialogue trees) | **No** — see below |

For the two that do: your file is loaded first, so every value you wrote is
kept. Only fields that didn't exist yet are added, at their defaults. The
server log names the file it upgraded.

Quest text files are copied aside before being rewritten, to
`YourFile.json.v0.bak` in the same folder. The number is the version it was
copied *from*, so a later update never overwrites an earlier backup. **If the
backup can't be made for any reason, the file is left completely untouched**
and `LoadLog.txt` says so — the mod would rather skip an upgrade than risk
your wording.

You can delete the `.bak` files once you're happy. The mod ignores them.

## Dialogue trees: two ways to catch up

Dialogue trees are deliberately left alone. They're the files with the most
work in them, and keeping the mod read-only on them means it can never damage
a conversation you spent hours on. There's no automatic step here on purpose.

Your trees keep working untouched. When you want the new tree-level options,
pick either route.

### The easy way — open and save in DialogueForge

1. Open [DialogueForge](https://github.com/ABTT-ESK/DialogueForge) — the free
   config editor for this mod — and point it at your server profile folder
2. Open the dialogue file
3. Save it

That's it. DialogueForge always writes the complete, current set of fields, so
saving an old file brings it up to date and leaves your conversation exactly
as it was. The new fields appear ready to edit on the relevant tab.

Do this once per NPC file you care about. Files you never open keep working
with built-in wording.

### The manual way — add the keys yourself

Open the `.json` in a text editor and add the missing keys anywhere alongside
the existing top-level ones, before `"Nodes"`. Any you leave out fall back to
built-in wording, so a partial file is fine.

The current tree-level fields are listed in
[CONFIG_REFERENCE.md](CONFIG_REFERENCE.md), and
[CHANGELOG.md](../CHANGELOG.md) says which release added what.

```json
{
    "ID": 1,
    "RootNodeID": 1,
    "GreetingVoiceLineIDs": [],
    "FarewellVoiceLineIDs": [],

    "QuestListTexts": [],
    "NoQuestsTexts": [],
    "NoQuestsBackTexts": [],
    "NoQuestsLeaveTexts": [],
    "NoQuestsVoiceLineIDs": [],

    "Nodes": [ ... ]
}
```

Empty arrays are exactly the same as leaving the key out — they just make the
option visible next time you edit the file.

## After any update

1. Restart the server
2. Read `Dialogues\LoadLog.txt` — it lists what loaded and anything that was
   wrong with it
3. Players need a **full client restart**, not just a reconnect, since trees
   and appearance are sent when they connect

A file the mod can't parse never stops the server. That one NPC falls back to
no custom dialogue until you fix it, and everyone else carries on.

## If something looks wrong

- **A new option isn't doing anything** — check it's actually in your file.
  If the key isn't there, the mod uses its built-in wording.
- **A quest text file didn't gain the new fields** — check `LoadLog.txt` for a
  backup failure. Usually a read-only file or a full disk.
- **You want to undo an upgrade** — rename the `.bak` back over the original.
  The mod will simply upgrade it again on the next start, so change the mod
  version back too if that's what you're after.
