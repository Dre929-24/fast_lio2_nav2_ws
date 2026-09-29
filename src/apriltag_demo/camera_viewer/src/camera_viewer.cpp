#include <chrono>
#include <memory>
#include <string>
#include <mutex>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "apriltag_msgs/msg/april_tag_detection_array.hpp"
#include "opencv2/opencv.hpp"

using namespace std::chrono_literals;

class CameraViewer : public rclcpp::Node
{
public:
    CameraViewer() : Node("camera_viewer")
    {
        this->declare_parameter<int>("display_width", 640);
        this->declare_parameter<std::string>("window_name", "Camera View");
        this->declare_parameter<double>("tag_size", 0.053);
        this->declare_parameter<double>("fovy_deg", 58.0);

        display_width_ = this->get_parameter("display_width").as_int();
        window_name_ = this->get_parameter("window_name").as_string();
        tag_size_ = this->get_parameter("tag_size").as_double();
        fovy_deg_ = this->get_parameter("fovy_deg").as_double();

        sub_image_ = this->create_subscription<sensor_msgs::msg::CompressedImage>(
            "/front_camera/image/compressed", 10,
            std::bind(&CameraViewer::imageCallback, this, std::placeholders::_1));

        sub_detections_ = this->create_subscription<apriltag_msgs::msg::AprilTagDetectionArray>(
            "/detections", 10,
            std::bind(&CameraViewer::detectionsCallback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Camera viewer started, display width: %d",
                    display_width_);
    }

private:
    void detectionsCallback(const apriltag_msgs::msg::AprilTagDetectionArray::SharedPtr msg)
    {
        std::lock_guard<std::mutex> lock(detections_mutex_);
        detections_ = msg->detections;
    }

    // 根据 Tag 四个角点的像素边长估算距离
    double estimateDistance(const apriltag_msgs::msg::AprilTagDetection &det, int image_height)
    {
        double sum = 0.0;
        int n = det.corners.size();
        for (int i = 0; i < n; i++)
        {
            const auto &p1 = det.corners[i];
            const auto &p2 = det.corners[(i + 1) % n];
            double dx = p2.x - p1.x;
            double dy = p2.y - p1.y;
            sum += std::sqrt(dx * dx + dy * dy);
        }
        double side_px = sum / n;
        if (side_px < 1.0)
            return -1.0;

        double fovy_rad = fovy_deg_ * M_PI / 180.0;
        double fy = (image_height / 2.0) / std::tan(fovy_rad / 2.0);

        return (tag_size_ * fy) / side_px;
    }

    void imageCallback(const sensor_msgs::msg::CompressedImage::SharedPtr msg)
    {
        try
        {
            cv::Mat image = cv::imdecode(cv::Mat(msg->data), cv::IMREAD_COLOR);
            if (image.empty())
            {
                RCLCPP_WARN(this->get_logger(), "Failed to decode image");
                return;
            }

            int image_h = image.rows;

            {
                std::lock_guard<std::mutex> lock(detections_mutex_);

                for (const auto &det : detections_)
                {
                    std::vector<cv::Point> pts;
                    for (const auto &corner : det.corners)
                    {
                        pts.push_back(cv::Point(static_cast<int>(corner.x),
                                                static_cast<int>(corner.y)));
                    }
                    cv::polylines(image, pts, true, cv::Scalar(0, 255, 0), 2);

                    cv::Point center(static_cast<int>(det.centre.x),
                                     static_cast<int>(det.centre.y));
                    cv::circle(image, center, 5, cv::Scalar(0, 0, 255), -1);

                    for (const auto &corner : det.corners)
                    {
                        cv::circle(image,
                                   cv::Point(static_cast<int>(corner.x),
                                             static_cast<int>(corner.y)),
                                   3, cv::Scalar(255, 0, 0), -1);
                    }

                    // 框内文字：ID + Confidence
                    double confidence = std::min(det.decision_margin / 240.0, 1.0);

                    std::vector<std::string> lines;
                    lines.push_back("ID: " + std::to_string(det.id));

                    std::ostringstream oss_conf;
                    oss_conf << std::fixed << std::setprecision(2)
                             << "Conf: " << confidence;
                    lines.push_back(oss_conf.str());

                    int min_x = image.cols;
                    int min_y = image.rows;
                    for (const auto &corner : det.corners)
                    {
                        min_x = std::min(min_x, static_cast<int>(corner.x));
                        min_y = std::min(min_y, static_cast<int>(corner.y));
                    }

                    int base_x = min_x + 15;
                    int base_y = min_y + 30;
                    int line_height = 32;

                    for (size_t i = 0; i < lines.size(); i++)
                    {
                        int y = base_y + i * line_height;

                        cv::putText(image, lines[i],
                                    cv::Point(base_x, y),
                                    cv::FONT_HERSHEY_SIMPLEX, 0.9,
                                    cv::Scalar(0, 0, 0), 4);
                        cv::putText(image, lines[i],
                                    cv::Point(base_x, y),
                                    cv::FONT_HERSHEY_SIMPLEX, 0.9,
                                    cv::Scalar(0, 255, 255), 2);
                    }
                }

                int panel_x = 15;
                int panel_y = 35;
                int panel_line_height = 30;
                int panel_lines = 0;

                for (const auto &det : detections_)
                {
                    double confidence = std::min(det.decision_margin / 240.0, 1.0);
                    double distance = estimateDistance(det, image_h);

                    // 第一行：ID + Center + Dist
                    std::ostringstream oss_line1;
                    oss_line1 << "ID=" << det.id
                              << "  Center=(" << std::fixed << std::setprecision(0)
                              << det.centre.x << "," << det.centre.y << ")"
                              << "  Dist=" << std::fixed << std::setprecision(2)
                              << distance << "m";

                    // 第二行：Conf
                    std::ostringstream oss_line2;
                    oss_line2 << "  Conf=" << std::fixed << std::setprecision(2)
                              << confidence;

                    // 检查第一行会不会超出右边界
                    int font_face = cv::FONT_HERSHEY_SIMPLEX;
                    double font_scale = 0.8;
                    int thickness = 1;
                    int baseline = 0;
                    cv::Size text_size1 = cv::getTextSize(oss_line1.str(), font_face, font_scale,
                                                          thickness, &baseline);

                    int y = panel_y + panel_lines * panel_line_height;

                    if (panel_x + text_size1.width < image.cols - 10)
                    {
                        // 第一行不超边界，检查第一行 + Conf 是否超边界
                        std::string combined = oss_line1.str() + oss_line2.str();
                        cv::Size text_size_combined = cv::getTextSize(combined, font_face, font_scale,
                                                                      thickness, &baseline);

                        if (panel_x + text_size_combined.width < image.cols - 10)
                        {
                            // 一行能放下，合并
                            cv::putText(image, combined,
                                        cv::Point(panel_x, y),
                                        font_face, font_scale,
                                        cv::Scalar(0, 0, 0), 4);
                            cv::putText(image, combined,
                                        cv::Point(panel_x, y),
                                        font_face, font_scale,
                                        cv::Scalar(0, 255, 0), thickness);
                            panel_lines++;
                        }
                        else
                        {
                            // 一行放不下，分两行
                            cv::putText(image, oss_line1.str(),
                                        cv::Point(panel_x, y),
                                        font_face, font_scale,
                                        cv::Scalar(0, 0, 0), 4);
                            cv::putText(image, oss_line1.str(),
                                        cv::Point(panel_x, y),
                                        font_face, font_scale,
                                        cv::Scalar(0, 255, 0), thickness);
                            panel_lines++;

                            int y2 = panel_y + panel_lines * panel_line_height;
                            cv::putText(image, oss_line2.str(),
                                        cv::Point(panel_x, y2),
                                        font_face, font_scale,
                                        cv::Scalar(0, 0, 0), 4);
                            cv::putText(image, oss_line2.str(),
                                        cv::Point(panel_x, y2),
                                        font_face, font_scale,
                                        cv::Scalar(0, 255, 0), thickness);
                            panel_lines++;
                        }
                    }
                    else
                    {
                        // 第一行本身就超边界，分两行
                        cv::putText(image, oss_line1.str(),
                                    cv::Point(panel_x, y),
                                    font_face, font_scale,
                                    cv::Scalar(0, 0, 0), 4);
                        cv::putText(image, oss_line1.str(),
                                    cv::Point(panel_x, y),
                                    font_face, font_scale,
                                    cv::Scalar(0, 255, 0), thickness);
                        panel_lines++;

                        int y2 = panel_y + panel_lines * panel_line_height;
                        cv::putText(image, oss_line2.str(),
                                    cv::Point(panel_x, y2),
                                    font_face, font_scale,
                                    cv::Scalar(0, 0, 0), 4);
                        cv::putText(image, oss_line2.str(),
                                    cv::Point(panel_x, y2),
                                    font_face, font_scale,
                                    cv::Scalar(0, 255, 0), thickness);
                        panel_lines++;
                    }
                }
            }

            // 等比例缩放
            cv::Mat resized;
            if (image.cols > display_width_)
            {
                double scale = static_cast<double>(display_width_) / image.cols;
                int new_height = static_cast<int>(image.rows * scale);
                cv::resize(image, resized,
                           cv::Size(display_width_, new_height), 0, 0, cv::INTER_LINEAR);
            }
            else
            {
                resized = image;
            }

            cv::imshow(window_name_, resized);
            cv::waitKey(1);
        }
        catch (const std::exception &e)
        {
            RCLCPP_ERROR(this->get_logger(), "Error: %s", e.what());
        }
    }

    rclcpp::Subscription<sensor_msgs::msg::CompressedImage>::SharedPtr sub_image_;
    rclcpp::Subscription<apriltag_msgs::msg::AprilTagDetectionArray>::SharedPtr sub_detections_;

    int display_width_;
    std::string window_name_;
    double tag_size_;
    double fovy_deg_;

    std::vector<apriltag_msgs::msg::AprilTagDetection> detections_;
    std::mutex detections_mutex_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CameraViewer>());
    rclcpp::shutdown();
    return 0;
}