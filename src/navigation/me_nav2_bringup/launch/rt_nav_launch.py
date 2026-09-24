import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    IncludeLaunchDescription,
    TimerAction,
)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    me_share = get_package_share_directory('me_nav2_bringup')

    # TODO: 改成你的地图文件路径
    default_pcd = os.path.join(me_share, 'my_maps', 'test.pcd')
    default_map_yaml = os.path.join(me_share, 'my_maps', 'map_20260924_125144.yaml')

    pcd_file = LaunchConfiguration('pcd_file')
    map_yaml = LaunchConfiguration('map_yaml')

    declare_pcd_file = DeclareLaunchArgument(
        'pcd_file',
        default_value=default_pcd,
        description='3D PCD 先验地图的绝对路径'
    )

    declare_map_yaml = DeclareLaunchArgument(
        'map_yaml',
        default_value=default_map_yaml,
        description='2D 栅格地图的绝对路径'
    )

    # odom → base_link
    mujoco_tf_bridge_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('mujoco_tf_bridge'),
                'launch',
                'mujoco_tf_bridge_launch.py'
            )
        ),
    )

    # base_link → lidar
    static_tf_base_to_lidar = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_base_to_lidar',
        arguments=[
            '--x', '0.0', '--y', '0.0', '--z', '0.5',
            '--roll', '0.0', '--pitch', '0.0', '--yaw', '0.0',
            '--frame-id', 'base_link',
            '--child-frame-id', 'lidar',
        ],
        output='screen',
    )

    # map → odom
    static_tf_map_to_odom = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_map_to_odom',
        arguments=[
            '--x', '0.0', '--y', '0.0', '--z', '0.0',
            '--roll', '0.0', '--pitch', '0.0', '--yaw', '0.0',
            '--frame-id', 'map',
            '--child-frame-id', 'odom',
        ],
        output='screen',
    )

    pointcloud_to_laserscan = Node(
        package='pointcloud_to_laserscan',
        executable='pointcloud_to_laserscan_node',
        name='pointcloud_to_laserscan',
        remappings=[
            ('cloud_in', '/front_lidar'),
            ('scan', '/scan'),
        ],
        parameters=[{
            'target_frame': 'base_link',
            'transform_tolerance': 0.01,
            'min_height': -0.3,
            'max_height': 1.5,
            'angle_min': -3.14159,
            'angle_max': 3.14159,
            'angle_increment': 0.0087,
            'scan_time': 0.1,
            'range_min': 0.1,
            'range_max': 30.0,
            'use_inf': True,
        }],
        output='screen',
    )

    zsibot_cmd_bridge_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('zsibot_cmd_bridge'),
                'launch',
                'zsibot_cmd_bridge_launch.py'
            )
        ),
    )

    # small_gicp_launch = IncludeLaunchDescription(
    #     PythonLaunchDescriptionSource(
    #         os.path.join(
    #             get_package_share_directory('small_gicp_relocalization'),
    #             'launch',
    #             'small_gicp_relocalization_launch.py'
    #         )
    #     ),
    #     launch_arguments={
    #         'prior_pcd_file': pcd_file,
    #     }.items(),
    # )

    nav2_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('me_nav2_bringup'),
                'launch',
                'my_nav2_launch.py'
            )
        ),
        launch_arguments={
            'map_yaml_file': map_yaml,
            'use_sim_time': 'false',
        }.items(),
    )

    return LaunchDescription([
        declare_pcd_file,
        declare_map_yaml,

        # 立即启动：TF 桥接 + 静态 TF
        mujoco_tf_bridge_launch,
        static_tf_base_to_lidar,
        static_tf_map_to_odom,
        pointcloud_to_laserscan,
        zsibot_cmd_bridge_launch,

        # 延迟启动：等 TF 稳定后再启重定位
        # TimerAction(period=3.0, actions=[small_gicp_launch]),

        # 再延迟：等重定位发布 camera_init → odom 后再启 Nav2
        TimerAction(period=6.0, actions=[nav2_launch]),
    ])