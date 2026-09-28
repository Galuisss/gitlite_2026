cmake_minimum_required(VERSION 3.10)
project(gitlite LANGUAGES CXX)

add_executable(gitlite
    main.cpp
    src/Utils.cpp
    src/Commit.cpp
    src/RepositoryCore.cpp
    src/Subtask1.cpp
    src/Subtask2.cpp
    src/Subtask3.cpp
    src/Subtask4.cpp
    src/Subtask5.cpp
    src/SomeObj.cpp
)

target_include_directories(gitlite PRIVATE include)
target_compile_features(gitlite PRIVATE cxx_std_17)
target_compile_options(gitlite PRIVATE -Wall -Wextra -Wpedantic)

# ==================== ACMOJ 配置（请勿删除）====================
add_custom_command(
    TARGET gitlite POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:gitlite>"
            "${CMAKE_SOURCE_DIR}/code"
    COMMENT "Copying gitlite executable to ACMOJ artifact: code"
    VERBATIM
)
# ================== ACMOJ 配置结束（请勿删除）==================
