cmake_minimum_required(VERSION 3.10)
project(ChronosLocker)

# Use C++17 standard (Required for Crow)
set(CMAKE_CXX_STANDARD 17)

# Find OpenSSL for hashing
find_package(OpenSSL REQUIRED)

# Tell CMake where to look for header files
include_directories(include)
include_directories(${OPENSSL_INCLUDE_DIR})

# Create the executable from our source code
add_executable(chronos_server src/main.cpp src/Database.cpp)

# Link Crow's threading (pthread), SQLite3, and OpenSSL Crypto
target_link_libraries(chronos_server pthread sqlite3 ${OPENSSL_CRYPTO_LIBRARY})