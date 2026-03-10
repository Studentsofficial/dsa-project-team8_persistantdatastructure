#include "Database.h"
#include <iostream>

Database::Database(const std::string& db_name) {
    if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error opening SQLite database: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Successfully connected to SQLite database: " << db_name << std::endl;
    }
}

Database::~Database() {
    sqlite3_close(db);
}

bool Database::initTables() {
    // Table 1: Stores the Persistent Merkle Tree nodes (Structural Sharing)
    std::string createNodesTable = R"(
        CREATE TABLE IF NOT EXISTS Nodes (
            hash TEXT PRIMARY KEY,
            left_child TEXT,
            right_child TEXT,
            data TEXT
        );
    )";

    // Table 2: Stores the Time-Travel Anchors (Root Hashes)
    std::string createHistoryTable = R"(
        CREATE TABLE IF NOT EXISTS History (
            version_id INTEGER PRIMARY KEY AUTOINCREMENT,
            root_hash TEXT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    char* errMsg = nullptr;

    // Execute Table 1
    if (sqlite3_exec(db, createNodesTable.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "Failed to create Nodes table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // Execute Table 2
    if (sqlite3_exec(db, createHistoryTable.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "Failed to create History table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    std::cout << "Database tables initialized successfully!" << std::endl;
    return true;
} // <-- Your new functions needed to go AFTER this brace!

// Saves a single node to the database
bool Database::saveNode(const std::string& hash, const std::string& left, const std::string& right, const std::string& data) {
    std::string sql = "INSERT OR IGNORE INTO Nodes (hash, left_child, right_child, data) VALUES ('" +
                      hash + "', '" + left + "', '" + right + "', '" + data + "');";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL Error (saveNode): " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

// Anchors the Root Hash into the Timeline
bool Database::saveVersion(const std::string& root_hash) {
    std::string sql = "INSERT INTO History (root_hash) VALUES ('" + root_hash + "');";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL Error (saveVersion): " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}
// Fetches the Time-Travel Timeline from SQLite
std::vector<std::string> Database::getHistory() {
    std::vector<std::string> history;
    std::string sql = "SELECT root_hash FROM History ORDER BY version_id ASC;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* hash = sqlite3_column_text(stmt, 0);
            if (hash) {
                history.push_back(reinterpret_cast<const char*>(hash));
            }
        }
        sqlite3_finalize(stmt);
    }
    return history;
}
#include "Node.h" // Needed for calculateHash()

// UPGRADE 2: Fetches all nodes to draw the live graph
std::vector<NodeRecord> Database::getAllNodes() {
    std::vector<NodeRecord> nodes;
    std::string sql = "SELECT hash, left_child, right_child, data FROM Nodes;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            NodeRecord nr;
            nr.hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            nr.left = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            nr.right = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            nr.data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            nodes.push_back(nr);
        }
        sqlite3_finalize(stmt);
    }
    return nodes;
}

// UPGRADE 1: The Tamper-Evident Scanner
std::string Database::verifyIntegrity() {
    std::vector<NodeRecord> nodes = getAllNodes();
    for(const auto& node : nodes) {
        std::string expected_hash;
        if(node.data == "INTERNAL_NODE") {
            expected_hash = calculateHash(node.left + node.right); // Check structural pointers
        } else {
            expected_hash = calculateHash(node.data); // Check raw evidence
        }

        if(expected_hash != node.hash) {
            return node.hash; // TAMPERING DETECTED! Return the broken node.
        }
    }
    return "SAFE"; // All hashes match cryptographically.
}
