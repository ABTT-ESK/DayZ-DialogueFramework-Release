# Menu Appearance Guide

> **Colours are easier to pick than to type.**
> [DialogueForge](https://github.com/ABTT-ESK/DialogueForge) has colour
> pickers and a live preview for all of this, and always writes the values in
> the right order — which removes the most common cause of an invisible
> dialogue box.


The dialogue window's position, size and colours are server-configurable —
no repacking, no script editing.

## Where the file lives

```
$profile:\DialogFramework\MenuConfig.json
```

It's created with default values on first server start, so start the server
once, then edit the file it writes. Changes take effect after a **server
restart**, and clients need a **full restart** (not just a reconnect) to
receive them, because the config is pushed on connect alongside the trees.

## Why this is a server file, not a client one

The menu is drawn client-side, but you configure it server-side so *your*
look applies to everyone who joins — players can't opt out or override it.
The config is sent to each client on connect in the same RPC as the
dialogue trees.

## Full file

```json
{
  "Position": "BOTTOM_CENTER",
  "PanelWidth": 0.6,
  "PanelHeight": 0.42,
  "OffsetX": 0.0,
  "OffsetY": 0.0,
  "EdgeMargin": 0.03,
  "BackgroundColor": [230, 0, 0, 0],
  "ResponseBackgroundColor": [200, 0, 0, 0],
  "HoverBorderColor": [255, 255, 215, 0],
  "SpeakerNameColor": [255, 255, 255, 255],
  "SpeakerTextColor": [255, 255, 255, 255],
  "ResponseTextColor": [255, 220, 220, 220],
  "RewardSelectedColor": [230, 90, 70, 20],
  "WindowBorderColor": [255, 255, 255, 255],
  "WindowBorderThickness": 2,
  "VisitedResponseOpacity": 0.4,
  "FontStyle": "DEFAULT",
  "Font": "DEFAULT",
  "TextSize": "NORMAL",
  "ShowResponseIcons": false,
  "ShowLanguageButton": true,
  "ScaleTextWithPanel": false,
  "ShowErrorNotifications": true,
  "ScrollSpeed": 1.0,
  "ShowReputationNotifications": true,
  "BookTabName": "",
  "BookPageTitle": "",
  "BookColumnName": "",
  "BookColumnStatus": "",
  "BookColumnReputation": "",
  "LayoutOverride": ""
}
```

`ConfigVersion` is written in too — leave it alone. It's how the mod knows to
add new settings to your file when you update, keeping everything you wrote.

## Position

| Field | Meaning |
|---|---|
| `Position` | One of the nine presets below |
| `PanelWidth` / `PanelHeight` | Size as a fraction of the screen (`0.6` = 60% of screen width) |
| `OffsetX` / `OffsetY` | Nudge applied *after* the preset resolves, in screen fractions. Use this to clear your own HUD elements |
| `EdgeMargin` | How far the edge-hugging presets sit from the screen edge |

Presets: `TOP_LEFT`, `TOP_CENTER`, `TOP_RIGHT`, `CENTER_LEFT`, `CENTER`,
`CENTER_RIGHT`, `BOTTOM_LEFT`, `BOTTOM_CENTER`, `BOTTOM_RIGHT`.

Default is `BOTTOM_CENTER`, which keeps the NPC's face visible above the
window. `CENTER` covers whoever you're talking to, which is usually not
what you want.

A worked example — a narrow panel tucked bottom-right, nudged up slightly:

```json
"Position": "BOTTOM_RIGHT",
"PanelWidth": 0.34,
"PanelHeight": 0.5,
"OffsetY": -0.04
```

## Colours

Every colour is `[Alpha, Red, Green, Blue]`, each component `0`–`255`.
Alpha `0` is fully transparent, `255` fully opaque.

| Field | What it tints |
|---|---|
| `BackgroundColor` | The main window panel |
| `ResponseBackgroundColor` | Each response button, and unselected reward tiles |
| `HoverBorderColor` | The border drawn on the option under the mouse |
| `SpeakerNameColor` | The NPC's name |
| `SpeakerTextColor` | The line of dialogue |
| `ResponseTextColor` | Response button text |
| `RewardSelectedColor` | The highlighted reward tile |
| `WindowBorderColor` | The thin frame around the window |

Example — a green terminal look:

```json
"BackgroundColor": [235, 5, 15, 5],
"ResponseBackgroundColor": [210, 10, 25, 10],
"HoverBorderColor": [255, 80, 255, 120],
"SpeakerNameColor": [255, 140, 255, 170],
"SpeakerTextColor": [255, 200, 255, 210],
"ResponseTextColor": [255, 120, 230, 140]
```

## Window border

`WindowBorderThickness` is in pixels. `2` is a subtle frame, `0` removes it
entirely, and anything above `20` is clamped back to `2`.

`WindowBorderColor` uses the same `[A, R, G, B]` format as everything else.
White reads as a clean outline on most backgrounds; matching it to your
`HoverBorderColor` ties the window together if you're using a strong accent.

## Already-picked responses

`VisitedResponseOpacity` dims options the player has already chosen during
the current conversation, so working back to the start makes it obvious
what's already been read.

It **scales the alpha of `ResponseTextColor`** rather than swapping in a
grey, so a server using a blue or green palette stays on-palette instead of
jumping to a different hue.

- `0.4` (default) — clearly dimmed but still readable
- `1.0` — no dimming at all
- `0.2` — heavily faded

The state resets when the conversation closes, so options are fresh next
time the player talks to that NPC.

## Fonts and text size: `Font` and `TextSize`

Two separate settings, so any typeface can be had at any size:

| `Font` | Look |
|---|---|
| `DEFAULT` | Metron Book — what DayZ uses everywhere |
| `LIGHT` | Metron Light — thinner, less shouty |
| `BLACK` | Metron Black — heavy, reads at a distance |
| `METRON` | Metron — the plain weight, a touch wider |
| `SERIF` | Amor Serif — a book face, for lore-heavy servers |
| `ETELKA` | Etelka Text — what Expansion's own menus use |

These ship inside the mod as their own typefaces:

| `Font` | Look | Russian |
|---|---|---|
| `INTER` | Inter — clean modern sans | **Yes** |
| `GARAMOND` | EB Garamond — classic book serif | **Yes** |
| `NOTOSERIF` | Noto Serif — sturdy, readable serif | **Yes** |
| `CONDENSED` | Condensed Sans — tall and narrow | No |
| `ZILLA` | Zilla Slab — chunky slab serif | No |
| `TYPEWRITER` | Special Elite — worn typewriter | No |
| `BLACKOPS` | Black Ops One — military stencil | No |

**A font without Russian draws Russian as boxes.** If your server has Russian
players, pick one marked Yes. Chinese and Japanese are drawn from DayZ's own
CJK font whatever you pick.

| `TextSize` | Look |
|---|---|
| `NORMAL` | The size the window was designed at |
| `LARGE` | 120% — easier to read at distance or on a TV |
| `COMPACT` | 85% — fits more options without scrolling |

```json
"Font": "SERIF",
"TextSize": "LARGE"
```

**Nothing to build.** Every pairing ships inside the mod as a ready-made
layout — set the two values, restart, done. DayZ reads a typeface only from a
`.layout` file and has no runtime call to change one, which is why they are
pre-built rather than assembled on the fly.

The first six are fonts DayZ itself ships, and they carry every letter of
every language the mod ships — Polish, Czech, Hungarian and Russian
included. The rest are the mod's own; check the Russian column above before
you pick one.

**Metron Book and Metron Light are the crisp two.** They ship as scalable
atlases, so they stay sharp at any size. The other four are fixed-size
images the game scales, so they soften a little at `LARGE`. Worth a look in
game before committing to one.

### If you used `FontStyle`

`FontStyle` was the single setting that did both jobs, and it still works.
The first time a server starts on 1.6.0 it is folded into the new pair and
your file is rewritten:

| Old `FontStyle` | Becomes |
|---|---|
| `DEFAULT` | `Font: DEFAULT`, `TextSize: NORMAL` |
| `LIGHT` | `Font: LIGHT`, `TextSize: NORMAL` |
| `LARGE` | `Font: DEFAULT`, `TextSize: LARGE` |
| `COMPACT` | `Font: DEFAULT`, `TextSize: COMPACT` |

Editing `FontStyle` by hand afterwards still works too, as long as you leave
`Font` and `TextSize` at their defaults — so a guide written for an older
version has not stopped being true.

If a style fails to load for any reason, the mod logs it and falls back to
`DEFAULT` rather than showing an empty window.

## A font the mod doesn't ship: `LayoutOverride`

`Font` covers the thirteen typefaces the mod ships. For anything else — a
face you licensed, or one from another addon — the engine gives no runtime
call to set a typeface, so it has to come from a `.layout` file of your own.

`LayoutOverride` is that escape hatch: point it at your own `.layout` file
shipped inside your own addon, and the mod builds the window from that
instead of the built-in one.

```json
"LayoutOverride": "MyServerMod/GUI/layouts/my_dialogue_menu.layout"
```

To make one, copy
`GUI/layouts/dialogue_menu.layout` out of the scripts mod, change whatever you like — fonts, text sizes, extra
decoration, background images — and ship it in your own addon.

**The widget names must be preserved exactly**, or the script won't find
them:

`DialoguePanel`, `DialoguePanelBackground`, `SpeakerName`, `SpeakerLine`,
`SpeakerLineScroll`, `ResponseScroll`, `ResponseList`, `CloseButton`,
`SettingsButton`, `SettingsButtonLabel`, `SettingsButtonBackground`,
`ConfirmPanel`, `ConfirmText`, `ConfirmYesButton`, `ConfirmYesLabel`,
`ConfirmNoButton`, `ConfirmNoLabel`, `WindowBorder`, `RewardStrip`,
`RewardStripLabel`, `RequiredStrip`, `RequiredStripLabel`

**New in 1.3.0:** `SettingsButton` (and its two children), `ConfirmYesLabel`
and `ConfirmNoLabel`. A layout copied from an older version still loads, but
players lose the settings screen entirely, and the confirm buttons keep their
baked-in English wording instead of following the player's language. Copy the
current `dialogue_menu.layout` and re-apply your changes rather than patching
an old copy.

The option label in `dialogue_response_button.layout` must also be a
`MultilineTextWidgetClass` with `wrap 1`, or long options are cut off at one
line.

If the override path fails to load, the mod logs an error to the client log
and falls back to the built-in layout rather than showing a blank screen.

Note that the config's position and colour settings still apply on top of a
custom layout, so you generally only need an override for fonts and
structural changes.

## Troubleshooting

- **Nothing changed** — the config only reloads on server start, and
  clients only receive it on connect. Restart both.
- **Window off-screen** — check `OffsetX`/`OffsetY`; they're added on top
  of the preset and aren't clamped, so a large offset can push the panel
  out of view. Set both to `0` to get back to a known state.
- **Colours look wrong** — the order is `[A, R, G, B]`, not `[R, G, B, A]`.
  An alpha of `0` makes the element invisible.
- **Check the client log** for `[DialogueFramework] [UI] Panel placed at
  ...`, which reports the resolved position and the preset it came from.


## Response icons

Set `"ShowResponseIcons": true` and every response button gains a small icon on
its right-hand end, so players can tell at a glance what a button will do:

- **Exit door with arrow** — this closes the menu
- **Shopping cart** — this opens the trader's market
- **Speech bubble** — the conversation carries on

The icons follow your `ResponseTextColor`, so they match whatever theme you
have set. Nothing to build; they ship with the mod.

It's off by default. Leave it off and your menu looks exactly as it did.


## How long options are sized

A response button holds one line of text at your chosen font size. When an
option is longer than that, it wraps and the button grows taller — up to three
lines. Past three lines the text shrinks instead, so one rambling option can't
swallow the whole list. Nothing is ever cut off, and a button never gets its
own scrollbar.

Short options are untouched: one line, the same 40px button as always.

`"ScaleTextWithPanel": true` additionally ties the text size to `PanelWidth`,
using `0.6` as the reference. A panel at `0.9` gets text 1.5× the size, one at
`0.4` gets two thirds — clamped to between 0.6× and 1.8× so it can't go
unreadable or absurd. It's **off by default**, so updating the mod changes
nothing until you turn it on.

`TextSize` still sets the base size, and the two combine: `LARGE` on a wide
panel is bigger than `LARGE` alone.

> **Custom layouts:** the option label has to be a `MultilineTextWidgetClass`
> with `wrap 1` for any of this to work. If your `LayoutOverride` uses a plain
> `TextWidgetClass` the text will still be clipped to one line — copy the
> widget block from `dialogue_response_button.layout`.

## How fast the wheel scrolls a long speech

`"ScrollSpeed": 1.0` sets how far one notch of the mouse wheel moves a speech
that is too long for its box. `2.0` covers twice as much ground per notch,
`0.5` half. Anything outside `0.25` – `4.0` is ignored and `1.0` is used.

This is your default, not a rule. Players who want it faster or slower set
their own under **Settings** in the conversation window, and theirs wins —
their choice is saved on their own machine and follows them to any server
running the mod. Whatever you set here is what a player sees until they
change it, and it is what they get back if they pick *Reset to the server's
settings*.

## Telling players who a choice pleased or annoyed

`"ShowReputationNotifications": true` (the default) puts a short pop-up on
screen when a choice a player made moves someone's standing — the character's
name and how far it moved, *Yefim +5*. The name is read out of the reputation
key, so `yefim_rep` shows as *Yefim*.

Only changes the player caused by picking something show. Reputation your
quests hand out with `RepOnComplete`, or anything the server moves elsewhere,
stays quiet — a pop-up in the middle of a hand-in reads as noise.

Set it to `false` and nobody sees them. Either way each player can override it
for themselves under **Settings** in the conversation window.

## The standing page in Expansion's book

If you have Expansion's book, players get a page listing where they stand with
every character on the server at once — the character, the rank they are on,
and the number behind it. It is built from what the player's own game already
knows, so it costs the server nothing, and it rebuilds each time the page is
opened. On a server with no reputations set up the tab doesn't appear at all.

Five fields word it:

| Field | What it names | Left empty |
|---|---|---|
| `BookTabName` | The tab in the book | *Standing* |
| `BookPageTitle` | The heading on the page | *Where you stand* |
| `BookColumnName` | The first column | *Name* |
| `BookColumnStatus` | The second column | *Status* |
| `BookColumnReputation` | The third column | *Reputation* |

**Leaving one empty is not the same as typing the English into it.** Empty
means each player reads that word in their own language; typing something in
picks one wording for everyone, whatever language they play in.

Give a character `ReputationMax` in its dialogue tree and its row reads
**10 / 100** instead of a bare *10*, so a player can see how far there is left
to go.

## Telling players when something is wrong

`"ShowErrorNotifications": true` (the default) puts a short pop-up on screen
when an option can't do what it says — the same toast style Expansion uses.

There are two kinds, and only one of them is an error:

| What happened | What the player sees |
|---|---|
| They can't take that quest yet | *"You can't take that quest right now."* |
| The option is misconfigured | *"That option isn't set up correctly. The server owner can find the reason in the log."* |

The first is normal gameplay feedback and **always shows** — it's how a player
learns they need to finish something else first.

The second is the one this setting controls. Without it, a misconfigured
option just closes the window and the player has no idea why — they'll assume
your server is broken. With it, they know it isn't them, and you get a report.
Set it to `false` if you'd rather players never saw it; the full reason goes to
your log either way.

## What players can change for themselves

There's a small settings button in the corner of the conversation window, next
to the close button. It opens a screen of preferences belonging to that player
alone, saved on their machine and carried to any server running the mod:

| Row | What it does |
|---|---|
| Language | Read in a language other than their game's. Only shown if you have translations |
| Window position | Move the window to any of the nine spots |
| Text size | 80%–150% of whatever your settings produce |
| Button icons | Show or hide the hint icons |
| Reset | Appears once they've changed something; puts it all back |

**Your look is still yours.** They can move the window and scale the text, but
not change your size, your colours, your font or your border — so a server
with a carefully built theme keeps it. Position and text size are the two
things that are genuinely about the player's screen rather than your design.

Everything defaults to "server's choice", so a player who never opens this
screen sees exactly what you configured.

## The language choice

`"ShowLanguageButton": true` (the default) puts a **Language** row on the
player settings screen, reached from the settings button in the corner of the
conversation window. It cycles through the player's own game language plus
every language you have translations for, and their choice is remembered on
their machine.

It only ever appears if `Localization\` actually has translations in it, so a
single-language server sees nothing regardless of this setting. Set it to
`false` if you'd rather every player simply got their own game language with no
option to change it.

The settings screen is drawn from ordinary response buttons, so it takes your
colours and font style with no extra work — including on a custom
`LayoutOverride`.
