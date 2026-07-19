# Example configs

Drop-in files with **no comments**, so they parse as-is. For the annotated
version of every field, see [`../docs/CONFIG_REFERENCE.md`](../docs/CONFIG_REFERENCE.md).

| File | Copy it to |
|---|---|
| `Dialogue.example.json` | `$profile:\DialogFramework\Dialogues\NPC_<id>\Dialogue.json` |
| `QuestText.example.json` | `$profile:\DialogFramework\QuestText\Example.json` |
| `TraderDialogue.example.json` | `$profile:\DialogFramework\Dialogues\Trader_<name>\Dialogue.json` |
| `MenuConfig.example.json` | `$profile:\DialogFramework\MenuConfig.json` |

## About the dialogue example

It contains **every field** the data model supports, so you can delete what
you don't need rather than hunt for what's missing. It demonstrates:

- A response gated behind quest completion (`"RequiredQuestID": 217`)
- Branch nodes that return to the root, and ones that end the conversation
- All five per-NPC quest button text arrays, plus the reward-select line

Remember `NPCIDs` stays empty in an `NPC_<id>` folder — it's inferred from
the folder name. It's only required in `Shared\`.

## Per-quest wording

`QuestText.example.json` shows how to give individual quests their own
accept / decline / turn-in wording, so a farming errand and a combat
mission don't share the same generic lines. Anything you don't set there
falls back to the NPC's tree, then to plain built-in wording.

## Why only three ActionTypes appear

`ACCEPT_QUEST`, `DECLINE_QUEST` and `TURN_IN_QUEST` are missing on purpose.
They're only meaningful inside the live quest-detail step, which the mod
builds from real Expansion quest data — you never author it. The text on
those buttons comes from `QuestText\*.json` instead.

The three that do appear (`NONE`, `SHOW_QUEST_LIST`, `END_CONVERSATION`) are
the only ones you write by hand.

## Trader dialogue

`TraderDialogue.example.json` is a trader conversation. Drop it in a
`Trader_<name>` folder and the trader talks before opening the market.
Responses using `"ActionType": "OPEN_TRADER"` take the player through to
the shop.

The folder name must match the trader definition. Open any trader in game
and the client log tells you both usable names:

```
[DialogueFramework] [TRADER] Trader opened -- fileName='Weapons' displayName='Weapons Trader'
```
