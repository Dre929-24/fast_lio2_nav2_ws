cmake_minimum_required(VERSION 3.16)
project(zsibot_cmd_bridge)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)

# zsibot SDK 路径
get_filename_component(ZSIBOT_SDK_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/zsibot_sdk"
    ABSOLUTE
)

# 自动识别架构
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
    set(ARCH "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(ARCH "aarch64")
else()
    message(FATAL_ERROR "不支持的架构: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(LIB_PATH "${ZSIBOT_SDK_DIR}/lib/zsl-1/${ARCH}")

find_library(MC_SDK_LIB
    NAMES "mc_sdk_zsl_1_${ARCH}"
    PATHS ${LIB_PATH}
    NO_DEFAULT_PATH
    REQUIRED
)

message(STATUS "Found zsibot SDK: ${MC_SDK_LIB}")

# --------------------------
# 编译桥接节点
# --------------------------
add_executable(zsibot_cmd_bridge src/zsibot_cmd_bridge.cpp)

target_link_libraries(zsibot_cmd_bridge ${MC_SDK_LIB})

target_include_directories(zsibot_cmd_bridge PRIVATE
    ${ZSIBOT_SDK_DIR}/include
    ${ZSIBOT_SDK_DIR}/include/zsl-1
)

ament_target_dependencies(zsibot_cmd_bridge
    rclcpp
    geometry_msgs
)

install(TARGETS
    zsibot_cmd_bridge
    DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()