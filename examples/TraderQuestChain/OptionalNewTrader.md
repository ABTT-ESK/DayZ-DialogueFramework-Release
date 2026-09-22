# Placing a new trader (optional)

Only needed if you want a **brand new** merchant. If the conversation is for a
trader you have already placed, skip this.

The line below goes in a trader map file:

```
mpmissions\<your mission>\expansion\traders\<any name>.map
```

Expansion loads every `.map` in that folder, so a separate file per trader
zone is fine. Edit the line, then save **only that line** there as a `.map`
file — Expansion reads every line of it as a trader, so no notes go in it.

```
<entity class>.<trader definition>|<x y z>|<yaw pitch roll>|name:<label>,loadout:<loadout>,faction:<faction>
```

```
ExpansionTraderAIIrena.Medicals|6616.0 8.38 2434.0|180 0 0|name:Vera,loadout:NBCLoadout,faction:Guards
```

| Part | What it is |
|---|---|
| entity class | `ExpansionTraderAI*` needs Expansion AI. Without it use a plain class such as `ExpansionTraderDenis`, or a static trader object. |
| trader definition | Must match a file in `ExpansionMod\Traders\`, so `Medicals` means `ExpansionMod\Traders\Medicals.json`. If it is missing the log says: *Trader does not exist: Medicals* |
| position | Write just `x z` (`6616.0 2434.0`) and Expansion puts the trader on the ground for you. |
| rotation | Yaw, pitch, roll — which way it faces. |
| last part | Optional. Display name, loadout and faction for AI traders. |

The position must be inside a trader zone — one of the files in
`mpmissions\<your mission>\expansion\traderzones\` — or the log says:
*Trader is not within a trader zone*

Restart the server afterwards. The trader then appears in DialogueForge's
**Pick from trader map...** list, ready for the conversation to attach to.
