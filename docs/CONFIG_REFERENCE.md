# Config Reference — every field, annotated

This is the complete reference for both config files, with **every possible
field** shown and explained inline.

> ### ⚠️ Read this before copying anything
>
> **JSON does not support comments.** The annotated blocks below are
> *documentation only* — the `<<<<<` notes will cause a parse error if you
> leave them in a real file. The mod will log the failure and that NPC (or
> your whole appearance config) will fall back to defaults.
>
> Copy-paste-ready versions with no comments are in [`../examples/`](../examples/):
> - `examples/Dialogue.example.json`
> - `examples/MenuConfig.example.json`

---

# 1. Dialogue tree — `Dialogues\NPC_<id>\Dialogue.json`

```jsonc
{
  "ID": 1,                              // <<<<< Your own label for this tree. Only ever appears in logs. Doesn't need to be unique or match anything
  "NPCIDs": [],                         // <<<<< Which NPCs use this tree. LEAVE EMPTY in an NPC_<id> folder (inferred from the folder name). REQUIRED and non-empty only in the Shared\ folder
  "TraderIDs": [],                      // <<<<< TRADER dialogue only. Every trader of this definition. Leave empty in a Trader_<name> folder (inferred from the folder)
  "TraderClassNames": [],               // <<<<< TRADER dialogue only. Every trader using this entity class, e.g. "eAI_SurvivorM_Denis"
  "TraderPositions": [],                // <<<<< TRADER dialogue only. ONE specific trader at this world position, e.g. "1234.50 300.00 5678.90". Most specific key -- wins over the two above
  "TraderPositionRadius": 50.0,         // <<<<< How close (metres) a trader must be to a listed position to match. Generous is fine -- AI traders drift, and the nearest listed position still breaks ties between outposts
  "TraderMinKeyMatches": 2,             // <<<<< How many of the three keys above must agree. 2 is recommended: definitions and entity classes are shared between outposts, and positions drift, but two agreeing is reliable
  "RootNodeID": 1,                      // <<<<< Which node ID is shown first when the conversation opens. Must match one of the node IDs below

  "GreetingVoiceLineIDs": [             // <<<<< Voice lines played when the conversation OPENS. One picked at random. Empty array = silent greeting
    "Trader_Greeting_1",                 // <<<<< Needs a sound set named DialogueFW_Trader_Greeting_1_SoundSet in your voice pack
    "Trader_Greeting_2"
  ],
  "FarewellVoiceLineIDs": [             // <<<<< Played when the conversation ENDS via END_CONVERSATION or the X button. One picked at random
    "Trader_Farewell_1"
  ],


  "Nodes": [                            // <<<<< Every screen of dialogue in this tree
    {
      "ID": 1,                          // <<<<< Unique within THIS tree. Referenced by NextNodeID. Start at 1 -- 0 is treated as "unset"
      "Type": "STANDARD",               // <<<<< Almost always STANDARD. QUEST_LIST / QUEST_DETAIL are built live by the mod and should never be authored by hand
      "SpeakerText": "Well. Look who's still breathing.",   // <<<<< The line the NPC says on this screen
      "VoiceLineIDs": [                 // <<<<< Voice lines for THIS node. One picked at random. Empty = text only
        "Trader_Node1_1"
      ],
      "Responses": [                    // <<<<< The player's options on this screen

        {
          "Text": "I'm looking for work.",   // <<<<< Button text shown to the player
          "NextNodeID": -1,                  // <<<<< Ignored unless ActionType is NONE. -1 means "end conversation"
          "RequiredQuestID": -1,             // <<<<< -1 = always visible. Any quest ID = hidden until that quest is COMPLETED
          "ActionType": "SHOW_QUEST_LIST",   // <<<<< Opens the LIVE quest list for this NPC, built from real Expansion data
        },
        {
          "Text": "Nevermind.",
          "NextNodeID": -1,
          "RequiredQuestID": -1,
          "ActionType": "END_CONVERSATION",  // <<<<< Plays a random farewell line, then closes the window
        },
        {
          "Text": "What is this place?",
          "NextNodeID": 2,                   // <<<<< Go to node ID 2 next
          "RequiredQuestID": -1,
          "ActionType": "NONE",              // <<<<< NONE = just navigate to NextNodeID
        },
        {
          "Text": "Any news from the outpost?",
          "NextNodeID": 3,
          "RequiredQuestID": 42,            // <<<<< HIDDEN until quest 42 is COMPLETED for this player. This is how you lock story topics
          "ActionType": "NONE"
        }
      ]
    },
    {
      "ID": 2,
      "Type": "STANDARD",
      "SpeakerText": "Used to be a checkpoint. Then a graveyard.",
      "VoiceLineIDs": [],
      "Responses": [
        {
          "Text": "Something else.",
          "NextNodeID": 1,              // <<<<< Send the player back to the root node
          "RequiredQuestID": -1,
          "ActionType": "NONE"
        },
        {
          "Text": "I'll get going.",
          "NextNodeID": -1,
          "RequiredQuestID": -1,
          "ActionType": "END_CONVERSATION"
        }
      ]
    }
  ]
}
```

## All `ActionType` values

| Value | What it does |
|---|---|
| `"NONE"` | Navigate to `NextNodeID`. If that's `-1`, ends the conversation |
| `"SHOW_QUEST_LIST"` | Opens the live quest list for this NPC |
| `"ACCEPT_QUEST"` | Accepts the quest being viewed. Only meaningful inside the live quest-detail step |
| `"DECLINE_QUEST"` | Ends the conversation without accepting |
| `"TURN_IN_QUEST"` | Hands in the quest being viewed. Opens the reward picker if the quest requires a choice |
| `"END_CONVERSATION"` | Plays a random farewell line, then closes |
| `"OPEN_TRADER"` | Traders only. Closes dialogue and opens the market menu |

## All `Type` values

| Value | Meaning |
|---|---|
| `"STANDARD"` | A normal authored node. Use this for everything |
| `"QUEST_LIST"` | Built live by the mod. Never author by hand |
| `"QUEST_DETAIL"` | Built live by the mod. Never author by hand |

## Traps worth knowing

- **Write every response field explicitly.** Omitting one does *not* give
  you the documented default — JSON loading bypasses constructors, so an
  omitted `RequiredQuestID` becomes `0`, which gates the response behind a
  quest that can't exist and makes **every button on that node vanish**.
  The mod now corrects this on load, but explicit fields are the safe habit.
- **Node IDs start at 1.** `0` is treated as "unset".
- **Don't gate every response on a node** — the player would get a line of
  dialogue with no buttons and only the X to escape.
- **Duplicate node IDs** silently break navigation. `LoadLog.txt` flags them.

---

# 2. Per-quest wording — `QuestText\*.json`

Optional. Gives individual quests their own button wording instead of the
NPC's generic fallback. Split across as many files as you like.

```jsonc
{
  "Quests": [                            // <<<<< One entry per quest you want to customise
    {
      "QuestID": 101,                    // <<<<< The Expansion quest ID this wording applies to
      "AcceptTexts": [                   // <<<<< Buttons shown when the quest is offered. EVERY entry is its own button
        "I'll get your wall built.",
        "Point me at the timber."
      ],
      "DeclineTexts": [                  // <<<<< Buttons for turning the quest down
        "I'm not a carpenter."
      ],
      "TurnInTexts": [                   // <<<<< Buttons shown when the quest is ready to hand in
        "Wall's up. Try knocking it down."
      ],
      "NotYetTexts": [                   // <<<<< The "back out without handing in" button
        "Not finished yet."
      ],
      "InProgressTexts": [               // <<<<< Buttons shown while the quest is still active
        "Still hauling timber."
      ],
      "RewardSelectText": "Take one."    // <<<<< SPOKEN line above the reward picker. A single string, not an array
    }
  ]
}
```

Anything left out falls back to plain built-in wording, so you only need
entries for the quests you actually want to customise. **Quest wording lives
only here** — dialogue tree files don't carry it.

---

# 3. Window appearance — `MenuConfig.json`

```jsonc
{
  "ConfigVersion": 1,                   // <<<<< Leave this alone. Lets the mod add new settings to your file automatically when you update
  "Position": "BOTTOM_CENTER",          // <<<<< Screen preset. One of the nine listed below. BOTTOM_CENTER keeps the NPC's face visible
  "PanelWidth": 0.6,                    // <<<<< Window width as a fraction of the screen. 0.6 = 60% of screen width
  "PanelHeight": 0.42,                  // <<<<< Window height as a fraction of the screen
  "OffsetX": 0.0,                       // <<<<< Nudge LEFT(-) / RIGHT(+) applied AFTER the preset resolves. Use to clear your own HUD. Not clamped -- big values push it off-screen
  "OffsetY": 0.0,                       // <<<<< Nudge UP(-) / DOWN(+), same idea
  "EdgeMargin": 0.03,                   // <<<<< How far the edge-hugging presets sit from the screen edge. Ignored by the CENTER presets

  "BackgroundColor":         [230, 0, 0, 0],        // <<<<< Main window panel.        Format is [Alpha, Red, Green, Blue], each 0-255
  "ResponseBackgroundColor": [200, 0, 0, 0],        // <<<<< Each option button, and unselected reward tiles
  "HoverBorderColor":        [255, 255, 215, 0],    // <<<<< Border drawn around whatever the mouse is over
  "SpeakerNameColor":        [255, 255, 255, 255],  // <<<<< The NPC's name at the top
  "SpeakerTextColor":        [255, 255, 255, 255],  // <<<<< The line of dialogue itself
  "ResponseTextColor":       [255, 220, 220, 220],  // <<<<< Text on the option buttons
  "RewardSelectedColor":     [230, 90, 70, 20],     // <<<<< The reward tile the player has clicked
  "WindowBorderColor":       [255, 255, 255, 255],  // <<<<< Thin frame around the whole window

  "WindowBorderThickness": 2,           // <<<<< Border thickness in pixels. 0 removes the border entirely
  "VisitedResponseOpacity": 0.4,        // <<<<< How faded an option looks once the player has picked it this conversation. 1.0 = no fading. Scales the alpha of ResponseTextColor so your palette is kept

  "LayoutOverride": ""                  // <<<<< Empty = use the built-in window. Set a path to YOUR OWN .layout file to change fonts or structure. See MENU_CONFIG_GUIDE.md
}
```

## All `Position` values

`TOP_LEFT`, `TOP_CENTER`, `TOP_RIGHT`,
`CENTER_LEFT`, `CENTER`, `CENTER_RIGHT`,
`BOTTOM_LEFT`, `BOTTOM_CENTER`, `BOTTOM_RIGHT`

## Colour format

Every colour is `[Alpha, Red, Green, Blue]`, each component `0`–`255`.

- Alpha `0` is invisible, `255` fully opaque
- The order is **not** `[R, G, B, A]` — getting this backwards is the most
  common mistake, and usually shows up as an invisible or black element

## Fonts

**Fonts cannot be set from this file.** DayZ only reads a font from a
`.layout` file and offers no runtime call to change one. Use
`LayoutOverride` with your own layout — see
[`MENU_CONFIG_GUIDE.md`](MENU_CONFIG_GUIDE.md) for the widget names you must
preserve.

---

# 4. When changes take effect

| Changed | Needs |
|---|---|
| Dialogue tree `.json` | Server restart + **full client restart** |
| `MenuConfig.json` | Server restart + **full client restart** |
| Voice `.ogg` files | Repack and republish the voice pack |

Both configs are pushed to clients when they connect, so a reconnect alone
won't pick up changes — the client process has to restart.

# 5. Where to look when something's wrong

| Symptom | Where to look |
|---|---|
| A tree didn't load | `Dialogues\LoadLog.txt` |
| An NPC has no buttons | `LoadLog.txt`, then check `RequiredQuestID` on that node |
| A voice line is silent | Client log — `[VOICE-AUDIT]` lists every missing sound set by name |
| Window in the wrong place | Client log — `[UI] Panel placed at X, Y (PRESET)` |
| Colours wrong | Check `[A,R,G,B]` ordering |
