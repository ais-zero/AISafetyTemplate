cmake_minimum_required(VERSION 3.20)
project(AIController VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find Python
find_package(Python3 REQUIRED COMPONENTS Development)

# Controller library
add_library(controller SHARED
    src/controller.cpp
    src/security.cpp
    src/helm_wrapper.cpp
    src/dataset_manager.cpp
)

target_include_directories(controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${Python3_INCLUDE_DIRS}
)

target_link_libraries(controller
    PRIVATE
        ${Python3_LIBRARIES}
)

# Set output name
set_target_properties(controller PROPERTIES
    OUTPUT_NAME "controller"
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
)

# Install rules
install(TARGETS controller
    LIBRARY DESTINATION lib
)

install(FILES python/controller.py
    DESTINATION lib/python
)

# Print configuration
message(STATUS "Python3 include dirs: ${Python3_INCLUDE_DIRS}")
message(STATUS "Python3 libraries: ${Python3_LIBRARIES}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
