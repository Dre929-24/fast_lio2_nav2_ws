cmake_minimum_required(VERSION 3.8)
project(robot_marker)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(robot_marker src/robot_marker.cpp)
ament_target_dependencies(robot_marker
  rclcpp
  visualization_msgs
  tf2_ros
  geometry_msgs
)

install(TARGETS robot_marker
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()