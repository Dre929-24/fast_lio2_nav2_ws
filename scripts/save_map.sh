#!/bin/bash

source /opt/ros/humble/setup.bash
export RMW_IMPLEMENTATION=rmw_zenoh_cpp
export ROS_DOMAIN_ID=89
export SDK_CLIENT_IP=127.0.0.1

echo "Saving map..."
echo "  RMW_IMPLEMENTATION = $RMW_IMPLEMENTATION"
echo "  ROS_DOMAIN_ID      = $ROS_DOMAIN_ID"
echo "  SDK_CLIENT_IP      = $SDK_CLIENT_IP"

# 脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 工作空间根目录
WS_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# 调用 /map_save 服务
ros2 service call /map_save std_srvs/srv/Trigger