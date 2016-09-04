#include "ros/ros.h"
#include "ros/time.h"
#include "std_msgs/Bool.h"
#include "geometry_msgs/Twist.h"
#include "geometry_msgs/AccelStamped.h"
#include "sensor_msgs/Joy.h"

const int wii_button_front = 8;
const int wii_button_back  = 9;
const int wii_button_rignt = 7;
const int wii_button_left  = 6;

ros::Publisher cmd_vel_pub;
ros::Publisher state_pub;
ros::Publisher imu_override_pub;
ros::Publisher body_scalar_pub;
auto body_scalar_seq = 0;
auto hexapod_wakeup_button = 0;

void joy_callback(const sensor_msgs::Joy::ConstPtr& msg){

    auto imu_override_msg = std_msgs::Bool();
    if(msg->buttons[2]){
        imu_override_msg.data = 1;
        imu_override_pub.publish(imu_override_msg);
    }
    else{
        imu_override_msg.data = 0;
        imu_override_pub.publish(imu_override_msg);
    }

    auto body_scalar_msg = geometry_msgs::AccelStamped();
    body_scalar_msg.header.stamp.sec = ros::Time::now().toSec();
    body_scalar_msg.header.stamp.nsec = ros::Time::now().toNSec();
    body_scalar_msg.accel.linear.x = msg->axes[0];
    body_scalar_msg.accel.linear.y = msg->axes[1];
    body_scalar_msg.accel.linear.z = msg->axes[2];
    body_scalar_msg.accel.angular.x = msg->axes[0]/9.8;
    body_scalar_msg.accel.angular.y = msg->axes[1]/9.8;
    body_scalar_pub.publish(body_scalar_msg);

}

void nunchuk_callback(const sensor_msgs::Joy::ConstPtr& msg) {
    if(hexapod_wakeup_button == 0 && msg->buttons[0] == 1) {
        ROS_INFO("hexpod wake up button pressed");
        auto state_msg = std_msgs::Bool();
        state_msg.data = 1;
        state_pub.publish(state_msg);
    }
    else if(hexapod_wakeup_button == 1 && msg->buttons[0] == 0){
        ROS_INFO("hexapod wake up button depressed");
        auto state_msg = std_msgs::Bool();
        state_msg.data = 0;
        state_pub.publish(state_msg);
    }
    hexapod_wakeup_button = msg->buttons[0];


    auto cmd_vel_msg = geometry_msgs::Twist();

    cmd_vel_msg.linear.x = 0.07*msg->axes[1];
    cmd_vel_msg.angular.z = -0.1*msg->axes[0];
    cmd_vel_pub.publish(cmd_vel_msg);
}



int main(int argc, char** argv){
    ros::init(argc, argv, "hexapod_wii");
    ros::NodeHandle n;
    cmd_vel_pub = n.advertise<geometry_msgs::Twist>("/cmd_vel", 1000);
    state_pub = n.advertise<std_msgs::Bool>("/state", 1000);
    imu_override_pub = n.advertise<std_msgs::Bool>("/imu/imu_override", 1000);
    body_scalar_pub = n.advertise<geometry_msgs::AccelStamped>("/body_scalar", 1000);
    auto joy_subscriber = n.subscribe("/joy", 1000, joy_callback);
    auto nunchuk_subscriber = n.subscribe("/wiimote/nunchuk", 1000, nunchuk_callback);
    while(ros::ok()){
        ros::spin();
    }
}