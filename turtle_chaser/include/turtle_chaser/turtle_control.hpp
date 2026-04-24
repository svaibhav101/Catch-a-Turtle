#pragma once
#ifndef TURTLE_CHASER__TURTLE_CONTROL_HPP
#define TURTLE_CHASER__TURTLE_CONTROL_HPP

// std
#include <memory>
#include <vector>
// ROS 2
#include <rclcpp/rclcpp.hpp>
#include <turtlesim/msg/pose.hpp>
#include <geometry_msgs/msg/twist.hpp>
// user interfaces
#include "turtle_interfaces/msg/turtle.hpp"
#include "turtle_interfaces/msg/turtle_array.hpp"
#include "turtle_interfaces/srv/turtle_catch.hpp"


class TurtleControl : public rclcpp::Node
{
public:
    TurtleControl();

private:
    void init_all();

    // Callbacks
    void t1_pose_callback(const turtlesim::msg::Pose::SharedPtr msg);
    void alive_turtles_callback(const turtle_interfaces::msg::TurtleArray::SharedPtr msg);
    void control_loop_callback();

    void catch_turtle_service_callback(const std::string & turtle_name);
    void catch_callback(rclcpp::Client<turtle_interfaces::srv::TurtleCatch>::SharedFuture future,
                        std::string turtle_name);

    // Members
    double lower_bound_;
    double upper_bound_;

    turtlesim::msg::Pose::SharedPtr t1_pose_;
    turtle_interfaces::msg::Turtle::SharedPtr turtle_to_catch_;

    std::vector<double> lin_pid_;
    std::vector<double> ang_pid_;

    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr t1_vel_pub_;
    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr t1_pose_sub_;
    rclcpp::Subscription<turtle_interfaces::msg::TurtleArray>::SharedPtr alive_turtles_sub_;

    rclcpp::TimerBase::SharedPtr control_loop_timer_;

    rclcpp::Client<turtle_interfaces::srv::TurtleCatch>::SharedPtr catch_client_;
};

#endif  // TURTLE_CHASER__TURTLE_CONTROL_HPP