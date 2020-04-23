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

map<uint64_t, list<tuple<ring_t, uint64_t, int>>> g_database;

//FIXME: this shouldn't be a global variable
HammingWrapper *g_tree;
vector<uint64_t> g_hashes;

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
        g_hashes.push_back(hash1);
        tree->add_item(g_hashes.size(), &unpacked[0], nullptr);
    }

    cout << "Added " << imghashes.size() << " hashes to the search tree." << endl;
    return imghashes;
}

//FIXME: it's weird that img_in is the database image....this is probably a bug
//FIXME: this is messy and can be cleaned up
map<uint64_t, vector<tuple<ring_t, ring_t, uint64_t, uint64_t, int>>> findMatchesBetweenTwoImages(
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

        for (auto [s, h, rot] : g_imghashes)
        {
            if ( g_database.find(h) == g_database.end() )
                g_database[h] = list<tuple<ring_t, uint64_t, int>>();

            g_database[h].push_back(make_tuple(s, h, rot));
        }

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

    map<uint64_t, vector<tuple<ring_t, ring_t, uint64_t, uint64_t, int>>> m;
    for (auto [shape2, hash2, rotation2] : img2hashes)
    {
        vector<float> unpacked(64, 0);
        g_tree->_unpack(&hash2, &unpacked[0]);

        if ( m.find(hash2) == m.end() )
            m[hash2] = vector<tuple<ring_t, ring_t, uint64_t, uint64_t, int>>();

        vector<int64_t> result;
        vector<float> distances;
        //TODO: is 10 too much here? how does this affect performace?
        g_tree->get_nns_by_vector(&unpacked[0], 10, -1, &result, &distances);
        for (int i = 0; i < result.size(); i++) {
            if (distances[i] < MATCHING_HASH_DIST) {
                auto foundHash = g_hashes[ result[i] - 1 ];
                auto resList = g_database[ foundHash ];

                for (auto [shape1, hash1, rotation1] : resList) {
                    m[hash2].push_back(make_tuple(shape1, shape2, hash1, hash2, rotation1));
                }
            }
        }
    }

    return m;
}
