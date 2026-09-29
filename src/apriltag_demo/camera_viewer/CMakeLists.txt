cmake_minimum_required(VERSION 3.8)
project(camera_viewer)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(apriltag_msgs REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(camera_viewer src/camera_viewer.cpp)
ament_target_dependencies(camera_viewer
  rclcpp
  sensor_msgs
  apriltag_msgs
)
target_link_libraries(camera_viewer ${OpenCV_LIBS})

install(TARGETS
  camera_viewer
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()