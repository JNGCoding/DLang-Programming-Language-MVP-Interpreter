cmake_minimum_required(VERSION 3.10)
project(CPP_Language_Implementation LANGUAGES CXX)

# Set a sensible default standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# Optional: set warnings on supported compilers
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Source files (explicitly listed, no globbing)
set(PROJECT_SOURCES
    main.cpp

    lang_files/interpreter.cpp
    lang_files/interpreter_instructions_implementation.cpp

    lang_files/space_allocator.cpp

    lang_files/io/code_buffer.cpp

    lang_files/tokenizer/word_factory.cpp

    lang_files/objects/variable.cpp

    lang_files/utility/type_converter.cpp
    lang_files/utility/logging.cpp
)

# If you have header-only modules, we still add include dirs
include_directories(
    "${CMAKE_SOURCE_DIR}/lang_files"
    "${CMAKE_SOURCE_DIR}/lang_files/ds"
    "${CMAKE_SOURCE_DIR}/lang_files/io"
    "${CMAKE_SOURCE_DIR}/lang_files/io/objects"
    "${CMAKE_SOURCE_DIR}/lang_files/tokenizer"
    "${CMAKE_SOURCE_DIR}/lang_files/utility"
)

# Create executable
add_executable(alpl ${PROJECT_SOURCES})

# Install target (optional)
install(TARGETS alpl RUNTIME DESTINATION bin)
