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
  "LayoutOverride": ""
}
```

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
`ResponseScroll`, `ResponseList`, `CloseButton`, `ConfirmPanel`,
`ConfirmText`, `ConfirmYesButton`, `ConfirmNoButton`

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
