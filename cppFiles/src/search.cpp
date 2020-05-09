#include <opencv2/core.hpp>

#include "search.h"

#include "boostGeometryTypes.hpp"
#include "defaultImageValues.h"

#include "mainImageProcessingFunctions.hpp"

#include "commonTestFunctions.h"

using namespace std;
using namespace cv;

void addImageToSearchTree(
        ImageHashDatabase &database,
        string imageName,
        Mat img_in,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        double zoom)
{
    int imageIdx = database.imagePaths.size();

    database.imagePaths.push_back(imageName);

    vector<tuple<ring_t, vector<uint64_t>>> imghashes = getAllTheHashesForImage(
            img_in,
            360,
            thresh,
            ratio,
            kernel_size,
            blur_width,
            areaThresh,
            1,
            zoom);

    int count = 0;
    for (auto [shape1, shape1hashes] : imghashes) {
        std::stringstream ss;
        ss << bg::wkt(shape1);
        database.shapesStrCache.push_back(make_tuple(shape1, ss.str(), imageIdx));

        for (int j = 0; j < shape1hashes.size(); j++)
        {
            uint64_t hash1 = shape1hashes[j];
            vector<float> unpacked(64, 0);
            database.tree._unpack(&hash1, &unpacked[0]);

            database.database.push_back(make_tuple(database.shapesStrCache.size() - 1, hash1, j));
            database.tree.add_item(database.database.size(), &unpacked[0], nullptr);
            count++;
        }
    }

    cout << "Added " << count << " hashes to the search database " << endl;
}

string shapeToString(ring_t shape) {
    std::stringstream ss;
    ss << bg::wkt(shape);
    return ss.str();
}

//FIXME: we aren't checking if these are valid matches...fuck....
map<string, map<string, map<string, vector< tuple<uint64_t, uint64_t, int, int> >>>> findDetailedMatches(
        ImageHashDatabase &database,
        Mat queryImg,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        double zoom
        )
{
    auto queryImageHashes = getAllTheHashesForImage(
            queryImg,
            1,
            thresh,
            ratio,
            kernel_size,
            blur_width,
            areaThresh,
            1,
            zoom);

    map<string, map<string, map<string, vector< tuple<uint64_t, uint64_t, int, int> >>>> m;
    for (auto [queryShape, shapeHashes] : queryImageHashes)
    {
        string queryShape_str = shapeToString(queryShape);

        for (auto queryHash : shapeHashes)
        {
            /*
             * Search the database for matches
             */
            vector<int64_t> result;
            vector<float> distances;
            vector<float> unpacked(64, 0);

            //TODO: is 40 too much here? how does this affect performace? 10 was too little when we have so many rotations
            database.tree._unpack(&queryHash, &unpacked[0]);
            database.tree.get_nns_by_vector(&unpacked[0], 40, -1, &result, &distances);

            /*
             * Handle the matches
             */

            //Sort the results into our map
            for (int i = 0; i < result.size(); i++)
            {
                if (distances[i] < MATCHING_HASH_DIST)
                {
                    auto [shapeIdx, resHash, rotation] = database.database[result[i] - 1];
                    auto [resShape, resShapeStr, resImgIdx] = database.shapesStrCache[shapeIdx];
                    string imgPath = database.imagePaths[resImgIdx];

                    m[imgPath][queryShape_str][resShapeStr].push_back(make_tuple(resHash, queryHash, rotation, distances[i]));
                }
            }
        }
    }

    return m;
}


ImageHashDatabase *cachedDatabase;

//FIXME: it's weird that img_in is the database image....this is probably a bug
//FIXME: this is messy and can be cleaned up
//FIXME: split into find invalid  and valid same image matches
map<string, map<string, vector< tuple<uint64_t, uint64_t, int, int> >>> findMatchesBetweenTwoImages(
        Mat img_in,
        Mat img_in2,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        double zoom,
        bool flushCache
) {
    if (flushCache) {
        cout << zoom << endl;
        cout << "recomputing cache..." << endl;
        if (cachedDatabase != nullptr)
            delete cachedDatabase;

        cachedDatabase = new ImageHashDatabase();
        addImageToSearchTree(
                *cachedDatabase,
                "None",
                img_in,
                thresh,
                ratio,
                kernel_size,
                blur_width,
                areaThresh,
                zoom);

        cachedDatabase->tree.build(20, nullptr);
        cout << "...done" << endl;
    }

    auto resmap = findDetailedMatches(
            *cachedDatabase,
            img_in2,
            thresh,
            ratio,
            kernel_size,
            blur_width,
            areaThresh,
            zoom);

    if (resmap.size() == 0)
        return map<string, map<string, vector< tuple<uint64_t, uint64_t, int, int> >>>();

    return resmap.begin()->second;
}

vector<map<string, map<string, tuple<ring_t, ring_t, vector<tuple<uint64_t, uint64_t, int, int>>> >>> findDetailedSameImageMatches_prepopulatedDatabase(
        Mat queryImage,
        ImageHashDatabase &localDatabase,
        string databaseImgKey,
        Mat databaseToQuery_CVMat,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        double zoom)
{

    auto matches = findDetailedMatches(
            localDatabase,
            queryImage,
            thresh,
            ratio,
            kernel_size,
            blur_width,
            areaThresh,
            zoom);

    map<string, map<string, tuple<ring_t, ring_t, vector<tuple<uint64_t, uint64_t, int, int>>> >> invalids;
    map<string, map<string, tuple<ring_t, ring_t, vector<tuple<uint64_t, uint64_t, int, int>>> >> valids;
    auto databaseToQuery_boostMat = convertCVMatrixToBoost(databaseToQuery_CVMat);

    for (auto [queryImageShape_str, v] : matches[databaseImgKey])
    {
        ring_t queryImageShape;
        bg::read_wkt(queryImageShape_str, queryImageShape);

        for (auto [databaseShape_str, ml] : v)
        {
            for (auto [hash1, hash2, rotation, dist] : ml)
            {
                ring_t databaseShape;
                bg::read_wkt(databaseShape_str, databaseShape);
                ring_t outPoly;
                boost::geometry::transform(databaseShape, outPoly, databaseToQuery_boostMat);

                if (dist < MATCHING_HASH_DIST && getPerctageOverlap(outPoly, queryImageShape) < .90) {
                    //mismatch
                    get<2>(invalids[queryImageShape_str][databaseShape_str]).push_back(make_tuple(hash1, hash2, rotation, dist));

                } else if (dist < MATCHING_HASH_DIST){
                    //match
                    get<2>(valids[queryImageShape_str][databaseShape_str]).push_back(make_tuple(hash1, hash2, rotation, dist));
                }
            }
        }
    }

    return {invalids, valids};
}


vector<map<string, map<string, tuple<ring_t, ring_t, vector<tuple<uint64_t, uint64_t, int, int>>> >>> findDetailedSameImageMatches(
        Mat queryImage,
        Mat databaseImage,
        Mat databaseToQuery_CVMat,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        double zoom)
{
    //FIXME: allow passing in a preloaded database
    ImageHashDatabase localDatabase;

    addImageToSearchTree(
            localDatabase,
            "None",
            databaseImage,
            thresh,
            ratio,
            kernel_size,
            blur_width,
            areaThresh,
            zoom);

    localDatabase.tree.build(20, nullptr);

    return findDetailedSameImageMatches_prepopulatedDatabase(
            queryImage,
            localDatabase,
            "None",
            databaseToQuery_CVMat,
            thresh,
            ratio,
            kernel_size,
            blur_width,
            areaThresh,
            zoom);
}

