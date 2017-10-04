#include "Detection.h"
namespace MajorProject
{
int Detector::InitSession( std::string& model_path )
{
    if( CloseSession() == -1 )
    {
        return -1;
    }
    tensorflow::Status status;
    tensorflow::SessionOptions opts;


    if( graph )
    {
        delete graph;
    }
    graph = new tensorflow::GraphDef();
    status = tensorflow::ReadBinaryProto( tensorflow::Env::Default(), model_path, graph );
    tensorflow::graph::SetDefaultDevice(
    ( gpu_device_id == -1 ) ? "/cpu:0" : ( "/gpu:" + std::to_string( gpu_device_id ) ), graph );
    if( !status.ok() )
    {
        database->LogError( status.ToString(), ErrorType::FATAL );
        return -1;
    }

    opts.config.mutable_gpu_options()->set_per_process_gpu_memory_fraction( session_gpu_memory_fraction );
    opts.config.mutable_gpu_options()->set_allow_growth( allow_growth );
    opts.config.set_allow_soft_placement( true );
    // opts.config.set_log_device_placement( true );

    // Initialize a tensorflow session
    status = tensorflow::NewSession( opts, &session );
    if( !status.ok() )
    {
        database->LogError( status.ToString(), ErrorType::FATAL );
        return -1;
    }

    // Add the graph to the session
    status = session->Create( *graph );
    if( !status.ok() )
    {
        database->LogError( status.ToString(), ErrorType::FATAL );
        return -1;
    }

    return 0;
}

int Detector::CloseSession()
{
    if( session )
    {
        tensorflow::Status status = session->Close();
        if( !status.ok() )
        {
            database->LogError( status.ToString(), ErrorType::FATAL );
            return -1;
        }
        session = nullptr;
    }
    if( graph )
    {
        delete graph;
    }
    return 0;
}

struct VisParam
{
    VisParam( std::queue< cv::Mat* >* _frame_queue, size_t _batch_size, Database* _database )
    {
        frame_queue = _frame_queue;
        batch_size = _batch_size;
        database = _database;
    }
    std::queue< cv::Mat* >* frame_queue;
    size_t batch_size;
    Database* database;
};

void* VisualiseThread( void* param_ptr )
{
    VisParam* param = (VisParam*)param_ptr;
    auto frame_queue = param->frame_queue;
    cv::namedWindow( "Video" );
    while( true )
    {
        auto start = std::chrono::steady_clock::now();
        while( frame_queue->empty() )
        {
            auto end = std::chrono::steady_clock::now();
            auto diff = end - start;
            if( std::chrono::duration< double, std::milli >( diff ).count() >= 10000 )
            {
                param->database->LogError( "Visualisation did not receive frame for 10 seconds. Terminating.",
                                           ErrorType::INFO );
                pthread_exit( NULL );
            }
        }

        if( frame_queue->front() == nullptr )
        {
            pthread_exit( NULL );
        }

        cv::Mat* frame = frame_queue->front();
        frame_queue->pop();
        if( cvGetWindowHandle( "Video" ) == 0 )
        {
            param->database->LogError( "Window closed. Stopping visualisation", ErrorType::INFO );
            pthread_exit( NULL );
        }
        cv::imshow( "Video", *frame );
        delete frame;
        if( cv::waitKey( 1000 / param->batch_size ) >= 0 )
        {
            pthread_exit( NULL );
        }
    }

    pthread_exit( NULL );
}

int Detector::ProcMP4( std::string& mp4_path, std::string outfile_name, bool visualise )
{
    if( !session )
    {
        database->LogError( "Session is not initialised", ErrorType::FATAL );
        return -1;
    }
    cv::VideoCapture cap( mp4_path );
    if( !cap.isOpened() )
    {
        database->LogError( "Failed to open: " + mp4_path, ErrorType::FATAL );
        return -1;
    }

    size_t frame_count = (size_t)cap.get( cv::CAP_PROP_FRAME_COUNT );
    std::queue< cv::Mat* > frame_queue;

    pthread_t vis_thread;
    VisParam param( &frame_queue, batch_size, database );
    if( visualise )
    {
        if( pthread_create( &vis_thread, NULL, VisualiseThread, (void*)&param ) )
        {
            database->LogError( "Failed to launch visualisation thread", ErrorType::WARNING );
            visualise = false;
        }
    }
    size_t fps = (size_t)cap.get( cv::CAP_PROP_FPS );
    // Aim for 1 batch per second
    size_t drop_frames = fps / batch_size;
    std::vector< cv::Mat* > frames;
    std::vector< size_t > frame_ids;
    frames.reserve( batch_size );
    frame_ids.reserve( batch_size );

    int return_code = 0;
    for( size_t i = 0; i < frame_count; i++ )
    {
        cv::Mat* frame = new cv::Mat();
        cap >> *frame;
        if( i % drop_frames == 0 )
        {
            frames.push_back( frame );
            frame_ids.push_back( i );
        }
        else
        {
            delete frame; // Drop frame
        }

        if( frames.size() == batch_size )
        {
            auto start = std::chrono::steady_clock::now();

            tensorflow::Tensor input_tensor = CreateTensor( frames );

            std::vector< tensorflow::Tensor > output_tensors;
            if( DetectObjects( input_tensor, output_tensors ) == -1 )
            {
                return_code = -1;
                break;
            }

            if( LogDetection( LogType::MP4, frames, output_tensors, mp4_path, outfile_name, frame_ids ) == -1 )
            {
                return_code = -1;
                break;
            }

            if( visualise )
            {
                if( VisualiseDetection( frames, output_tensors ) == -1 )
                {
                    return_code = -1;
                    break;
                }

                for( size_t j = 0; j < frames.size(); j++ )
                {
                    frame_queue.push( frames[ j ] );
                }
            }

            frames.clear();
            frame_ids.clear();


            auto end = std::chrono::steady_clock::now();
            auto diff = end - start;
            std::cout << std::chrono::duration< double, std::milli >( diff ).count() << " ms" << std::endl;
        }
    }


    if( visualise )
    {
        frame_queue.push( nullptr );
        pthread_join( vis_thread, NULL );
    }

    while( !frame_queue.empty() )
    {
        auto frame = frame_queue.front();
        frame_queue.pop();
        if( frame != nullptr )
        {
            delete frame;
        }
    }

    return return_code;
}


tensorflow::Tensor Detector::CreateTensor( std::vector< cv::Mat* >& frames )
{
    int channels = frames[ 0 ]->channels();
    tensorflow::Tensor frame_tensor(
    tensorflow::DT_UINT8,
    tensorflow::TensorShape( { (int)batch_size, frames[ 0 ]->rows, frames[ 0 ]->cols, channels } ) );
    auto mapped_frame = frame_tensor.tensor< uint8_t, 4 >();
    for( size_t h = 0; h < batch_size; h++ )
    {
        cv::Mat frame = *frames[ h ];
        for( size_t i = 0; i < frame.rows; i++ )
        {
            for( size_t j = 0; j < frame.cols; j++ )
            {
                mapped_frame( h, i, j, 0 ) = frame.data[ ( i * frame.cols + j ) * channels + 0 ];
                mapped_frame( h, i, j, 1 ) = frame.data[ ( i * frame.cols + j ) * channels + 1 ];
                mapped_frame( h, i, j, 2 ) = frame.data[ ( i * frame.cols + j ) * channels + 2 ];
            }
        }
    }

    return frame_tensor;
}

int Detector::DetectObjects( tensorflow::Tensor& input_tensor, std::vector< tensorflow::Tensor >& outputs )
{
    std::vector< std::pair< std::string, tensorflow::Tensor > > inputs = {
        { "image_tensor:0", input_tensor },
    };

    tensorflow::Status status = session->Run(
    inputs, { "detection_boxes:0", "detection_scores:0", "detection_classes:0", "num_detections:0" }, {}, &outputs );
    if( !status.ok() )
    {
        database->LogError( status.ToString(), ErrorType::FATAL );
        return -1;
    }

    return 0;
}

int Detector::VisualiseDetection( std::vector< cv::Mat* >& frames,
                                  std::vector< tensorflow::Tensor >& detection_results )
{
    for( size_t i = 0; i < batch_size; i++ )
    {
        cv::Mat frame = *frames[ i ];
        size_t detections = detection_results[ 3 ].vec< float >()( i );
        for( size_t j = 0; j < detections; j++ )
        {
            auto box = detection_results[ 0 ].tensor< float, 3 >();
            float confidence = detection_results[ 1 ].matrix< float >()( i, j );
            float box_class = detection_results[ 2 ].matrix< float >()( i, j );
            if( confidence >= confidence_threshold && label_map.find( box_class ) != label_map.end() )
            {
                size_t pixel_min_y = frame.rows * box( i, j, 0 );
                size_t pixel_min_x = frame.cols * box( i, j, 1 );
                size_t pixel_max_y = frame.rows * box( i, j, 2 );
                size_t pixel_max_x = frame.cols * box( i, j, 3 );
                size_t blue = ( pixel_min_x >= pixel_max_x ) ? 255 : 0;
                size_t red = ( pixel_min_y >= pixel_max_y ) ? 255 : 0;
                cv::rectangle( frame,
                               cv::Point( pixel_min_x, pixel_min_y ),
                               cv::Point( pixel_max_x, pixel_max_y ),
                               cv::Scalar( red, 255, blue ),
                               2 );
                cv::putText( frame,
                             label_map[ box_class ] + " " + std::to_string( ( size_t )( confidence * 100.0f ) ),
                             cv::Point( pixel_min_x, pixel_min_y ),
                             cv::FONT_HERSHEY_PLAIN,
                             1.5,
                             cv::Scalar( 0, 0, 0 ),
                             2 );
            }
        }
    }

    return 0;
}

int Detector::LogDetection( LogType log_type,
                            std::vector< cv::Mat* > frames,
                            std::vector< tensorflow::Tensor >& detection_results,
                            std::string& file_name,
                            std::string& outfile_name,
                            std::vector< size_t > frame_ids )
{
    for( size_t i = 0; i < batch_size; i++ )
    {
        std::vector< BoundingBox > log_data;
        cv::Mat frame = *frames[ i ];
        size_t detections = detection_results[ 3 ].vec< float >()( i );
        for( size_t j = 0; j < detections; j++ )
        {
            auto box_tensor = detection_results[ 0 ].tensor< float, 3 >();
            float confidence = detection_results[ 1 ].matrix< float >()( i, j );
            size_t box_class = (size_t)detection_results[ 2 ].matrix< float >()( i, j );
            if( confidence >= confidence_threshold && label_map.find( box_class ) != label_map.end() )
            {
                BoundingBox box;
                box.y_min = frame.rows * box_tensor( i, j, 0 );
                box.x_min = frame.cols * box_tensor( i, j, 1 );
                box.y_max = frame.rows * box_tensor( i, j, 2 );
                box.x_max = frame.cols * box_tensor( i, j, 3 );

                box.label_id = (size_t)box_class;
                box.label = label_map[ box_class ];
                box.confidence = confidence;
                log_data.push_back( box );
            }
        }

        if( database->LogDetection(
            log_type, log_data, file_name, outfile_name, frame_ids.empty() ? -1 : (ssize_t)frame_ids[ i ] ) == -1 )
        {
            return -1;
        }
    }

    return 0;
}
}
