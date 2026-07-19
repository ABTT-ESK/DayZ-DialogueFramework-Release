# Roadmap

What's planned, what's being considered, and what the mod currently can't
do. Nothing here has a date attached.

---

## Next up

### Dialogue for any friendly AI

Right now a character can talk only if Expansion already opens a menu for
them — quest NPCs and traders. Everyone else is silent.

The goal is dialogue for **any** friendly AI: guards, camp residents,
civilians, anyone a server owner places. It's the most requested shape of
"more dialogue" and it also removes the dependency on hooking Expansion's
menus.

This needs a custom interaction rather than a hook, since there's no
existing menu to intercept — a "Talk" action on friendly AI, plus a
server-side path to open the window. Expect it to key on world position and
entity class, the same way traders do, since a plain AI has no quest or
trader identity to match against.

---

## Under consideration

**A "random reward" label.** Quests using Expansion's random reward pick
show every possible reward on the detail screen, which reads as "you get all
of these" when the player will receive one. Needs a label to say so.

**Item preview framing.** Reward thumbnails use a fixed camera, so unusually
large or small items can sit awkwardly in their tile.

**Localisation.** Dialogue text is written literally in the JSON. Supporting
`#STR_` keys would let multi-language servers translate conversations.

**Conditions beyond quest completion.** Responses can currently be gated on
a completed quest. Gating on faction, reputation, items carried, or time of
day would open up considerably more roleplay.

---

## Known limitations

**Voice lines need their own Workshop item.** Sound sets are read from
packed addons at game start and the audio has to reach each player's
machine, so custom voice audio has to ship in a mod clients download. The
voice pack is deliberately tiny and separate so you republish only that, and
never re-sign the scripts mod. This is a DayZ constraint rather than a
design choice.

**Fonts can't be changed from `MenuConfig.json`.** The engine only reads a
font from a `.layout` file and offers no runtime call to change one. Use
`LayoutOverride` with your own layout file — colours, sizes and position
still come from the config.

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
