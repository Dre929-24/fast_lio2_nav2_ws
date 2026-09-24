from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    mujoco_tf_bridge_node = Node(
        package='mujoco_tf_bridge',
        executable='mujoco_tf_bridge',
        name='mujoco_tf_bridge',
        output='screen',
    )

    return LaunchDescription([
        mujoco_tf_bridge_node,
    ])