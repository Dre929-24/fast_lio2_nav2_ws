#!/bin/bash
# 启动 FAST-LIO 建图

# ROS 2 环境
source /opt/ros/humble/setup.bash
export RMW_IMPLEMENTATION=rmw_zenoh_cpp
export ROS_DOMAIN_ID=89
export SDK_CLIENT_IP=127.0.0.1

echo "FAST-LIO mapping starting..."
echo "  RMW_IMPLEMENTATION = $RMW_IMPLEMENTATION"
echo "  ROS_DOMAIN_ID      = $ROS_DOMAIN_ID"
echo "  SDK_CLIENT_IP      = $SDK_CLIENT_IP"

# 脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 工作空间根目录
WS_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

source "$WS_ROOT/install/setup.bash"

# 启动 FAST-LIO
ros2 launch fast_lio mapping.launch.py config_file:=mid360.yaml