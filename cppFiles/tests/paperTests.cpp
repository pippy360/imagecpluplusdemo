//
// Created by Tom Murphy on 2020-04-13.
//
#include <functional>

#include <gtest/gtest.h>

#include <src/PerceptualHash.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <src/commonTestFunctions.h>

#include <src/defaultImageValues.h>

#include <src/search.h>

TEST(papertest, basic) {
    const cv::Mat rickandmortyImage = cv::imread("../webFiles/images/download_3.png", cv::IMREAD_GRAYSCALE);
    assert(rickandmortyImage.data);
    cv::Matx33d m_in(0.7, 0.7, -100,
                     -0.7, 0.7, -2,
                     0.0, 0.0, 1.0);

    //show the imageThank
    auto res = compareImageShapes(rickandmortyImage,
            DrawingOptions(),
            m_in, true);

    //ok so we know which shapes match
    cout << "res size: " << res.size() << endl;
}

TEST(papertest, pc_overlap) {

    ring_t ring1;
    bg::append(ring1, point_t(0.0, 0.0));
    bg::append(ring1, point_t(0.0, 5.0));
    bg::append(ring1, point_t(5.0, 5.0));
    bg::append(ring1, point_t(5.0, 0.0));

    ring_t ring2;
    bg::append(ring2, point_t(0.0, 0.0));
    bg::append(ring2, point_t(0.0, 1.0));
    bg::append(ring2, point_t(1.0, 1.0));
    bg::append(ring2, point_t(1.0, 0.0));

    double res1 = getPerctageOverlap(ring1, ring2);
    EXPECT_EQ(res1, 0.1);
    double res2 = getPerctageOverlap(ring2, ring2);
    EXPECT_EQ(res2, 0.1);
}

void loadDatabase(ImageHashDatabase &database, vector<string> files, DrawingOptions d)
{
    for (auto file : files)
    {
        const cv::Mat rickandmortyImage = cv::imread(file, cv::IMREAD_GRAYSCALE);
        addImageToSearchTree(database, file, rickandmortyImage, d);
    }

    database.tree.build(20, nullptr);
}

#define SIN(x) sin(x * 3.141592653589/180.0)
#define COS(x) cos(x * 3.141592653589/180.0)

pt::ptree searchWithRotation_hashDistance(ImageHashDatabase &database, string imagePath, double rotation,
                                           map<string, int> &imageMismatches, DrawingOptions d)
{
    const cv::Mat databaseImg = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    const cv::Mat databaseImg_clr = cv::imread(imagePath);

    auto m33 = Matx33f(COS(rotation), SIN(rotation), 0,
                       -SIN(rotation), COS(rotation), 0,
                       0, 0, 1);
    return getMatchesForTransformation_hashDistances_json(database, databaseImg, imagePath, m33, d);
}

pt::ptree searchWithRotation(ImageHashDatabase &database, string imagePath, double rotation,
        map<string, int> &imageMismatches, DrawingOptions d)
{
    const cv::Mat databaseImg = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    const cv::Mat databaseImg_clr = cv::imread(imagePath);

    auto m33 = Matx33f(COS(rotation), SIN(rotation), 0,
                       -SIN(rotation), COS(rotation), 0,
                       0, 0, 1);

    return getMatchesForTransformation_json(database, imageMismatches, databaseImg, imagePath, m33, d);
}

TEST(papertest, findDetailedMatches)
{
    ImageHashDatabase database;

    vector<string> images = {
            "../webFiles/images/download_3.png",
            "../webFiles/images/richandmalty.jpg",
            "../webFiles/images/tech.png",
            "../webFiles/images/forest.jpg",
            "../webFiles/images/font.jpg"
    };
    DrawingOptions d;

    loadDatabase(database, images, d);

    pt::ptree root;
    pt::ptree children[images.size()];

//#pragma omp parallel for
    for (int idx = 0; idx < images.size(); idx++)
    {
        auto imagePath = images[idx];

        map<string, int> imageMismatches;
        pt::ptree rotations;
        for (int i = 0; i < 360; i++)
        {
            cout << i << endl;
            auto ret = searchWithRotation(database, imagePath, i, imageMismatches, d);
            rotations.push_back(make_pair("", ret));
        }

        pt::ptree imgroot;
        imgroot.add_child("rotations", rotations);
        //now add the map for this image

        pt::ptree misMatches;
        for (auto [k, v] : imageMismatches)
        {
            misMatches.add(pt::ptree::path_type(k, '|'), v);
        }

        imgroot.add_child("rotations", rotations);
        imgroot.add_child("misMatches", misMatches);
        children[idx] = imgroot;
    }

    for (int i = 0; i < images.size(); i++) {
        root.add_child(pt::ptree::path_type(images[i], '|'), children[i]);
    }


    pt::write_json("output_all_images.json", root);
}



TEST(papertest, hashdistances)
{
    ImageHashDatabase database;

    vector<string> images = {
            "../webFiles/images/download_3.png",
            "../webFiles/images/richandmalty.jpg",
            "../webFiles/images/tech.png",
            "../webFiles/images/forest.jpg",
            "../webFiles/images/font.jpg"
    };

    DrawingOptions d;

    loadDatabase(database, images, d);

    pt::ptree root;
    pt::ptree children[images.size()];

#pragma omp parallel for
    for (int idx = 0; idx < images.size(); idx++)
    {
        auto imagePath = images[idx];

        map<string, int> imageMismatches;
        pt::ptree rotations;
        for (int i = 0; i < 360; i++)
        {
            cout << i << endl;
            auto ret = searchWithRotation_hashDistance(database, imagePath, i, imageMismatches, d);
            rotations.push_back(make_pair("", ret));
        }

        pt::ptree imgroot;
        imgroot.add_child("hashDistances", rotations);
        //now add the map for this image

        children[idx] = imgroot;
    }

    for (int i = 0; i < images.size(); i++) {
        root.add_child(pt::ptree::path_type(images[i], '|'), children[i]);
    }

    pt::write_json("output_all_images_hashDistances.json", root);
}


static class PerfectShapes
{
public:
    vector<ring_t> m_shapes;

    PerfectShapes(vector<ring_t> shapes) :
        m_shapes(shapes)
    {}

    vector<ring_t> operator()(int, int, int, int, int, cv::Mat &)
    {
        return m_shapes;
    }
};

TEST(papertest, perfectShapeExtraction)
{
    ImageHashDatabase database;

    vector<string> images = {
            "../webFiles/images/download_3.png",
            "../webFiles/images/richandmalty.jpg",
            "../webFiles/images/tech.png",
            "../webFiles/images/forest.jpg",
            "../webFiles/images/font.jpg"
    };

    DrawingOptions d_default;

    loadDatabase(database, images, d_default);

    pt::ptree root;
    pt::ptree children[images.size()];


//#pragma omp parallel for
    for (int idx = 0; idx < images.size(); idx++)
    {
        auto imagePath = images[idx];

        map<string, int> imageMismatches;
        pt::ptree rotations;
        for (int i = 0; i < 360; i++)
        {
            DrawingOptions d;

            //ONLY replace for query image!!
            vector<ring_t> outputShapes;

            {
                auto imagePath = images[idx];
                cv::Mat databaseImg = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);

                auto m33 = Matx33f(COS(i), SIN(i), 0,
                                   -SIN(i), COS(i), 0,
                                   0, 0, 1);
                auto[queryImg, queryImgToDatabase_mat] = transfromImage_keepVisable(databaseImg, m33);
                //calc the shapes
                auto resShapes = extractShapes(
                        d.thresh,
                        d.ratio,
                        d.kernel_size,
                        d.blur_width,
                        d.area_thresh,
                        databaseImg);

                auto boostMat = convertCVMatrixToBoost(queryImgToDatabase_mat);

                for (auto shape : resShapes)
                {
                    ring_t outPoly;
                    boost::geometry::transform(shape, outPoly, boostMat);
                    outputShapes.push_back(outPoly);
                }
            }

            PerfectShapes p(outputShapes);

            d.replaceExtractShapesFunction = true;
            d.extractShapes = std::function<vector<ring_t> (int, int, int, int, int, cv::Mat &)> (p);

            cout << i << endl;
            auto ret = searchWithRotation(database, imagePath, i, imageMismatches, d);
            rotations.push_back(make_pair("", ret));
        }

        pt::ptree imgroot;
        imgroot.add_child("rotations", rotations);
        //now add the map for this image

        pt::ptree misMatches;
        for (auto [k, v] : imageMismatches)
        {
            misMatches.add(pt::ptree::path_type(k, '|'), v);
        }

        imgroot.add_child("rotations", rotations);
        imgroot.add_child("misMatches", misMatches);
        children[idx] = imgroot;
    }

    for (int i = 0; i < images.size(); i++) {
        root.add_child(pt::ptree::path_type(images[i], '|'), children[i]);
    }


    pt::write_json("output_all_images_perfect.json", root);
}


TEST(papertest, secondRotation)
{
    ImageHashDatabase database;

    vector<string> images = {
            "../webFiles/images/download_3.png",
            "../webFiles/images/richandmalty.jpg",
            "../webFiles/images/tech.png",
            "../webFiles/images/forest.jpg",
            "../webFiles/images/font.jpg"
    };

    DrawingOptions d;
    d.fragment_rotations = 1;
    d.second_rotation = 360;

    loadDatabase(database, images, d);

    pt::ptree root;
    pt::ptree children[images.size()];


//#pragma omp parallel for
    for (int idx = 0; idx < images.size(); idx++)
    {
        auto imagePath = images[idx];

        map<string, int> imageMismatches;
        pt::ptree rot[360];

//#pragma omp parallel for
        for (int i = 346; i < 347; i++)
        {
            cout << i << endl;
            auto ret = searchWithRotation(database, imagePath, i, imageMismatches, d);
            rot[i] = ret;
        }

        pt::ptree rotations;
        for (int i = 0; i < 360; i++)
        {
            rotations.push_back(make_pair("", rot[i]));
        }

        pt::ptree imgroot;
        imgroot.add_child("rotations", rotations);
        //now add the map for this image

        pt::ptree misMatches;
        for (auto [k, v] : imageMismatches)
        {
            misMatches.add(pt::ptree::path_type(k, '|'), v);
        }

        imgroot.add_child("rotations", rotations);
        imgroot.add_child("misMatches", misMatches);
        children[idx] = imgroot;
    }

    for (int i = 0; i < images.size(); i++) {
        root.add_child(pt::ptree::path_type(images[i], '|'), children[i]);
    }


    pt::write_json("output_all_images_second_rotation.json", root);
}


