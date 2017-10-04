#include "Detection.h"
#include "Logging.h"

int main( int argc, char** argv )
{
    if( argc != 4 )
    {
        std::cout << "Usage: " << argv[ 0 ] << " infile logfile model-pb" << std::endl;
        return -1;
    }
    std::string infile( argv[ 1 ] );
    std::string outpath( argv[ 2 ] );
    std::string model( argv[ 3 ] );

    std::string outdir = outpath.substr( 0, outpath.rfind( "/" ) + 1 );
    std::string outfile = outpath.substr( outpath.rfind( "/" ) + 1 );
    if( outdir == "" )
    {
        outdir = ".";
    }

    MajorProject::Logger* logger = new MajorProject::Logger( outdir, "" );

    MajorProject::Detector detector( logger );
    detector.SetConfidenceThreshold( 0.1 );
    detector.SetBatchSize( 1 );
    detector.SetAllowGrowth( true );
    detector.SetSessionGpuMemoryFraction( 1.0 );
    detector.SetGpuDeviceId( 0 );
    detector.SetTensorflowLogLevel( 2 );
    detector.SetTensorflowVLogLevel( 3 );
    detector.InitSession( model );
    detector.ProcMP4( infile, outfile, true );

    return 0;
}
