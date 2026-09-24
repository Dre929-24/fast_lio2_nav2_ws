#include <cmath>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/msg/transform_stamped.hpp"

class MujocoTfBridge : public rclcpp::Node
{
public:
    MujocoTfBridge() : Node("mujoco_tf_bridge")
    {
        tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);
    
        sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
            "/odom/mujoco_odom", 10,
            std::bind(&MujocoTfBridge::odomCallback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Mujoco TF bridge started");
    }

private:
    void odomCallback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        geometry_msgs::msg::TransformStamped t;
        t.header.stamp = msg->header.stamp;
        t.header.frame_id = "odom";
        t.child_frame_id = "base_link";

        // 位置：只保留 x, y，z 压平到 0
        t.transform.translation.x = msg->pose.pose.position.x;
        t.transform.translation.y = msg->pose.pose.position.y;
        t.transform.translation.z = 0.0;

        // 姿态：只保留 yaw，去掉 roll 和 pitch
        const auto &q = msg->pose.pose.orientation;
        double siny_cosp = 2.0 * (q.w * q.z + q.x * q.y);
        double cosy_cosp = 1.0 - 2.0 * (q.y * q.y + q.z * q.z);
        double yaw = std::atan2(siny_cosp, cosy_cosp);

        t.transform.rotation.x = 0.0;
        t.transform.rotation.y = 0.0;
        t.transform.rotation.z = std::sin(yaw * 0.5);
        t.transform.rotation.w = std::cos(yaw * 0.5);

        tf_broadcaster_->sendTransform(t);
    }

    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr sub_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MujocoTfBridge>());
    rclcpp::shutdown();
    return 0;
}