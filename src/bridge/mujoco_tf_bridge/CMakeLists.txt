cmake_minimum_required(VERSION 3.8)
project(mujoco_tf_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(mujoco_tf_bridge src/mujoco_tf_bridge.cpp)
ament_target_dependencies(mujoco_tf_bridge
  rclcpp
  nav_msgs
  tf2_ros
  geometry_msgs
)

install(TARGETS
  mujoco_tf_bridge
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()