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

uint64_t ImageHash::matHashToBoolArr(cv::Mat const inHash) {
    return *(inHash.ptr<uint64_t>(0));
}

uint64_t ImageHash::hex_str_to_hash(std::string inputString) {
    uint64_t hash;
    std::stringstream ss;
    ss << std::hex << inputString;
    ss >> hash;
    return hash;
}
