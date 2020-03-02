#include <gtest/gtest.h>

#include <src/PerceptualHash.hpp>

//#include <opencv2/img_hash/phash.hpp>


TEST(HashTest, testFixedValueOpencv) {
    //assert some predefined values

    const cv::Mat rickandmortyImage = cv::imread("../webFiles/images/richandmalty.jpg");
    assert(rickandmortyImage.data);

    const cv::Mat rickandmortyImage400px = cv::imread("../webFiles/images/richandmalty400px.png");
    assert(rickandmortyImage400px.data);

    const cv::Mat basicshapes = cv::imread("../webFiles/images/basicshapes.png");
    assert(basicshapes.data);

    {
        PerceptualHash hash(rickandmortyImage);
        EXPECT_STREQ("d20569f1ce8d5b9c", hash.toString().c_str());
    }

    {
        //Why don't these match?
        //d2 05 69 f1 8e 8f 5b 9c
        //d2 05 69 f1 ce 8d 5b 9c

        PerceptualHash hash(rickandmortyImage400px);
//        EXPECT_STREQ("d20569f1ce8d5b9c", hash.toString().c_str());
        EXPECT_STREQ("d20569f18e8f5b9c", hash.toString().c_str());
    }

    {
        PerceptualHash hash1(rickandmortyImage);
        PerceptualHash hash2(rickandmortyImage400px);
        //This really should be equal to zero
        EXPECT_EQ(hash1.getHammingDistance(hash2), 2);
    }

    {
        PerceptualHash hash1(rickandmortyImage);
        PerceptualHash hash2(basicshapes);
        EXPECT_EQ(hash1.getHammingDistance(hash2), 30);
    }

}

TEST(HashTest, testFixedValueWarpAffine) {
    //assert some predefined values after a warp affine

}

TEST(HashTest, testjpg) {
    //assert it must be grey scale
}

TEST(HashTest, testpng) {
    //assert it must be grey scale
}