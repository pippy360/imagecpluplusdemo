//
// Created by Tom Murphy on 2020-03-26.
//

#ifndef IMAGECPLUPLUSDEMO2_DEFAULTIMAGEVALUES_H
#define IMAGECPLUPLUSDEMO2_DEFAULTIMAGEVALUES_H

#define CANNY_THRESH 100
#define CANNY_RATIO 3
#define CANNY_KERNEL_SIZE 3
#define CANNY_BLUR_WIDTH 3
#define CANNY_AREA_THRESH 200

#define HASH_ZOOM .4

#define SMOOTH_CONTOURS_EPSILON 1
#define SMOOTH_CONTOURS_BOOL true

#define MATCHING_HASH_DIST 4

//FIXME: group all these default and comment each group
#define NUMBER_OF_IMAGE_RESIZES 1 //FIXME: comment all these in this file
#define PERCENTAGE_IMAGE_RESIZE .7

class DrawingOptions {
public:
    int thresh;
    int ratio;
    int kernel_size;
    int blur_width;
    int area_thresh;
    double hash_zoom;
    int second_rotation;
    int matchingHashDistance;
    bool replaceExtractShapesFunction;
    int fragment_rotations;
    double fragment_rotation_jump;
    double fragment_rotation_start;
    std::function<vector<ring_t> (int, int, int, int, int, cv::Mat &)> extractShapes;

    //constructor to default values
    DrawingOptions() :
            thresh(CANNY_THRESH),
            ratio(CANNY_RATIO),
            kernel_size(CANNY_KERNEL_SIZE),
            blur_width(CANNY_BLUR_WIDTH),
            area_thresh(CANNY_AREA_THRESH),
            hash_zoom(HASH_ZOOM),
            second_rotation(1),
            matchingHashDistance(MATCHING_HASH_DIST),
            replaceExtractShapesFunction(false),
            fragment_rotations(360),
            fragment_rotation_jump(1),
            fragment_rotation_start(0)
    {}
};


#endif //IMAGECPLUPLUSDEMO2_DEFAULTIMAGEVALUES_H
