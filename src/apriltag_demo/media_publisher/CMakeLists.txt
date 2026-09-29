cmake_minimum_required(VERSION 3.8)
project(media_publisher)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(media_publisher src/media_publisher.cpp)
ament_target_dependencies(media_publisher
  rclcpp
  sensor_msgs
)
target_link_libraries(media_publisher ${OpenCV_LIBS})

install(TARGETS
  media_publisher
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()