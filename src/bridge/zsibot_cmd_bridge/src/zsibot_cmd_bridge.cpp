#include <algorithm>
#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "zsl-1/highlevel.h"

using namespace mc_sdk::zsl_1;
using namespace std::chrono_literals;


class ZsibotCmdBridge : public rclcpp::Node
{
public:
    ZsibotCmdBridge() : Node("zsibot_cmd_bridge")
    {
        this->declare_parameter<std::string>("client_ip", "127.0.0.1");
        this->declare_parameter<int>("client_port", 43988);
        this->declare_parameter<std::string>("dog_ip", "127.0.0.1");
        // speed: [x] 0.05~3.0m/s    [y] 0.1~1.0m/s      [z] 0.02~3.0 rad/s
        this->declare_parameter<double>("max_vx", 3.0);
        this->declare_parameter<double>("max_vy", 1.0);
        this->declare_parameter<double>("max_wz", 3.0);
        this->declare_parameter<double>("watchdog_timeout", 0.5);

        std::string client_ip = this->get_parameter("client_ip").as_string();
        int client_port = this->get_parameter("client_port").as_int();
        std::string dog_ip = this->get_parameter("dog_ip").as_string();
        max_vx_ = this->get_parameter("max_vx").as_double();
        max_vy_ = this->get_parameter("max_vy").as_double();
        max_wz_ = this->get_parameter("max_wz").as_double();
        watchdog_timeout_ = this->get_parameter("watchdog_timeout").as_double();

        highlevel_.initRobot(client_ip, client_port, dog_ip);
        RCLCPP_INFO(this->get_logger(), "Zsibot SDK connected: %s:%d -> %s",
                    client_ip.c_str(), client_port, dog_ip.c_str());
        
        RCLCPP_INFO(this->get_logger(), "Standing up robot...");
        highlevel_.standUp();
        std::this_thread::sleep_for(2s);
        RCLCPP_INFO(this->get_logger(), "Robot standing, ready to move");

        sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/cmd_vel", 10,
            std::bind(&ZsibotCmdBridge::cmdVelCallback, this, std::placeholders::_1));

        last_cmd_time_ = this->now();
        watchdog_timer_ = this->create_wall_timer(
            100ms, std::bind(&ZsibotCmdBridge::watchdogCallback, this));

        RCLCPP_INFO(this->get_logger(), "Zsibot cmd bridge started");
    }

private:
    void cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        last_cmd_time_ = this->now();

        double vx = std::clamp(msg->linear.x, -max_vx_, max_vx_);
        double vy = std::clamp(msg->linear.y, -max_vy_, max_vy_);
        double wz = std::clamp(msg->angular.z, -max_wz_, max_wz_);

        RCLCPP_DEBUG(this->get_logger(), "cmd_vel: vx=%.2f vy=%.2f wz=%.2f", vx, vy, wz);
        highlevel_.move(static_cast<float>(vx),
                        static_cast<float>(vy),
                        static_cast<float>(wz));
    }

    void watchdogCallback()
    {
        auto elapsed = (this->now() - last_cmd_time_).seconds();
        if (elapsed > watchdog_timeout_) {
            highlevel_.move(0.0f, 0.0f, 0.0f);
        }
    }

    HighLevel highlevel_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_;
    rclcpp::TimerBase::SharedPtr watchdog_timer_;
    rclcpp::Time last_cmd_time_;

    double max_vx_, max_vy_, max_wz_, watchdog_timeout_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ZsibotCmdBridge>());
    rclcpp::shutdown();
    return 0;
}