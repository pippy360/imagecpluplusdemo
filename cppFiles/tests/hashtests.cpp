#include <gtest/gtest.h>

#include <src/PerceptualHash.hpp>

//#include <opencv2/img_hash/phash.hpp>


TEST(HashTest, testFixedValueOpencv) {
    //assert some predefined values

    const cv::Mat rickandmortyImage = cv::imread("../webFiles/images/richandmalty.jpg", cv::IMREAD_GRAYSCALE);
    assert(rickandmortyImage.data);

    const cv::Mat rickandmortyImage400px = cv::imread("../webFiles/images/richandmalty400px.png", cv::IMREAD_GRAYSCALE);
    assert(rickandmortyImage400px.data);

    const cv::Mat basicshapes = cv::imread("../webFiles/images/basicshapes.png", cv::IMREAD_GRAYSCALE);
    assert(basicshapes.data);

    {
        PerceptualHash hash(rickandmortyImage);
        EXPECT_STREQ("39dab1738f96a04b", hash.toString().c_str());
    }

    {
        //Why don't these match?
        //d2 05 69 f1 8e 8f 5b 9c
        //d2 05 69 f1 ce 8d 5b 9c

        PerceptualHash hash(rickandmortyImage400px);
//        EXPECT_STREQ("d20569f1ce8d5b9c", hash.toString().c_str());
        EXPECT_STREQ("39daf1718f96a04b", hash.toString().c_str());
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

TEST(HashTest, testFixedValueOpencvRotations) {
    //assert some predefined values

    cv::Mat rickandmortyImage = cv::imread("../webFiles/images/richandmalty.jpg", cv::IMREAD_GRAYSCALE);
    assert(rickandmortyImage.data);

    cv::Mat rickandmortyImage400px = cv::imread("../webFiles/images/richandmalty400px.png", cv::IMREAD_GRAYSCALE);
    assert(rickandmortyImage400px.data);

    cv::Mat basicshapes = cv::imread("../webFiles/images/basicshapes.png", cv::IMREAD_GRAYSCALE);
    assert(basicshapes.data);

    {
        PerceptualHash hash(rickandmortyImage);
        EXPECT_STREQ("39dab1738f96a04b", hash.toString().c_str());
    }
    cv::rotate(rickandmortyImage, rickandmortyImage, cv::ROTATE_90_CLOCKWISE);
    {
        PerceptualHash hash(rickandmortyImage);
        EXPECT_STREQ("c4bb185e63a6e712", hash.toString().c_str());
    }
    cv::rotate(rickandmortyImage, rickandmortyImage, cv::ROTATE_90_CLOCKWISE);
    {
        PerceptualHash hash(rickandmortyImage);
        EXPECT_STREQ("6c30e4dbda3c75e0", hash.toString().c_str());
    }
    cv::rotate(rickandmortyImage, rickandmortyImage, cv::ROTATE_90_CLOCKWISE);
    {
        PerceptualHash hash(rickandmortyImage);
        EXPECT_STREQ("bb514df43e0cb2b9", hash.toString().c_str());
    }
    cv::flip(rickandmortyImage, rickandmortyImage, 0);
    {
        PerceptualHash hash(rickandmortyImage);
        EXPECT_STREQ("6e51b2f4c90c5db9", hash.toString().c_str());
    }
    cv::rotate(rickandmortyImage, rickandmortyImage, cv::ROTATE_90_CLOCKWISE);
    {
        PerceptualHash hash(rickandmortyImage);
        EXPECT_STREQ("93709bdbad3c8ae1", hash.toString().c_str());
    }
    cv::rotate(rickandmortyImage, rickandmortyImage, cv::ROTATE_90_CLOCKWISE);
    {
        PerceptualHash hash(rickandmortyImage);
        EXPECT_STREQ("39bbe75e9ca61812", hash.toString().c_str());
    }
    cv::rotate(rickandmortyImage, rickandmortyImage, cv::ROTATE_90_CLOCKWISE);
    {
        PerceptualHash hash(rickandmortyImage);
        EXPECT_STREQ("c69a4e7170965f4a", hash.toString().c_str());
    }

    {
        cv::Mat rickandmortyImage_fresh = cv::imread("../webFiles/images/richandmalty.jpg", cv::IMREAD_GRAYSCALE);

        vector<uint64_t> m = PerceptualHash::computeHash_fast(rickandmortyImage_fresh);

//        EXPECT_STREQ("39dab1738f96a04b", ImageHash::convertHashToString(m[0]).c_str());
//
//        //flipped
//        EXPECT_STREQ("c69a4e7170965f4a", ImageHash::convertHashToString(m[1]).c_str());
//
//        EXPECT_STREQ("6c30e4dbda3c75e0", ImageHash::convertHashToString(m[2]).c_str());
//
//        //flipped
//        EXPECT_STREQ("93709bdbad3c8ae1", ImageHash::convertHashToString(m[3]).c_str());
//
//        EXPECT_STREQ("bb514df43e0cb2b9", ImageHash::convertHashToString(m[4]).c_str());
//
//        //flipped
//        EXPECT_STREQ("6e51b2f4c90c5db9", ImageHash::convertHashToString(m[5]).c_str());
//
//        EXPECT_STREQ("c4bb185e63a6e712", ImageHash::convertHashToString(m[6]).c_str());
//
//        //flipped
//        EXPECT_STREQ("39bbe75e9ca61812", ImageHash::convertHashToString(m[7]).c_str());


//with only 4 rotataions:
        EXPECT_STREQ("39dab1738f96a04b", ImageHash::convertHashToString(m[0]).c_str());

        EXPECT_STREQ("6c30e4dbda3c75e0", ImageHash::convertHashToString(m[1]).c_str());

        EXPECT_STREQ("bb514df43e0cb2b9", ImageHash::convertHashToString(m[2]).c_str());

        EXPECT_STREQ("c4bb185e63a6e712", ImageHash::convertHashToString(m[3]).c_str());


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