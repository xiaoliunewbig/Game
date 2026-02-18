/*
 * 文件名: PlayerService.cpp
 * 说明: 玩家服务类的实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 */
#include "player_service/PlayerService.h"
#include "security/PasswordHasher.h"
#include <regex>
#include <iostream>

namespace strategy {

PlayerService::PlayerService(std::unique_ptr<IPlayerRepository> repository)
    : repository_(std::move(repository)) {
}

RegisterResult PlayerService::RegisterPlayer(const std::string& username, const std::string& password, const std::string& email) {
    RegisterResult result;
    result.success = false;

    if (!IsValidUsername(username)) {
        result.message = "用户名格式无效";
        return result;
    }

    if (password.length() < 6) {
        result.message = "密码长度至少6位";
        return result;
    }

    if (!IsValidEmail(email)) {
        result.message = "邮箱格式无效";
        return result;
    }

    auto existing_player = repository_->FindPlayerByUsername(username);
    if (existing_player.has_value()) {
        result.message = "用户名已存在";
        return result;
    }

    std::string password_hash = HashPassword(password);
    auto new_player = repository_->CreatePlayer(username, password_hash, email);
    
    if (new_player.has_value()) {
        result.success = true;
        result.message = "注册成功";
        result.player = new_player;
    } else {
        result.message = "注册失败，请稍后重试";
    }

    return result;
}

LoginResult PlayerService::LoginPlayer(const std::string& username, const std::string& password) {
    LoginResult result;
    result.success = false;

    auto player = repository_->FindPlayerByUsername(username);
    if (!player.has_value()) {
        result.message = "用户名或密码错误";
        return result;
    }

    if (!VerifyPassword(password, player->password_hash)) {
        result.message = "用户名或密码错误";
        return result;
    }

    result.success = true;
    result.message = "登录成功";
    result.player = player;
    return result;
}

std::optional<Player> PlayerService::GetPlayerById(long long id) {
    return repository_->FindPlayerById(id);
}

bool PlayerService::UpdatePlayerEmail(long long id, const std::string& new_email) {
    if (!IsValidEmail(new_email)) {
        return false;
    }

    auto player = repository_->FindPlayerById(id);
    if (!player.has_value()) {
        return false;
    }

    player->email = new_email;
    return repository_->UpdatePlayer(*player);
}

bool PlayerService::UpdatePlayerPassword(long long id, const std::string& old_password, const std::string& new_password) {
    if (new_password.length() < 6) {
        return false;
    }

    auto player = repository_->FindPlayerById(id);
    if (!player.has_value()) {
        return false;
    }

    if (!VerifyPassword(old_password, player->password_hash)) {
        return false;
    }

    player->password_hash = HashPassword(new_password);
    return repository_->UpdatePlayer(*player);
}

bool PlayerService::DeletePlayer(long long id) {
    return repository_->DeletePlayer(id);
}

std::string PlayerService::HashPassword(const std::string& password) {
    return PasswordHasher::hashPassword(password);
}

bool PlayerService::VerifyPassword(const std::string& password, const std::string& hash) {
    return PasswordHasher::verifyPassword(password, hash);
}

bool PlayerService::IsValidEmail(const std::string& email) {
    const std::regex email_pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, email_pattern);
}

bool PlayerService::IsValidUsername(const std::string& username) {
    if (username.length() < 3 || username.length() > 20) {
        return false;
    }
    
    const std::regex username_pattern(R"([a-zA-Z0-9_]+)");
    return std::regex_match(username, username_pattern);
}

} // namespace strategy

