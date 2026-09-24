from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='robot_marker',
            executable='robot_marker',
            name='robot_marker',
            output='screen',
        ),
    ])