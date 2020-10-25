#include "calibration/CirclesGridDetector.h"
#include <chrono>
#include <fstream>
#include <boost/filesystem.hpp>
#include "calibration/Chessboard.h"

namespace calibration {

CirclesGridDetector::CirclesGridDetector(cv::Size patternSize, float gridSize, bool symmetric) :
    _patternSize(patternSize),
    _gridSize(gridSize)
{
    _flags = 0;
    if (symmetric) { _flags |= cv::CALIB_CB_SYMMETRIC_GRID; }

    for (int i = 0; i < patternSize.height; i++) {
        for (int j = 0; j < patternSize.width; j++) {
            _patternPoints.push_back(cv::Point3f(j * _gridSize, i * _gridSize, 0));
        }
    }
}

bool CirclesGridDetector::detect(const cv::Mat& image)
{
    cv::Mat gray;

    if (image.channels() != 1) {
        cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image;
    }

    auto start = std::chrono::high_resolution_clock::now();

#if CV_MAJOR_VERSION > 2
    cv::SimpleBlobDetector::Params params;
    params.filterByArea = false;
    cv::Ptr<cv::FeatureDetector> blobDetector = cv::SimpleBlobDetector::create(params);

    bool found = findCirclesGrid(gray, _patternSize, _imagePoints, _flags | cv::CALIB_CB_CLUSTERING, blobDetector);
#else
    cv::SimpleBlobDetector::Params params;
    params.filterByArea = false;
    cv::Ptr<cv::FeatureDetector> blobDetector = new cv::SimpleBlobDetector(params);

    bool found = findCirclesGrid(gray, _patternSize, _imagePoints, _flags | cv::CALIB_CB_CLUSTERING, blobDetector);
#endif

    auto end = std::chrono::high_resolution_clock::now();
    _detectionTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    if (_imagePoints.size() == _patternPoints.size()) {
        _objectPoints = _patternPoints;
    } else {
        _objectPoints = std::vector<cv::Point3f>();
    }

    return found;
}

void CirclesGridDetector::writeDetectionsToFile(const std::string& path)
{
    std::ofstream f;
    f.open(path, std::ios::out);
    f << "#CHESSBOARD" << std::endl;
    f << Chessboard::toString(_imagePoints);
    f.close();
}

void CirclesGridDetector::readDetectionsFromFile(const std::string& path)
{
    _imagePoints.clear();

    if (boost::filesystem::exists(boost::filesystem::path(path))) {
        _imagePoints = Chessboard::readPoint2fChessboardFromFile(path);
    }
}

} // namespace calibration
