# RobotAC 四足机器狗自主导航系统

基于 FAST-LIO + Nav2 + zsibot SDK 的四足机器狗自主导航方案。

---

## ⚠️ 首次使用必读：需要修改的路径

克隆项目后，以下位置**必须按你的实际环境修改**：

### 1. `scripts/sim_start.sh`

```bash
# TODO：修改成仿真包对应的路径
cd ~/Desktop/matrix_robotac_first
./run_sim.sh
```

**改成你的 RobotAC 仿真目录。**

### 2. `src/navigation/me_nav2_bringup/launch/rt_nav_launch.py`

```python
# TODO: 改成你的地图文件路径
default_pcd = os.path.join(me_share, 'my_maps', 'test.pcd')
default_map_yaml = os.path.join(me_share, 'my_maps', 'map_20260924_125144.yaml')
```

**改成你实际生成的 PCD 和 YAML 文件名。**

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
    libeigen3-dev \
    libpcl-dev \
    cmake \
    build-essential
```

### 3. 外部 SDK

**zsibot_sdk**（机器狗控制 SDK）：

```bash
# 已放在 deps/zsibot_sdk/
# 需要单独编译 highlevel_demo：
cd ~/fast_lio2_nav2_ws/deps/zsibot_sdk/demo/zsl-1/cpp
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

**livox_ros_driver2**（激光雷达驱动，编译依赖）：

```bash
# 已放在 driver/livox_ros_driver2/
# colcon 编译时会自动处理
```

### 4. 脚本执行权限

```bash
chmod +x ~/Desktop/fast_lio2_nav2_ws/scripts/*.sh
```

### 5. 编译工作空间

```bash
cd ~/Desktop/fast_lio2_nav2_ws
./scripts/build.sh
```

---

## 🚀 快速开始

### 启动（4 个终端）

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
    ├── navigation/me_nav2_bringup/           # 导航层
    │   ├── launch/
    │   ├── config/nav2_params.yaml
    │   ├── rviz/
    │   └── my_maps/
    └── tools/                     #工具层
        ├── pcd2pgm/
        └── robot_marker/
```

---

## 🏗️ 架构

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

| 模块 | 职责 |
|------|------|
| `mujoco_tf_bridge` | MuJoCo 里程计 → `odom → base_link` TF |
| `pointcloud_to_laserscan` | 3D 点云 → 2D 激光 `/scan` |
| `planner_server` | 全局路径规划 |
| `controller_server` | 局部速度控制 → `/cmd_vel` |
| `zsibot_cmd_bridge` | `/cmd_vel` → SDK 指令 |

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
chmod +x ~/Desktop/fast_lio2_nav2_ws/scripts/*.sh
```

---

## 🔄 完整流程

### 建图

```bash
# 开启路由
./scripts/rmw_zenohd.sh     

# 开启仿真
./scripts/sim_start.sh       

# 开启建图
./scripts/fast_lio.sh        

# 手动控制，键盘 WASD 控制（j,k 控制速度）
./scripts/manual_control.sh  

# 保存 PCD
./scripts/save_map.sh

# 转 2D 地图
./scripts/pcd2pgm.sh         
```

### 导航

```bash
./scripts/rmw_zenohd.sh      # 终端 1
./scripts/sim_start.sh       # 终端 2
./scripts/nav_start.sh       # 终端 3
# RViz 里点 2D Goal Pose
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

---

## 📄 License

Apache-2.0