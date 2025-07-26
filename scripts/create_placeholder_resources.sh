#!/bin/bash

# 创建占位符资源文件脚本
# 作者: 彭承康

echo "🎨 创建占位符资源文件..."

# 创建资源目录
mkdir -p application/resources/{images,sounds,fonts,icons,configs}

# 创建占位符图片文件（使用ImageMagick创建简单图片）
if command -v convert &> /dev/null; then
    echo "📸 使用ImageMagick创建占位符图片..."
    
    # 玩家头像
    convert -size 64x64 xc:lightblue -fill darkblue -gravity center -pointsize 20 -annotate +0+0 "P" application/resources/images/player.png
    
    # 敌人图片
    convert -size 64x64 xc:red -fill darkred -gravity center -pointsize 20 -annotate +0+0 "E" application/resources/images/enemies.png
    
    # 物品图片
    convert -size 32x32 xc:gold -fill darkorange -gravity center -pointsize 12 -annotate +0+0 "I" application/resources/images/items.png
    
    # 主菜单背景
    convert -size 1280x720 gradient:blue-black application/resources/images/main_menu_bg.jpg
    
    # 应用图标
    convert -size 64x64 xc:green -fill darkgreen -gravity center -pointsize 20 -annotate +0+0 "G" application/resources/images/app_icon.png
    
    # 创建图标文件
    for icon in new_game load_game settings exit warrior mage archer inventory quests skills skill_1 skill_2 skill_3; do
        convert -size 32x32 xc:gray -fill black -gravity center -pointsize 10 -annotate +0+0 "${icon:0:1}" "application/resources/icons/${icon}.png"
    done
    
    echo "✅ 图片文件创建完成"
else
    echo "⚠️  ImageMagick未安装，创建空文件..."
    
    # 创建空的占位符文件
    touch application/resources/images/{player.png,enemies.png,items.png,main_menu_bg.jpg,app_icon.png}
    touch application/resources/icons/{new_game,load_game,settings,exit,warrior,mage,archer,inventory,quests,skills,skill_1,skill_2,skill_3}.png
fi

# 创建占位符音频文件
echo "🎵 创建占位符音频文件..."
touch application/resources/sounds/{bgm.mp3,effects.wav}

# 创建占位符字体文件
echo "🔤 创建占位符字体文件..."
touch application/resources/fonts/game_font.ttf

# 创建配置文件
echo "⚙️ 创建配置文件..."
cat > application/resources/configs/game_config.json << 'EOF'
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

cat > application/resources/configs/ui_config.json << 'EOF'
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

echo "✅ 所有占位符资源文件创建完成！"
echo ""
echo "📝 注意事项："
echo "1. 这些是占位符文件，请替换为实际的游戏资源"
echo "2. 图片文件可能需要实际的PNG/JPG格式"
echo "3. 音频文件需要实际的MP3/WAV格式"
echo "4. 字体文件需要实际的TTF格式"
echo ""
echo "🚀 现在可以尝试重新编译项目："
echo "   cd application/build && make -j6"