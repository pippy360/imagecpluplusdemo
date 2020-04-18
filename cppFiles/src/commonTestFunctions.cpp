//
// Created by Tom Murphy on 2020-04-10.
//

#include "annoymodule.cc"
#include "commonTestFunctions.h"


double getPerctageOverlap(ring_t s1, ring_t s2, double s1_area) {
    double s2_area = bg::area(s2);
    vector<ring_t> output;
    bg::intersection(s1, s2, output);
    double area = 0;
    for (auto p : output)
        area += bg::area(p);

    return ((area/s1_area) + (area/s2_area))/2.0;
}

double getPerctageOverlap(ring_t s1, ring_t s2) {
    double s1_area = bg::area(s1);
    return getPerctageOverlap(s1, s2, s1_area);
}

vector<tuple<ring_t, vector<tuple<ring_t, double>>>> compareShapes(vector<ring_t> img1shape,
        vector<ring_t> img2shapes, Mat trans)
{
    vector<tuple<ring_t, vector<tuple<ring_t, double>>>> res;

    Mat outRot;
    cv::invertAffineTransform(trans, outRot);
    auto invmat = convertInvMatrixToBoost(trans);

    for (auto _s : img1shape) {
        ring_t outPoly;
        boost::geometry::transform(_s, outPoly, invmat);

        double s_area = bg::area(outPoly);
        vector<tuple<ring_t, double>> cur;
        for (auto s2 : img2shapes) {
            double area_pc = getPerctageOverlap(outPoly, s2, s_area);
            //sort by best matching
            //TODO: ignore 0 overlap
            if (area_pc > 0.0000001) {
                cur.push_back(make_tuple(s2, area_pc));
            }
        }
        res.push_back(make_tuple(_s, cur));
    }

    return res;
}

extern vector<tuple<ring_t, uint64_t, int>> g_imghashes;
extern HammingWrapper *tree;

vector<tuple<ring_t, vector<tuple<ring_t, double>>>> compareImages(Mat img_in, Mat img_in2, int thresh,
                   int ratio,
                   int kernel_size,
                   int blur_width,
                   int areaThresh,
                   Mat transmat)
{
    Mat grayImg1 = convertToGrey(img_in);
    auto shapes1 = extractShapes(thresh, ratio, kernel_size, blur_width, areaThresh, grayImg1);
    Mat grayImg2 = convertToGrey(img_in2);
    auto shapes2 = extractShapes(thresh, ratio, kernel_size, blur_width, areaThresh, grayImg2);
    //how many actually match and at what hash distance

    //compare the shapes with the transformation matrix
    auto comparedShapes = compareShapes(shapes1, shapes2, transmat);
    return comparedShapes;
//    //find matches but check shapes
//    //now find the ones that match
//    //for each shape
//    for (auto &s1 : shapes1)
//    {
//        //so we basically do a find matches? and then check out the results?
//
//        //get all the matches and check if they're valid
//        //hm...if they have more than one match it won't be valid, that's a problem in testing
//        //ok confirm the match by checking overlap
//    }
//
//    //FIXME: make sure image order /img1/img2 is right
//    //FIXME: make sure image 1 is lookup image
//    //FIXME: we still haven't fixed collision issues......do that now
//
//    auto img2hashes = getHashesForMatching(
//            img_in, img_in2, thresh, ratio,
//            kernel_size, blur_width, areaThresh, true);
//
//    cout << "img2hashes size: " << img2hashes.size() << endl;
//
//    //This is valid matches
//
//    vector<tuple<ring_t, ring_t, uint64_t, uint64_t, int>> res;
//
//    for (auto h2 : img2hashes) {
//        vector<int32_t> result;
//        vector<float> distances;
//        auto [shape2, hash2, rotation2] = h2;
//        vector<float> unpacked(64, 0);
//        tree->_unpack(&hash2, &unpacked[0]);
//        tree->get_nns_by_vector(&unpacked[0], 6, -1, &result, &distances);
//        for (int i = 0; i < result.size(); i++) {
//            if (distances[i] < 8) {
//                auto [shape1, hash1, rotation1] = g_imghashes[result[i]];
//
//                res.push_back(std::tie(shape1, shape2, hash1, hash2, rotation1));
//            } else {
//
//            }
//        }
//    }
//    return res;
}

tuple<Mat, Mat> handleImageForTransformation(Mat img_in, cv::Matx33d transmat) {
    //apply the transformation and return the image

    Mat dynTransMat = covertToDynamicallyAllocatedMatrix(transmat);

    std::vector<Point2f> vec;
    std::vector<Point2f> outvec;
    // points or a circle
    vec.push_back(Point2f(0, 0));
    vec.push_back(Point2f(0, img_in.rows));
    vec.push_back(Point2f(img_in.cols, 0));
    vec.push_back(Point2f(img_in.cols, img_in.rows));

    cv::transform(vec, outvec, dynTransMat);

    cv::Rect r = cv::boundingRect(outvec);

    if (r.x < 0 || r.y < 0) {
        double xmove = (r.x < 0)? -r.x : 0;
        double ymove = (r.y < 0)? -r.y : 0;

        cv::Matx33d m_in(1.0, 0.0, xmove,
                         0.0, 1.0, ymove,
                         0.0, 0.0, 1.0);

        dynTransMat = covertToDynamicallyAllocatedMatrix(m_in * transmat);
        cv::transform(vec, outvec, dynTransMat);
        r = cv::boundingRect(outvec);
    }

    //so we need to calculate the size of the output image? right?
    Mat outputImage(r.y+r.height, r.x+r.width, CV_8UC4, Scalar(0, 0, 0, 0));
    warpAffine(img_in, outputImage, dynTransMat, outputImage.size());

    return make_tuple(outputImage, dynTransMat);
}