#!/bin/bash
# 启动 RobotAC 仿真

source /opt/ros/humble/setup.bash
export RMW_IMPLEMENTATION=rmw_zenoh_cpp
export ROS_DOMAIN_ID=89
export SDK_CLIENT_IP=127.0.0.1

echo "RobotAC simulation starting..."
echo "  RMW_IMPLEMENTATION = $RMW_IMPLEMENTATION"
echo "  ROS_DOMAIN_ID      = $ROS_DOMAIN_ID"
echo "  SDK_CLIENT_IP      = $SDK_CLIENT_IP"

# TODO：修改成仿真包对应的路径
cd ~/Desktop/matrix_robotac_first
./run_sim.sh