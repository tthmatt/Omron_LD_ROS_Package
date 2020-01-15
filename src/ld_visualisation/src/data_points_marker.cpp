#include <ros/ros.h>
#include <ros/package.h>
#include <visualization_msgs/Marker.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

// Configurable values.
std::string PACK_NAME;
std::string MAP_NAME;
std::string HEAD_FRAME;
std::string VIS_TOPIC;
float POINTS_A_CLR;
float POINTS_R_CLR;
float POINTS_G_CLR;
float POINTS_B_CLR;
float LINES_A_CLR;
float LINES_R_CLR;
float LINES_G_CLR;
float LINES_B_CLR;

// Non configurable values.
const std::string MAP_FOLDER = "/map";
const std::string POINTS_NUM_H = "NumPoints";
const std::string LINES_NUM_H = "NumLines";
const std::string POINTS_H = "DATA";
const std::string LINES_H = "LINES";
const std::string POINTS_NS = "points";
const std::string LINES_NS = "lines";
const int32_t POINTS_M_ID = 0;
const int32_t LINES_M_ID = 1;
const double POINTS_X_SCALE = 0.045;
const double POINTS_Y_SCALE = 0.045;
const double LINES_X_SCALE = 0.045;

// Names for all config parameters.
const std::string PACK_NAME_PARAM = "pkg_name";
const std::string MAP_NAME_PARAM = "map_name";
const std::string HEAD_FRAME_PARAM = "head_frame";
const std::string VIS_TOPIC_PARAM = "vis_topic";
const std::string POINTS_A_CLR_PARAM = "points_a_colour";
const std::string POINTS_R_CLR_PARAM = "points_r_colour";
const std::string POINTS_G_CLR_PARAM = "points_g_colour";
const std::string POINTS_B_CLR_PARAM = "points_b_colour";
const std::string LINES_A_CLR_PARAM = "lines_a_colour";
const std::string LINES_R_CLR_PARAM = "lines_r_colour";
const std::string LINES_G_CLR_PARAM = "lines_g_colour";
const std::string LINES_B_CLR_PARAM = "lines_b_colour";

// Function prototypes
bool get_map_data(std::string filename, 
    std::vector<geometry_msgs::Point>& points, 
    std::vector<geometry_msgs::Point>& lines);

int main(int argc, char** argv)
{
    ros::init(argc, argv, "data_points_marker");
    ros::NodeHandle nh;
    ros::Rate rate(30);

    // Get all parameters.
    nh.param<std::string>(PACK_NAME_PARAM, PACK_NAME, "pkg");
    nh.param<std::string>(MAP_NAME_PARAM, MAP_NAME, "data.map");
    nh.param<std::string>(HEAD_FRAME_PARAM, HEAD_FRAME, "/pose");
    nh.param<std::string>(VIS_TOPIC_PARAM, VIS_TOPIC, "visualization_marker");
    nh.param<float>(POINTS_A_CLR_PARAM, POINTS_A_CLR, 1.0);
    nh.param<float>(POINTS_R_CLR_PARAM, POINTS_R_CLR, 0);
    nh.param<float>(POINTS_G_CLR_PARAM, POINTS_G_CLR, 0);
    nh.param<float>(POINTS_B_CLR_PARAM, POINTS_B_CLR, 0);
    nh.param<float>(LINES_A_CLR_PARAM, LINES_A_CLR, 1.0);
    nh.param<float>(LINES_R_CLR_PARAM, LINES_R_CLR, 0);
    nh.param<float>(LINES_G_CLR_PARAM, LINES_G_CLR, 0);
    nh.param<float>(LINES_B_CLR_PARAM, LINES_B_CLR, 0);

    ros::Publisher points_pub = nh.advertise<visualization_msgs::Marker>(VIS_TOPIC, 10);

    //// Begin drawing points and line on RVIZ ////

    std::vector<geometry_msgs::Point> points;
    std::vector<geometry_msgs::Point> lines;
    if (!get_map_data(MAP_NAME, points, lines)) ROS_ERROR("Reading map failed");
    ROS_INFO("%s: Waiting for RVIZ", ros::this_node::getName().c_str());

    // Check that someone is listening to this topic first.
    while (points_pub.getNumSubscribers() <= 0) {};
    ROS_INFO("%s: Publishing to RVIZ", ros::this_node::getName().c_str());
    // Configure the messages to publish.
    visualization_msgs::Marker points_arr;
    visualization_msgs::Marker lines_list;
    points_arr.header.frame_id = lines_list.header.frame_id = HEAD_FRAME;
    points_arr.header.stamp = lines_list.header.stamp = ros::Time::now();
    points_arr.ns = POINTS_NS;
    lines_list.ns = LINES_NS;
    points_arr.action = lines_list.action = visualization_msgs::Marker::ADD;
    points_arr.pose.orientation.w = lines_list.pose.orientation.w = 1.0;
    points_arr.id = POINTS_M_ID;
    lines_list.id = LINES_M_ID;
    points_arr.type = visualization_msgs::Marker::POINTS;
    lines_list.type = visualization_msgs::Marker::LINE_LIST;
    points_arr.scale.x = POINTS_X_SCALE;
    points_arr.scale.y = POINTS_Y_SCALE;
    lines_list.scale.x = LINES_X_SCALE;
    points_arr.color.a = POINTS_A_CLR;
    points_arr.color.r = POINTS_R_CLR;
    points_arr.color.g = POINTS_G_CLR;
    points_arr.color.b = POINTS_B_CLR;
    lines_list.color.a = LINES_A_CLR;
    lines_list.color.r = LINES_R_CLR;
    lines_list.color.g = LINES_G_CLR;
    lines_list.color.b = LINES_B_CLR;

    // Prepare all points.
    for (int i = 0; i < points.size(); i++)
    {
        points_arr.points.push_back(points[i]);
    }

    // Prepare all lines.
    for (int i = 0; i < lines.size(); i++)
    {
        lines_list.points.push_back(lines[i]);
    }
    
    
    while (ros::ok())
    {
        points_pub.publish(points_arr);
        points_pub.publish(lines_list);
        
        rate.sleep();
    }
    return 0;
}

/**
 * @brief Attemps to open .map file with the given file name and read all points and lines coordinates.
 * 
 * @param filename name of .map file to read, including file extension.
 * @param points Vector to store all read points.
 * @param lines Vector to store all read lines.
 * @return true if data is read without issue.
 * @return false if file fails to open or there is error in reading data.
 */
bool get_map_data(std::string filename, 
    std::vector<geometry_msgs::Point>& points,
    std::vector<geometry_msgs::Point>& lines)
{
    // Read map file
    std::string path = ros::package::getPath(PACK_NAME);
    path = path.append(MAP_FOLDER).append("/");
    std::string map_path = path.append(filename);
    std::ifstream map_file(map_path);
    if (map_file.fail()) return false;

    int num_of_points = 0;
    int num_of_lines = 0;
    std::string line;
    std::string ignore_head;
    while (std::getline(map_file, line))
    {
        // Retrieve the number of points.
        if (line.find(POINTS_NUM_H) != std::string::npos)
        {
            std::istringstream iss(line);
            iss >> ignore_head;
            iss >> num_of_points;
        }

        // Retrieve the number of lines.
        if (line.find(LINES_NUM_H) != std::string::npos)
        {
            std::istringstream iss(line);
            iss >> ignore_head;
            iss >> num_of_lines;
        }

        // We have seen the points data, collect them.
        if (line.find(POINTS_H) != std::string::npos)
        {
            int x, y;
            for (int i = 0; i < num_of_points; i++)
            {
                std::getline(map_file, line);
                std::istringstream iss(line);
                if (!(iss >> x >> y)) ROS_ERROR("Error reading points from file.");
                geometry_msgs::Point p;
                p.x = (double)x / 1000.0; // Values are in millimeter.
                p.y = (double)y / 1000.0;
                p.z = 0;
                points.push_back(p);
            }
        }

        // We have seen the lines data, collect them.
        if (line.find(LINES_H) != std::string::npos)
        {
            int x1, y1, x2, y2;
            for (int i = 0; i < num_of_lines; i++)
            {
                std::getline(map_file, line);
                std::istringstream iss(line);
                if(!(iss >> x1 >> y1 >> x2 >> y2)) ROS_ERROR("Error reading points from file.");
                geometry_msgs::Point p1;
                geometry_msgs::Point p2;
                p1.x = (double)x1 / 1000.0; // Values are in millimeter.
                p1.y = (double)y1 / 1000.0;
                p1.z = 0;
                p2.x = (double)x2 / 1000.0;
                p2.y = (double)y2 / 1000.0;
                p2.z = 0;
                // Needs two consecutive points to form a line.
                lines.push_back(p1);
                lines.push_back(p2);
            }
        }
    }
    return true;
}
