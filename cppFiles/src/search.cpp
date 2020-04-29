#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "search.h"

#include "boostGeometryTypes.hpp"
#include "defaultImageValues.h"

#include "mainImageProcessingFunctions.hpp"


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
        int areaThresh)
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
            areaThresh);

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

map<string, map<string, map<string, vector< tuple<uint64_t, uint64_t, int> >>>> findDetailedMatches(
        ImageHashDatabase &database,
        Mat img_in2,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh
        )
{
    auto queryImageHashes = getAllTheHashesForImage(
            img_in2,
            1,
            thresh,
            ratio,
            kernel_size,
            blur_width,
            areaThresh);

    map<string, map<string, map<string, vector< tuple<uint64_t, uint64_t, int> >>>> m;
    for (auto [queryShape, shapehashes] : queryImageHashes)
    {
        std::stringstream ss;
        ss << bg::wkt(queryShape);

        for (auto queryHash : shapehashes)
        {
            vector<int64_t> result;
            vector<float> distances;
            vector<float> unpacked(64, 0);

            //TODO: is 40 too much here? how does this affect performace? 10 was too little when we have so many rotations
            database.tree._unpack(&queryHash, &unpacked[0]);
            database.tree.get_nns_by_vector(&unpacked[0], 40, -1, &result, &distances);

            //Sort the results into our map
            for (int i = 0; i < result.size(); i++)
            {
                if (distances[i] < MATCHING_HASH_DIST)
                {
                    auto [shapeIdx, resHash, rotation] = database.database[result[i] - 1];
                    auto [resShape, resShapeStr, resImgIdx] = database.shapesStrCache[shapeIdx];
                    string imgPath = database.imagePaths[resImgIdx];

                    if ( m.find(imgPath) == m.end() )
                        m[imgPath] = map<string, map<string, vector< tuple<uint64_t, uint64_t, int> >>>();

                    if ( m[imgPath].find(ss.str()) == m[imgPath].end() )
                        m[imgPath][ss.str()] = map<string, vector< tuple<uint64_t, uint64_t, int> >>();

                    if ( m[imgPath][ss.str()].find(resShapeStr) == m[imgPath][ss.str()].end() )
                        m[imgPath][ss.str()][resShapeStr] = vector< tuple<uint64_t, uint64_t, int> >();

                    m[imgPath][ss.str()][resShapeStr].push_back(make_tuple(resHash, queryHash, rotation));
                }
            }
        }
    }

    return m;
}


ImageHashDatabase *cachedDatabase;

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
                areaThresh);

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
            areaThresh
            );

    return resmap.begin()->second;
}
