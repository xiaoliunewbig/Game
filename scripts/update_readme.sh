#!/bin/bash

# ============================================================================
# 《幻境传说》游戏框架 - README更新脚本
# ============================================================================
# 文件名: scripts/update_readme.sh
# 说明: 更新README.md中的GitHub仓库链接
# 作者: 彭承康
# 创建时间: 2026-02-18
# ============================================================================

set -e

# 获取当前Git远程仓库URL
if git remote get-url origin >/dev/null 2>&1; then
    REPO_URL=$(git remote get-url origin)
    
    # 转换SSH URL为HTTPS URL
    if [[ $REPO_URL == git@github.com:* ]]; then
        REPO_URL=$(echo "$REPO_URL" | sed 's/git@github.com:/https:\/\/github.com\//' | sed 's/\.git$//')
    fi
    
    # 移除.git后缀
    REPO_URL=$(echo "$REPO_URL" | sed 's/\.git$//')
    
    echo "检测到仓库URL: $REPO_URL"
    
    # 更新README.md中的链接
    if [ -f "README.md" ]; then
        # 备份原文件
        cp README.md README.md.bak
        
        # 替换仓库链接
        sed -i.tmp "s|https://github.com/xiaoliunewbig/Game|$REPO_URL|g" README.md
        
        # 清理临时文件
        rm -f README.md.tmp
        
        echo "✅ README.md已更新仓库链接"
    else
        echo "❌ 未找到README.md文件"
    fi
else
    echo "❌ 未检测到Git远程仓库"
fi