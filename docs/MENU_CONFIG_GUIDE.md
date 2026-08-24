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
  "ShowResponseIcons": false,
  "ShowLanguageButton": true,
  "ScaleTextWithPanel": false,
  "ShowErrorNotifications": true,
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

## Fonts and text size: `FontStyle`

Four built-in styles, no repacking needed:

| `FontStyle` | Look |
|---|---|
| `DEFAULT` | Metron Book, standard sizes |
| `LIGHT` | Metron Light — thinner, less shouty |
| `LARGE` | Metron Book at 120% — easier to read at distance or on a TV |
| `COMPACT` | Metron Book at 85% — fits more options without scrolling |

```json
"FontStyle": "LARGE"
```

**Nothing to build.** All four styles ship inside the mod as ready-made
layouts — set the value, restart, done. DayZ reads fonts only from `.layout`
files and has no runtime call to change one, which is why they're pre-built
rather than assembled on the fly.

The two typefaces are what DayZ itself ships. `LARGE` and `COMPACT` change
text sizes rather than typeface, which in practice makes a bigger difference
to how the window feels.

If a style fails to load for any reason, the mod logs it and falls back to
`DEFAULT` rather than showing an empty window.

## Anything else: `LayoutOverride`

**Fonts can't be changed from the config.** The engine only reads a font
from a `.layout` file, and there's no runtime script call to swap it — this
is a DayZ limitation, not a choice.

The escape hatch is `LayoutOverride`: point it at your own `.layout` file
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

`FontStyle` still sets the base size, and the two combine: `LARGE` on a wide
panel is bigger than `LARGE` alone.

> **Custom layouts:** the option label has to be a `MultilineTextWidgetClass`
> with `wrap 1` for any of this to work. If your `LayoutOverride` uses a plain
> `TextWidgetClass` the text will still be clipped to one line — copy the
> widget block from `dialogue_response_button.layout`.

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
