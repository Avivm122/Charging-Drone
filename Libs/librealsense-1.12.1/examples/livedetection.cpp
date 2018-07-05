///////////////////////////////////////////////////////
// Wire detection in a live video //
///////////////////////////////////////////////////////

#include <librealsense/rs.hpp>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


//#include <string>

using namespace cv;
using namespace std;

int main(){
    Scalar lowGray = Scalar(0, 0, 100);
    Scalar highGray = Scalar(255, 40, 130);

    // Turn on logging. We can separately enable logging to console or to file, and use different severity filters for each.
    rs::log_to_console(rs::log_severity::warn);
    //rs::log_to_file(rs::log_severity::debug, "librealsense.log");

    // Create a context object. This object owns the handles to all connected realsense devices.
    rs::context ctx;
    printf("There are %d connected RealSense devices.\n", ctx.get_device_count());
    if(ctx.get_device_count() == 0) return EXIT_FAILURE;

    // This tutorial will access only a single device, but it is trivial to extend to multiple devices
    rs::device * dev = ctx.get_device(0);
    printf("\nUsing device 0, an %s\n", dev->get_name());
    printf("    Serial number: %s\n", dev->get_serial());
    printf("    Firmware version: %s\n", dev->get_firmware_version());

    const double onehalfmeters = 1.5;

    // Configure depth and color to run with the device's preferred settings
    dev->enable_stream(rs::stream::depth, rs::preset::best_quality);
    dev->enable_stream(rs::stream::color, rs::preset::best_quality);
    dev->start();


    if (dev->is_streaming()){ //check if video device has been initialised
        cout << "cannot open camera";
    }
    //while the stream is enabled
    while (dev->is_streaming()){
        dev->wait_for_frames();
        const uint16_t * depth_image = (const uint16_t *)dev->get_frame_data(rs::stream::depth);
        const uint8_t * color_image = (const uint8_t *)dev->get_frame_data(rs::stream::color);

        // Retrieve camera parameters for mapping between depth and color
        rs::intrinsics depth_intrin = dev->get_stream_intrinsics(rs::stream::depth);
        rs::extrinsics depth_to_color = dev->get_extrinsics(rs::stream::depth, rs::stream::color);
        rs::intrinsics color_intrin = dev->get_stream_intrinsics(rs::stream::color);

        int FrameHeight = color_intrin.height;
        int FrameWidth = color_intrin.width;

        printf("Frame size %d x %d\n", FrameWidth, FrameHeight);


        Mat cameraFrame(color_intrin.height, color_intrin.width, CV_8UC3, (unsigned char*)color_image);

        cvtColor(cameraFrame, cameraFrame, COLOR_BGR2RGB);

        //printf("Frame size %d x %d\n", cameraFrame.rows, cameraFrame.cols);

        //cameraFrame = rotate(cameraFrame,90);
        //flip(cameraFrame, cameraFrame, 0);

        float scale = dev->get_depth_scale();

        for(int dy=0; dy<depth_intrin.height; ++dy)
        {
            for(int dx=0; dx<depth_intrin.width; ++dx)
            {
                //printf("(%d,%d)\n",dx,dy);
                // Retrieve the 16-bit depth value and map it into a depth in meters
                uint16_t depth_value = depth_image[dy * depth_intrin.width + dx];
                float depth_in_meters = depth_value * scale;

                // Skip over pixels with a depth value of zero, which is used to indicate no data
                if(depth_value == 0) continue;

                // Map from pixel coordinates in the depth image to pixel coordinates in the color image
                rs::float2 depth_pixel = {(float)dx, (float)dy};
                rs::float3 depth_point = depth_intrin.deproject(depth_pixel, depth_in_meters);
                double distance = sqrt(pow(depth_point.x,2)+pow(depth_point.y,2)+pow(depth_point.z,2));
                rs::float3 color_point = depth_to_color.transform(depth_point);
                rs::float2 color_pixel = color_intrin.project(color_point);

//                printf("Frame size %d x %d\n", FrameWidth, FrameHeight);
//                printf("(%d,%d) is (%f,%f)\n", dx, dy, color_pixel.x, color_pixel.y);

                // Use the color from the nearest color pixel, or pure white if this point falls outside the color image
                int cx = (int)std::round(color_pixel.x), cy = (int)std::round(color_pixel.y);
                if(cx < 0 || cy < 0 || cx >= color_intrin.width || cy >= color_intrin.height)
                {
                    if(cx < 0){
                        cx = 0;
                    }
                    else if(cy<0){
                        cy = 0;
                    }
                    else if(cx<0 && cy<0){
                        cx = 0;
                        cy = 0;
                    }
                    else if(cx >= color_intrin.width){
                        cx = color_intrin.width;
                    }
                    else if(cy>=color_intrin.height){
                        cy = color_intrin.height;
                    }
                    else if(cx>=color_intrin.width && cy>=color_intrin.height){
                        cx = color_intrin.width;
                        cy = color_intrin.height;
                    }
                    cameraFrame.at<Vec3b>(cy,cx) = Vec3b(255, 255, 255);
                    //printf("(%f,%f) is (%d,%d)\n", color_pixel.x, color_pixel.y, cx, cy);
                }
                else if (!(distance<=onehalfmeters))
                {
                    for(int i = -5;)
                    cameraFrame.at<Vec3b>(cy,cx) = Vec3b(0,0,0);
                    printf("for pixel (%d,%d) the coordinates are (%f,%f,%f) and %f\n", cx, cy, depth_point.x, depth_point.y, depth_point.z, distance);
                }

            }
        }

        //resize(cameraFrame, cameraFrame, Size(depth_intrin.height, depth_intrin.width));
        //flip(cameraFrame, cameraFrame, 0);
        //cameraFrame = rotate(cameraFrame, -90);
        Mat GrayImage;
        cvtColor(cameraFrame, GrayImage, COLOR_BGR2GRAY);
//        Mat InRangeImage;
//        inRange(HSVImage, lowGray, highGray, InRangeImage);
//        Mat ColorInRangeImage;
//        bitwise_and(cameraFrame, cameraFrame, ColorInRangeImage, InRangeImage);
//        Mat GrayInRangeImage;
//        cvtColor(ColorInRangeImage, GrayInRangeImage, COLOR_BGR2GRAY);
        Mat CannyImage;
        Canny(GrayImage, CannyImage, 500, 800, 3);

        vector<Vec4i> lines;
        HoughLinesP(CannyImage, lines, 1, CV_PI/180, 100, 100, 100);

        int NumberOfLines = lines.size();

        for(size_t i = 0; i < NumberOfLines; i++ ){
            line(cameraFrame, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8);
        }

        String status = to_string(NumberOfLines) + " lines";
        putText(cameraFrame, status, cvPoint(20, 30), FONT_HERSHEY_SIMPLEX, 0.5, {255, 0, 0}, 2);
        circle(cameraFrame,Size(0,0),100,Scalar(255,0,0));


        /*namedWindow("Canny", CV_WINDOW_AUTOSIZE);
        resize(CannyImage, CannyImage, Size(FrameHeight, FrameWidth));
        imshow("Canny", CannyImage);
        //cout << Size(cameraFrame.cols/2,cameraFrame.rows/2) <<endl;

        namedWindow("Threshold", CV_WINDOW_AUTOSIZE);
        resize(InRangeImage, InRangeImage, Size(FrameHeight, FrameWidth)); // resizes it so it fits on our screen
        imshow("Threshold", InRangeImage); // displays the source iamge*/

        /*namedWindow("Color In Range frame",CV_WINDOW_AUTOSIZE);
        resize(ColorInRangeImage, ColorInRangeImage, Size(FrameHeight, FrameWidth)); // resizes it so it fits on our screen
        imshow("Color In Range frame",ColorInRangeImage); // displays the source image*/

        namedWindow("Source Image",CV_WINDOW_AUTOSIZE);
        //resize(cameraFrame, cameraFrame, Size(FrameHeight, FrameWidth)); // resizes it so it fits on our screen
        imshow("Source Image",cameraFrame); // displays the source iamge

        cout<<"Next frame"<<endl;

        if (waitKey(30) >= 0)
            break;
    }
    return 0;
}
