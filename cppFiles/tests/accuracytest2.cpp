#include <gtest/gtest.h>

#include <src/PerceptualHash.hpp>

#include <src/mainImageProcessingFunctions.hpp>

#define PI 3.14159265

int g_testmatches[] = {
        178,
        25,
        30,
        42,
        32,
        22,
        39,
        22,
        29,
        9

};

TEST(AccuracyTest, testMatches) {
    cv::Mat rickandmortyImage = cv::imread("../webFiles/images/richandmalty.jpg", cv::IMREAD_GRAYSCALE);
    assert(rickandmortyImage.data);

    //something........

    for (int i = 0; i < 10; i++) {
        cv::Mat r = rickandmortyImage.clone();

        double rotation = i;
        double cosval = cos( rotation*PI / 180.0 );
        double sinval = sin( rotation*PI / 180.0 );
        double transx = r.cols;
        double transy = r.rows;

        cv::Matx33d transpose_1(1.0, 0.0, -transx/2.0,
                                0.0, 1.0, -transy/2.0,
                                0.0, 0.0, 1.0);

        cv::Matx33d transpose_rot(cosval, -sinval, 0,
                                  sinval, cosval, 0,
                                  0.0, 0.0, 1.0);

        cv::Matx33d transpose_3(1.0, 0.0, transx/2.0,
                                0.0, 1.0, transy/2.0,
                                0.0, 0.0, 1.0);

        cv::Mat m = covertToDynamicallyAllocatedMatrix(transpose_3*transpose_rot*transpose_1);
        cv::warpAffine(rickandmortyImage, r, m, r.size());
//        cv::imshow("here", r);
//        cv::waitKey(0);
        //now hash it and see how many shapes we get
        //and in the next test check for matches
        //just force the number of shapes
        auto matches = findMatches(rickandmortyImage, r);
        std::cout << matches.size() << std::endl;
        EXPECT_EQ(g_testmatches[i], matches.size());
    }
}

TEST(AccuracyTest, DISABLED_testGetHashesForShape) {
    cv::Mat rickandmortyImage = cv::imread("../webFiles/images/richandmalty.jpg", cv::IMREAD_GRAYSCALE);
    assert(rickandmortyImage.data);

    int rotations=360,
            thresh=100,
            ratio=3,
            kernel_size=3,
            blur_width=3,
            areaThresh=200,
            output_width=32;

    Mat grayImg = convertToGrey(rickandmortyImage);
    vector<ring_t> shapes = extractShapes(thresh, ratio, kernel_size, blur_width, areaThresh, grayImg);

    auto shape = shapes[0];

    point_t p;
    bg::centroid(shape, p);
    auto [a, b] = getAandBWrapper(shape, p);
    double area = bg::area(shape);

    std::cout << "HERE...." << std::endl;

    {
        auto ret = vector<pair<ring_t, uint64_t >>();
//        handleForRotation(grayImg, shape, output_width, ret, p, a, b, area, 1);
        handleForRotation2(grayImg, shape, output_width, ret, p, a, b, area, 1);
        assert(ret.size() == 4);
        EXPECT_STREQ("ffea7faaaaa2b20b", ImageHash::convertHashToString(ret[0].second).c_str());
        EXPECT_STREQ("feea76a8fb80fd2b", ImageHash::convertHashToString(ret[1].second).c_str());
        EXPECT_STREQ("feea7ba8fba8fba1", ImageHash::convertHashToString(ret[2].second).c_str());
        EXPECT_STREQ("ffe8fba2fea8e8a1", ImageHash::convertHashToString(ret[3].second).c_str());
    }

    std::cout << "HERE...." << std::endl;

    {
        auto ret = vector<pair<ring_t, uint64_t >>();
        handleForRotation(grayImg, shape, output_width, ret, p, a, b, area, 1);
//        handleForRotation2(grayImg, shape, output_width, ret, p, a, b, area, 1);
        assert(ret.size() == 4);
        EXPECT_STREQ("ffea7faaaaa2b20b", ImageHash::convertHashToString(ret[0].second).c_str());
        EXPECT_STREQ("feea76a8fb80fd2b", ImageHash::convertHashToString(ret[1].second).c_str());
        EXPECT_STREQ("feea7ba8fba8fba1", ImageHash::convertHashToString(ret[2].second).c_str());
        EXPECT_STREQ("ffe8fba2fea8e8a1", ImageHash::convertHashToString(ret[3].second).c_str());
    }

    std::cout << "HERE...." << std::endl;

}

int rescounts[] = {
        23040,
        270,
        358,
        295,
        461,
        185,
        334,
        183,
        161,
        141,
        366,
        364,
        305,
        279,
        266,
        162,
        274,
        172,
        170,
        232,
        196,
        151,
        224,
        46,
        187,
        247,
        83,
        32,
        46,
        158,
        222,
        140,
        144,
        157,
        159,
        19,
        163,
        161,
        276,
        53,
        75,
        71,
        111,
        133,
        44,
        158,
        204,
        113,
        127,
        133,
        34,
        61,
        269,
        157,
        263,
        150,
        203,
        232,
        172,
        174,
        139,
        185,
        310,
        173,
        78,
        193,
        205,
        178,
        323,
        184,
        90,
        88,
        61,
        161,
        105,
        193,
        336,
        113,
        45,
        105,
        91,
        116,
        88,
        285,
        260,
        315,
        320,
        536,
        247,
        270,
        158,
        327,
        66,
        160,
        139,
        98,
        93,
        70,
        13,
        203,
        193,
        63,
        97,
        66,
        181,
        77,
        59,
        105,
        58,
        11,
        35,
        11,
        109,
        28,
        81,
        51,
        111,
        37,
        72,
        72,
        63,
        127,
        84,
        113,
        87,
        41,
        110,
        41,
        56,
        45,
        110,
        95,
        98,
        52,
        169,
        102,
        123,
        3,
        72,
        50,
        67,
        142,
        56,
        40,
        40,
        78,
        21,
        34,
        184,
        100,
        35,
        13,
        10,
        61,
        57,
        28,
        90,
        52,
        95,
        205,
        181,
        259,
        211,
        243,
        187,
        148,
        144,
        208,
        170,
        431,
        365,
        418,
        203,
        246,
        205,
        393,
        375,
        190,
        293,
        417,
        3142,
        240,
        275,
        304,
        368,
        146,
        338,
        207,
        159,
        120,
        263,
        345,
        278,
        220,
        243,
        93,
        373,
        191,
        104,
        239,
        162,
        177,
        229,
        40,
        271,
        132,
        99,
        117,
        69,
        161,
        119,
        123,
        122,
        211,
        169,
        45,
        135,
        196,
        297,
        57,
        97,
        13,
        152,
        149,
        37,
        183,
        142,
        84,
        19,
        160,
        169,
        69,
        245,
        326,
        317,
        208,
        215,
        146,
        164,
        234,
        116,
        221,
        317,
        239,
        89,
        211,
        250,
        259,
        267,
        282,
        268,
        89,
        89,
        127,
        73,
        240,
        361,
        282,
        51,
        156,
        129,
        168,
        91,
        272,
        354,
        366,
        204,
        335,
        188,
        253,
        99,
        240,
        193,
        108,
        74,
        62,
        92,
        13,
        12,
        173,
        196,
        67,
        109,
        68,
        202,
        119,
        54,
        87,
        46,
        31,
        19,
        47,
        69,
        26,
        40,
        56,
        21,
        56,
        47,
        22,
        63,
        54,
        54,
        149,
        57,
        71,
        123,
        90,
        46,
        96,
        75,
        66,
        26,
        73,
        51,
        37,
        34,
        63,
        103,
        14,
        35,
        68,
        72,
        95,
        53,
        106,
        11,
        36,
        93,
        64,
        67,
        136,
        98,
        127,
        39,
        55,
        101,
        40,
        225,
        272,
        102,
        275,
        209,
        325,
        307,
        173,
        201,
        287,
        297,
        386,
        469,
        415,
        324,
        286,
        255,
        542,
        338,
        251,
        450,
        473
};

TEST(AccuracyTest, testRotated) {

    cv::Mat rickandmortyImage = cv::imread("../webFiles/images/richandmalty.jpg", cv::IMREAD_GRAYSCALE);
    assert(rickandmortyImage.data);

    //something........

    for (int i = 0; i < 360; i++) {
        cv::Mat r = rickandmortyImage.clone();

        double rotation = i;
        double cosval = cos( rotation*PI / 180.0 );
        double sinval = sin( rotation*PI / 180.0 );
        double transx = r.cols;
        double transy = r.rows;

        cv::Matx33d transpose_1(1.0, 0.0, -transx/2.0,
                                0.0, 1.0, -transy/2.0,
                                0.0, 0.0, 1.0);

        cv::Matx33d transpose_rot(cosval, -sinval, 0,
                                  sinval, cosval, 0,
                                  0.0, 0.0, 1.0);

        cv::Matx33d transpose_3(1.0, 0.0, transx/2.0,
                                0.0, 1.0, transy/2.0,
                                0.0, 0.0, 1.0);

        cv::Mat m = covertToDynamicallyAllocatedMatrix(transpose_3*transpose_rot*transpose_1);
        cv::warpAffine(rickandmortyImage, r, m, r.size());
//        cv::imshow("here", r);
//        cv::waitKey(0);
        //now hash it and see how many shapes we get
        //and in the next test check for matches
        //just force the number of shapes

    }
}

TEST(AccuracyTest, DISABLED_testRotatedAndResults) {

    cv::Mat rickandmortyImage = cv::imread("../webFiles/images/richandmalty.jpg", cv::IMREAD_GRAYSCALE);
    assert(rickandmortyImage.data);

    vector<pair<ring_t, uint64_t>> res = getAllTheHashesForImage(rickandmortyImage);
    std::map<std::string,ring_t> mtmap;
    for (auto &r : res) {

        //FIXME: assert( map.find(r.second.toString()) == map.end() );
        mtmap[ImageHash::convertHashToString(r.second)] = r.first;
    }
    //something........

    for (int i = 0; i < 360; i++) {
        cv::Mat r = rickandmortyImage.clone();

        double rotation = i;
        double cosval = cos( rotation*PI / 180.0 );
        double sinval = sin( rotation*PI / 180.0 );
        double transx = r.cols;
        double transy = r.rows;

        cv::Matx33d transpose_1(1.0, 0.0, -transx/2.0,
                                0.0, 1.0, -transy/2.0,
                                0.0, 0.0, 1.0);

        cv::Matx33d transpose_rot(cosval, -sinval, 0,
                                  sinval, cosval, 0,
                                  0.0, 0.0, 1.0);

        cv::Matx33d transpose_3(1.0, 0.0, transx/2.0,
                                0.0, 1.0, transy/2.0,
                                0.0, 0.0, 1.0);

        cv::Mat m = covertToDynamicallyAllocatedMatrix(transpose_3*transpose_rot*transpose_1);
        cv::warpAffine(rickandmortyImage, r, m, r.size());

        vector<pair<ring_t, uint64_t>> res = getAllTheHashesForImage(r);
        int count = 0;
        for (auto &r : res) {
            if ( mtmap.count(ImageHash::convertHashToString(r.second)) > 0 ) {
//                std::cout << "found" << r.second.toString() << std::endl;
                count++;
            }
        }

        cout << "count: " << count << endl;

        EXPECT_EQ(count, rescounts[i]) << " for rotation: " << i;
//        cv::imshow("here", r);
//        cv::waitKey(0);
        //now hash it and see how many shapes we get
        //and in the next test check for matches
        //just force the number of shapes

    }
}

