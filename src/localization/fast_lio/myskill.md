source /opt/ros/humble/setup.bash
source ~/Desktop/fast_lio2_ws/install/setup.bash
export RMW_IMPLEMENTATION=rmw_zenoh_cpp
export ORS_DOMAIN_ID=89
export SDK_CLIENT_IP=127.0.0.1

ros2 launch fast_lio mapping.launch.py config_file:=mid360.yaml
