# Trader + quest chain example

One trader who talks, trades, and gives two quests in a row:

- **1001 *Clean Dressings*** — bring 3 rags.
- **1002 *Fresh Fruit*** — bring 2 apples. It only opens once 1001 is handed
  in.

The trader's greeting changes after each quest, a new line of conversation
opens at the end, and the shop is always one click away.

**The full walkthrough is in
[docs/TRADER_QUEST_CHAIN.md](../../docs/TRADER_QUEST_CHAIN.md).** It covers
what every field does, how to build the same thing in DialogueForge, and how
to move your own Expansion traders and quests over.

## Quick start

1. Make sure quest IDs **1001/1002** and collection objective IDs
   **1001/1002** are free on your server.
2. Stop the server and copy the **contents** of `Profiles` into your server
   profile folder:

   ```
   Profiles/
     ExpansionMod/Quests/Quests/Quest_1001.json
     ExpansionMod/Quests/Quests/Quest_1002.json
     ExpansionMod/Quests/Objectives/Collection/Objective_C_1001.json
     ExpansionMod/Quests/Objectives/Collection/Objective_C_1002.json
     DialogFramework/Dialogues/Trader_Medic/Dialogue.json
     DialogFramework/QuestText/TraderQuestChain.json
   ```

3. Start the server, **fully restart the game**, and talk to a **Medicals**
   trader.

As shipped, the conversation attaches to *every* trader using the Medicals
definition. To tie it to one trader, open `Dialogue.json` in DialogueForge
and use **Pick from trader map...**. The guide has the details.

**Haven't placed the trader yet?** These files give a trader a conversation;
they don't put one in the world. [`OptionalNewTrader.md`](OptionalNewTrader.md) has the
trader map line that does, with every part explained.

Needs Dialogue Framework 1.4.0 or later. DialogueForge 1.5.0 or later is
recommended — its trader picker reads every trader map file you have.
