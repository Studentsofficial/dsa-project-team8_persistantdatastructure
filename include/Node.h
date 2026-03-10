#ifndef NODE_H
#define NODE_H

#include <string>
#include <openssl/evp.h>  // Changed from sha.h to the modern evp.h
#include <iomanip>
#include <sstream>

// Helper function: Calculates SHA-256 using modern OpenSSL 3.0 EVP API
inline std::string calculateHash(const std::string& input) {
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha256();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;

    EVP_DigestInit_ex(context, md, nullptr);
    EVP_DigestUpdate(context, input.c_str(), input.size());
    EVP_DigestFinal_ex(context, hash, &lengthOfHash);
    EVP_MD_CTX_free(context);
    
    std::stringstream ss;
    for(unsigned int i = 0; i < lengthOfHash; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// Our Persistent Data Structure Node
struct Node {
    std::string hash;
    std::string left_hash;
    std::string right_hash;
    std::string data;

    // Constructor 1: For a Data Leaf (e.g., New Evidence)
    Node(const std::string& evidence_data) {
        data = evidence_data;
        left_hash = "NULL";
        right_hash = "NULL";
        hash = calculateHash(data); // Hash the data
    }

    // Constructor 2: For a Parent Node (Structural Sharing)
    Node(const std::string& old_root_hash, const std::string& new_leaf_hash) {
        data = "INTERNAL_NODE";
        left_hash = old_root_hash;   // Points to the entire old history!
        right_hash = new_leaf_hash;  // Points to the new evidence
        hash = calculateHash(left_hash + right_hash); // The Merkle mechanism
    }
};

#endif