cmake_minimum_required(VERSION 3.8)
project(camera_info_pub)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(camera_info_pub src/camera_info_pub.cpp)
ament_target_dependencies(camera_info_pub
  rclcpp
  sensor_msgs
  OpenCV
)
target_link_libraries(camera_info_pub ${OpenCV_LIBS})

install(TARGETS
  camera_info_pub
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()