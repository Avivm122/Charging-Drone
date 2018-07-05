///////////////////////////////////////////////////////
// Coordinates for a specific pixel in an image //
///////////////////////////////////////////////////////

// First include the librealsense C++ header file
#include <librealsense/rs.hpp>
#include <cstdio>

// Also include GLFW to allow for graphical display
#define GLFW_INCLUDE_GLU
//#include <GLFW/glfw3.h>
#include <iostream>
#include <typeinfo>

rs::float3 CoordinatesCalculation(float scale, rs::intrinsics depth_intrin, const uint16_t* depth_image, int x, int y);

int main() try
{
    // Turn on logging. We can separately enable logging to console or to file, and use different severity filters for each.
    rs::log_to_console(rs::log_severity::warn);
    //rs::log_to_file(rs::log_severity::debug, "librealsense.log");

    // Create a context object. This object owns the handles to all connected realsense devices.
    rs::context ctx;
    printf("There are %d connected RealSense devices.\n", ctx.get_device_count());
    if(ctx.get_device_count() == 0) return EXIT_FAILURE;

    // This tutorial will access only a single device, but it is trivial to extend to multiple devices
    rs::device *dev = ctx.get_device(0);
    printf("\nUsing device 0, an %s\n", dev->get_name());
    printf("    Serial number: %s\n", dev->get_serial());
    printf("    Firmware version: %s\n", dev->get_firmware_version());

    // Configure depth and color to run with the device's preferred settings
    dev->enable_stream(rs::stream::depth, rs::preset::best_quality);
    //dev->enable_stream(rs::stream::color, rs::preset::best_quality);
    dev->start();




    bool CameraConnected = dev->is_streaming();

    while(CameraConnected)
    {

        // Wait for new frame data
        dev->wait_for_frames();


        // Retrieve our images
        const uint16_t *depth_image = (const uint16_t *) dev->get_frame_data(rs::stream::depth);

        // Retrieve camera parameters for mapping between depth and color
        rs::intrinsics depth_intrin = dev->get_stream_intrinsics(rs::stream::depth);
        float scale = dev->get_depth_scale();
        int x = 200;
        int y = 200;

        rs::float3 depth_coordinates = CoordinatesCalculation(scale, depth_intrin, depth_image, x, y);
        //std::cout << typeid(depth_coordinates.x).name() << std::endl;
        printf("pixel (%d,%d) have a coordinate of (%.2f,%.2f,%.2f)\n", x, y, depth_coordinates.x, depth_coordinates.y, depth_coordinates.z);
//        std::cout << "x: " << depth_coordinates.x << std::endl;
//        std::cout << "y: " << depth_coordinates.y << std::endl;
//        std::cout << "z: " << depth_coordinates.z << std::endl;

        CameraConnected = dev->is_streaming();
    }
}

catch(const rs::error & e)
{
    // Method calls against librealsense objects may throw exceptions of type rs::error
    printf("rs::error was thrown when calling %s(%s):\n", e.get_failed_function().c_str(), e.get_failed_args().c_str());
    printf("    %s\n", e.what());
    return EXIT_FAILURE;
}



rs::float3 CoordinatesCalculation(float scale, rs::intrinsics depth_intrin, const uint16_t* depth_image, int x, int y)
{
    // Retrieve the 16-bit depth value and map it into a depth in meters
    uint16_t depth_value = depth_image[y*depth_intrin.width + x];
    float depth_in_meters = depth_value * scale;

    if (depth_value == 0) return {0,0,0};

    // Map from pixel coordinates in the depth image to pixel coordinates in the color image
    rs::float2 depth_pixel = {(float)x, (float)y};
    rs::float3 depth_point = depth_intrin.deproject(depth_pixel, depth_in_meters);
    printf("pixel (%s,%s) have a coordinate of (%s,%s,%s)\n", x, y, depth_point.x, depth_point.y, depth_point.z);
    return depth_point;
}
