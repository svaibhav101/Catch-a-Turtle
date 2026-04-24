cmake_minimum_required(VERSION 3.8)
project(turtle_chaser)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(turtlesim REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(turtle_interfaces REQUIRED)

include_directories(include)

add_executable(turtle_spawner_node src/turtle_spawn.cpp)
ament_target_dependencies(turtle_spawner_node
  rclcpp turtlesim geometry_msgs turtle_interfaces
)

add_executable(turtle_control_node src/turtle_control.cpp)
ament_target_dependencies(turtle_control_node
  rclcpp turtlesim geometry_msgs turtle_interfaces)

install(TARGETS
  turtle_spawner_node
  turtle_control_node
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include/
)

install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
