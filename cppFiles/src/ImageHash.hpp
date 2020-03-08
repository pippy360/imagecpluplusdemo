#ifndef image_hash_h
#define image_hash_h

#include <string>
#include <vector>
#include <memory>

#include <opencv2/opencv.hpp>

using namespace std;


class ImageHash
{
private:


protected:


    uint64_t hex_str_to_hash(std::string inputString);

    static uint64_t matHashToBoolArr(cv::Mat const inHash);

public:
    uint64_t m_hash;


    static unsigned int bitCount(uint64_t i) {
        i = i - ((i >> 1) & 0x5555555555555555);
        i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333);
        return (((i + (i >> 4)) & 0xF0F0F0F0F0F0F0F) * 0x101010101010101) >> 56;
    }

    //returns hamming distance
    static int getHashDistance(const ImageHash &first, const ImageHash &second) {
        return bitCount(first.m_hash ^ second.m_hash);
    }

    static std::string convertHashToString(uint64_t hash) {
        std::stringstream stream;
        stream << std::hex << hash;
        return stream.str();
    }

    string toString() {
        return convertHashToString(m_hash);
    }

    int getHammingDistance(const ImageHash& inHash) {
        return getHashDistance(*this, inHash);
    }

    friend std::ostream& operator<<(std::ostream &os, const ImageHash &in) {
        return os << convertHashToString(in.m_hash);
    }
};


enum HashType {
    PerceptualHashType,
};

ImageHash *computeHash(cv::Mat &imageData, enum HashType hashType);


#endif // image_hash_h
