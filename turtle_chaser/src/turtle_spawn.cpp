#include "turtle_chaser/turtle_spawn.hpp"

TurtleSpawn::TurtleSpawn()
    : Node("turtle_spawn_cpp"),
      lower_bound_(1.0),
      upper_bound_(10.0),
      generator_(std::random_device{}()),
      random_location_(lower_bound_, upper_bound_),
      random_angle_(0.0, 2 * M_PI),
      turtle_count_(1)
{
    init_all();

    RCLCPP_INFO(get_logger(), "TurtleSpawn node started.");
}

void TurtleSpawn::init_all()
{
    spawn_client_ = create_client<turtlesim::srv::Spawn>("spawn");
    kill_client_ = create_client<turtlesim::srv::Kill>("kill");

    alive_pub_ = create_publisher<turtle_interfaces::msg::TurtleArray>(
        "alive_turtles", rclcpp::QoS(10));

    catch_srv_ = create_service<turtle_interfaces::srv::TurtleCatch>(
        "catch_turtle",
        std::bind(&TurtleSpawn::catch_turtle_callback, this,
                  std::placeholders::_1, std::placeholders::_2));

    spawn_timer_ = create_wall_timer(
        std::chrono::seconds(3),
        // std::chrono::milliseconds(10),
        std::bind(&TurtleSpawn::spawn_timer_callback, this));
}

void TurtleSpawn::spawn_turtle(double x, double y, double theta, const std::string &name)
{
    if (!spawn_client_->wait_for_service(std::chrono::seconds(1)))
    {
        RCLCPP_WARN(get_logger(), "Spawn service not available");
        return;
    }

    auto request = std::make_shared<turtlesim::srv::Spawn::Request>();
    request->x = x;
    request->y = y;
    request->theta = theta;
    request->name = name;

    spawn_client_->async_send_request(
        request,
        [this, x, y, theta, name](rclcpp::Client<turtlesim::srv::Spawn>::SharedFuture future)
        {
            try
            {
                auto response = future.get();

                RCLCPP_INFO(get_logger(), "%s spawned", response->name.c_str());

                turtle_interfaces::msg::Turtle turtle;
                turtle.x = x;
                turtle.y = y;
                turtle.theta = theta;
                turtle.name = name;

                {
                    std::lock_guard<std::mutex> lock(turtles_mutex_);
                    alive_turtles_.push_back(turtle);
                }

                publish_alive_turtles();
            }
            catch (const std::exception &e)
            {
                RCLCPP_ERROR(get_logger(), "Spawn failed: %s", e.what());
            }
        });
}

void TurtleSpawn::kill_turtle(const std::string &name)
{
    if (!kill_client_->wait_for_service(std::chrono::seconds(1)))
    {
        RCLCPP_WARN(get_logger(), "Kill service not available");
        return;
    }

    auto request = std::make_shared<turtlesim::srv::Kill::Request>();
    request->name = name;

    kill_client_->async_send_request(
        request,
        [this, name](rclcpp::Client<turtlesim::srv::Kill>::SharedFuture future)
        {
            try
            {
                future.get();

                {
                    std::lock_guard<std::mutex> lock(turtles_mutex_);
                    alive_turtles_.remove_if(
                        [&](const auto &turtle)
                        {
                            return turtle.name == name;
                        });
                }

                publish_alive_turtles();
                RCLCPP_INFO(get_logger(), "%s removed", name.c_str());
            }
            catch (const std::exception &e)
            {
                RCLCPP_ERROR(get_logger(), "Kill failed: %s", e.what());
            }
        });
}

void TurtleSpawn::spawn_timer_callback()
{
    turtle_count_++;

    double x = random_location_(generator_);
    double y = random_location_(generator_);
    double theta = random_angle_(generator_);

    std::string name = "turtle" + std::to_string(turtle_count_);

    spawn_turtle(x, y, theta, name);
}

void TurtleSpawn::catch_turtle_callback(
    const turtle_interfaces::srv::TurtleCatch::Request::SharedPtr req,
    const turtle_interfaces::srv::TurtleCatch::Response::SharedPtr res)
{
    kill_turtle(req->name);
    res->success = true;
}

void TurtleSpawn::publish_alive_turtles()
{
    turtle_interfaces::msg::TurtleArray msg;

    {
        std::lock_guard<std::mutex> lock(turtles_mutex_);
        for (const auto &turtle : alive_turtles_)
        {
            msg.turtles.push_back(turtle);
        }
    }

    alive_pub_->publish(msg);
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<TurtleSpawn>();
    rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}