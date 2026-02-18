#!/bin/bash

# 创建占位符资源文件脚本
# 作者: 彭承康
#
# 优先使用 Python + Pillow 生成有效格式的占位符文件。
# 如果 Python 不可用，回退到 ImageMagick；如果 ImageMagick 也不可用，
# 则使用 touch 创建空文件（Qt 会报加载警告，但不阻塞编译）。

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
RESOURCES_DIR="$PROJECT_ROOT/application/resources"

echo "Creating placeholder resources..."

# ---------------------------------------------------------------------------
# 方式 1: 优先使用 Python 脚本（生成真正有效的文件）
# ---------------------------------------------------------------------------
if command -v python3 &> /dev/null && python3 -c "from PIL import Image; from fontTools.fontBuilder import FontBuilder" 2>/dev/null; then
    echo "Using Python + Pillow + fontTools to generate valid placeholders..."
    python3 "$SCRIPT_DIR/generate_placeholders.py"
    echo "Done."
    exit 0
elif command -v python &> /dev/null && python -c "from PIL import Image; from fontTools.fontBuilder import FontBuilder" 2>/dev/null; then
    echo "Using Python + Pillow + fontTools to generate valid placeholders..."
    python "$SCRIPT_DIR/generate_placeholders.py"
    echo "Done."
    exit 0
fi

echo "Python with Pillow/fontTools not available, falling back..."

# ---------------------------------------------------------------------------
# 创建目录结构
# ---------------------------------------------------------------------------
mkdir -p "$RESOURCES_DIR"/{images/ui,sounds,fonts,icons,configs}

# ---------------------------------------------------------------------------
# 所有需要的图片文件列表
# ---------------------------------------------------------------------------
ALL_IMAGES=(
    images/player.png
    images/enemies.png
    images/items.png
    images/main_menu_bg.jpg
    images/app_icon.png
    images/player_avatar.png
    images/default_item.png
    images/ui/button_normal.png
    images/ui/button_hover.png
    images/ui/button_pressed.png
    images/ui/background.png
)

ALL_ICONS=(
    icons/new_game.png
    icons/load_game.png
    icons/settings.png
    icons/exit.png
    icons/warrior.png
    icons/mage.png
    icons/archer.png
    icons/inventory.png
    icons/quests.png
    icons/skills.png
    icons/skill_1.png
    icons/skill_2.png
    icons/skill_3.png
    icons/save.png
    icons/chapter.png
    icons/location.png
    icons/time.png
    icons/quest.png
    icons/skill_attack.png
    icons/skill_heal.png
    icons/skill_fireball.png
    icons/skill_default.png
    icons/health_potion.png
    icons/mana_potion.png
    icons/iron_sword.png
)

# ---------------------------------------------------------------------------
# 方式 2: ImageMagick
# ---------------------------------------------------------------------------
if command -v convert &> /dev/null; then
    echo "Using ImageMagick to create placeholder images..."

    # Images
    convert -size 64x64 xc:lightblue  "$RESOURCES_DIR/images/player.png"
    convert -size 64x64 xc:red        "$RESOURCES_DIR/images/enemies.png"
    convert -size 32x32 xc:gold       "$RESOURCES_DIR/images/items.png"
    convert -size 1280x720 gradient:blue-black "$RESOURCES_DIR/images/main_menu_bg.jpg"
    convert -size 64x64 xc:green      "$RESOURCES_DIR/images/app_icon.png"
    convert -size 64x64 xc:lightblue  "$RESOURCES_DIR/images/player_avatar.png"
    convert -size 32x32 xc:gray       "$RESOURCES_DIR/images/default_item.png"
    convert -size 200x50 xc:'#3A5A8C' "$RESOURCES_DIR/images/ui/button_normal.png"
    convert -size 200x50 xc:'#4A7ABE' "$RESOURCES_DIR/images/ui/button_hover.png"
    convert -size 200x50 xc:'#2A3A5C' "$RESOURCES_DIR/images/ui/button_pressed.png"
    convert -size 1280x720 xc:'#1E1E2E' "$RESOURCES_DIR/images/ui/background.png"

    # Icons
    for icon in "${ALL_ICONS[@]}"; do
        convert -size 32x32 xc:gray "$RESOURCES_DIR/$icon"
    done

    echo "Images created with ImageMagick."
else
    echo "ImageMagick not available, creating empty placeholder files..."

    for img in "${ALL_IMAGES[@]}"; do
        touch "$RESOURCES_DIR/$img"
    done
    for icon in "${ALL_ICONS[@]}"; do
        touch "$RESOURCES_DIR/$icon"
    done
fi

# ---------------------------------------------------------------------------
# Audio files (silent placeholders)
# ---------------------------------------------------------------------------
echo "Creating placeholder audio files..."
# Create a minimal WAV (44-byte header + 1 sample)
if command -v python3 &> /dev/null; then
    python3 -c "
import wave, sys
with wave.open('$RESOURCES_DIR/sounds/effects.wav', 'w') as w:
    w.setnchannels(1); w.setsampwidth(2); w.setframerate(22050)
    w.writeframes(b'\\x00\\x00' * 22050)
"
else
    touch "$RESOURCES_DIR/sounds/effects.wav"
fi
touch "$RESOURCES_DIR/sounds/bgm.mp3"

# ---------------------------------------------------------------------------
# Font files
# ---------------------------------------------------------------------------
echo "Creating placeholder font files..."
# Try to copy a system font; otherwise create empty files
SYSTEM_FONT=""
for candidate in \
    /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf \
    /usr/share/fonts/TTF/DejaVuSans.ttf \
    /usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf \
    /System/Library/Fonts/Helvetica.ttc; do
    if [ -f "$candidate" ]; then
        SYSTEM_FONT="$candidate"
        break
    fi
done

if [ -n "$SYSTEM_FONT" ]; then
    cp "$SYSTEM_FONT" "$RESOURCES_DIR/fonts/game_font.ttf"
    cp "$SYSTEM_FONT" "$RESOURCES_DIR/fonts/ui_font.ttf"
    echo "Copied system font: $SYSTEM_FONT"
else
    touch "$RESOURCES_DIR/fonts/game_font.ttf"
    touch "$RESOURCES_DIR/fonts/ui_font.ttf"
    echo "Warning: No system font found. Font files are empty placeholders."
fi

# ---------------------------------------------------------------------------
# Config files (only if they don't already exist)
# ---------------------------------------------------------------------------
echo "Creating config files..."
if [ ! -s "$RESOURCES_DIR/configs/game_config.json" ]; then
cat > "$RESOURCES_DIR/configs/game_config.json" << 'EOF'
{
    "game": {
        "title": "幻境传说",
        "version": "1.0.0",
        "window": {
            "width": 1280,
            "height": 720,
            "fullscreen": false
        }
    },
    "graphics": {
        "quality": "medium",
        "vsync": true,
        "antialiasing": true
    },
    "audio": {
        "master_volume": 1.0,
        "music_volume": 0.7,
        "sfx_volume": 0.8
    }
}
EOF
fi

if [ ! -s "$RESOURCES_DIR/configs/ui_config.json" ]; then
cat > "$RESOURCES_DIR/configs/ui_config.json" << 'EOF'
{
    "ui": {
        "theme": "dark",
        "font_size": 12,
        "language": "zh_CN"
    },
    "panels": {
        "inventory_visible": true,
        "chat_visible": true,
        "minimap_visible": true
    }
}
EOF
fi

echo ""
echo "All placeholder resources created."
echo ""
echo "Note: For best results, install Python with Pillow and fontTools:"
echo "  pip install Pillow fontTools"
echo "  python scripts/generate_placeholders.py"
