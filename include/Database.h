#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>

// A struct to hold node data for our graph
struct NodeRecord {
    std::string hash;
    std::string left;
    std::string right;
    std::string data;
};

class Database {
private:
    sqlite3* db;

public:
    Database(const std::string& db_name);
    ~Database();

    bool initTables();
    bool saveNode(const std::string& hash, const std::string& left, const std::string& right, const std::string& data);
    bool saveVersion(const std::string& root_hash);
    std::vector<std::string> getHistory(); 
    
    // NEW: Upgrades 1 & 2
    std::vector<NodeRecord> getAllNodes(); 
    std::string verifyIntegrity();
};

#endif