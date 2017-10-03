#include <opencv2/opencv.hpp>
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/platform/env.h>

using namespace tensorflow;

int ProcFrame( cv::Mat& frame, Session* session )
{
    Tensor frame_tensor( DT_UINT8, TensorShape({ 1, frame.rows, frame.cols, frame.channels()}));
    auto mapped_frame = frame_tensor.tensor<uint8_t, 4>();
    for(size_t i = 0; i < frame.rows; i++)
    {
        for(size_t j = 0; j < frame.cols; j++)
        {
        	mapped_frame(0, i, j, 0) = frame.data[( i * frame.cols + j ) * 3 + 0];
        	mapped_frame(0, i, j, 1) = frame.data[( i * frame.cols + j ) * 3 + 1];
        	mapped_frame(0, i, j, 2) = frame.data[( i * frame.cols + j ) * 3 + 2];
        }
    }

    std::vector< std::pair< string, tensorflow::Tensor > > inputs = {
        { "image_tensor:0", frame_tensor },
    };

    // The session will initialize the outputs
    std::vector< tensorflow::Tensor > outputs;

    Status status = session->Run( inputs, { "detection_boxes:0", "detection_scores:0", "detection_classes:0", "num_detections:0" }, {}, &outputs );
    if( !status.ok() )
    {
        std::cout << status.ToString() << "\n";
        return 1;
    }

    size_t num_detected = (size_t)outputs[3].scalar<float>()();
    std::cout << "Detected " <<  std::to_string(num_detected) << " faces" << std::endl;
    for(size_t i = 0; i < num_detected; i ++)
    {
    	auto box = outputs[0].tensor<float, 3>();
    	float confidence = outputs[1].matrix<float>()(0, i);
    	float box_class = outputs[2].matrix<float>()(0, i);
    	if(confidence > 1e-4 && box_class == 1)
    	{
			size_t pixel_min_x = frame.cols * box(0, i, 1);
			size_t pixel_min_y = frame.rows * box(0, i, 0);
			size_t pixel_max_x = frame.cols * box(0, i, 3);
			size_t pixel_max_y = frame.rows * box(0, i, 2);
			for(size_t j = pixel_min_y; j < pixel_max_y; j++)
			{
				for(size_t k = pixel_min_x; k < pixel_max_x; k++)
				{
					frame.data[( j * frame.cols + k ) * 3 + 0] = 0;
					frame.data[( j * frame.cols + k ) * 3 + 1] = 255;
					frame.data[( j * frame.cols + k ) * 3 + 2] = 0;

					if(!(j == pixel_min_y || j + 1 == pixel_max_y))
					{
						k = pixel_max_x - 1;
						frame.data[( j * frame.cols + k ) * 3 + 0] = 0;
						frame.data[( j * frame.cols + k ) * 3 + 1] = 255;
						frame.data[( j * frame.cols + k ) * 3 + 2] = 0;
						break;
					}
				}
			}
    	}
    }


//
//    // Grab the first output (we only evaluated one graph node: "c")
//    // and convert the node to a scalar representation.
//    auto output_c = outputs[ 0 ].scalar< float >();
//
//    // (There are similar methods for vectors and matrices here:
//    // https://github.com/tensorflow/tensorflow/blob/master/tensorflow/core/public/tensor.h)
//
//    // Print the results
//    std::cout << outputs[ 0 ].DebugString() << "\n"; // Tensor<type: float shape: [] values: 30>
//    std::cout << output_c() << "\n"; // 30

    return 0;
}

int main( int argc, char** argv )
{
    std::cout << "OpenCV version : " << CV_VERSION << std::endl;
    std::cout << "Major version : " << CV_MAJOR_VERSION << std::endl;
    std::cout << "Minor version : " << CV_MINOR_VERSION << std::endl;
    std::cout << "Subminor version : " << CV_SUBMINOR_VERSION << std::endl;

    if( argc != 4 )
    {
        std::cout << "Usage: " << argv[ 0 ] << " infile outfile model-pb" << std::endl;
        return -1;
    }
    char* infile = argv[ 1 ];
    char* outfile = argv[ 2 ];
    char* model = argv[ 3 ];

    // Initialize a tensorflow session
    Session* session;
    Status status = NewSession( SessionOptions(), &session );
    if( !status.ok() )
    {
        std::cout << status.ToString() << "\n";
        return 1;
    }

    GraphDef graph_def;
    status = ReadBinaryProto( Env::Default(), model, &graph_def );
    if( !status.ok() )
    {
        std::cout << status.ToString() << "\n";
        return 1;
    }

    // Add the graph to the session
    status = session->Create( graph_def );
    if( !status.ok() )
    {
        std::cout << status.ToString() << "\n";
        return 1;
    }

    cv::VideoCapture cap( infile ); // open the default camera
    if( !cap.isOpened() ) // check if we succeeded
    {
        std::cout << "Failed to open: " << infile;
        return -1;
    }

    cv::VideoWriter write( outfile,
                           cap.get( cv::CAP_PROP_FOURCC ),
                           cap.get( cv::CAP_PROP_FPS ),
                           cv::Size( cap.get( cv::CAP_PROP_FRAME_WIDTH ), cap.get( cv::CAP_PROP_FRAME_HEIGHT ) ) );
    if( !write.isOpened() )
    {
        std::cout << "Failed to open: " << outfile;
        return -1;
    }

    cv::namedWindow( "Video", 1 );
    size_t frames = (size_t)cap.get( cv::CAP_PROP_FRAME_COUNT );
    for( size_t i = 0; i < frames; i++ )
    {
        cv::Mat frame;
        cap >> frame; // get a new frame from camera
        ProcFrame( frame, session );
        if( cvGetWindowHandle( "Video" ) == 0 )
        {
            std::cout << "Window closed" << std::endl;
            return 0;
        }
        cv::imshow( "Video", frame );
        write << frame;
        if( cv::waitKey( 30 ) >= 0 )
            break;
    }

    // Free any resources used by the session
    status = session->Close();
    if( !status.ok() )
    {
        std::cout << status.ToString() << "\n";
        return 1;
    }
    return 0;
}
