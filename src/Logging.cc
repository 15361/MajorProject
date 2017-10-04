#include "Logging.h"
namespace MajorProject
{
std::string Logger::GetErrorString( ErrorType error )
{
    switch( error )
    {
        case ErrorType::FATAL:
            return "FATAL: ";
        case ErrorType::WARNING:
            return "WARNING: ";
        case ErrorType::INFO:
            return "INFO: ";
        default:
            return "";
    }
}

std::string Logger::EscapeInfile( std::string& infile )
{
    std::string outfile;

    std::string full_path;
    char full_path_buf[ PATH_MAX ];
    if( realpath( infile.c_str(), full_path_buf ) == NULL )
    {
        this->LogError( "Failed to get full path. Using filename instead", ErrorType::WARNING );
        full_path = ( infile.substr( infile.rfind( "/" ) ) ).c_str();
    }
    else
    {
        full_path = std::string( full_path_buf );
    }

    if( full_path[ 0 ] == '/' )
    {
        full_path.erase( 0, 1 );
    }

    for( const auto& character : full_path )
    {
        outfile += ( character == '/' || character == '.' ) ? '_' : character;
    }

    return outfile;
}

int Logger::LogDetection(
LogType log_type, std::vector< BoundingBox >& detections, std::string& infile, std::string& outfile, ssize_t frame_id )
{
    std::ofstream writer;
    if( outfile == "" )
    {
        writer.open( data_directory + "/" + EscapeInfile( infile ) + ".txt", std::ios_base::out | std::ios_base::app );
    }
    else
    {
        writer.open( data_directory + "/" + outfile, std::ios_base::out | std::ios_base::app );
    }

    if( !writer.good() )
    {
        this->LogError( "Failed to open writer", ErrorType::FATAL );
        writer.close();
        return -1;
    }

    writer << infile << std::endl;
    writer << std::to_string( detections.size() ) << delim << std::to_string( frame_id ) << std::endl;

    for( const auto& detection : detections )
    {
        writer << std::to_string( detection.x_min ) << delim << std::to_string( detection.x_max ) << delim
               << std::to_string( detection.y_min ) << delim << std::to_string( detection.y_max ) << delim
               << std::to_string( detection.label_id ) << delim << detection.label << delim
               << std::to_string( detection.confidence ) << std::endl;
    }
    // End with empty line
    writer << std::endl;


    writer.close();

    return 0;
}

void Logger::LogError( std::string message, ErrorType error )
{
    if( error_file != "" )
    {
        std::ofstream writer;
        writer.open( error_file, std::ios_base::out | std::ios_base::app );
        writer << GetErrorString( error ) << message << std::endl;
        writer.close();
    }
    else
    {
        std::cerr << std::flush << GetErrorString( error ) << message << std::endl;
    }
}
}
