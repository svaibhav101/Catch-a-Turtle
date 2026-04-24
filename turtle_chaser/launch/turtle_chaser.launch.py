from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():

    turtlesim_node = Node(
        package="turtlesim",
        executable="turtlesim_node",
        name="turtlesim",
        output="screen",
    )

    control_node = Node(
        package="turtle_chaser",
        executable="turtle_control_node",
        name="turtle_control_node",
        output="screen",
    )

    spawner_node = Node(
        package="turtle_chaser",
        executable="turtle_spawner_node",
        name="turtle_spawner",
        output="screen",
    )

    ld = LaunchDescription
    ld.add_action(turtlesim_node)
    ld.add_action(control_node)
    ld.add_action(spawner_node)

    return ld
