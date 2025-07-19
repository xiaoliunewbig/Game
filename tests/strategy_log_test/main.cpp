#include "Log/AsyncLogService.h"
#include "Log/LogServiceConsole.h"
#include "Log/LogServiceFile.h"
#include "Log/LogMacros.h"
#include "player_service/PlayerServiceFactory.h"
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>

void TestLoggingSystem(strategy::ILogService& logger) {
    std::cout << "\n--- 日志系统测试开始 ---" << std::endl;
    STRATEGY_LOG(logger, strategy::LogLevel::INFO, "LoggingTest", "开始测试日志系统...");
    STRATEGY_LOG(logger, strategy::LogLevel::ERROR, "LoggingTest", "这是一条测试错误日志。");
    std::cout << "--- 日志系统测试结束 ---\n" << std::endl;
}

void TestPlayerService() {
    std::cout << "\n--- 玩家服务完整测试开始 ---" << std::endl;
    const std::string conn_str = "dbname=game_db user=game_user password=game_password hostaddr=127.0.0.1 port=5432";
    const std::string test_username = "service_test_user";
    const std::string test_password = "password123";
    const std::string test_email = "service_test@example.com";

    try {
        auto player_service = strategy::PlayerServiceFactory::CreatePostgresPlayerService(conn_str);

        // 清理可能存在的旧测试数据
        auto old_player = player_service->GetPlayerById(1);
        if (old_player && old_player->username == test_username) {
            player_service->DeletePlayer(old_player->id);
        }

        // 1. 测试注册
        std::cout << "\n[1. 注册测试] 正在注册新用户..." << std::endl;
        auto register_result = player_service->RegisterPlayer(test_username, test_password, test_email);
        
        if (register_result.success) {
            std::cout << "✓ 注册成功: " << register_result.message << std::endl;
            std::cout << "  用户ID: " << register_result.player->id << std::endl;
            std::cout << "  用户名: " << register_result.player->username << std::endl;
            std::cout << "  邮箱: " << register_result.player->email << std::endl;

            // 2. 测试登录
            std::cout << "\n[2. 登录测试] 正在登录..." << std::endl;
            auto login_result = player_service->LoginPlayer(test_username, test_password);
            
            if (login_result.success) {
                std::cout << "✓ 登录成功: " << login_result.message << std::endl;
                
                long long player_id = login_result.player->id;

                // 3. 测试更新邮箱
                std::cout << "\n[3. 更新邮箱测试]" << std::endl;
                bool email_update = player_service->UpdatePlayerEmail(player_id, "updated_email@example.com");
                std::cout << (email_update ? "✓" : "✗") << " 更新邮箱: " << (email_update ? "成功" : "失败") << std::endl;

                // 4. 测试更新密码
                std::cout << "\n[4. 更新密码测试]" << std::endl;
                bool password_update = player_service->UpdatePlayerPassword(player_id, test_password, "newpassword123");
                std::cout << (password_update ? "✓" : "✗") << " 更新密码: " << (password_update ? "成功" : "失败") << std::endl;

                // 5. 测试用新密码登录
                if (password_update) {
                    std::cout << "\n[5. 新密码登录测试]" << std::endl;
                    auto new_login = player_service->LoginPlayer(test_username, "newpassword123");
                    std::cout << (new_login.success ? "✓" : "✗") << " 新密码登录: " << new_login.message << std::endl;
                }

                // 6. 测试获取玩家信息
                std::cout << "\n[6. 获取玩家信息测试]" << std::endl;
                auto player_info = player_service->GetPlayerById(player_id);
                if (player_info) {
                    std::cout << "✓ 获取玩家信息成功" << std::endl;
                    std::cout << "  当前邮箱: " << player_info->email << std::endl;
                } else {
                    std::cout << "✗ 获取玩家信息失败" << std::endl;
                }

                // 7. 测试删除玩家
                std::cout << "\n[7. 删除玩家测试]" << std::endl;
                bool delete_result = player_service->DeletePlayer(player_id);
                std::cout << (delete_result ? "✓" : "✗") << " 删除玩家: " << (delete_result ? "成功" : "失败") << std::endl;

            } else {
                std::cout << "✗ 登录失败: " << login_result.message << std::endl;
            }
        } else {
            std::cout << "✗ 注册失败: " << register_result.message << std::endl;
        }

        // 8. 测试无效输入
        std::cout << "\n[8. 输入验证测试]" << std::endl;
        auto invalid_register = player_service->RegisterPlayer("ab", "123", "invalid-email");
        std::cout << (invalid_register.success ? "✗" : "✓") << " 无效输入验证: " << invalid_register.message << std::endl;

    } catch (const std::exception& e) {
        std::cout << "✗ 测试异常: " << e.what() << std::endl;
    }
    
    std::cout << "--- 玩家服务完整测试结束 ---\n" << std::endl;
}

int main() {
    // 设置日志系统
    auto console_logger = std::make_shared<strategy::LogServiceConsole>();
    auto file_logger = std::make_shared<strategy::LogServiceFile>("test_log.txt");
    std::vector<std::shared_ptr<strategy::ILogService>> loggers = {console_logger, file_logger};
    strategy::AsyncLogService async_logger(loggers);

    // 运行测试
    TestLoggingSystem(async_logger);
    TestPlayerService();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return 0;
} 
