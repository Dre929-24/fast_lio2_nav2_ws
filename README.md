# RobotAC 四足机器狗自主导航系统

基于 FAST-LIO + Nav2 + zsibot SDK 的四足机器狗自主导航方案，集成 AprilTag 检测与视觉对准。

---

## ⚠️ 首次使用必读：需要修改的路径

克隆项目后，以下位置**必须按你的实际环境修改**：

### 1. `scripts/sim_start.sh`

```bash
# TODO：修改成仿真包对应的路径
cd ~/Desktop/matrix_robotac_first
./run_sim.sh
```

### 2. `src/navigation/me_nav2_bringup/launch/my_nav2_launch.py`

```python
# TODO: 改成你的地图文件路径
map_yaml_file = os.path.join(me_share_path, 'my_maps', 'map.yaml')
```

### 3. `src/bridge/zsibot_cmd_bridge/launch/zsibot_cmd_bridge_launch.py`

```python
# TODO: 仿真用 127.0.0.1，真机用实际 IP
declare_dog_ip = DeclareLaunchArgument(
    'dog_ip', default_value='127.0.0.1',
    ...
)
```

---

## 📦 安装依赖

### 1. ROS 2 Humble

```bash
# 参考官方文档安装：
# https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debians.html
```

### 2. 系统依赖

```bash
sudo apt update
sudo apt install -y \
    ros-humble-nav2-bringup \
    ros-humble-nav2-map-server \
    ros-humble-nav2-lifecycle-manager \
    ros-humble-pointcloud-to-laserscan \
    ros-humble-tf2-tools \
    ros-humble-tf2-ros \
    ros-humble-pcl-ros \
    ros-humble-pcl-conversions \
    ros-humble-rmw-zenoh-cpp \
    ros-humble-apriltag-ros \
    ros-humble-cv-bridge \
    libeigen3-dev \
    libpcl-dev \
    libopencv-dev \
    cmake \
    build-essential
```

### 3. 外部 SDK

**zsibot_sdk**：

```bash
cd ~/fast_lio2_nav2_ws/deps/zsibot_sdk/demo/zsl-1/cpp
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### 4. 脚本执行权限

```bash
chmod +x ~/fast_lio2_nav2_ws/scripts/*.sh
```

### 5. 编译工作空间

```bash
cd ~/fast_lio2_nav2_ws
./scripts/build.sh
```

---

## 🚀 快速开始

### 导航启动（4 个终端）

```bash
# 终端 1：Zenoh 路由
./scripts/rmw_zenohd.sh

# 终端 2：RobotAC 仿真
./scripts/sim_start.sh

# 终端 3：FAST-LIO（建图阶段，可选）
./scripts/fast_lio.sh

# 终端 4：导航
./scripts/nav_start.sh
```

### 发送目标点

RViz 里点击 **`2D Goal Pose`**，在地图上点一个位置，拖动指定朝向，松开即可。

---

## 🎯 AprilTag 检测与对准

### 功能

- **三种输入模式**：图片、视频、摄像头
- **Tag 检测**：识别 `36h11` 家族的 Tag
- **可视化**：
  - 每个 Tag 框内显示 `ID` 和 `Confidence`
  - 图像左上角汇总所有 Tag 的 `ID`、`Center`、`Distance`、`Confidence`
- **距离估算**：基于 Tag 的像素边长和相机内参

### 一键启动

**图片模式**：

```bash
cd ~/fast_lio2_nav2_ws
source install/setup.bash

ros2 launch src/apriltag_demo/Aprili_config/apriltag_demo_launch.py \
  mode:=image \
  file_path:=$HOME/fast_lio2_nav2_ws/src/apriltag_demo/assert/images/0.png
```

**视频模式**：

```bash
ros2 launch src/apriltag_demo/Aprili_config/apriltag_demo_launch.py \
  mode:=video \
  file_path:=$HOME/fast_lio2_nav2_ws/src/apriltag_demo/assert/video/test.mp4
```

**摄像头模式**：

```bash
ros2 launch src/apriltag_demo/Aprili_config/apriltag_demo_launch.py \
  mode:=camera \
  camera_id:=0 \
  publish_rate:=30.0
```

---

## 📁 项目结构

```
fast_lio2_nav2_ws/
├── deps/zsibot_sdk/                # 机器狗控制 SDK
├── scripts/                        # 启动脚本
│   ├── build.sh
│   ├── sim_start.sh
│   ├── rmw_zenohd.sh
│   ├── fast_lio.sh
│   ├── save_map.sh
│   ├── pcd2pgm.sh
│   ├── nav_start.sh
│   ├── manual_control.sh
│   └── tf.sh
└── src/
    ├── bridge/                     # 桥接层
    │   ├── mujoco_tf_bridge/
    │   ├── ue_zenoh_bridge/
    │   └── zsibot_cmd_bridge/
    ├── driver/                     # 激光雷达驱动
    │   └── livox_ros_driver2/
    ├── localization/               # 定位层
    │   ├── fast_lio/
    │   ├── lio_interface/
    │   └── small_gicp_relocalization/
    ├── navigation/me_nav2_bringup/ # 导航层
    │   ├── launch/
    │   ├── config/nav2_params.yaml
    │   ├── rviz/
    │   └── my_maps/
    ├── apriltag_demo/              # AprilTag 检测与可视化
    │   ├── Aprili_config/
    │   │   ├── apriltag_params.yaml
    │   │   └── apriltag_demo_launch.py
    │   ├── assert/
    │   │   ├── images/             # 测试图片
    │   │   └── video/              # 测试视频
    │   ├── media_publisher/        # 图片/视频/摄像头发布
    │   ├── camera_info_pub/        # 相机内参发布
    │   └── camera_viewer/          # 可视化
    └── tools/
        ├── pcd2pgm/
        └── robot_marker/
```

---

## 🏗️ 架构

### 导航链路

```
RobotAC 仿真
  /front_lidar（3D点云）、/odom/mujoco_odom（真值里程计）
        ↓
TF 链：map → odom → base_link → lidar
        ↓
/scan（3D点云切片）+ map.pgm/map.yaml（2D栅格地图）
        ↓
Nav2（planner_server → controller_server → /cmd_vel）
        ↓
zsibot_cmd_bridge → zsibot_sdk → 机器狗
```

### AprilTag 检测链路

```
图片/视频/摄像头
        ↓
media_publisher（发布 CompressedImage）
        ↓
camera_info_pub（发布 CameraInfo，时间戳同步）
        ↓
apriltag_ros（检测 Tag）
        ↓
/detections（Tag ID、中心、角点、decision_margin）
        ↓
camera_viewer（可视化：框、ID、距离、置信度）
```

| 模块 | 职责 |
|------|------|
| `mujoco_tf_bridge` | MuJoCo 里程计 → `odom → base_link` TF |
| `pointcloud_to_laserscan` | 3D 点云 → 2D 激光 `/scan` |
| `planner_server` | 全局路径规划 |
| `controller_server` | 局部速度控制 → `/cmd_vel` |
| `zsibot_cmd_bridge` | `/cmd_vel` → SDK 指令 |
| `media_publisher` | 发布图片/视频/摄像头图像 |
| `camera_info_pub` | 发布相机内参 |
| `camera_viewer` | AprilTag 可视化 |

---

## 📜 脚本说明

| 脚本 | 作用 |
|------|------|
| `build.sh` | 编译工作空间 |
| `rmw_zenohd.sh` | 启动 Zenoh 路由（**必须先启动**） |
| `sim_start.sh` | 启动 RobotAC 仿真 |
| `fast_lio.sh` | 启动 FAST-LIO 建图 |
| `save_map.sh` | 保存 PCD 地图 |
| `pcd2pgm.sh` | PCD → 2D 栅格地图 |
| `nav_start.sh` | 启动导航系统 |
| `manual_control.sh` | 手动控制机器狗 |
| `tf.sh` | 查看 TF 树 |

首次克隆后加权限：

```bash
chmod +x ~/fast_lio2_nav2_ws/scripts/*.sh
```

---

## 🔄 完整流程

### 建图

```bash
./scripts/rmw_zenohd.sh     # 终端 1
./scripts/sim_start.sh      # 终端 2
./scripts/fast_lio.sh       # 终端 3
./scripts/manual_control.sh # 终端 4，键盘 WASD 控制
./scripts/save_map.sh       # 保存 PCD
./scripts/pcd2pgm.sh        # 转 2D 地图
```

### 导航

```bash
./scripts/rmw_zenohd.sh      # 终端 1
./scripts/sim_start.sh       # 终端 2
./scripts/nav_start.sh       # 终端 3
# RViz 里点 2D Goal Pose
```

### AprilTag 检测

```bash
ros2 launch src/apriltag_demo/Aprili_config/apriltag_demo_launch.py \
  mode:=image \
  file_path:=$HOME/fast_lio2_nav2_ws/src/apriltag_demo/assert/images/0.png
```

---

## ⚙️ 关键参数

### `nav2_params.yaml`

```yaml
FollowPath:
  max_vel_x: 1.0
  max_vel_theta: 1.5
  max_speed_xy: 1.0

footprint: "[ [0.25, 0.18], [0.25, -0.18], [-0.25, -0.18], [-0.25, 0.18] ]"

inflation_layer:
  cost_scaling_factor: 5.0
  inflation_radius: 0.3

velocity_smoother:
  max_velocity: [1.0, 0.0, 1.5]
  min_velocity: [-1.0, 0.0, -1.5]
```

### `mid360.yaml`（FAST-LIO）

```yaml
common:
  lid_topic: "/front_lidar"
  imu_topic: "/front_lidar/imu"

preprocess:
  lidar_type: 4
```

### 速度限制链

```
controller_server: 1.0 m/s
        ↓
velocity_smoother: 1.0 m/s
        ↓
zsibot_cmd_bridge: 1.0 m/s
        ↓
zsibot_sdk: 3.0 m/s（硬件极限）
```

---

## ❓ 常见问题

### `PackageNotFoundError`

脚本里缺少 `source install/setup.bash`。

### `bind: Address already in use`

```bash
pkill -f zsibot_cmd_bridge
```

### 机器狗不动

```bash
ros2 topic echo /cmd_vel   # 检查 /cmd_vel
./scripts/tf.sh            # 检查 TF 链
```

### `/scan` 没数据

```bash
ros2 topic hz /front_lidar
ros2 topic hz /scan
```

### 机器人撞墙

- 降低 `max_vel_x` 到 `0.5`
- 精确设置 `footprint`
- 调整 `inflation_radius` 和 `cost_scaling_factor`

### 膨胀区域太大

```yaml
inflation_layer:
  cost_scaling_factor: 5.0
  inflation_radius: 0.3
```

### RViz 看不到机器人位置

Add → MarkerArray → Topic `/robot_marker`。

---

## 📝 备注

- 定位用 **MuJoCo 真值里程计**，无漂移
- FAST-LIO 仅用于**建图阶段**
- `small_gicp_relocalization` 和 `lio_interface` 为**真机部署预留**
- **AprilTag 检测**支持图片、视频、摄像头三种输入

---

## 📄 License

Apache-2.0