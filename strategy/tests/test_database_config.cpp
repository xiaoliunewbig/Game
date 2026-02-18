/*
 * test_database_config.cpp
 * DatabaseConfig unit tests
 */
#include <gtest/gtest.h>
#include "database/DatabaseConfig.h"

using namespace strategy;

TEST(DatabaseConfig, DefaultValues) {
    DatabaseConfig config;
    EXPECT_EQ(config.max_connections, 10);
    EXPECT_EQ(config.min_connections, 1);
    EXPECT_EQ(config.connection_timeout, 30);
}

TEST(DatabaseConfig, PostgreSQLConnectionString) {
    DatabaseConfig config;
    config.type = DatabaseType::POSTGRESQL;
    config.host = "localhost";
    config.port = 5432;
    config.database = "gamedb";
    config.username = "admin";
    config.password = "secret";

    std::string conn = config.GetConnectionString();
    EXPECT_NE(conn.find("host=localhost"), std::string::npos);
    EXPECT_NE(conn.find("port=5432"), std::string::npos);
    EXPECT_NE(conn.find("dbname=gamedb"), std::string::npos);
    EXPECT_NE(conn.find("user=admin"), std::string::npos);
    EXPECT_NE(conn.find("password=secret"), std::string::npos);
}

TEST(DatabaseConfig, MySQLConnectionString) {
    DatabaseConfig config;
    config.type = DatabaseType::MYSQL;
    config.host = "db.example.com";
    config.port = 3306;
    config.database = "mydb";
    config.username = "root";
    config.password = "pass";

    std::string conn = config.GetConnectionString();
    EXPECT_NE(conn.find("mysql://"), std::string::npos);
    EXPECT_NE(conn.find("root:pass@"), std::string::npos);
    EXPECT_NE(conn.find("db.example.com:3306"), std::string::npos);
    EXPECT_NE(conn.find("/mydb"), std::string::npos);
}

TEST(DatabaseConfig, SQLiteConnectionStringIsJustPath) {
    DatabaseConfig config;
    config.type = DatabaseType::SQLITE;
    config.database = "/data/game.db";

    std::string conn = config.GetConnectionString();
    EXPECT_EQ(conn, "/data/game.db");
}

TEST(DatabaseConfig, MongoDBConnectionString) {
    DatabaseConfig config;
    config.type = DatabaseType::MONGODB;
    config.host = "mongo.example.com";
    config.port = 27017;
    config.database = "gamedb";
    config.username = "user";
    config.password = "pass";

    std::string conn = config.GetConnectionString();
    EXPECT_NE(conn.find("mongodb://"), std::string::npos);
    EXPECT_NE(conn.find("user:pass@"), std::string::npos);
    EXPECT_NE(conn.find("mongo.example.com:27017"), std::string::npos);
    EXPECT_NE(conn.find("/gamedb"), std::string::npos);
}

TEST(DatabaseConfig, PostgreSQLConnectionStringWithExtraParams) {
    DatabaseConfig config;
    config.type = DatabaseType::POSTGRESQL;
    config.host = "localhost";
    config.port = 5432;
    config.database = "gamedb";
    config.username = "admin";
    config.password = "secret";
    config.extra_params["sslmode"] = "require";

    std::string conn = config.GetConnectionString();
    EXPECT_NE(conn.find("sslmode=require"), std::string::npos);
}

TEST(DatabaseConfig, MySQLNoPasswordOmitsColon) {
    DatabaseConfig config;
    config.type = DatabaseType::MYSQL;
    config.host = "localhost";
    config.port = 3306;
    config.database = "db";
    config.username = "user";
    config.password = "";

    std::string conn = config.GetConnectionString();
    // Should be "mysql://user@localhost:3306/db" without ":@"
    EXPECT_NE(conn.find("mysql://user@"), std::string::npos);
    EXPECT_EQ(conn.find("user:@"), std::string::npos);
}

TEST(DatabaseConfig, MySQLNoUsernameOrPassword) {
    DatabaseConfig config;
    config.type = DatabaseType::MYSQL;
    config.host = "localhost";
    config.port = 3306;
    config.database = "db";
    config.username = "";
    config.password = "";

    std::string conn = config.GetConnectionString();
    EXPECT_EQ(conn, "mysql://localhost:3306/db");
}
