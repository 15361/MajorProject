#pragma once
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

    int
    LogDetection( LogType log_type, std::vector< BoundingBox >& detections, std::string& infile, size_t frame_id = 0 )
    {
        return 0;
    }

    void SetDataDir( std::string& _data_directory )
    {
        data_directory = _data_directory;
    }

private:
    std::string data_directory;
};
}
