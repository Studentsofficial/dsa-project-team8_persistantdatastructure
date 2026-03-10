#include "crow_all.h"
#include "Database.h"
#include "Node.h"
#include <fstream>
#include <sstream>

int main() {
    Database db("chronos.db");
    db.initTables();

    std::string current_root_hash = "GENESIS_00000000000000000000";

    crow::SimpleApp app;

    // --- NEW: Serve the HTML Dashboard ---
    CROW_ROUTE(app, "/")([](){
        std::ifstream file("index.html");
        if (!file.is_open()) {
            return crow::response(404, "Error: index.html not found! Make sure it is inside the 'build' folder.");
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        auto response = crow::response(200, buffer.str());
        response.add_header("Content-Type", "text/html");
        return response;
    });

    // THE CORE API ENDPOINT: Add Evidence
    CROW_ROUTE(app, "/api/add").methods(crow::HTTPMethod::POST)([&db, &current_root_hash](const crow::request& req){

        auto body = crow::json::load(req.body);
        if (!body || !body.has("data")) return crow::response(400, "Invalid JSON");
        std::string new_evidence = body["data"].s();

        Node leaf(new_evidence);
        Node new_root(current_root_hash, leaf.hash);

        db.saveNode(leaf.hash, leaf.left_hash, leaf.right_hash, leaf.data);
        db.saveNode(new_root.hash, new_root.left_hash, new_root.right_hash, new_root.data);
        db.saveVersion(new_root.hash);

        current_root_hash = new_root.hash;
        std::cout << "[+] New Version Created! Root Hash: " << current_root_hash << std::endl;

        crow::json::wvalue res;
        res["status"] = "success";
        res["new_root_hash"] = current_root_hash;
        return crow::response(200, res);
    });
    // THE TIME MACHINE ENDPOINT: Get all historical versions
    CROW_ROUTE(app, "/api/history")([&db](){
        std::vector<std::string> history = db.getHistory();
        crow::json::wvalue res;
        res["history"] = history;
        return crow::response(200, res);
    });
    // UPGRADE 2 ROUTE: Send tree data to the frontend graph
    CROW_ROUTE(app, "/api/tree")([&db](){
        auto nodes = db.getAllNodes();
        crow::json::wvalue res;
        int i = 0;
        for(const auto& n : nodes) {
            res["nodes"][i]["id"] = n.hash;
            res["nodes"][i]["left"] = n.left;
            res["nodes"][i]["right"] = n.right;
            res["nodes"][i]["data"] = n.data;
            i++;
        }
        return crow::response(200, res);
    });

    // UPGRADE 1 ROUTE: Run the Cryptographic Integrity Scan
    CROW_ROUTE(app, "/api/verify")([&db](){
        std::string status = db.verifyIntegrity();
        crow::json::wvalue res;
        if(status == "SAFE") {
            res["status"] = "SAFE";
        } else {
            res["status"] = "TAMPERED";
            res["broken_node"] = status;
        }
        return crow::response(200, res);
    });
    app.bindaddr("127.0.0.1").port(8080).multithreaded().run();
}
