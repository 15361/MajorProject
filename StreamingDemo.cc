#include "Detection.h"
#include "Database.h"

int main( int argc, char** argv )
{
    if( argc != 4 )
    {
        std::cout << "Usage: " << argv[ 0 ] << " infile logdir model-pb" << std::endl;
        return -1;
    }
    std::string infile( argv[ 1 ] );
    std::string outfile( argv[ 2 ] );
    std::string model( argv[ 3 ] );

    MajorProject::Database* database = new MajorProject::Database( outfile );

    MajorProject::Detector detector( database );
    detector.InitSession( model );
    detector.SetConfidenceThreshold( 0.1 );
    detector.SetBatchSize( 1 );
    detector.ProcMP4( infile );

    return 0;
}
