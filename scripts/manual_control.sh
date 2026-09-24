#!/bin/bash
# 手动控制机器狗脚本

# ROS 2 环境
source /opt/ros/humble/setup.bash
export RMW_IMPLEMENTATION=rmw_zenoh_cpp
export ROS_DOMAIN_ID=89
export SDK_CLIENT_IP=127.0.0.1

echo "Manual control starting..."
echo "  RMW_IMPLEMENTATION = $RMW_IMPLEMENTATION"
echo "  ROS_DOMAIN_ID      = $ROS_DOMAIN_ID"
echo "  SDK_CLIENT_IP      = $SDK_CLIENT_IP"

# 脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 工作空间根目录（scripts/ 的上一级）
WS_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# highlevel_demo 所在目录
DEMO_DIR="$WS_ROOT/deps/zsibot_sdk/demo/zsl-1/cpp/build"

echo "  DEMO_DIR           = $DEMO_DIR"

if [ ! -f "$DEMO_DIR/highlevel_demo" ]; then
    echo "Error: highlevel_demo not found in $DEMO_DIR"
    echo ""
    echo "Please build it first:"
    echo "  cd $WS_ROOT/deps/zsibot_sdk/demo/zsl-1/cpp"
    echo "  mkdir -p build && cd build"
    echo "  cmake .. && make -j\$(nproc)"
    exit 1
fi

cd "$DEMO_DIR"
./highlevel_demo