cmake_minimum_required(VERSION 3.15)
project(RateTracker)

set(CMAKE_CXX_STANDARD 17)

set(OPENSSL_ROOT_DIR /opt/homebrew/opt/openssl@3)

find_package(cpr REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(httplib REQUIRED)

# Main server
add_executable(rate_tracker main.cpp)
target_link_libraries(rate_tracker PRIVATE cpr::cpr nlohmann_json::nlohmann_json httplib::httplib)

# Copy the public folder into the build directory so the server can find it at runtime
add_custom_command(TARGET rate_tracker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/public
    $<TARGET_FILE_DIR:rate_tracker>/public
)

# Unit tests (pure logic only, no network needed to run these)
add_executable(run_tests test_engine.cpp)
target_link_libraries(run_tests PRIVATE cpr::cpr nlohmann_json::nlohmann_json)