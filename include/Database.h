#pragma once
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits.h>

namespace MajorProject
{
enum class LogType : int
{
    MP4,
    JPEG
};

struct BoundingBox
{
    size_t x_min;
    size_t x_max;
    size_t y_min;
    size_t y_max;

    float confidence;

    size_t label_id;
    std::string label;
};

class Database
{
public:
    Database( std::string& _data_directory )
        : data_directory( _data_directory )
    {
    }

    int LogDetection( LogType log_type,
                      std::vector< BoundingBox >& detections,
                      std::string& infile,
                      ssize_t frame_id = -1 );

    void SetDataDir( std::string& _data_directory )
    {
        data_directory = _data_directory;
    }

private:
    std::string data_directory;
};
}
