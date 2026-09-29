#include <chrono>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "opencv2/opencv.hpp"

using namespace std::chrono_literals;

class MediaPublisher : public rclcpp::Node
{
public:
    MediaPublisher() : Node("media_publisher")
    {
        this->declare_parameter<std::string>("mode", "image");     // "image" / "video" / "camera"
        this->declare_parameter<std::string>("file_path", "");     // 文件路径（image/video 用）
        this->declare_parameter<int>("camera_id", 0);              // 摄像头 ID（camera 用）
        this->declare_parameter<double>("publish_rate", 10.0);     // 发布频率
        this->declare_parameter<bool>("loop", true);               // 视频是否循环

        mode_ = this->get_parameter("mode").as_string();
        std::string file_path = this->get_parameter("file_path").as_string();
        int camera_id = this->get_parameter("camera_id").as_int();
        double rate = this->get_parameter("publish_rate").as_double();
        loop_ = this->get_parameter("loop").as_bool();

        if (mode_ == "image") {
            if (file_path.empty()) {
                RCLCPP_ERROR(this->get_logger(), "file_path is empty!");
                return;
            }
            loadImage(file_path);
        } else if (mode_ == "video") {
            if (file_path.empty()) {
                RCLCPP_ERROR(this->get_logger(), "file_path is empty!");
                return;
            }
            openVideo(file_path);
        } else if (mode_ == "camera") {
            openCamera(camera_id);
        } else {
            RCLCPP_ERROR(this->get_logger(),
                         "Unknown mode: %s (use 'image', 'video', or 'camera')",
                         mode_.c_str());
            return;
        }

        pub_ = this->create_publisher<sensor_msgs::msg::CompressedImage>(
            "/front_camera/image/compressed", 10);

        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / rate));
        timer_ = this->create_wall_timer(
            period, std::bind(&MediaPublisher::publishCallback, this));

        RCLCPP_INFO(this->get_logger(), "Mode: %s, publishing at %.1f Hz",
                    mode_.c_str(), rate);
    }

private:
    // ============ 图片模式 ============
    void loadImage(const std::string & path)
    {
        cv::Mat image = cv::imread(path, cv::IMREAD_COLOR);
        if (image.empty()) {
            RCLCPP_ERROR(this->get_logger(), "Failed to read image: %s", path.c_str());
            return;
        }

        if (image.channels() == 4) {
            cv::cvtColor(image, image, cv::COLOR_BGRA2BGR);
        }

        std::vector<uint8_t> buffer;
        cv::imencode(".png", image, buffer);
        image_data_ = buffer;
        image_loaded_ = true;

        RCLCPP_INFO(this->get_logger(), "Loaded image: %s (%dx%d)",
                    path.c_str(), image.cols, image.rows);
    }

    // ============ 视频模式 ============
    void openVideo(const std::string & path)
    {
        cap_.open(path);
        if (!cap_.isOpened()) {
            RCLCPP_ERROR(this->get_logger(), "Failed to open video: %s", path.c_str());
            return;
        }

        int width = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_WIDTH));
        int height = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_HEIGHT));
        double fps = cap_.get(cv::CAP_PROP_FPS);
        int total = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_COUNT));

        RCLCPP_INFO(this->get_logger(), "Opened video: %s (%dx%d, %.1f fps, %d frames)",
                    path.c_str(), width, height, fps, total);

        video_loaded_ = true;
    }

    // ============ 摄像头模式 ============
    void openCamera(int camera_id)
    {
        // cap_.open(camera_id, cv::CAP_V4L2);
        // if (!cap_.isOpened()) {
        //     RCLCPP_ERROR(this->get_logger(), "Failed to open camera %d", camera_id);
        //     return;
        // }

        std::string device = "/dev/video" + std::to_string(camera_id);

        cap_.open(device, cv::CAP_V4L2);

        // 设置分辨率
        cap_.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        cap_.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

        int width = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_WIDTH));
        int height = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_HEIGHT));
        double fps = cap_.get(cv::CAP_PROP_FPS);

        RCLCPP_INFO(this->get_logger(), "Opened camera %d (%dx%d, %.1f fps)",
                    camera_id, width, height, fps);

        camera_loaded_ = true;
    }

    void publishCallback()
    {
        if (mode_ == "image") {
            publishImage();
        } else if (mode_ == "video") {
            publishVideoFrame();
        } else if (mode_ == "camera") {
            publishCameraFrame();
        }
    }

    void publishImage()
    {
        if (!image_loaded_) return;

        sensor_msgs::msg::CompressedImage msg;
        msg.header.stamp = this->get_clock()->now();
        msg.header.frame_id = "front_camera";
        msg.format = "png";
        msg.data = image_data_;
        pub_->publish(msg);
    }

    void publishVideoFrame()
    {
        if (!video_loaded_) return;

        cv::Mat frame;
        if (!cap_.read(frame)) {
            if (loop_) {
                cap_.set(cv::CAP_PROP_POS_FRAMES, 0);
                if (!cap_.read(frame)) {
                    RCLCPP_WARN(this->get_logger(), "Failed to read frame");
                    return;
                }
            } else {
                RCLCPP_INFO(this->get_logger(), "Video ended");
                timer_->cancel();
                return;
            }
        }

        publishFrame(frame);
    }

    void publishCameraFrame()
    {
        if (!camera_loaded_) return;

        cv::Mat frame;
        if (!cap_.read(frame)) {
            RCLCPP_WARN(this->get_logger(), "Failed to read camera frame");
            return;
        }

        publishFrame(frame);
    }

    void publishFrame(cv::Mat & frame)
    {
        if (frame.channels() == 4) {
            cv::cvtColor(frame, frame, cv::COLOR_BGRA2BGR);
        }

        std::vector<uint8_t> buffer;
        cv::imencode(".png", frame, buffer);

        sensor_msgs::msg::CompressedImage msg;
        msg.header.stamp = this->get_clock()->now();
        msg.header.frame_id = "front_camera";
        msg.format = "png";
        msg.data = buffer;
        pub_->publish(msg);
    }

    rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;

    std::string mode_;
    bool loop_;

    std::vector<uint8_t> image_data_;
    bool image_loaded_ = false;

    cv::VideoCapture cap_;
    bool video_loaded_ = false;
    bool camera_loaded_ = false;
};

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MediaPublisher>());
    rclcpp::shutdown();
    return 0;
}