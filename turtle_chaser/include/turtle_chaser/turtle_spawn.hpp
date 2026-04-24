#pragma once
#ifndef TURTLE_CHASER__TURTLE_SPAWN_HPP
#define TURTLE_CHASER__TURTLE_SPAWN_HPP

// std
#include <random>
#include <list>
#include <mutex>
// ROS 2
#include <rclcpp/rclcpp.hpp>
#include <turtlesim/srv/spawn.hpp>
#include <turtlesim/srv/kill.hpp>
// user interfaces
#include "turtle_interfaces/msg/turtle.hpp"
#include "turtle_interfaces/msg/turtle_array.hpp"
#include "turtle_interfaces/srv/turtle_catch.hpp"


class TurtleSpawn : public rclcpp::Node
{
public:
    TurtleSpawn();

private:
    // Random generation
    double lower_bound_;
    double upper_bound_;
    std::mt19937 generator_;
    std::uniform_real_distribution<double> random_location_;
    std::uniform_real_distribution<double> random_angle_;

    // State
    int turtle_count_;
    std::list<turtle_interfaces::msg::Turtle> alive_turtles_;
    std::mutex turtles_mutex_;

    
    rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr spawn_client_;
    rclcpp::Client<turtlesim::srv::Kill>::SharedPtr kill_client_;

    rclcpp::Publisher<turtle_interfaces::msg::TurtleArray>::SharedPtr alive_pub_;
    rclcpp::Service<turtle_interfaces::srv::TurtleCatch>::SharedPtr catch_srv_;
    rclcpp::TimerBase::SharedPtr spawn_timer_;

private:
    void init_all();

    // logic
    void spawn_turtle(double x, double y, double theta, const std::string &name);
    void kill_turtle(const std::string &name);

    // Callbacks
    void spawn_timer_callback();
    void catch_turtle_callback(
        const turtle_interfaces::srv::TurtleCatch::Request::SharedPtr req,
        const turtle_interfaces::srv::TurtleCatch::Response::SharedPtr res);

    // Helper
    void publish_alive_turtles();
};

#endif // TURTLE_CHASER__TURTLE_SPAWN_HPP