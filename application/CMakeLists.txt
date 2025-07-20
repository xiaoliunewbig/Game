# Qt应用层 CMakeLists.txt
# 文件名: CMakeLists.txt
# 说明: Qt应用层构建配置文件
# 作者: 彭承康
# 创建时间: 2025-07-20

cmake_minimum_required(VERSION 3.16)

# 设置项目名称
project(GameApplication VERSION 1.0.0 LANGUAGES CXX)

# C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6组件
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Quick QuickControls2 Multimedia Network)

# 查找其他依赖
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# 查找gRPC
pkg_check_modules(GRPC REQUIRED grpc++)
pkg_check_modules(PROTOBUF REQUIRED protobuf)

# 启用Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 添加应用程序可执行文件
qt6_add_executable(game_client
    src/main.cpp
    src/GameApplication.cpp
    src/core/GameEngine.cpp
    src/core/SceneManager.cpp
    src/core/ResourceManager.cpp
    src/core/EventSystem.cpp
    src/ui/GameWindow.cpp
    src/ui/MainMenuWindow.cpp
    src/ui/GameplayWindow.cpp
    src/ui/InventoryWindow.cpp
    src/network/StrategyServiceClient.cpp
    src/network/AlgorithmServiceClient.cpp
    src/network/NetworkManager.cpp
    src/game/Player.cpp
    src/game/GameState.cpp
    src/game/BattleSystem.cpp
    src/game/InventorySystem.cpp
    src/audio/AudioManager.cpp
    src/input/InputManager.cpp
    src/graphics/SpriteRenderer.cpp
    src/graphics/AnimationManager.cpp
    src/config/GameConfig.cpp
    src/config/ConfigManager.cpp
    src/save/SaveManager.cpp
    src/utils/Logger.cpp
    src/utils/PerformanceMonitor.cpp
)

# QML资源文件
qt6_add_qml_module(game_client
    URI GameUI
    VERSION 1.0
    QML_FILES
        qml/main.qml
        qml/MainMenu.qml
        qml/GameplayView.qml
        qml/BattleView.qml
        qml/InventoryView.qml
        qml/SettingsView.qml
        qml/components/PlayerInfo.qml
        qml/components/SkillTree.qml
        qml/components/ItemSlot.qml
        qml/components/MainMenu.qml
        qml/components/MenuButton.qml
        qml/components/ProfessionCard.qml
        qml/components/SaveSlotItem.qml
        qml/components/GameplayUI.qml
        qml/components/PlayerInfoPanel.qml
        qml/components/GameStatePanel.qml
        qml/components/QuickActionButton.qml
        qml/components/SkillSlot.qml
        qml/components/BattleUI.qml
        qml/components/EnemyCard.qml
        qml/components/ActionButton.qml
        qml/components/InventoryGrid.qml
        qml/components/ItemCard.qml
)

# 资源文件
qt6_add_resources(game_client "resources"
    PREFIX "/"
    FILES
        resources/images/player.png
        resources/images/enemies.png
        resources/images/items.png
        resources/images/main_menu_bg.jpg
        resources/images/app_icon.png
        resources/sounds/bgm.mp3
        resources/sounds/effects.wav
        resources/fonts/game_font.ttf
        resources/icons/new_game.png
        resources/icons/load_game.png
        resources/icons/settings.png
        resources/icons/exit.png
        resources/icons/warrior.png
        resources/icons/mage.png
        resources/icons/archer.png
        resources/icons/inventory.png
        resources/icons/quests.png
        resources/icons/skills.png
        resources/icons/skill_1.png
        resources/icons/skill_2.png
        resources/icons/skill_3.png
        resources/configs/game_config.json
        resources/configs/ui_config.json
)

# 头文件目录
target_include_directories(game_client PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../algorithm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../strategy/include
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../strategy/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../algorithm/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../proto"
    ${GRPC_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(game_client PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
    Qt6::Network
    Threads::Threads
    ${GRPC_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
)

# 编译选项
target_compile_options(game_client PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
    ${GRPC_CFLAGS_OTHER}
    ${PROTOBUF_CFLAGS_OTHER}
)

# 设置Qt Quick样式
set_target_properties(game_client PROPERTIES
    QT_QML_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/qml"
    QT_QUICK_CONTROLS_STYLE "Material"
)

# 安装规则
install(TARGETS game_client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 复制资源文件到构建目录
add_custom_command(TARGET game_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:game_client>/resources
)

# 链接策略层和算法层
# target_link_libraries(game_client
#     "${CMAKE_CURRENT_SOURCE_DIR}/../strategy/build/libstrategy_service.a"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../algorithm/build/libalgorithm_service.a"
# )
