// std
#include <cmath>
#include <chrono>
// user
#include "turtle_chaser/turtle_control.hpp"

using namespace std::chrono_literals;

TurtleControl::TurtleControl()
    : Node("turtle_control_cpp"),
      lower_bound_{1.0},
      upper_bound_{10.0},
      t1_pose_{nullptr},
      turtle_to_catch_{nullptr},
      lin_pid_{1.0, 0.0, 0.0},
      ang_pid_{4.0, 0.0, 0.0}

{
    init_all();
    RCLCPP_INFO(this->get_logger(), "TurtleControl node created.");
}

void TurtleControl::init_all()
{

    t1_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("turtle1/cmd_vel", 10);

    t1_pose_sub_ = this->create_subscription<turtlesim::msg::Pose>(
        "turtle1/pose", 10,
        std::bind(&TurtleControl::t1_pose_callback, this, std::placeholders::_1));

    alive_turtles_sub_ = this->create_subscription<turtle_interfaces::msg::TurtleArray>(
        "alive_turtles", 5,
        std::bind(&TurtleControl::alive_turtles_callback, this, std::placeholders::_1));

    control_loop_timer_ = this->create_wall_timer(
        10ms,
        std::bind(&TurtleControl::control_loop_callback, this));

    catch_client_ = this->create_client<turtle_interfaces::srv::TurtleCatch>("catch_turtle");
}

void TurtleControl::t1_pose_callback(const turtlesim::msg::Pose::SharedPtr msg)
{
    t1_pose_ = msg;
}

void TurtleControl::alive_turtles_callback(
    const turtle_interfaces::msg::TurtleArray::SharedPtr msg)
{
    if (!t1_pose_)
        return;

    double shortest_distance = 0.0;
    turtle_interfaces::msg::Turtle::SharedPtr nearest = nullptr;

    for (const auto &turtle : msg->turtles)
    {
        double dx = turtle.x - t1_pose_->x;
        double dy = turtle.y - t1_pose_->y;
        double distance = std::sqrt(dx * dx + dy * dy);

        if (!nearest || distance < shortest_distance)
        {
            shortest_distance = distance;
            nearest = std::make_shared<turtle_interfaces::msg::Turtle>(turtle);
        }
    }

    turtle_to_catch_ = nearest;
}

void TurtleControl::catch_turtle_service_callback(const std::string &turtle_name)
{
    while (!catch_client_->wait_for_service(1s))
    {
        RCLCPP_WARN(this->get_logger(), "Waiting for /catch_turtle service...");
    }

    auto request = std::make_shared<turtle_interfaces::srv::TurtleCatch::Request>();
    request->name = turtle_name;

    auto future = catch_client_->async_send_request(
        request,
        [this, turtle_name](rclcpp::Client<turtle_interfaces::srv::TurtleCatch>::SharedFuture future)
        {
            this->catch_callback(future, turtle_name);
        });
}

void TurtleControl::catch_callback(
    rclcpp::Client<turtle_interfaces::srv::TurtleCatch>::SharedFuture future,
    std::string turtle_name)
{
    try
    {
        future.get();
        RCLCPP_INFO(this->get_logger(), "%s removed.", turtle_name.c_str());
    }
    catch (const std::exception &e)
    {
        RCLCPP_ERROR(this->get_logger(), "Service call failed: %s", e.what());
    }
}

void TurtleControl::control_loop_callback()
{
    if (!t1_pose_ || !turtle_to_catch_)
        return;

    double dx = turtle_to_catch_->x - t1_pose_->x;
    double dy = turtle_to_catch_->y - t1_pose_->y;
    double distance = std::sqrt(dx * dx + dy * dy);

    geometry_msgs::msg::Twist vel;

    if (distance > 0.1)
    {
        vel.linear.x = lin_pid_[0] * distance;

        double target_theta = std::atan2(dy, dx);
        double diff_theta = target_theta - t1_pose_->theta;

        if (diff_theta > M_PI)
            diff_theta -= 2 * M_PI;
        else if (diff_theta < -M_PI)
            diff_theta += 2 * M_PI;

        vel.angular.z = ang_pid_[0] * diff_theta;
    }
    else
    {
        vel.linear.x = 0.0;
        vel.angular.z = 0.0;

        catch_turtle_service_callback(turtle_to_catch_->name);
        turtle_to_catch_ = nullptr;
    }

    t1_vel_pub_->publish(vel);
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<TurtleControl>();
    RCLCPP_INFO(node->get_logger(), "turtle_control_node is initialized.");

    rclcpp::spin(node);
    rclcpp::shutdown();

    return 0;
}