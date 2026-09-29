# AprilTag 检测与可视化

基于 `apriltag_ros` 的 Tag 检测系统，支持图片、视频、摄像头三种输入模式，实时检测并可视化 Tag 的 ID、中心坐标、距离和确信度。

---

## 📋 目录

- [功能](#功能)
- [依赖](#依赖)
- [编译](#编译)
- [一键启动](#一键启动)
- [手动启动](#手动启动)
- [参数说明](#参数说明)
- [常见问题](#常见问题)

---

## 功能

- **三种输入模式**：图片、视频、摄像头
- **Tag 检测**：识别 36h11 家族的 Tag
- **可视化**：
  - 每个 Tag 框内显示 `ID` 和 `Confidence`
  - 图像左上角汇总所有 Tag 的 `ID`、`Center`、`Distance`、`Confidence`
- **距离估算**：基于 Tag 的像素边长和相机内参

---

## 依赖

### 系统依赖

```bash
sudo apt install -y \
    ros-humble-apriltag-ros \
    ros-humble-cv-bridge \
    libopencv-dev
```

### 工作空间内的包

| 包 | 说明 |
|----|------|
| `media_publisher` | 发布图片/视频/摄像头图像 |
| `camera_info_pub` | 发布相机内参 |
| `camera_viewer` | 可视化 Tag 检测结果 |

---

## 编译

```bash
cd ~/fast_lio2_nav2_ws
./scripts/build.sh
source install/setup.bash
```

---

## 一键启动

**一条命令启动所有节点**（`media_publisher` + `camera_info_pub` + `apriltag_ros` + `camera_viewer`）。

### 图片模式

```bash
cd ~/fast_lio2_nav2_ws
source install/setup.bash

ros2 launch src/apriltag_demo/Aprili_config/apriltag_demo_launch.py \
  mode:=image \
  file_path:=$HOME/fast_lio2_nav2_ws/src/apriltag_demo/assert/images/0.png
```

### 视频模式

```bash
ros2 launch src/apriltag_demo/Aprili_config/apriltag_demo_launch.py \
  mode:=video \
  file_path:=$HOME/fast_lio2_nav2_ws/src/apriltag_demo/assert/video/test.mp4
```

### 摄像头模式

```bash
ros2 launch src/apriltag_demo/Aprili_config/apriltag_demo_launch.py \
  mode:=camera \
  camera_id:=0 \
  publish_rate:=30.0
```

### 自定义显示大小

```bash
# 小窗口
ros2 launch src/apriltag_demo/Aprili_config/apriltag_demo_launch.py \
  mode:=video \
  display_width:=480

# 大窗口
ros2 launch src/apriltag_demo/Aprili_config/apriltag_demo_launch.py \
  mode:=video \
  display_width:=960
```

### launch 参数

| 参数 | 默认 | 说明 |
|------|------|------|
| `mode` | `image` | `image` / `video` / `camera` |
| `file_path` | `.../images/0.png` | 图片/视频文件路径 |
| `camera_id` | `0` | 摄像头 ID |
| `publish_rate` | `10.0` | 发布频率（Hz） |
| `loop` | `true` | 视频是否循环 |
| `display_width` | `640` | 显示窗口宽度（高度按比例） |

---

## 手动启动

如果需要单独调试，按以下顺序启动 4 个终端。

### 终端 1：发布图片/视频/摄像头

```bash
cd ~/fast_lio2_nav2_ws
source install/setup.bash

# 图片模式
ros2 run media_publisher media_publisher --ros-args \
  -p mode:=image \
  -p file_path:=$HOME/fast_lio2_nav2_ws/src/apriltag_demo/assert/images/0.png \
  -p publish_rate:=10.0

# 视频模式
ros2 run media_publisher media_publisher --ros-args \
  -p mode:=video \
  -p file_path:=$HOME/fast_lio2_nav2_ws/src/apriltag_demo/assert/video/test.mp4 \
  -p publish_rate:=10.0 \
  -p loop:=true

# 摄像头模式
ros2 run media_publisher media_publisher --ros-args \
  -p mode:=camera \
  -p camera_id:=0 \
  -p publish_rate:=30.0
```

### 终端 2：发布 camera_info

```bash
cd ~/fast_lio2_nav2_ws
source install/setup.bash

ros2 run camera_info_pub camera_info_pub
```

### 终端 3：启动 apriltag_ros

```bash
cd ~/fast_lio2_nav2_ws
source install/setup.bash

ros2 run apriltag_ros apriltag_node --ros-args \
  --params-file ~/fast_lio2_nav2_ws/src/apriltag_demo/Aprili_config/apriltag_params.yaml \
  -r image_rect:=/front_camera/image \
  -r camera_info:=/front_camera/camera_info \
  -p image_transport:=compressed
```

### 终端 4：可视化

```bash
cd ~/fast_lio2_nav2_ws
source install/setup.bash

ros2 run camera_viewer camera_viewer
```

### 验证检测

```bash
ros2 topic echo /detections
```

---

## 参数说明

### `media_publisher`

| 参数 | 默认 | 说明 |
|------|------|------|
| `mode` | `image` | `image` / `video` / `camera` |
| `file_path` | `""` | 图片/视频文件路径 |
| `camera_id` | `0` | 摄像头 ID |
| `publish_rate` | `10.0` | 发布频率（Hz） |
| `loop` | `true` | 视频是否循环 |

### `camera_viewer`

| 参数 | 默认 | 说明 |
|------|------|------|
| `display_width` | `640` | 显示窗口宽度（高度按比例） |
| `tag_size` | `0.053` | Tag 实际边长（米） |
| `fovy_deg` | `58.0` | 相机垂直视场角（度） |

### launch 参数

| 参数 | 默认 | 说明 |
|------|------|------|
| `mode` | `image` | 媒体模式 |
| `file_path` | `.../images/0.png` | 文件路径 |
| `camera_id` | `0` | 摄像头 ID |
| `publish_rate` | `10.0` | 发布频率 |
| `loop` | `true` | 视频循环 |
| `display_width` | `640` | 显示窗口宽度 |

---

## 常见问题

### 1. `camera_info` 没有数据

**原因**：`camera_info_pub` 没启动。

**解决**：先启动终端 2。

### 2. `Synchronized pairs: 0`

**原因**：图像和 `camera_info` 的时间戳不同步。

**解决**：`camera_info_pub` 会订阅图像，用图像的时间戳发布，确保同步。

### 3. 检测不到 Tag

**可能原因**：

- Tag 家族不对（`apriltag_params.yaml` 里的 `family`）
- 图像质量差
- Tag 太小

**解决**：检查 `apriltag_params.yaml`，确认 `family: 36h11`。

### 4. 距离估算不准

**原因**：`fovy_deg` 或 `tag_size` 不准确。

**解决**：

- `tag_size`：用尺子量 Tag 黑色边框的边长
- `fovy_deg`：从相机规格或仿真 XML 查

### 5. 摄像头打不开

**原因**：`camera_id` 不对。

**解决**：

```bash
# 列出所有摄像头
v4l2-ctl --list-devices

# 列出视频流节点
for dev in /dev/video*; do
    echo "=== $dev ==="
    v4l2-ctl -d $dev --all 2>/dev/null | grep -A 5 "Device Caps"
done
```

**找到 `Video Capture` 的节点，用对应的 `camera_id`。**

### 6. 显示窗口太大

**解决**：用 `display_width` 参数：

```bash
ros2 launch src/apriltag_demo/Aprili_config/apriltag_demo_launch.py \
  mode:=video \
  display_width:=480
```

---

## 文件结构

```
src/apriltag_demo/
├── Aprili_config/
│   ├── apriltag_params.yaml          # apriltag_ros 配置
│   └── apriltag_demo_launch.py       # 一键启动 launch
├── assert/
│   ├── images/                       # 测试图片
│   │   ├── 0.png
│   │   ├── 1.png
│   │   ├── 2.png
│   │   └── 3.png
│   └── video/
│       └── test.mp4                  # 测试视频
├── media_publisher/                  # 图片/视频/摄像头发布
├── camera_info_pub/                  # 相机内参发布
└── camera_viewer/                    # 可视化
```

---

## 许可证

Apache-2.0