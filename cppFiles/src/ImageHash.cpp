//
// Created by Tom Murphy on 2020-03-01.
//
#include "ImageHash.hpp"
#include "PerceptualHash.hpp"

ImageHash *computeHash(cv::Mat &imageData, enum HashType hashType) {
    if (HashType::PerceptualHashType == hashType) {
        return new PerceptualHash(imageData);
    }
    assert(false);
    return nullptr;
}

std::vector<bool> ImageHash::matHashToBoolArr(cv::Mat const inHash) {
    const unsigned char *data = inHash.data;
    std::vector<bool> v;
    for (int i = 0; i < 8; i++) {
        unsigned char c = data[i];
        for (int j = 0; j < 8; j++) {
            int shift = (8 - j) - 1;
            bool val = ((c >> shift) & 1);
            v.push_back(val);
        }
    }
    return v;
}

vector<bool> ImageHash::hex_str_to_hash(std::string inputString) {
    std::vector<bool> hash;
    int size = inputString.size() / 2;
    for (int i = 0; i < size; i++) {
        std::string str2 = inputString.substr(i * 2, 2);
        if (str2.empty()) {
            continue;
        }

        unsigned int value = 0;
        std::stringstream SS(str2);
        SS >> std::hex >> value;
        for (int j = 0; j < 8; j++) {
            bool check = !!((value >> j) & 1);
            hash.push_back(check);
        }
    }
    return hash;
}
