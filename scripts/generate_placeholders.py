#!/usr/bin/env python3
"""
Generate valid placeholder resource files for the game client.

Creates proper PNG/JPG images, WAV audio, and TTF font files so Qt can
load them without errors.  Run from the project root:

    python scripts/generate_placeholders.py

Dependencies: Pillow, fontTools  (pip install Pillow fontTools)
"""

import os
import struct
import wave
from pathlib import Path

from PIL import Image, ImageDraw
from fontTools.fontBuilder import FontBuilder

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

PROJECT_ROOT = Path(__file__).resolve().parent.parent
RESOURCES_DIR = PROJECT_ROOT / "application" / "resources"

# (relative_path, width, height, color, label, format)
# format: "PNG" or "JPEG"
IMAGE_SPECS = [
    # --- Existing 0-byte files that need valid content ---
    ("images/player.png",        64,   64,  "#5B8DEE", "P",  "PNG"),
    ("images/enemies.png",       64,   64,  "#EE5B5B", "E",  "PNG"),
    ("images/items.png",         32,   32,  "#F0C040", "I",  "PNG"),
    ("images/main_menu_bg.jpg", 1280, 720,  "#1A1A3E", "BG", "JPEG"),
    ("images/app_icon.png",      64,   64,  "#40B050", "G",  "PNG"),
    ("icons/new_game.png",       32,   32,  "#6090D0", "N",  "PNG"),
    ("icons/load_game.png",      32,   32,  "#6090D0", "L",  "PNG"),
    ("icons/settings.png",       32,   32,  "#808080", "S",  "PNG"),
    ("icons/exit.png",           32,   32,  "#D06060", "X",  "PNG"),
    ("icons/warrior.png",        32,   32,  "#C08040", "W",  "PNG"),
    ("icons/mage.png",           32,   32,  "#8040C0", "M",  "PNG"),
    ("icons/archer.png",         32,   32,  "#40C040", "A",  "PNG"),
    ("icons/inventory.png",      32,   32,  "#A0A060", "In", "PNG"),
    ("icons/quests.png",         32,   32,  "#D0A040", "Q",  "PNG"),
    ("icons/skills.png",         32,   32,  "#40A0D0", "Sk", "PNG"),
    ("icons/skill_1.png",        32,   32,  "#E06040", "1",  "PNG"),
    ("icons/skill_2.png",        32,   32,  "#40B0E0", "2",  "PNG"),
    ("icons/skill_3.png",        32,   32,  "#60E060", "3",  "PNG"),

    # --- Missing files that need to be created ---
    ("icons/save.png",           32,   32,  "#5080C0", "Sv", "PNG"),
    ("icons/chapter.png",        32,   32,  "#7060A0", "Ch", "PNG"),
    ("icons/location.png",       32,   32,  "#60A060", "Lo", "PNG"),
    ("icons/time.png",           32,   32,  "#A09050", "Ti", "PNG"),
    ("icons/quest.png",          32,   32,  "#D0A040", "Qu", "PNG"),
    ("icons/skill_attack.png",   32,   32,  "#E04040", "At", "PNG"),
    ("icons/skill_heal.png",     32,   32,  "#40E040", "He", "PNG"),
    ("icons/skill_fireball.png", 32,   32,  "#E08020", "Fi", "PNG"),
    ("icons/skill_default.png",  32,   32,  "#808080", "Df", "PNG"),
    ("icons/health_potion.png",  32,   32,  "#E04040", "HP", "PNG"),
    ("icons/mana_potion.png",    32,   32,  "#4040E0", "MP", "PNG"),
    ("icons/iron_sword.png",     32,   32,  "#A0A0B0", "Sw", "PNG"),
    ("images/player_avatar.png", 64,   64,  "#5B8DEE", "PA", "PNG"),
    ("images/default_item.png",  32,   32,  "#808080", "?",  "PNG"),
    ("images/ui/button_normal.png",  200, 50, "#3A5A8C", "BTN",  "PNG"),
    ("images/ui/button_hover.png",   200, 50, "#4A7ABE", "BTN",  "PNG"),
    ("images/ui/button_pressed.png", 200, 50, "#2A3A5C", "BTN",  "PNG"),
    ("images/ui/background.png",    1280, 720, "#1E1E2E", "",    "PNG"),
]

FONT_FILES = [
    "fonts/game_font.ttf",
    "fonts/ui_font.ttf",
]

AUDIO_SPECS = [
    # (relative_path, duration_seconds, sample_rate)
    ("sounds/bgm.mp3", 1, 22050),
    ("sounds/effects.wav", 1, 22050),
]


# ---------------------------------------------------------------------------
# Generators
# ---------------------------------------------------------------------------

def generate_image(rel_path: str, width: int, height: int,
                   color: str, label: str, fmt: str) -> None:
    """Create a simple placeholder image with a colored background and label."""
    out = RESOURCES_DIR / rel_path
    out.parent.mkdir(parents=True, exist_ok=True)

    img = Image.new("RGBA" if fmt == "PNG" else "RGB", (width, height), color)
    draw = ImageDraw.Draw(img)

    if label:
        # Choose a font size proportional to the image
        font_size = min(width, height) // 3
        try:
            from PIL import ImageFont
            font = ImageFont.load_default(size=font_size)
        except TypeError:
            font = ImageFont.load_default()

        bbox = draw.textbbox((0, 0), label, font=font)
        tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
        x = (width - tw) // 2
        y = (height - th) // 2
        draw.text((x, y), label, fill="white", font=font)

    img.save(str(out), fmt)
    print(f"  [IMG] {rel_path}  ({width}x{height} {fmt})")


def generate_minimal_ttf(rel_path: str) -> None:
    """Create a minimal valid TrueType font using fontTools pen API."""
    out = RESOURCES_DIR / rel_path
    out.parent.mkdir(parents=True, exist_ok=True)

    fb = FontBuilder(1000, isTTF=True)
    fb.setupGlyphOrder([".notdef", "space"])
    fb.setupCharacterMap({32: "space"})

    # Use pen-based drawing for glyf table
    fb.setupGlyf({})
    glyf = fb.font["glyf"]

    from fontTools.ttLib import TTFont
    from fontTools.pens.ttGlyphPen import TTGlyphPen

    for glyph_name in [".notdef", "space"]:
        pen = TTGlyphPen(None)
        # Draw an empty glyph (no contours)
        glyf[glyph_name] = pen.glyph()

    metrics = {
        ".notdef": (500, 0),
        "space":   (250, 0),
    }
    fb.setupHorizontalMetrics(metrics)
    fb.setupHorizontalHeader(ascent=800, descent=-200)
    fb.setupNameTable({
        "familyName": "GamePlaceholder",
        "styleName": "Regular",
    })
    fb.setupOS2(sTypoAscender=800, sTypoDescender=-200,
                sTypoLineGap=0, usWinAscent=800, usWinDescent=200)
    fb.setupPost()
    fb.setupHead(unitsPerEm=1000)

    fb.font.save(str(out))
    print(f"  [TTF] {rel_path}")


def generate_wav(rel_path: str, duration: float, sample_rate: int) -> None:
    """Create a short silent WAV file."""
    out = RESOURCES_DIR / rel_path
    out.parent.mkdir(parents=True, exist_ok=True)

    n_frames = int(sample_rate * duration)
    with wave.open(str(out), "w") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(sample_rate)
        wf.writeframes(b"\x00\x00" * n_frames)
    print(f"  [WAV] {rel_path}  ({duration}s silence)")


def generate_silent_mp3(rel_path: str) -> None:
    """Create a minimal valid MP3 file (single silent MPEG frame)."""
    out = RESOURCES_DIR / rel_path
    out.parent.mkdir(parents=True, exist_ok=True)

    # Minimal valid MPEG1 Layer3 frame header: sync=0xFFE, version=MPEG1,
    # layer=3, no CRC, 32kbps, 44100Hz, stereo.
    # Header bytes: FF FB 90 00
    header = bytes([0xFF, 0xFB, 0x90, 0x00])
    # MPEG1 Layer3 32kbps 44100Hz frame size = 104 bytes (including header)
    frame_data = header + b"\x00" * 100
    with open(str(out), "wb") as f:
        f.write(frame_data)
    print(f"  [MP3] {rel_path}  (minimal silent frame)")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> None:
    print(f"Resource directory: {RESOURCES_DIR}\n")

    print("Generating images...")
    for spec in IMAGE_SPECS:
        generate_image(*spec)

    print("\nGenerating fonts...")
    for fpath in FONT_FILES:
        generate_minimal_ttf(fpath)

    print("\nGenerating audio...")
    for spec in AUDIO_SPECS:
        rel = spec[0]
        if rel.endswith(".wav"):
            generate_wav(*spec)
        elif rel.endswith(".mp3"):
            generate_silent_mp3(rel)

    print("\nDone. All placeholder resources generated.")


if __name__ == "__main__":
    main()
