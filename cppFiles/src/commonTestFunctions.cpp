//
// Created by Tom Murphy on 2020-04-10.
//

#include "annoymodule.cc"
#include "commonTestFunctions.h"

#include "search.h"

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

vector<tuple<ring_t, vector<tuple<ring_t, double, int>>>> compareImages(Mat img_in, Mat img_in2, int thresh,
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

    vector<tuple<ring_t, vector<tuple<ring_t, double, int>>>> res;
    for (auto c : comparedShapes)
    {
        vector<tuple<ring_t, double, int>> res_part;

        auto [s, list] = c;
        uint64_t hash1= getHashesForShape_singleRotation(grayImg1, s, 0);
        for (auto l : list)
        {
            //hm...we can do it for 360 degree rotations
            auto [s2, perc] = l;
            //getHashesForShape
            //vector<tuple<ring_t, uint64_t, int>>
            vector<uint64_t> hashes = getHashesForShape(grayImg2, s2, 360, 1);
            int min_hash_distance = -1;
            for (uint64_t hash2 : hashes)
            {
                int hash_dist = ImageHash::bitCount(hash1 ^ hash2);
                if (min_hash_distance == -1 || hash_dist < min_hash_distance)
                {
                    min_hash_distance = hash_dist;
                }
            }
            res_part.push_back(make_tuple(s2, perc, min_hash_distance));
        }
        res.push_back(make_tuple(s, res_part));
    }

    return res;
}

vector<tuple<ring_t, ring_t, uint64_t, uint64_t, int, int>> findInvalidMatches(Mat img_in, Mat img_in2, Mat t)
{

    map<string, map<string, vector< tuple<uint64_t, uint64_t, int> >>> test = findMatchesBetweenTwoImages(img_in, img_in2);
    vector<tuple<ring_t, ring_t, uint64_t, uint64_t, int, int>> ret;
    auto invmat = convertInvMatrixToBoost(t);
    for (auto [s1_str, v] : test)
    {
        ring_t s1;
        bg::read_wkt(s1_str, s1);
        for (auto [s2_str, ml] : v) {
            for (auto [hash1, hash2, ignore] : ml) {
                ring_t s2;
                bg::read_wkt(s2_str, s2);
                //need to transform s2
                ring_t outPoly;
                boost::geometry::transform(s1, outPoly, invmat);

                int dist = ImageHash::bitCount(hash1 ^ hash2);
                if (dist < MATCHING_HASH_DIST && getPerctageOverlap(outPoly, s2) < .90) {
                    ret.push_back(make_tuple(s1, s2, hash1, hash2, 0, dist));
                }
            }
        }
    }
    return ret;
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