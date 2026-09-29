#include <chrono>
#include <cmath>
#include <memory>
#include <algorithm>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "opencv2/opencv.hpp"


using namespace std::chrono_literals;

class CameraInfoPublisher : public rclcpp::Node
{
public:
    CameraInfoPublisher() : Node("camera_info_pub")
    {
        // 从仿真 XML 读的 fovy = 58°
        this->declare_parameter<double>("fovy_deg", 58.0);

        fovy_deg_ = this->get_parameter("fovy_deg").as_double();

        pub_ = this->create_publisher<sensor_msgs::msg::CameraInfo>(
            "/front_camera/camera_info", 10);

        sub_ = this->create_subscription<sensor_msgs::msg::CompressedImage>(
            "/front_camera/image/compressed", 10,
            std::bind(&CameraInfoPublisher::imageCallback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Camera info publisher started (dynamic, fovy=%.1f°)",
                    fovy_deg_);
    }

private:
    void imageCallback(const sensor_msgs::msg::CompressedImage::SharedPtr msg)
    {
        // 解码图像，获取分辨率
        cv::Mat image = cv::imdecode(cv::Mat(msg->data), cv::IMREAD_COLOR);
        if (image.empty()) {
            RCLCPP_WARN(this->get_logger(), "Failed to decode image");
            return;
        }

        int width = image.cols;
        int height = image.rows;

        // 根据分辨率和 fovy 算内参
        double fovy_rad = fovy_deg_ * M_PI / 180.0;
        double fy = (height / 2.0) / std::tan(fovy_rad / 2.0);
        double fx = fy;   // 假设像素正方形
        double cx = width / 2.0;
        double cy = height / 2.0;

        sensor_msgs::msg::CameraInfo info;
        info.header.stamp = msg->header.stamp;
        info.header.frame_id = "front_camera";
        info.width = width;
        info.height = height;
        info.distortion_model = "plumb_bob";
        info.d = {0.0, 0.0, 0.0, 0.0, 0.0};
        info.k = {
            fx, 0.0, cx,
            0.0, fy, cy,
            0.0, 0.0, 1.0
        };
        info.r = {
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0
        };
        info.p = {
            fx, 0.0, cx, 0.0,
            0.0, fy, cy, 0.0,
            0.0, 0.0, 1.0, 0.0
        };
        pub_->publish(info);
    }

    rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr pub_;
    rclcpp::Subscription<sensor_msgs::msg::CompressedImage>::SharedPtr sub_;

    double fovy_deg_;
};

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CameraInfoPublisher>());
    rclcpp::shutdown();
    return 0;
}