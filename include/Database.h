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

enum class ErrorType : int
{
    FATAL,
    WARNING,
    INFO
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
    Database( std::string _data_directory, std::string _error_file )
        : data_directory( _data_directory )
        , error_file( _error_file )
        , delim( "," )
    {
        // Default to cwd
        if( data_directory == "" )
        {
            data_directory = ".";
        }
    }

    int LogDetection( LogType log_type,
                      std::vector< BoundingBox >& detections,
                      std::string& infile,
                      std::string& outfile,
                      ssize_t frame_id = -1 );

    void LogError( std::string error_message, ErrorType error );

    void SetDataDir( std::string& _data_directory )
    {
        data_directory = _data_directory;
        if( data_directory == "" )
        {
            data_directory = ".";
        }
    }

    void SetErrorFile( std::string& _error_file )
    {
        error_file = _error_file;
    }

    void SetDelim( std::string& _delim )
    {
        delim = _delim;
    }

private:
    std::string GetErrorString( ErrorType error );
    std::string EscapeInfile( std::string& infile );

    std::string data_directory;
    std::string error_file;
    std::string delim;
};
}
