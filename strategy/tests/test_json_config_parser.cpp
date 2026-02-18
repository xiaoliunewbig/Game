/*
 * test_json_config_parser.cpp
 * JsonConfigParser unit tests
 */
#include <gtest/gtest.h>
#include "database/JsonConfigParser.h"
#include <cstdlib>

using namespace strategy;

// ============================================================================
// ParseSingleConfig tests
// ============================================================================

TEST(JsonConfigParser, ParseSingleConfigAllFields) {
    std::string json = R"({
        "type": "postgresql",
        "host": "db.example.com",
        "port": 5433,
        "database": "gamedb",
        "username": "admin",
        "password": "secret123",
        "max_connections": 20,
        "min_connections": 5,
        "connection_timeout": 60
    })";

    auto config = JsonConfigParser::ParseSingleConfig(json);
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->type, DatabaseType::POSTGRESQL);
    EXPECT_EQ(config->host, "db.example.com");
    EXPECT_EQ(config->port, 5433);
    EXPECT_EQ(config->database, "gamedb");
    EXPECT_EQ(config->username, "admin");
    EXPECT_EQ(config->password, "secret123");
    EXPECT_EQ(config->max_connections, 20);
    EXPECT_EQ(config->min_connections, 5);
    EXPECT_EQ(config->connection_timeout, 60);
}

TEST(JsonConfigParser, ParseSingleConfigDefaults) {
    // Missing host and port should use defaults
    std::string json = R"({
        "type": "postgresql",
        "database": "testdb",
        "username": "user"
    })";

    auto config = JsonConfigParser::ParseSingleConfig(json);
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->host, "localhost");
    EXPECT_EQ(config->port, 5432);
    EXPECT_EQ(config->max_connections, 10);
    EXPECT_EQ(config->min_connections, 1);
    EXPECT_EQ(config->connection_timeout, 30);
}

TEST(JsonConfigParser, ParseSingleConfigMysqlType) {
    std::string json = R"({
        "type": "mysql",
        "host": "localhost",
        "port": 3306,
        "database": "mydb"
    })";

    auto config = JsonConfigParser::ParseSingleConfig(json);
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->type, DatabaseType::MYSQL);
}

TEST(JsonConfigParser, ParseSingleConfigNoType) {
    // Missing type should default to POSTGRESQL
    std::string json = R"({
        "host": "localhost",
        "database": "testdb"
    })";

    auto config = JsonConfigParser::ParseSingleConfig(json);
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->type, DatabaseType::POSTGRESQL);
}

// ============================================================================
// ParseDatabaseType tests (indirectly through ParseSingleConfig)
// ============================================================================

TEST(JsonConfigParser, ParseTypePostgresql) {
    auto config = JsonConfigParser::ParseSingleConfig(R"({"type": "postgresql", "database": "db"})");
    EXPECT_EQ(config->type, DatabaseType::POSTGRESQL);
}

TEST(JsonConfigParser, ParseTypePostgres) {
    auto config = JsonConfigParser::ParseSingleConfig(R"({"type": "postgres", "database": "db"})");
    EXPECT_EQ(config->type, DatabaseType::POSTGRESQL);
}

TEST(JsonConfigParser, ParseTypePgsql) {
    auto config = JsonConfigParser::ParseSingleConfig(R"({"type": "pgsql", "database": "db"})");
    EXPECT_EQ(config->type, DatabaseType::POSTGRESQL);
}

TEST(JsonConfigParser, ParseTypeMysql) {
    auto config = JsonConfigParser::ParseSingleConfig(R"({"type": "mysql", "database": "db"})");
    EXPECT_EQ(config->type, DatabaseType::MYSQL);
}

TEST(JsonConfigParser, ParseTypeSqlite) {
    auto config = JsonConfigParser::ParseSingleConfig(R"({"type": "sqlite", "database": "test.db"})");
    EXPECT_EQ(config->type, DatabaseType::SQLITE);
}

TEST(JsonConfigParser, ParseTypeMongodb) {
    auto config = JsonConfigParser::ParseSingleConfig(R"({"type": "mongodb", "database": "db"})");
    EXPECT_EQ(config->type, DatabaseType::MONGODB);
}

// ============================================================================
// ParseDatabaseConfigs tests
// ============================================================================

TEST(JsonConfigParser, ParseMultipleDatabaseConfigs) {
    std::string json = R"({
        "databases": {
            "primary": {
                "type": "postgresql",
                "host": "primary.db.com",
                "port": 5432,
                "database": "game_main",
                "username": "admin",
                "password": "pass1"
            },
            "readonly": {
                "type": "postgresql",
                "host": "readonly.db.com",
                "port": 5432,
                "database": "game_main",
                "username": "reader",
                "password": "pass2"
            }
        }
    })";

    auto configs = JsonConfigParser::ParseDatabaseConfigs(json);
    EXPECT_EQ(configs.size(), 2u);
    EXPECT_NE(configs.find("primary"), configs.end());
    EXPECT_NE(configs.find("readonly"), configs.end());
    EXPECT_EQ(configs["primary"]->host, "primary.db.com");
    EXPECT_EQ(configs["readonly"]->host, "readonly.db.com");
}

TEST(JsonConfigParser, ParseSingleConfigAsDatabases) {
    // Without "databases" wrapper, should be treated as single "default" config
    std::string json = R"({
        "type": "postgresql",
        "host": "localhost",
        "database": "testdb"
    })";

    auto configs = JsonConfigParser::ParseDatabaseConfigs(json);
    EXPECT_EQ(configs.size(), 1u);
    EXPECT_NE(configs.find("default"), configs.end());
}

// ============================================================================
// ValidateConfig tests
// ============================================================================

TEST(JsonConfigParser, ValidateConfigValidReturnsEmpty) {
    DatabaseConfig config;
    config.type = DatabaseType::POSTGRESQL;
    config.host = "localhost";
    config.port = 5432;
    config.database = "testdb";
    config.max_connections = 10;
    config.min_connections = 1;
    config.connection_timeout = 30;

    auto errors = JsonConfigParser::ValidateConfig(config);
    EXPECT_TRUE(errors.empty());
}

TEST(JsonConfigParser, ValidateConfigInvalidPort) {
    DatabaseConfig config;
    config.host = "localhost";
    config.database = "testdb";
    config.port = 0;
    config.max_connections = 10;
    config.min_connections = 1;
    config.connection_timeout = 30;

    auto errors = JsonConfigParser::ValidateConfig(config);
    EXPECT_FALSE(errors.empty());
}

TEST(JsonConfigParser, ValidateConfigPortTooHigh) {
    DatabaseConfig config;
    config.host = "localhost";
    config.database = "testdb";
    config.port = 70000;
    config.max_connections = 10;
    config.min_connections = 1;
    config.connection_timeout = 30;

    auto errors = JsonConfigParser::ValidateConfig(config);
    EXPECT_FALSE(errors.empty());
}

TEST(JsonConfigParser, ValidateConfigEmptyHost) {
    DatabaseConfig config;
    config.host = "";
    config.database = "testdb";
    config.port = 5432;
    config.max_connections = 10;
    config.min_connections = 1;
    config.connection_timeout = 30;

    auto errors = JsonConfigParser::ValidateConfig(config);
    EXPECT_FALSE(errors.empty());
}

TEST(JsonConfigParser, ValidateConfigEmptyDatabase) {
    DatabaseConfig config;
    config.host = "localhost";
    config.database = "";
    config.port = 5432;
    config.max_connections = 10;
    config.min_connections = 1;
    config.connection_timeout = 30;

    auto errors = JsonConfigParser::ValidateConfig(config);
    EXPECT_FALSE(errors.empty());
}

TEST(JsonConfigParser, ValidateConfigMinGreaterThanMax) {
    DatabaseConfig config;
    config.host = "localhost";
    config.database = "testdb";
    config.port = 5432;
    config.max_connections = 5;
    config.min_connections = 10;
    config.connection_timeout = 30;

    auto errors = JsonConfigParser::ValidateConfig(config);
    EXPECT_FALSE(errors.empty());
}

TEST(JsonConfigParser, ValidateConfigZeroTimeout) {
    DatabaseConfig config;
    config.host = "localhost";
    config.database = "testdb";
    config.port = 5432;
    config.max_connections = 10;
    config.min_connections = 1;
    config.connection_timeout = 0;

    auto errors = JsonConfigParser::ValidateConfig(config);
    EXPECT_FALSE(errors.empty());
}

// ============================================================================
// Environment variable processing
// ============================================================================

TEST(JsonConfigParser, ProcessEnvironmentVariables) {
    // Set a test environment variable
#ifdef _WIN32
    _putenv_s("GAME_TEST_HOST", "env-host.example.com");
#else
    setenv("GAME_TEST_HOST", "env-host.example.com", 1);
#endif

    std::string json = R"({
        "type": "postgresql",
        "host": "${GAME_TEST_HOST}",
        "database": "testdb"
    })";

    auto config = JsonConfigParser::ParseSingleConfig(json);
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->host, "env-host.example.com");

    // Clean up
#ifdef _WIN32
    _putenv_s("GAME_TEST_HOST", "");
#else
    unsetenv("GAME_TEST_HOST");
#endif
}

TEST(JsonConfigParser, UnsetEnvironmentVariableReplacedWithEmpty) {
    std::string json = R"({
        "type": "postgresql",
        "host": "${UNLIKELY_TO_EXIST_VAR_12345}",
        "database": "testdb"
    })";

    auto config = JsonConfigParser::ParseSingleConfig(json);
    ASSERT_NE(config, nullptr);
    // Unset env var replaced with empty string; empty host falls back to "localhost"
    EXPECT_EQ(config->host, "localhost");
}
