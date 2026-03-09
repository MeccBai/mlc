set(PUBLIC_PATH "${CMAKE_CURRENT_LIST_DIR}")

file(GLOB MODULES_FILES ${PUBLIC_PATH}/*.cppm)

add_library(public)

target_sources(public PUBLIC FILE_SET CXX_MODULES FILES ${MODULES_FILES})

find_package(taskflow CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(public PRIVATE nlohmann_json::nlohmann_json)