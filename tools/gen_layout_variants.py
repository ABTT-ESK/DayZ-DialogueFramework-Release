#!/usr/bin/env python3
"""
Generates the per-style layout variants from the Default layouts.

DayZ reads fonts only from .layout files and offers no runtime call to change
one, so a font choice means a different layout. Rather than ask server owners
to ship their own addon, we pre-bake a set per style and let MenuConfig pick.

Run from the repo root after editing anything in GUI/layouts/:

    python tools/gen_layout_variants.py

Only the Default layouts are hand-edited. Everything else is generated, so
never edit a *_light / *_large / *_compact file directly -- it gets
overwritten.
"""

import os
import re
import sys

LAYOUT_DIR = "GUI/layouts"

MASTERS = [
    "dialogue_menu",
    "dialogue_response_button",
    "dialogue_reward_button",
    "dialogue_reward_display",
]

# style: (font substitutions, text-size scale)
#
# Fonts are limited to what DayZ actually ships. Metron Book and Metron Light
# are the two weights confirmed in use by vanilla, CF and Expansion layouts.
# The trailing number is the SDF atlas resolution, not the display size.
STYLES = {
    "light": {
        "fonts": {
            "gui/fonts/sdf_MetronBook72": "gui/fonts/sdf_MetronLight24",
            "gui/fonts/sdf_MetronBook24": "gui/fonts/sdf_MetronLight24",
        },
        "scale": 1.0,
    },
    "large": {
        "fonts": {},
        "scale": 1.2,
    },
    "compact": {
        "fonts": {},
        "scale": 0.85,
    },
}


def scale_text_sizes(text, factor):
    """Scale every "exact text size" value, keeping them whole numbers."""
    def repl(match):
        size = int(match.group(1))
        scaled = max(8, int(round(size * factor)))
        return '"exact text size" %d' % scaled

    return re.sub(r'"exact text size" (\d+)', repl, text)


def main():
    if not os.path.isdir(LAYOUT_DIR):
        print("error: run this from the repository root (no %s found)" % LAYOUT_DIR)
        return 1

    written = 0
    for style, spec in STYLES.items():
        for master in MASTERS:
            src = os.path.join(LAYOUT_DIR, master + ".layout")
            if not os.path.exists(src):
                print("error: missing master layout %s" % src)
                return 1

            content = open(src, encoding="utf-8").read()

            for old_font, new_font in spec["fonts"].items():
                content = content.replace(old_font, new_font)

            if spec["scale"] != 1.0:
                content = scale_text_sizes(content, spec["scale"])

            dst = os.path.join(LAYOUT_DIR, "%s_%s.layout" % (master, style))
            open(dst, "w", encoding="utf-8").write(content)
            written += 1

    print("wrote %d layout files for styles: %s"
          % (written, ", ".join(sorted(STYLES))))

    # sanity: every generated file must still be brace-balanced
    bad = []
    for f in sorted(os.listdir(LAYOUT_DIR)):
        if not f.endswith(".layout"):
            continue
        text = open(os.path.join(LAYOUT_DIR, f), encoding="utf-8").read()
        if text.count("{") != text.count("}"):
            bad.append(f)

    if bad:
        print("BRACE MISMATCH in: %s" % ", ".join(bad))
        return 1

    print("all layouts brace-balanced")
    return 0


if __name__ == "__main__":
    sys.exit(main())
