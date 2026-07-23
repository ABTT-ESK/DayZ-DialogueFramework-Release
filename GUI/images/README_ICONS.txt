Response button icons
=====================

icon_exit_ca.png   buttons that close the menu
icon_chat_ca.png   buttons that continue the conversation
icon_cart_ca.png   buttons that open the trader market

These are the SOURCE files. DayZ cannot load .png at runtime -- each must be
converted to .paa before the mod is packed, like logo.paa was:

  1. Open ImageToPAA (ships with DayZ Tools)
  2. Drag all three .png files in
  3. Confirm icon_exit_ca.paa, icon_chat_ca.paa and icon_cart_ca.paa land in
     this folder
  4. Repack the PBO


THE _ca SUFFIX IS REQUIRED -- DO NOT RENAME
-------------------------------------------
Bohemia's texture tools choose the output format from the FILE NAME, not from
the image contents:

  _co  = colour, NO alpha channel
  _ca  = colour WITH alpha channel

Convert these as "icon_exit.paa" instead of "icon_exit_ca.paa" and the alpha
channel is thrown away. The icon then renders as a solid block instead of a
transparent glyph. This is documented under "Arma: Texture Naming Rules"; the
same rule governs TexView 2 and ImageToPAA, which share a conversion engine.

If you rename these files you must also update ICON_EXIT / ICON_CHAT /
ICON_CART in Scripts/5_Mission/Dialogue/GUI/DialogueWindowMenu.c to match.


WHY THE ART IS WHITE
--------------------
The mod tints the icons at runtime with SetColor so they follow the server's
ResponseTextColor. Tinting only works from white.

The pixels UNDER the transparent areas are also white, deliberately. If the
alpha channel is ever lost, the icon degrades to a faint white block rather
than a hard black one -- and image editors that discard colour beneath zero
alpha will otherwise leave black there.

They are 128x128, a power of two, which ImageToPAA requires.


IF THEY STILL RENDER AS SOLID BLOCKS
------------------------------------
The layout widget needs three attributes for transparency and scaling, all
already set in dialogue_response_button.layout:

  mode blend        standard alpha blending
  "src alpha" 1     use the texture's alpha channel
  stretch 1         scale the texture to the widget instead of clipping it

Without "src alpha" the icon draws opaque. Without stretch, a 128x128 texture
drawn into a 20x20 widget renders at native size and is cropped.

If a solid block survives all of that, try .edds instead of .paa -- .edds is
the standard UI texture format for DayZ. Open the .paa in TexView 2, save as
.edds beside it, then change ICON_EXT in
Scripts/5_Mission/Dialogue/GUI/DialogueWindowMenu.c from ".paa" to ".edds".


SWAPPING IN YOUR OWN ART
------------------------
Keep the same file names including the _ca suffix, keep the art white on
transparent, fill the RGB under transparent pixels with white too, and keep
the size a power of two.
