# 测试配置
enable_testing()

# 添加测试可执行文件
add_executable(strategy_tests
    test_strategy_service.cpp
    test_database_config.cpp
    test_player_service.cpp
)

target_link_libraries(strategy_tests 
    strategy_service
    gtest 
    gtest_main
    pthread
)

# 添加测试
add_test(NAME StrategyTests COMMAND strategy_tests)