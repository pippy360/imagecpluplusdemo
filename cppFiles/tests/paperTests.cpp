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

TEST(papertest, basic) {
    const cv::Mat rickandmortyImage = cv::imread("../webFiles/images/download_3.png", cv::IMREAD_GRAYSCALE);
    assert(rickandmortyImage.data);
    cv::Matx33d m_in(0.7, 0.7, -100,
                     -0.7, 0.7, -2,
                     0.0, 0.0, 1.0);

    auto [img2, dyntrans] = handleImageForTransformation(rickandmortyImage, m_in);
    //show the imageThank
    auto res = compareImages(rickandmortyImage,
            img2,
           CANNY_THRESH,
           CANNY_RATIO,
           CANNY_KERNEL_SIZE,
           CANNY_BLUR_WIDTH,
           CANNY_AREA_THRESH,
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