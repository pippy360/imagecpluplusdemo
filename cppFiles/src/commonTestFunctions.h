//
// Created by Tom Murphy on 2020-04-10.
//

#ifndef IMAGECPLUPLUSDEMO2_COMMONTESTFUNCTIONS_H
#define IMAGECPLUPLUSDEMO2_COMMONTESTFUNCTIONS_H

#include "annoylib.h"


#include <vector>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <math.h>




#include "ImageHash.hpp"
#include "boostGeometryTypes.hpp"
#include "miscUtils.hpp"
#include "shapeNormalise.hpp"
#include "PerceptualHash.hpp"
#include "mainImageProcessingFunctions.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include <algorithm>
#include <chrono>
#include <iostream>


tuple<Mat, Mat> handleImageForTransformation(Mat img_in, cv::Matx33d transmat);

vector<tuple<ring_t, vector<tuple<ring_t, double, int>>>> compareImages(Mat img_in, Mat img_in2, int thresh,
                   int ratio,
                   int kernel_size,
                   int blur_width,
                   int areaThresh,
                   Mat transmat);

double getPerctageOverlap(ring_t s1, ring_t s2);

#endif //IMAGECPLUPLUSDEMO2_COMMONTESTFUNCTIONS_H
