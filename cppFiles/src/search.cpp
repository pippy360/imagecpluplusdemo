//
// Created by Tom Murphy on 2020-04-21.
//
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "search.h"

#include "annoylib.h"
#include "annoymodule.cc"

#include "boostGeometryTypes.hpp"
#include "defaultImageValues.h"

#include "mainImageProcessingFunctions.hpp"


using namespace std;
using namespace cv;

vector<tuple<ring_t, uint64_t, int>> g_imghashes;
//FIXME: this shouldn't be a global variable
HammingWrapper *g_tree;


vector<tuple<ring_t, uint64_t, int>> addImageToSearchTree(
        HammingWrapper *tree,
        Mat img_in,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh
) {
    auto imghashes = getAllTheHashesForImage(
            img_in,
            360,
            thresh,
            ratio,
            kernel_size,
            blur_width,
            areaThresh);

    int count = 0;
    for (auto h : imghashes) {
        auto[shape1, hash1, rotation1] = h;
        vector<float> unpacked(64, 0);
        tree->_unpack(&hash1, &unpacked[0]);
        tree->add_item(count++, &unpacked[0], nullptr);
    }

    cout << "Added " << imghashes.size() << " hashes to the search tree." << endl;
    return imghashes;
}

//FIXME: it's weird that img_in is the database image....this is probably a bug
vector<tuple<ring_t, ring_t, uint64_t, uint64_t, int>> findMatchesBetweenTwoImages(
        Mat img_in,
        Mat img_in2,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        bool flushCache
) {

    if (flushCache) {
        cout << "recomputing cache..." << endl;
        if (g_tree != nullptr)
            delete g_tree;

        g_tree = new HammingWrapper(64);
        g_imghashes = addImageToSearchTree(
                g_tree,
                img_in,
                thresh,
                ratio,
                kernel_size,
                blur_width,
                areaThresh);

        g_tree->build(20, nullptr);
        cout << "...done" << endl;
    }

    auto img2hashes = getAllTheHashesForImage(
            img_in2,
            1,
            thresh,
            ratio,
            kernel_size,
            blur_width,
            areaThresh);

    vector<tuple<ring_t, ring_t, uint64_t, uint64_t, int>> res;
    for (auto h2 : img2hashes)
    {
        vector<int32_t> result;
        vector<float> distances;
        auto[shape2, hash2, rotation2] = h2;
        vector<float> unpacked(64, 0);
        g_tree->_unpack(&hash2, &unpacked[0]);
        g_tree->get_nns_by_vector(&unpacked[0], 6, -1, &result, &distances);
        for (int i = 0; i < result.size(); i++) {
            if (distances[i] < MATCHING_HASH_DIST) {
                auto[shape1, hash1, rotation1] = g_imghashes[result[i]];
                res.push_back(std::tie(shape1, shape2, hash1, hash2, rotation1));
            }
        }
    }

    return res;
}
