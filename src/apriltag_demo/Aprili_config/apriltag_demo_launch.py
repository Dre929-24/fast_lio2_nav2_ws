import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, TimerAction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    # ============ 可配置参数 ============
    mode = LaunchConfiguration('mode')
    file_path = LaunchConfiguration('file_path')
    camera_id = LaunchConfiguration('camera_id')
    publish_rate = LaunchConfiguration('publish_rate')
    loop = LaunchConfiguration('loop')
    display_width = LaunchConfiguration('display_width')

    # ============ 声明参数 ============
    declare_mode = DeclareLaunchArgument(
        'mode', default_value='image',
        description='媒体模式：image / video / camera'
    )

    declare_file_path = DeclareLaunchArgument(
        'file_path',
        default_value=os.path.expanduser(
            '~/fast_lio2_nav2_ws/src/test/assert/images/0.png'),
        description='图片/视频文件路径'
    )

    declare_camera_id = DeclareLaunchArgument(
        'camera_id', default_value='0',
        description='摄像头 ID'
    )

    declare_publish_rate = DeclareLaunchArgument(
        'publish_rate', default_value='10.0',
        description='发布频率（Hz）'
    )

    declare_loop = DeclareLaunchArgument(
        'loop', default_value='true',
        description='视频是否循环播放'
    )

    declare_display_width = DeclareLaunchArgument(
        'display_width', default_value='480',
        description='显示窗口宽度'
    )

    # ============ 1. 媒体发布 ============
    media_publisher = Node(
        package='media_publisher',
        executable='media_publisher',
        name='media_publisher',
        output='screen',
        parameters=[{
            'mode': mode,
            'file_path': file_path,
            'camera_id': camera_id,
            'publish_rate': publish_rate,
            'loop': loop,
        }],
    )

    # ============ 2. camera_info ============
    camera_info_pub = Node(
        package='camera_info_pub',
        executable='camera_info_pub',
        name='camera_info_pub',
        output='screen',
    )

    # ============ 3. apriltag_ros ============
    apriltag_params = os.path.join(
        os.path.expanduser('~'),
        'fast_lio2_nav2_ws/src/test/Aprili_config/apriltag_params.yaml'
    )

    apriltag_node = Node(
        package='apriltag_ros',
        executable='apriltag_node',
        name='apriltag',
        output='screen',
        parameters=[apriltag_params],
        remappings=[
            ('image_rect', '/front_camera/image'),
            ('camera_info', '/front_camera/camera_info'),
        ],
        # image_transport 通过参数传
        arguments=['--ros-args', '-p', 'image_transport:=compressed'],
    )

    # ============ 4. camera_viewer ============
    camera_viewer = Node(
        package='camera_viewer',
        executable='camera_viewer',
        name='camera_viewer',
        output='screen',
        parameters=[{
            'display_width': display_width,
        }],
    )

    # ============ 组合 ============
    return LaunchDescription([
        declare_mode,
        declare_file_path,
        declare_camera_id,
        declare_publish_rate,
        declare_loop,
        declare_display_width,

        # 立即启动媒体发布和 camera_info
        media_publisher,
        camera_info_pub,

        # 延迟 2 秒启动 apriltag_ros（等图像话题就绪）
        TimerAction(period=2.0, actions=[apriltag_node]),

        # 延迟 4 秒启动 camera_viewer（等检测结果就绪）
        TimerAction(period=4.0, actions=[camera_viewer]),
    ])
