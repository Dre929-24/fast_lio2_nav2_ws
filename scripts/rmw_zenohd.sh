#!/bin/bash
# 启动 rmw_zenohd 路由守护进程

source /opt/ros/humble/setup.bash
export RMW_IMPLEMENTATION=rmw_zenoh_cpp
export ROS_DOMAIN_ID=89
export SDK_CLIENT_IP=127.0.0.1

echo "Starting rmw_zenohd..."
echo "  RMW_IMPLEMENTATION = $RMW_IMPLEMENTATION"
echo "  ROS_DOMAIN_ID      = $ROS_DOMAIN_ID"
echo "  SDK_CLIENT_IP      = $SDK_CLIENT_IP"

ros2 run rmw_zenoh_cpp rmw_zenohd