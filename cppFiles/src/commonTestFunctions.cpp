//
// Created by Tom Murphy on 2020-04-10.
//

#include "commonTestFunctions.h"

#include "search.h"

double getPerctageOverlap(ring_t s1, ring_t s2, double s1_area) {
    double s2_area = bg::area(s2);
    vector<ring_t> output;
    bg::intersection(s1, s2, output);
    double area = 0;
    for (auto p : output)
        area += bg::area(p);

    return ((area / s1_area) + (area / s2_area)) / 2.0;
}

double getPerctageOverlap(ring_t s1, ring_t s2) {
    double s1_area = bg::area(s1);
    return getPerctageOverlap(s1, s2, s1_area);
}

vector<tuple<ring_t, vector<tuple<ring_t, double>>>> compareShapes(vector<ring_t> img1shape,
                                                                   vector<ring_t> img2shapes, Mat trans) {
    vector<tuple<ring_t, vector<tuple<ring_t, double>>>> res;

    Mat outRot;
    cv::invertAffineTransform(trans, outRot);
    auto invmat = convertCVMatrixToBoost(trans);

    for (auto _s : img1shape) {
        ring_t outPoly;
        boost::geometry::transform(_s, outPoly, invmat);

        double s_area = bg::area(outPoly);
        vector<tuple<ring_t, double>> cur;
        for (auto s2 : img2shapes) {
            double area_pc = getPerctageOverlap(outPoly, s2, s_area);
            //sort by best matching
            //TODO: ignore 0 overlap
            if (area_pc > 0.0000001) {
                cur.push_back(make_tuple(s2, area_pc));
            }
        }
        res.push_back(make_tuple(_s, cur));
    }

    return res;
}

vector<tuple<ring_t, vector<tuple<ring_t, double, int>>>> compareImages(Mat img_in, Mat img_in2, DrawingOptions d, Mat transmat)
{
    Mat grayImg1 = convertToGrey(img_in);
    auto shapes1 = extractShapes(d.thresh, d.ratio, d.kernel_size, d.blur_width, d.area_thresh, grayImg1);
    Mat grayImg2 = convertToGrey(img_in2);
    auto shapes2 = extractShapes(d.thresh, d.ratio, d.kernel_size, d.blur_width, d.area_thresh, grayImg2);
    //how many actually match and at what hash distance

    //compare the shapes with the transformation matrix
    auto comparedShapes = compareShapes(shapes1, shapes2, transmat);

    vector<tuple<ring_t, vector<tuple<ring_t, double, int>>>> res;
    for (auto c : comparedShapes) {
        vector<tuple<ring_t, double, int>> res_part;

        auto[s, list] = c;
        uint64_t hash1 = getHashesForShape_singleRotation(grayImg1, s, 0, d.hash_zoom);
        for (auto l : list) {
            //hm...we can do it for 360 degree rotations
            auto[s2, perc] = l;
            //getHashesForShape
            //vector<tuple<ring_t, uint64_t, int>>
            vector<uint64_t> hashes = getHashesForShape(grayImg2, s2, 360, 1, 32, 0, d.hash_zoom);
            int min_hash_distance = -1;
            for (uint64_t hash2 : hashes) {
                int hash_dist = ImageHash::bitCount(hash1 ^ hash2);
                if (min_hash_distance == -1 || hash_dist < min_hash_distance) {
                    min_hash_distance = hash_dist;
                }
            }
            res_part.push_back(make_tuple(s2, perc, min_hash_distance));
        }
        res.push_back(make_tuple(s, res_part));
    }

    return res;
}

//FIXME: test
tuple<pair<int, int>, map<string, int>> getMatchesForTransformation(
        ImageHashDatabase &database,
        Mat databaseImg,
        string databaseImgKey,
        Matx33f m33,
        DrawingOptions d)
{
    auto[queryImg, queryImgToDatabase_mat] = transfromImage_keepVisable(databaseImg, m33);

    tuple<pair<int, int>, map<string, int>> res;

    {
        auto validsAndInvalids = findDetailedSameImageMatches_prepopulatedDatabase(queryImg, database, databaseImgKey,
                                                                                   queryImgToDatabase_mat, d);
        pair<int, int> validAndInvalidsCount;
        {
            auto invalidsCount = 0;
            for (auto [queryShape, v1] : validsAndInvalids[0])
            {
                for (auto [databaseShapeStr, v2] : v1) {
                    auto [s1, s2, actualMatches] = v2;
                    invalidsCount += actualMatches.size();
                }
            }
            get<0>(validAndInvalidsCount) = invalidsCount;
        }

        {
            auto validsCount = 0;
            for (auto [queryShape, v1] : validsAndInvalids[1])
            {
                for (auto [databaseShapeStr, v2] : v1) {
                    //FIXME: do we want to seperate out same rotation matches??? we should plot these too

                    auto [s1, s2, actualMatches] = v2;
                    validsCount += actualMatches.size();
                }
            }
            get<1>(validAndInvalidsCount) = validsCount;
        }

        get<0>(res) = validAndInvalidsCount;
    }

    {
        auto detailedMatches = findDetailedMatches(database, queryImg, d);

        for (auto[imagePath, v] : detailedMatches)
        {
            auto count = 0;
            for (auto [queryShapeStr, v1] : v)
                for (auto [databaseShapeStr, actualMatches] : v1)
                        count += actualMatches.size();

            get<1>(res)[imagePath] = count;
        }
    }
    return res;
}

pt::ptree getMatchesForTransformation_json(
        ImageHashDatabase &database,
        map<string, int> &imageMismatches,
        Mat databaseImg,
        string databaseImgKey,
        Matx33f m33,
        DrawingOptions d)
{
    tuple<pair<int, int>, map<string, int>> matches = getMatchesForTransformation( database, databaseImg, databaseImgKey, m33, d);

    auto [iv, v] = matches;
    auto invalids = get<0>(iv);
    auto valids = get<1>(iv);
    int otherImageMismatches = 0;
    int matchesCount;
    for (auto[n, c] : v) {
        if (databaseImgKey.compare(n) == 0) {
            //FIXME: we haven't checked if these are actually valid matches
            matchesCount = c;
        } else {
            imageMismatches[n] += c;
            otherImageMismatches += c;
        }
    }

    pt::ptree ret;
    ret.add("sameImageMismatches", invalids);
    ret.add("otherImageMismatches", otherImageMismatches);
    ret.add("__ignore__", matchesCount);

    cout << matchesCount << " = "<< valids << " + " << invalids << endl;
    assert((invalids+valids) == matchesCount);
    return ret;
}

//FIXME: we haven't tested this yet
vector<vector<int>> getMatchesForTransformation_hashDistances(
        ImageHashDatabase &database,
        Mat databaseImg,
        string databaseImgKey,
        Matx33f m33,
        DrawingOptions d) {
    auto[queryImg, queryImgToDatabase_mat] = transfromImage_keepVisable(databaseImg, m33);

    vector<int> invalidsHashDistCount(64, 0);
    vector<int> validsHashDistCount(64, 0);

    {
        auto invalidsAndValids = findDetailedSameImageMatches_prepopulatedDatabase(queryImg,
                                                                                   database,
                                                                                   databaseImgKey,
                                                                                   queryImgToDatabase_mat,
                                                                                   d);
        map<string, map<string, tuple<ring_t, ring_t, vector<tuple<uint64_t, uint64_t, int, int>>> >> invalids = invalidsAndValids[0];

        //get the hash histances and add them to our count
        for (auto[k1, v1] : invalids) {
            for (auto[k2, v2] : v1) {
                auto[s1, s2, v] = v2;
                for (auto[h1, h2, rotation, hashDistance] : v) {
                    invalidsHashDistCount[hashDistance] += 1;
                }
            }
        }
    }

    vector<int> valids(64, 0);

    {
        map<string, map<string, map<string, vector< tuple<uint64_t, uint64_t, int, int> >>>> detailedMatches
                    = findDetailedMatches(database, queryImg, d);

        //add everything that isn't our key to the invalids
        for (auto [k, v] : detailedMatches)
        {
            if (databaseImgKey.compare(k) == 0)
                continue;

            for (auto [s1, m1] : v) {
                for (auto [s2, vt] : m1) {
                    for (auto [shape1, shape2, rot, dist] : vt) {
                        validsHashDistCount[dist] += 1;
                    }
                }
            }
        }
    }

    return {invalidsHashDistCount, validsHashDistCount};
}

pt::ptree getMatchesForTransformation_hashDistances_json(
        ImageHashDatabase &database,
        Mat databaseImg,
        string databaseImgKey,
        Matx33f m33,
        DrawingOptions d)
{
    pt::ptree res;
    vector<vector<int>> hashDist = getMatchesForTransformation_hashDistances(database, databaseImg, databaseImgKey, m33, d);

    auto hashDistInvalid = hashDist[0];
    pt::ptree invalids;
    for (int hl : hashDistInvalid)
    {
        pt::ptree arrayElement;
        arrayElement.put_value(hl);
        invalids.push_back(make_pair("", arrayElement));
    }
    res.add_child("invalids", invalids);

    auto hashDistValid = hashDist[1];
    pt::ptree valids;
    for (int hl : hashDistValid)
    {
        pt::ptree arrayElement;
        arrayElement.put_value(hl);
        valids.push_back(make_pair("", arrayElement));
    }
    res.add_child("valids", valids);

    return res;
}


//FIXME: test this with different image types
tuple<Mat, Mat> transfromImage_keepVisable(Mat img_in, cv::Matx33d transmat) {
    std::vector<Point2f> vec;
    std::vector<Point2f> outvec;

    Mat dynTransMat = covertToDynamicallyAllocatedMatrix(transmat);

    // points or a circle
    vec.push_back(Point2f(0, 0));
    vec.push_back(Point2f(0, img_in.rows));
    vec.push_back(Point2f(img_in.cols, 0));
    vec.push_back(Point2f(img_in.cols, img_in.rows));

    cv::transform(vec, outvec, dynTransMat);

    cv::Rect r = cv::boundingRect(outvec);

    if (r.x < 0 || r.y < 0) {
        double xmove = (r.x < 0) ? -r.x : 0;
        double ymove = (r.y < 0) ? -r.y : 0;

        cv::Matx33d m_in(1.0, 0.0, xmove,
                         0.0, 1.0, ymove,
                         0.0, 0.0, 1.0);

        dynTransMat = covertToDynamicallyAllocatedMatrix(m_in * transmat);
        cv::transform(vec, outvec, dynTransMat);
        r = cv::boundingRect(outvec);
    }

    //so we need to calculate the size of the output image? right?
    //FIXME: test this type stuff here...will this work with jpgs????
    Mat outputImage = Mat::zeros(r.y + r.height, r.x + r.width, img_in.type());
    warpAffine(img_in, outputImage, dynTransMat, outputImage.size());

    return make_tuple(outputImage, dynTransMat);
}