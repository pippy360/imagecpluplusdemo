#include <stdlib.h> // required for malloc definition
#include <opencv2/opencv.hpp>
#include <sstream>

#include <map>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "boostGeometryTypes.hpp"
#include "mainImageProcessingFunctions.hpp"

#include "shapeNormalise.hpp"
#include "search.h"

#include "commonTestFunctions.h"


using namespace cv;

class ValWrapper {
public:

    ValWrapper(size_t size, unsigned char *ptr)
            : ptr_(ptr),
              size_(size),
              val_(emscripten::typed_memory_view(size, ptr)) {
//        std::cout << "ptr created with size " << size_ << std::endl;

    }

    ~ValWrapper() {
//        std::cout << "ptr free with size " << size_ << std::endl;
        free(ptr_);
    }

    unsigned char *ptr_;
    size_t size_;
    emscripten::val val_;
};

class ValHolder {
public:
    ValHolder(size_t size)
            : edgeImage(size, (unsigned char *) malloc(size)),
              outputImage1(size, (unsigned char *) malloc(size)),
              outputImage2(size, (unsigned char *) malloc(size)),
              outputImage3(size, (unsigned char *) malloc(size)) {
    }


    string shapeStr;
    ValWrapper edgeImage;
    ValWrapper outputImage1;
    ValWrapper outputImage2;
    ValWrapper outputImage3;
};


static RNG rng(12345);

string calcMatrixFromString(string shapeStr, int output_width, double zoom, int rotation) {
    ring_t shape;
    bg::read_wkt(shapeStr, shape);
    Mat m = calcMatrix(shape, rotation, output_width, zoom);

    std::stringstream polygonString;
    polygonString << "{ \"mat\" : [";
    double *data = (double *) m.data;
    for (int i = 0; i < 2; i++) {
        if (i > 0)
            polygonString << ",";

        polygonString << "[";
        for (int j = 0; j < 3; j++) {
            if (j > 0)
                polygonString << ",";

            polygonString << data[(3 * i) + j];
        }
        polygonString << "]";
    }
    polygonString << ",[0,0,1]]}";
    return polygonString.str();
}

void getImageFragmentFromShape(uintptr_t img_in, int width, int height, ValHolder *valsOut, string shapeStr,
                               int output_width, double zoom, int rotation) {
    point_t p;
    ring_t transformedPoly;
    ring_t shape;

    bg::read_wkt(shapeStr, shape);

    Mat m = calcMatrix(shape, rotation, output_width, zoom);

    Mat outputImage(output_width, output_width, CV_8UC3, Scalar(0, 0, 0));
    Mat image(cv::Size(width, height), CV_8UC4, (void *) img_in, cv::Mat::AUTO_STEP);

    warpAffine(image, outputImage, m, outputImage.size());
    memcpy(valsOut->outputImage2.ptr_, outputImage.data, output_width * output_width * 4);
}

//FIXME: we aren't using the rotation here...why not?
std::string findMatchesForImageFromCanvas(
        uintptr_t img_in, int img_in_width, int img_in_height,
        uintptr_t img_in2, int img_in2_width, int img_in2_height,
        int rotation,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        double zoom,
        bool flush_cache
        )
{
    DrawingOptions d;
    d.thresh = thresh;
    d.ratio = ratio;
    d.kernel_size = kernel_size;
    d.blur_width = blur_width;
    d.area_thresh = areaThresh;

    Mat image(cv::Size(img_in_width, img_in_height), CV_8UC4, (void *) img_in, cv::Mat::AUTO_STEP);
    Mat image2(cv::Size(img_in2_width, img_in2_height), CV_8UC4, (void *) img_in2, cv::Mat::AUTO_STEP);

    //FIXME: the database is the wrong one here...
    auto vec = findMatchesBetweenTwoImages(
            image,
            image2,
            d,
            flush_cache
    );
    std::cout << "findMatchesForImageFromCanvas called, with this many matches: " << vec.size() << std::endl;

    std::stringstream polygonString;
    polygonString << "{ ";

    bool firstRun = true;
    for (auto [shapeStr, l] : vec)
    {
        //FIXME: we need to check that no two hashes are the same, otherwise we can create invalid json
        if (!firstRun) {
            polygonString << ",";
        }
        firstRun = false;

        polygonString << "\"" << shapeStr
                      << "\" : [";

        bool _firstRun2 = true;
        for (auto [matchesShape, hashMatches] : l ) {
            if (!_firstRun2) {
                polygonString << ",";
            }
            _firstRun2 = false;
            polygonString << "{\"" << matchesShape
                          << "\" : [";
            for (int j = 0; j < hashMatches.size(); j++) {
                if (j > 0) {
                    polygonString << ",";
                }

                auto [hash1, queryHash, rot, dist] = hashMatches[j];
                polygonString << "[\"" << ImageHash::bitCount(hash1 ^ queryHash) << "\", " << rot << "]";
            }
            polygonString << "]}";
        }
        polygonString << "]";
    }
    polygonString << "} ";
    return polygonString.str();
}

int getHashDistanceFromCanvas(
        string string1,
        double rotation,
        uintptr_t database_img_in,
        int db_width,
        int db_height,
        string string2,
        uintptr_t lookup_img_in,
        int lk_width,
        int lk_height,
        double zoom)
{
    ring_t shape_db;
    bg::read_wkt(string1, shape_db);

    ring_t shape_lk;
    bg::read_wkt(string2, shape_lk);

    Mat image_db(cv::Size(db_width, db_height), CV_8UC4, (void *) database_img_in, cv::Mat::AUTO_STEP);
    Mat image_lk(cv::Size(lk_width, lk_height), CV_8UC4, (void *) lookup_img_in, cv::Mat::AUTO_STEP);

    uint64_t hash1 = getHashesForShape_singleRotation(image_db, shape_db, rotation, zoom);
    uint64_t hash2 = getHashesForShape_singleRotation(image_lk, shape_lk, 0, zoom);
    return ImageHash::bitCount(hash1 ^ hash2);
}

class MatWraper {
    //FIXME: remember to call delete on this!!!
public:
    string outStr;
    unsigned char *m_data;
    emscripten::val v;
    int m_height, m_width;

    MatWraper(unsigned char *data, size_t size, int width, int height):
            m_width(width),
            m_height(height),
            m_data(data),
            v(emscripten::typed_memory_view(size, data))
    {};

    ~MatWraper() {
        free(m_data);
    }
};

class PerfectShapes
{
public:
    vector<ring_t> m_shapes;

    PerfectShapes(vector<ring_t> shapes) :
            m_shapes(shapes)
    {}

    vector<ring_t> operator()(int, int, int, int, int, cv::Mat &)
    {
        return m_shapes;
    }
};

MatWraper transfromImage_keepVisable_wrapper(
        uintptr_t img_in_ptr,
        double zoom,
        int width,
        int height,
        double a,
        double b,
        double c,
        double d,
        double e,
        double f,
        bool usePerfectShapes)
{
    Mat img_in(cv::Size(width, height), CV_8UC4, (void *) img_in_ptr, cv::Mat::AUTO_STEP);

    DrawingOptions draw;
    draw.hash_zoom = zoom;
    cv::Matx33d m_in(a, b, c,
                     d, e, f,
                 0.0, 0.0, 1.0);

    vector<tuple<ring_t, vector<tuple<ring_t, double, int>>>>  res = compareImageShapes(img_in,
                             draw,
                             m_in,
                             usePerfectShapes);

    std::stringstream polygonString;

    polygonString << "{ \"shapes\" : [";
    for (int i = 0; i < res.size(); i++) {
        auto [shape1, list] = res[i];
        if (i > 0)
            polygonString << ",";

        polygonString << "{" << " \"shape1\" : \"" << bg::wkt(shape1) << "\",";
        polygonString << "\"shapes\" : [";

        for (int j = 0; j < list.size(); j++) {
            auto [shape2, dist, hashdist] = list[j];
            if (j > 0)
                polygonString << ",";

            polygonString << "{\"shape\" : \"" << bg::wkt(shape2) << "\", \"dist\" : " << dist
                          << ", \"hashDist\" : " << hashdist <<" }";
        }
        polygonString << "]}";
    }

    polygonString << "], \"invalids\": [";

    ImageHashDatabase database;
    addImageToSearchTree(database, "None", img_in, draw);
    database.tree.build(20, nullptr);

//    auto json = getMatchesForTransformation(database, img_in, "None", m33, draw);

    auto [m, trans] = transfromImage_keepVisable(img_in, m_in);

    vector<ring_t> outputShapes;
    if (usePerfectShapes)
    {
        Mat grayImg1 = convertToGrey(img_in);
        auto resShapes = extractShapes(
                draw.thresh,
                draw.ratio,
                draw.kernel_size,
                draw.blur_width,
                draw.area_thresh,
                grayImg1);

        auto boostTrans = convertCVMatrixToBoost(trans);

        for (auto shape : resShapes)
        {
            ring_t outPoly;
            boost::geometry::transform(shape, outPoly, boostTrans);
            outputShapes.push_back(outPoly);
        }
        PerfectShapes p(resShapes);
        draw.replaceExtractShapesFunction = true;
        draw.extractShapes = std::function<vector<ring_t> (int, int, int, int, int, cv::Mat &)> (p);
    }

    map<string, int> imageMismatches;
    vector<map<string, map<string, tuple<ring_t, ring_t, vector<tuple<uint64_t, uint64_t, int, int>>> >>> validsAndInvalids;
    validsAndInvalids = findDetailedSameImageMatches_prepopulatedDatabase(
            m,
            database,
            "None",
            trans,
            draw);

    {
        bool firstRun = true;
        for (auto [queryShape, v1] : validsAndInvalids[0])
        {
            for (auto [database, v2] : v1)
            {
                auto [s1, s2, actualMatches ]  = v2;

                if (!firstRun) {
                    polygonString << ",";
                }
                firstRun = false;

                polygonString << "{\"shape1\" : \"" << queryShape << "\", \"shape2\" : \"" << database << "\", \"actualMatches\" : " << actualMatches.size() <<" }";
            }
        }
    }

    polygonString << "], \"valids\": [";

    {
        bool firstRun = true;
        for (auto [queryShape, v1] : validsAndInvalids[1])
        {
            for (auto [database, v2] : v1)
            {
                auto [s1, s2, actualMatches ]  = v2;

                if (!firstRun) {
                    polygonString << ",";
                }
                firstRun = false;

                polygonString << "{\"shape1\" : \"" << queryShape << "\", \"shape2\" : \"" << database << "\", \"actualMatches\" : " << actualMatches.size() <<" }";
            }

        }
    }
    polygonString << "]}";

    unsigned char *data = (unsigned char *) malloc(m.total() * m.elemSize());
    memcpy(data, m.data, m.total() * m.elemSize());
    MatWraper ret(data, m.total() * m.elemSize(), m.cols, m.rows);
    ret.outStr = polygonString.str();
    return ret;
}

string getShapeWithPointInside(
        uintptr_t img_in_ptr,
        int width,
        int height,
        double x,
        double y,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh)
{
    Mat img_in(cv::Size(width, height), CV_8UC4, (void *) img_in_ptr, cv::Mat::AUTO_STEP);
    Mat img = img_in.clone();

    Mat src_gray;
    cvtColor(img, src_gray, COLOR_BGRA2GRAY);//FIXME: detect and assert
    Mat canny_output = applyCanny(src_gray,
                                  thresh,
                                  ratio,
                                  kernel_size,
                                  blur_width);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContoursWrapper(canny_output, contours);

    vector<ring_t> shapes = extractShapesFromContours(contours, areaThresh);

    std::stringstream polygonString;
    for (auto &shape : shapes) {
        if (bg::within(point_t(x, y), shape))
            polygonString << bg::wkt(shape) << endl;
    }
    return polygonString.str();
}

string getContoursWithCurvature(
        uintptr_t img_in_ptr,
        int width,
        int height,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh)
{
    Mat img_in(cv::Size(width, height), CV_8UC4, (void *) img_in_ptr, cv::Mat::AUTO_STEP);
    Mat img = img_in.clone();
    Mat src_gray;
    cvtColor(img, src_gray, COLOR_BGRA2GRAY);//FIXME: detect and assert

    Mat _canny_output = applyCanny(src_gray,
                                   thresh,
                                   ratio,
                                   kernel_size,
                                   blur_width);
    Mat imageCannyOut;
    cvtColor(_canny_output, imageCannyOut, COLOR_GRAY2RGBA);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContoursWrapper(_canny_output, contours);

    std::stringstream polygonString;
    polygonString << "{ \"shapes\" : [";
    int count = 0;
    for (int i = 0; i < contours.size(); i++) {
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255), 255);
        linestring_t outPoly;
        if (!convert_to_boost_linestring(contours[i], outPoly)) {
            continue;
        }

//        if (bg::area(outPoly) <= areaThresh) {
//            continue;
//        }

        if (count != 0)
            polygonString << ",";

        count++;
        vector<string> vec = {"1.0","2.0"};//getMaximumPointsFromCurvature(outPoly);//FIXME:


        polygonString << "{ \"shape\" : \"" << bg::wkt(outPoly) << "\" , \"curves\" : [";
        for (int j = 0; j < vec.size(); j++) {
            if (j != 0)
                polygonString << ",";

            polygonString << vec[j];
        }
        polygonString << "] }";

        polygonString << endl;
    }
    polygonString << "] }";
    return polygonString.str();
}

//FIXME: rename now...
void encode(
        uintptr_t img_in_ptr,
        int width,
        int height,
        ValHolder *valsOut,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh)
{
    Mat img_in(cv::Size(width, height), CV_8UC4, (void *) img_in_ptr, cv::Mat::AUTO_STEP);
    Mat img = img_in.clone();

    Mat src_gray;
    cvtColor(img, src_gray, COLOR_BGRA2GRAY);//FIXME: detect and assert

    Mat _canny_output = applyCanny(src_gray,
                                   thresh,
                                   ratio,
                                   kernel_size,
                                   blur_width);

    Mat imageCannyOut;
    cvtColor(_canny_output, imageCannyOut, COLOR_GRAY2RGBA);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContoursWrapper(_canny_output, contours);

    //Draw contours------
    Mat contours_img = imageCannyOut.clone();
    for (int i = 0; i < contours.size(); i++) {
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255), 255);
        drawContours(contours_img, contours, i, color, 1, 8, hierarchy, 0, Point());
    }
    //--------

    //Draw hulls------
    Mat hulls_img = imageCannyOut.clone();
    for (int i = 0; i < contours.size(); i++) {
        vector<Point> hull;
        convexHull(contours[i], hull);
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255), 255);
        drawContours(hulls_img, vector<vector<Point> >(1, hull), -1, color, 2);
    }

    Mat valid_hulls_img = imageCannyOut.clone();
    int failed_area = 0;
    for (int i = 0; i < contours.size(); i++) {
        vector<Point> hull;
        convexHull(contours[i], hull);
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255), 255);
        ring_t outPoly;
        if (!convert_to_boost(hull, outPoly)) {
            continue;
        }

        if (bg::area(outPoly) <= areaThresh) {
            failed_area++;
            continue;
        }

        drawContours(valid_hulls_img, vector<vector<Point> >(1, hull), -1, color, 2);
    }
    cout << "Number of shapes which failed area: " << failed_area << endl;

    //--------

    cout << "contours.size(): " << contours.size() << endl;
    vector<ring_t> shapes = extractShapesFromContours(contours, areaThresh);
    cout << "shapes.size(): " << shapes.size() << endl;

    std::stringstream polygonString;
    for (auto &shape : shapes) {
        polygonString << bg::wkt(shape) << endl;
    }

    size_t size = width * height * 4;

    valsOut->shapeStr = polygonString.str();
    memcpy(valsOut->outputImage1.ptr_, contours_img.data, size);
    memcpy(valsOut->outputImage2.ptr_, hulls_img.data, size);
    memcpy(valsOut->outputImage3.ptr_, valid_hulls_img.data, size);

    memcpy(valsOut->edgeImage.ptr_, imageCannyOut.data, size);
}


int get_CANNY_THRESH() {
    return CANNY_THRESH;
}

int get_CANNY_RATIO() {
    return CANNY_RATIO;
}

int get_CANNY_KERNEL_SIZE() {
    return CANNY_KERNEL_SIZE;
}

int get_CANNY_BLUR_WIDTH() {
    return CANNY_BLUR_WIDTH;
}

int get_CANNY_AREA_THRESH() {
    return CANNY_AREA_THRESH;
}

double get_HASH_ZOOM() {
    return HASH_ZOOM;
}

using namespace emscripten;

EMSCRIPTEN_BINDINGS(my_value_example) {

        class_<ValWrapper>("ValWrapper")
                .property("val_", &ValWrapper::val_)
        ;

        class_<ValHolder>("ValHolder")
        .constructor<size_t>()
        .property("shapeStr", &ValHolder::shapeStr)
        .property("edgeImage", &ValHolder::edgeImage)
        .property("outputImage1", &ValHolder::outputImage1)
        .property("outputImage2", &ValHolder::outputImage2)
        .property("outputImage3", &ValHolder::outputImage3);

        emscripten::function("encode", &encode, allow_raw_pointers());
        emscripten::function("getShapeWithPointInside", &getShapeWithPointInside, allow_raw_pointers());
        emscripten::function("getImageFragmentFromShape", &getImageFragmentFromShape, allow_raw_pointers());
        emscripten::function("calcMatrixFromString", &calcMatrixFromString);
        emscripten::function("findMatchesForImageFromCanvas", &findMatchesForImageFromCanvas);
        emscripten::function("getHashDistanceFromCanvas", &getHashDistanceFromCanvas);

        emscripten::function("get_CANNY_THRESH", &get_CANNY_THRESH);
        emscripten::function("get_CANNY_RATIO", &get_CANNY_RATIO);
        emscripten::function("get_CANNY_KERNEL_SIZE", &get_CANNY_KERNEL_SIZE);
        emscripten::function("get_CANNY_BLUR_WIDTH", &get_CANNY_BLUR_WIDTH);
        emscripten::function("get_CANNY_AREA_THRESH", &get_CANNY_AREA_THRESH);

        emscripten::function("get_HASH_ZOOM", &get_HASH_ZOOM);

        emscripten::function("getContoursWithCurvature", &getContoursWithCurvature);

        class_<MatWraper>("MatWraper")
                .constructor<unsigned char *, size_t, int, int>()
                .property("outStr", &MatWraper::outStr)
                .property("v", &MatWraper::v)
                .property("width", &MatWraper::m_width)
                .property("height", &MatWraper::m_height)
        ;
        emscripten::function("transfromImage_keepVisable_wrapper", &transfromImage_keepVisable_wrapper);
}
