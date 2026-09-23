# Roadmap

What's planned, what's being considered, and what the mod currently can't
do. Nothing here has a date attached.

---

## Next up

### Dialogue for AI you didn't spawn through this mod

AI spawned through the mod's own `AIPatrols.json` can talk: each patrol
carries an ID, dialogue locks onto the exact unit, and each unit can have its
own name and reputation. DialogueForge's Import tab copies your existing
Expansion patrols across for you.

What's still missing is talking to AI the mod didn't spawn — a guard placed
by another mod, or one spawned at runtime by something else. That needs a
custom interaction rather than a patrol link, since there's no menu to
intercept and no ID to match: a "Talk" action on friendly AI, plus a
server-side path to open the window, keyed on world position and entity class
the way traders are.

---

## Under consideration

**A faction's standing where no conversation names it.** Factions keep a
reputation as of 1.7.0, but the standing page in the book is built from the
conversations the player has been sent, so a faction's number is only shown
once one of its conversations points at it. A faction with no talkable AI at
all — one that exists purely to have an opinion about the player — keeps a
standing nothing displays. Showing those would mean sending the faction list
to the client, which nothing else needs yet.

**Reputation between factions.** A deed's effect on every faction is written
out per quest, which is what makes it expressive: any faction can react in any
direction. What it isn't is automatic — two factions that hate each other
don't know it, so every quest has to say so again. A relations table could
spread a change outward on its own, at the cost of being much harder to
predict.

---

## Known limitations

**Voice lines need their own Workshop item.** Sound sets are read from
packed addons at game start and the audio has to reach each player's
machine, so custom voice audio has to ship in a mod clients download. The
voice pack is deliberately tiny and separate so you republish only that, and
never re-sign the scripts mod. This is a DayZ constraint rather than a
design choice.

**Only the fonts the mod ships can be picked from `MenuConfig.json`.** The
engine reads a typeface from a `.layout` file and offers no runtime call to
change one — `SetFontSize` exists, `SetFont` does not. So every font on offer
has to be a layout baked into the mod ahead of time, which is what `Font` and
`TextSize` pick between. For a typeface that isn't on that list, use
`LayoutOverride` with your own layout file; colours, sizes and position still
come from the config.

**The picked font has to cover the language being read.** DayZ's own six
carry every letter of every language the mod ships, so they are safe
everywhere. A character a font lacks is drawn from DayZ's built-in fallback,
`sdf_NotoSansCJK-Light28` — which covers Chinese, Japanese and Western
European accents, but not Polish, Czech or Hungarian letters and not
Cyrillic. Those show as boxes in a font that lacks them.

**Trader dialogue is per configured trader, not per person.** Two traders
sharing a definition, an entity class *and* a position are
indistinguishable. In practice they differ on at least one, which is why two
agreeing keys is the recommended setting.

**The talking animation only works on AI NPCs.** Expansion's regular NPCs
and traders are deliberately inert — their `CommandHandler` runs no
animation command and keeps the body physics-inactive — so there's nothing
to drive a mouth movement from. Dialogue, quests, trading and voice lines
all work normally on them; the NPC simply doesn't move while a line plays.
Use the AI variants (`ExpansionQuestNPCAI*`, `ExpansionTraderAI*`) if you
want the animation.

**A conversation window can very occasionally come up empty.** The game
sometimes hands back a window layout with nothing inside it — no panel, no
speech, no buttons — and once it starts, every conversation in that session
does the same until the game is restarted. Nothing in the script API resets
it. The mod notices, closes the empty window before it can take a player's
controls, and builds it a second time in the standard font, which has so far
always worked; only if that also comes back empty is the player told to
restart. It is most easily provoked by forcing two windows open in the same
moment, which the mod now refuses to do.

**Config changes need a full client restart.** Trees and menu settings are
sent to players when they connect, so a reconnect won't pick up changes —
the client process has to restart.

**The reward strip assumes a reasonably sized window.** It reserves a fixed
share of the panel height, so a window shrunk well below the default leaves
the response list cramped when a quest has rewards to show.

---

## Not planned

**Replacing Expansion's market UI.** Dialogue leads into the shop and then
gets out of the way. Rebuilding the trading interface is a different mod.

**A quest system.** This presents Expansion's quests conversationally; it
doesn't define them. Quests stay where server owners already manage them.

**In-game dialogue editing.** Trees are JSON files, deliberately — they diff
in version control, generate from scripts, and don't need the mod running to
author.
