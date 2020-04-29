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

//TODO: remove this

//TODO: comment
vector<string> g_images;
vector<tuple<ring_t, string, int>> g_shapesStrCache;

vector<tuple<int, uint64_t, double>> g_database;
vector<uint64_t> g_hashes;


//FIXME: this shouldn't be a global variable
HammingWrapper *g_tree;

void addImageToSearchTree(
        HammingWrapper *tree,
        Mat img_in,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        string imageName)
{
    int imageIdx = g_images.size();

    g_images.push_back(imageName);

    vector<tuple<ring_t, vector<uint64_t>>> imghashes = getAllTheHashesForImage(
            img_in,
            360,
            thresh,
            ratio,
            kernel_size,
            blur_width,
            areaThresh);

    for (auto [shape1, shape1hashes] : imghashes) {
        std::stringstream ss;
        ss << bg::wkt(shape1);
        g_shapesStrCache.push_back( make_tuple(shape1, ss.str(), imageIdx));
        cout << "doing shape : " << ss.str() << " - " << shape1hashes.size() << endl;
        for (int j = 0; j < shape1hashes.size(); j++)
        {
            uint64_t hash1 = shape1hashes[j];
            vector<float> unpacked(64, 0);
            tree->_unpack(&hash1, &unpacked[0]);

            g_database.push_back(make_tuple(g_shapesStrCache.size() - 1, hash1, j));
            tree->add_item(g_database.size(), &unpacked[0], nullptr);
        }
    }

    cout << "Added " << imghashes.size() << " hashes to the search tree." << endl;
}

//FIXME: it's weird that img_in is the database image....this is probably a bug
//FIXME: this is messy and can be cleaned up
map<string, map<string, vector< tuple<uint64_t, uint64_t, int> >>> findMatchesBetweenTwoImages(
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
        addImageToSearchTree(
                g_tree,
                img_in,
                thresh,
                ratio,
                kernel_size,
                blur_width,
                areaThresh,
                "None");

        g_tree->build(20, nullptr);
        cout << "...done" << endl;
    }

    auto queryImageHashes = getAllTheHashesForImage(
            img_in2,
            1,
            thresh,
            ratio,
            kernel_size,
            blur_width,
            areaThresh);

    map<string, map<string, vector< tuple<uint64_t, uint64_t, int> >>> m;
    for (auto [queryShape, shapehashes] : queryImageHashes)
    {
        std::stringstream ss;
        ss << bg::wkt(queryShape);
        cout << "shapehashes.size(): " << shapehashes.size() << " - " << ss.str() << endl;
        for (auto queryHash : shapehashes) {
            vector<float> unpacked(64, 0);
            g_tree->_unpack(&queryHash, &unpacked[0]);

            if ( m.find(ss.str()) == m.end() )
                m[ss.str()] = map<string, vector< tuple<uint64_t, uint64_t, int> >>();

            vector<int64_t> result;
            vector<float> distances;
            //TODO: is 40 too much here? how does this affect performace? 10 was too little when we have so many rotations
            g_tree->get_nns_by_vector(&unpacked[0], 40, -1, &result, &distances);
            for (int i = 0; i < result.size(); i++) {
                if (distances[i] < MATCHING_HASH_DIST) {
                    auto [shapeIdx, resHash, rotation] = g_database[result[i] - 1];
                    auto [resShape, resShapeStr, resImgIdx] = g_shapesStrCache[shapeIdx];
                    cout << "filling with hash: " << resHash << " and rotation: " << rotation << " shape: " << resShapeStr << endl;

                    if ( m.find(ss.str()) == m.end() )
                        m[ss.str()][resShapeStr] = vector< tuple<uint64_t, uint64_t, int> >();

                    m[ss.str()][resShapeStr].push_back(make_tuple(resHash, queryHash, rotation));
                }
            }
        }
    }

    return m;
}
