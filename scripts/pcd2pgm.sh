#!/bin/bash

source /opt/ros/humble/setup.bash
export RMW_IMPLEMENTATION=rmw_zenoh_cpp
export ROS_DOMAIN_ID=89
export SDK_CLIENT_IP=127.0.0.1

echo "Navigation starting..."
echo "  RMW_IMPLEMENTATION = $RMW_IMPLEMENTATION"
echo "  ROS_DOMAIN_ID      = $ROS_DOMAIN_ID"
echo "  SDK_CLIENT_IP      = $SDK_CLIENT_IP"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WS_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
source "$WS_ROOT/install/setup.bash"

PCD_FILE="$WS_ROOT/src/navigation/me_nav2_bringup/my_maps/map.pcd"
OUTPUT_DIR="$WS_ROOT/src/navigation/me_nav2_bringup/my_maps"

# 输出文件名：map_年月日_时分秒
OUTPUT_NAME="map_$(date +%Y%m%d_%H%M%S)"

# 启动 pcd2pgm 发布 /map
ros2 run pcd2pgm pcd2pgm_node --ros-args \
    -p pcd_file:="$PCD_FILE" \
    -p map_topic_name:=map &
PCD_PID=$!

sleep 3

# 保存成 PGM + YAML（带时间戳，不覆盖旧地图）
cd "$OUTPUT_DIR"
ros2 run nav2_map_server map_saver_cli -f "$OUTPUT_NAME" --ros-args -r map:=/map

kill $PCD_PID 2>/dev/null

echo "Saved: $OUTPUT_DIR/$OUTPUT_NAME.pgm"
echo "Saved: $OUTPUT_DIR/$OUTPUT_NAME.yaml"