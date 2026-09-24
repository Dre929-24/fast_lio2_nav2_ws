#include <chrono>
#include <memory>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/marker.hpp"
#include "visualization_msgs/msg/marker_array.hpp"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"
#include "geometry_msgs/msg/transform_stamped.hpp"

using namespace std::chrono_literals;

class RobotMarker : public rclcpp::Node
{
public:
    RobotMarker() : Node("robot_marker")
    {
        pub_ = this->create_publisher<visualization_msgs::msg::MarkerArray>(
            "/robot_marker", 10);

        tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

        timer_ = this->create_wall_timer(100ms, std::bind(&RobotMarker::update, this));

        RCLCPP_INFO(this->get_logger(), "Robot marker started");
    }

private:
    void update()
    {
        geometry_msgs::msg::TransformStamped t;
        try
        {
            t = tf_buffer_->lookupTransform("map", "base_link", tf2::TimePointZero);
        }
        catch (const tf2::TransformException &ex)
        {
            return;
        }

        double x = t.transform.translation.x;
        double y = t.transform.translation.y;

        visualization_msgs::msg::MarkerArray arr;

        // ============ 1. 绿色圈（位置）============
        visualization_msgs::msg::Marker circle;
        circle.header.frame_id = "map";
        circle.header.stamp = this->get_clock()->now();
        circle.ns = "robot_circle";
        circle.id = 0;
        circle.type = visualization_msgs::msg::Marker::LINE_STRIP; // ← 改成 LINE_STRIP
        circle.action = visualization_msgs::msg::Marker::ADD;

        circle.pose.position.x = 0.0;
        circle.pose.position.y = 0.0;
        circle.pose.position.z = 0.0;
        circle.pose.orientation.w = 1.0;

        circle.scale.x = 0.05; // 圈线的粗细

        circle.color.r = 0.0f;
        circle.color.g = 1.0f;
        circle.color.b = 0.0f;
        circle.color.a = 1.0f;

        // 画一个圆（36 段）
        const double radius = 0.3;
        const int segments = 36;
        circle.points.clear();
        for (int i = 0; i <= segments; i++)
        {
            double theta = 2.0 * M_PI * i / segments;
            geometry_msgs::msg::Point p;
            p.x = x + radius * std::cos(theta);
            p.y = y + radius * std::sin(theta);
            p.z = 0.05;
            circle.points.push_back(p);
        }

        circle.lifetime = rclcpp::Duration::from_seconds(0.5);
        arr.markers.push_back(circle);

        // ============ 2. 红色箭头（朝向）============
        visualization_msgs::msg::Marker arrow;
        arrow.header.frame_id = "map";
        arrow.header.stamp = this->get_clock()->now();
        arrow.ns = "robot_arrow";
        arrow.id = 1;
        arrow.type = visualization_msgs::msg::Marker::ARROW;
        arrow.action = visualization_msgs::msg::Marker::ADD;

        arrow.pose.position.x = x;
        arrow.pose.position.y = y;
        arrow.pose.position.z = 0.1;
        arrow.pose.orientation = t.transform.rotation; // 用 TF 的朝向

        arrow.scale.x = 0.6;  // 箭头长度
        arrow.scale.y = 0.12; // 箭头宽度
        arrow.scale.z = 0.12; // 箭头高度

        arrow.color.r = 1.0f; // 红色
        arrow.color.g = 0.0f;
        arrow.color.b = 0.0f;
        arrow.color.a = 1.0f;

        arrow.lifetime = rclcpp::Duration::from_seconds(0.5);
        arr.markers.push_back(arrow);

        // 发布
        pub_->publish(arr);
    }

    rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr pub_;
    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<RobotMarker>());
    rclcpp::shutdown();
    return 0;
}