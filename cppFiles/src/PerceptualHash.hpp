#ifndef perceptual_hash_h
#define perceptual_hash_h

#include <string>
#include <vector>
#include <memory>
#include <iomanip>

#include "opencv2/opencv.hpp"
#include "ImageHash.hpp"
#include "img_hash_opencv_module/phash.hpp"

//#include <opencv2/img_hash/average_hash.hpp>
//#include <opencv2/img_hash/block_mean_hash.hpp>
//#include <opencv2/img_hash/color_moment_hash.hpp>
//#include <opencv2/img_hash/radial_variance_hash.hpp>

using namespace std;

class PerceptualHash : public ImageHash {
private:
    static vector<uint64_t> computeHash(cv::Mat const input) {
        vector<uint64_t> res;
        res.push_back(cv::img_hash::PHash::compute(input));
        return res;
    }

public:

    static vector<uint64_t> computeHash_fast(cv::Mat const input) {
        vector<uint64_t> res;
        for (auto hash : cv::img_hash::PHash::compute_fast(input))
            res.push_back(hash);
        return res;
    }

    PerceptualHash(uint64_t hash)
    {
        m_hash = hash;
    }

    PerceptualHash(cv::Mat image_data)
    {
        //assert this here because to make sure we're not taking a performance hit by using a non-black and white image
        assert(image_data.type() == CV_8U);
        m_hash = computeHash(image_data)[0];
        //TODO: let's consider shortening the hash to "improve" the number of results
        //m_hash.erase(m_hash.begin()+32, m_hash.end()-1);
    }

    PerceptualHash(string getHashFromString)
    {
        m_hash = hex_str_to_hash(getHashFromString);
    }
};

//
//class PerceptualHashRotationHandler {
//private:
//    static vector<vector<bool > > computeHash(cv::Mat const input) {
//        cv::Mat inHash;
//        auto algo = cv::img_hash::ColorMomentHash::create();
//        algo->computeForEach90DegreeRotation(input, inHash);
//        return matHashToBoolArr(inHash);
//    }
//
//public:
//
//    PerceptualHashRotationHandler(cv::Mat image_data)
//    {
//        m_hash = computeHash(image_data);
//    }
//};
//

#endif // perceptual_hash_h


