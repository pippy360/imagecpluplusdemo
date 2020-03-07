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

    static unsigned int bitCount(uint64_t value) {
        unsigned int count = 0;
        while (value > 0) {           // until all bits are zero
            if ((value & 1) == 1)     // check lower bit
                count++;
            value >>= 1;              // shift bits, removing lower bit
        }
        return count;
    }

    //returns hamming distance
    static int getHashDistance(const ImageHash &first, const ImageHash &second) {
        return bitCount(first.m_hash ^ second.m_hash);
    }
protected:

    uint64_t m_hash;

    uint64_t hex_str_to_hash(std::string inputString);

    static uint64_t matHashToBoolArr(cv::Mat const inHash);

public:

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
