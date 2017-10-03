#include "Database.h"
namespace MajorProject
{
std::string EscapeInfile( std::string& infile )
{
    std::string outfile;

    std::string full_path;
    char full_path_buf[ PATH_MAX ];
    if( realpath( infile.c_str(), full_path_buf ) == NULL )
    {
        std::cout << "Failed to get full path. Using filename instead" << std::endl;
        full_path = ( infile.substr( infile.rfind( "/" ) ) ).c_str();
    }
    else
    {
        full_path = std::string( full_path_buf );
    }

    if(full_path[0] == '/')
    {
    	full_path.erase(0, 1);
    }

    for( const auto& character : full_path )
    {
        outfile += ( character == '/' || character == '.' ) ? '_' : character;
    }

    return outfile;
}

int Database::LogDetection( LogType log_type,
                            std::vector< BoundingBox >& detections,
                            std::string& infile,
                            ssize_t frame_id )
{
	std::ofstream writer;
    writer.open( data_directory + "/" + EscapeInfile( infile ) + ".txt", std::ios_base::out | std::ios_base::app );

    if(!writer.good())
    {
    	std::cout << "Failed to open writer: " << std::endl;
    	writer.close();
    	return -1;
    }

    std::string delim( "," );

    writer << std::to_string( detections.size() ) << std::endl;

    for( const auto& detection : detections )
    {
        writer << std::to_string( detection.x_min ) << delim << std::to_string( detection.x_max ) << delim
               << std::to_string( detection.y_min ) << delim << std::to_string( detection.y_max ) << delim
               << std::to_string( detection.label_id ) << delim << detection.label << delim
               << std::to_string( detection.confidence ) << std::endl;
    }

	writer << std::to_string( frame_id ) << std::endl;

    writer.close();

    return 0;
}
}
