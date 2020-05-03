//
// Created by Tom Murphy on 2020-04-13.
//
#include <gtest/gtest.h>

#include <src/PerceptualHash.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <src/commonTestFunctions.h>

#include <src/defaultImageValues.h>

#include <src/search.h>

TEST(papertest, basic) {
    const cv::Mat rickandmortyImage = cv::imread("../webFiles/images/download_3.png", cv::IMREAD_GRAYSCALE);
    assert(rickandmortyImage.data);
    cv::Matx33d m_in(0.7, 0.7, -100,
                     -0.7, 0.7, -2,
                     0.0, 0.0, 1.0);

    auto [img2, dyntrans] = transfromImage_keepVisable(rickandmortyImage, m_in);
    //show the imageThank
    auto res = compareImages(rickandmortyImage,
            img2,
           CANNY_THRESH,
           CANNY_RATIO,
           CANNY_KERNEL_SIZE,
           CANNY_BLUR_WIDTH,
           CANNY_AREA_THRESH,
           HASH_ZOOM,
           dyntrans);

    //ok so we know which shapes match
    cout << "res size: " << res.size() << endl;
}

TEST(papertest, pc_overlap) {

    ring_t ring1;
    bg::append(ring1, point_t(0.0, 0.0));
    bg::append(ring1, point_t(0.0, 5.0));
    bg::append(ring1, point_t(5.0, 5.0));
    bg::append(ring1, point_t(5.0, 0.0));

    ring_t ring2;
    bg::append(ring2, point_t(0.0, 0.0));
    bg::append(ring2, point_t(0.0, 1.0));
    bg::append(ring2, point_t(1.0, 1.0));
    bg::append(ring2, point_t(1.0, 0.0));

    double res1 = getPerctageOverlap(ring1, ring2);
    EXPECT_EQ(res1, 0.1);
    double res2 = getPerctageOverlap(ring2, ring2);
    EXPECT_EQ(res2, 0.1);
}

void loadDatabase(ImageHashDatabase &database, vector<string> files)
{
    for (auto file : files)
    {
        const cv::Mat rickandmortyImage = cv::imread(file, cv::IMREAD_GRAYSCALE);
        addImageToSearchTree(database, file, rickandmortyImage);
    }

    database.tree.build(20, nullptr);
}

#define SIN(x) sin(x * 3.141592653589/180.0)
#define COS(x) cos(x * 3.141592653589/180.0)

tuple<int, map<string, int>> searchWithRotation(ImageHashDatabase &database, string imagePath, double rotation)
{
    const cv::Mat databaseImg = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    const cv::Mat databaseImg_clr = cv::imread(imagePath);

    auto m33 = Matx33f(COS(rotation), SIN(rotation), 0,
                       -SIN(rotation), COS(rotation), 0,
                       0, 0, 1);


    auto [queryImg, queryImgToDatabase_mat] = transfromImage_keepVisable(databaseImg, m33);
    auto [queryImg_clr, ignore] = transfromImage_keepVisable(databaseImg_clr, m33);

    tuple<int, map<string, int>> res;

    {
        map<string, map<string, tuple<ring_t, ring_t, vector<tuple<uint64_t, uint64_t, int, int>>> >> invalids =
                findInvalidMatches(queryImg, databaseImg, queryImgToDatabase_mat);

//        for (auto [queryImgStr, v] : invalids)
//        {
//            for (auto [databaseImgStr, t] : v)
//            {
//                auto [r1, r2, v] = t;
//                drawContoursWithRing(queryImg_clr, vector<ring_t>(1, r1));
//                drawContoursWithRing(databaseImg_clr, vector<ring_t>(1, r2));
//            }
//        }
//
//        int rows = (queryImg_clr.rows > databaseImg_clr.rows)? queryImg_clr.rows : databaseImg_clr.rows;
//        Mat dst = Mat::zeros(rows, queryImg_clr.cols + databaseImg_clr.cols, queryImg_clr.type());
//        databaseImg_clr.copyTo(dst(Rect(0, 0, databaseImg_clr.cols, databaseImg_clr.rows)));
//        queryImg_clr.copyTo(dst(Rect(databaseImg_clr.cols, 0, queryImg_clr.cols, queryImg_clr.rows)));
//
//        {
//            //concat the images and save...
//            for (auto [queryImgStr, v] : invalids)
//            {
//                for (auto [databaseImgStr, t] : v)
//                {
//                    auto [r1, r2, v] = t;
//                    point_t p1;
//                    bg::centroid(r1, p1);
//                    point_t p2;
//                    bg::centroid(r2, p2);
//                    Scalar color( rand()&255, rand()&255, rand()&255 );
//                    line(dst, Point(p1.get<0>(), p1.get<1>()), Point(databaseImg_clr.cols+p2.get<0>(), p2.get<1>()), color, 4);
//                }
//            }
//            imshow("databaseImg_clr", databaseImg_clr);
//            imshow("queryImg_clr", queryImg_clr);
//            imshow("dst", dst);
//            waitKey(0);
//        }

        get<0>(res) = invalids.size();
    }


    {
        auto detailedMatches = findDetailedMatches(database, queryImg);

        for (auto [k, v] : detailedMatches) {
            get<1>(res)[k] = v.size();
        }
    }
    return res;
}

TEST(papertest, findDetailedMatches)
{
    ImageHashDatabase database;

    loadDatabase(database, {
            "../webFiles/images/download_3.png",
            "../webFiles/images/richandmalty.jpg",
            "../webFiles/images/tech.png"
    });

    searchWithRotation(database, "../webFiles/images/richandmalty.jpg", 90);

    for (int i = 0; i < 360; i++)
    {
        auto ret = searchWithRotation(database, "../webFiles/images/richandmalty.jpg", i);

        cout << endl << "Results:" << endl << endl;
        //FIXME: check if the image name is wrong and print as invalid
        for (auto [k, v] : get<1>(ret)) {
//            if (k.compare("../webFiles/images/richandmalty.jpg") != 0)
            cout << k << " : " << v << endl;
        }
        cout << "Invalid matches in the same image: " << get<0>(ret) << endl << endl;
    }
}


