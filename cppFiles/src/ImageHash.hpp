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

    static std::string convertHashToString(vector<bool> hash) {
        std::string ret = "";
        int h = 0;
        for (unsigned int i = 0; i < hash.size(); i++) {
            if (hash[i]) {
                h += pow(2, (i % 8));
            }

            if (i % 8 == 7) {
                std::stringstream buffer;
                buffer << std::hex << std::setfill('0') << std::setw(2) << h;
                ret += buffer.str();
                h = 0;
            }
        }
        return ret;
    }

    //returns hamming distance
    static int getHashDistance(const ImageHash &first, const ImageHash &second) {
        const vector<bool> hash1 = first.m_hash;
        const vector<bool> hash2 = second.m_hash;
        assert(hash1.size() == hash2.size());

        int dist = 0;
        for (unsigned int i = 0; i < hash1.size(); i++) {
            dist += (hash1[i] != hash2[i]);
        }
        return dist;
    }
protected:

    vector<bool> m_hash;

    vector<bool> hex_str_to_hash(std::string inputString);

    static std::vector<bool> matHashToBoolArr(cv::Mat const inHash);

public:

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
