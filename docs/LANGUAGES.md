# Running your server in more than one language

Short version: **you write your conversations once, and players read them in
their own language automatically.** Nobody has to press anything.

If your server is English-only, or you're happy writing in one language,
there is nothing here you need to do. Skip this whole page.

---

## How it works

Every player's **DayZ language setting** decides what they see.

| Player's DayZ language | You have a folder for it? | What they read |
|---|---|---|
| German | Yes | Your German translation |
| Russian | Yes | Your Russian translation |
| French | No | Your original wording |
| Anything | You have no translations at all | Your original wording |

That's the whole mechanism. No commands, no per-player setup, nothing to
explain to your community.

**Nothing can come out blank.** Translation happens line by line, so a line
you haven't translated shows your original wording — not an empty button.
That means a half-finished language is perfectly safe to put on a live
server.

---

## Which setup are you?

### "My server is English and so are my players"

Do nothing. You'll see an empty `Localization` folder appear in your profile
after a server start — ignore it.

### "My whole server is in one language, and it isn't English"

Just write your conversations in that language in DialogueForge and save.
No `Localization` folder, no settings.

Every player sees your language, because that's simply what your files say.
The only thing that follows each player's own language is the mod's own
furniture — `Reward:`, `Turn in:`, `Confirm`, `Cancel` — which already ships
translated into all 14 languages and needs nothing from you.

### "I want one server serving players in several languages"

This is the only setup with real work in it, and it's the one the rest of
this page is about.

---

## Setting up a second language, step by step

1. **Write the conversation normally** on the Dialogue tab and save it into
   your profile folder. Do this first — translations point at a conversation
   that already exists.

2. **Go to the Translations tab.** It lists every line in the conversation
   you have open: what the character says, every button, every alternate
   line, every story tree.

3. **Pick the language** at the top left.

4. **Work down the list.** Click a line, type the translation on the right,
   press Apply. The counter at the top right tells you how many are done.
   "Next one missing" jumps to the next gap.

5. **Save**, using the Save button along the top like any other tab.

6. **Repeat for each conversation**, and for each language.

7. **Restart the server, and fully restart your game client.** A reconnect is
   not enough — translations are sent when you connect.

Your files land here, one folder per language:

```
$profile\DialogFramework\Localization\
    german\
        npc_9999_dialogue.json
    russian\
        npc_9999_dialogue.json
```

Folder names have to be exactly one of these — they're DayZ's own names:

```
english   czech    german     russian    polish       hungarian   italian
spanish   french   chinese    japanese   portuguese   chinesesimp
```

`chinese` is traditional, `chinesesimp` is simplified.

**Your conversation files are never touched.** A translation is a separate
file listing only the lines you translated, so translating can't damage work
you've already done.

---

## Checking it worked

After a server restart, your server log should say:

```
[DialogueFramework] [LOC] Loaded 21 translated line(s) for 'german'.
[DialogueFramework] [LOC] Loaded 10 translated line(s) for 'russian'.
```

If a language is missing from that list, its folder name is probably spelled
differently from the list above, or the file didn't save where you think.

On a client, the log says which language that player ended up with:

```
[DialogueFramework] [LOC] Detected game language: german
[DialogueFramework] [LOC] Active translation: german (21 line(s)).
```

**Run "Check ALL config files" in DialogueForge before going live.** It
reports every translated line pointing at text you've since edited, plus how
many lines of each conversation are done. Which brings us to the one thing
that can quietly go wrong:

---

## The one gotcha: editing after translating

A translation points at a line by its position — "the second option on node
5". If you **reorder or delete** options in a conversation after translating
it, the translation on disk is still pointing at the old positions, and
players get the wrong line.

Nothing errors. It just quietly comes out wrong.

**So: after any round of editing, open the Translations tab and save the
translation again.** The tab re-reads the conversation every time you open
it and lines everything back up for you. And "Check ALL config files" will
tell you if you forgot.

Adding options at the *end*, or editing the wording of an existing option, is
harmless.

---

## Letting players choose for themselves

Some players want to read in a language other than the one they play the game
in — a Russian speaker who runs DayZ in English, for instance.

There's a small **settings button in the corner of the conversation window**,
next to the close button. It opens a screen of the player's own preferences —
language among them. Clicking a row cycles it. Their choices are saved on
their own machine and follow them to any server running the mod.

The Language row only appears if your server actually has translations
installed, so a single-language server never sees it.

**To switch it off**, untick "Let players pick their language" on
DialogueForge's Menu appearance tab, or set `"ShowLanguageButton": false` in
`MenuConfig.json`. Automatic detection carries on working — you're only
removing the manual choice. The other settings stay available.

> One wrinkle worth knowing: if a player *overrides* to a different language
> than their game is set to, the character speaks their chosen language but
> the mod's own labels (`Reward:`, `Confirm`) stay in their game language,
> because those come from DayZ itself. Players who simply leave it on
> automatic never see this.

---

## What if a player's language isn't detected?

Automatic detection reads the player's language from DayZ itself. If that
lookup fails on their machine — some clients don't have every language's
files installed — they fall back to English, and therefore to your original
wording.

Nothing breaks, and they can still pick a language manually. If you see
`Detected game language: english` in a client log belonging to someone who
plays in another language, that's what happened.

**How to tell it apart from a problem in your setup.** On some installs
DayZ's own menus translate perfectly while *mod* text does not — Expansion's
included. So checking the vanilla menu proves nothing on its own. The reliable
test is to look at **another mod's** text in that language:

- Expansion's text shows as raw `STR_EXPANSION_...` keys → that client can't
  read mod translations at all. Nothing you do to your server will change it,
  and it affects every mod they run, not just this one.
- Expansion reads fine but yours doesn't → that one is worth telling me about.

Either way the player is never stuck: the language row on the settings screen
doesn't use detection at all. It asks the server directly, so it keeps working
even on a client where automatic detection can't.
