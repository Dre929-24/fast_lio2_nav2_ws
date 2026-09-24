#!/bin/bash

source /opt/ros/humble/setup.bash
export RMW_IMPLEMENTATION=rmw_zenoh_cpp
export ROS_DOMAIN_ID=89
export SDK_CLIENT_IP=127.0.0.1

echo "Navigation starting..."
echo "  RMW_IMPLEMENTATION = $RMW_IMPLEMENTATION"
echo "  ROS_DOMAIN_ID      = $ROS_DOMAIN_ID"
echo "  SDK_CLIENT_IP      = $SDK_CLIENT_IP"

# 脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 工作空间根目录
WS_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# source 工作空间
if [ -f "$WS_ROOT/install/setup.bash" ]; then
    source "$WS_ROOT/install/setup.bash"
else
    echo "Error: $WS_ROOT/install/setup.bash not found"
    echo "Please build first:"
    echo "  cd $WS_ROOT && ./scripts/build.sh"
    exit 1
fi

# 启动导航
ros2 launch me_nav2_bringup rt_nav_launch.py