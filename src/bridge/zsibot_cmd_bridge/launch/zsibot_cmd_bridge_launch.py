from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    client_ip = LaunchConfiguration('client_ip')
    client_port = LaunchConfiguration('client_port')
    dog_ip = LaunchConfiguration('dog_ip')
    max_vx = LaunchConfiguration('max_vx')
    max_vy = LaunchConfiguration('max_vy')
    max_wz = LaunchConfiguration('max_wz')
    watchdog_timeout = LaunchConfiguration('watchdog_timeout')

    declare_client_ip = DeclareLaunchArgument(
        'client_ip', default_value='127.0.0.1',
        description='本地 IP'
    )
    declare_client_port = DeclareLaunchArgument(
        'client_port', default_value='43988',
        description='本地端口'
    )
    declare_dog_ip = DeclareLaunchArgument(
        'dog_ip', default_value='127.0.0.1',
        description='机器狗 IP（仿真用 127.0.0.1，真机用实际 IP）'
    )
    declare_max_vx = DeclareLaunchArgument(
        'max_vx', default_value='3.0',
        description='前进最大速度 (m/s)，SDK 范围 0.05~3.0'
    )
    declare_max_vy = DeclareLaunchArgument(
        'max_vy', default_value='1.0',
        description='侧向最大速度 (m/s)，SDK 范围 0.1~1.0'
    )
    declare_max_wz = DeclareLaunchArgument(
        'max_wz', default_value='3.0',
        description='转向最大角速度 (rad/s)，SDK 范围 0.02~3.0'
    )
    declare_watchdog_timeout = DeclareLaunchArgument(
        'watchdog_timeout', default_value='0.5',
        description='看门狗超时时间 (s)'
    )

    zsibot_cmd_bridge_node = Node(
        package='zsibot_cmd_bridge',
        executable='zsibot_cmd_bridge',
        name='zsibot_cmd_bridge',
        output='screen',
        parameters=[{
            'client_ip': client_ip,
            'client_port': client_port,
            'dog_ip': dog_ip,
            'max_vx': max_vx,
            'max_vy': max_vy,
            'max_wz': max_wz,
            'watchdog_timeout': watchdog_timeout,
        }],
    )

    return LaunchDescription([
        declare_client_ip,
        declare_client_port,
        declare_dog_ip,
        declare_max_vx,
        declare_max_vy,
        declare_max_wz,
        declare_watchdog_timeout,
        zsibot_cmd_bridge_node,
    ])