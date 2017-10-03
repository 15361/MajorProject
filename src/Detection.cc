#include "Detection.h"
namespace MajorProject
{
int Detector::InitSession( std::string& model_path )
{
    if( CloseSession() == -1 )
    {
        return -1;
    }

    // Initialize a tensorflow session
    tensorflow::Status status = tensorflow::NewSession( tensorflow::SessionOptions(), &session );
    if( !status.ok() )
    {
        std::cout << status.ToString() << std::endl;
        return -1;
    }

    tensorflow::GraphDef graph_def;
    status = tensorflow::ReadBinaryProto( tensorflow::Env::Default(), model_path, &graph_def );
    if( !status.ok() )
    {
        std::cout << status.ToString() << std::endl;
        return -1;
    }

    // Add the graph to the session
    status = session->Create( graph_def );
    if( !status.ok() )
    {
        std::cout << status.ToString() << std::endl;
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
            std::cout << status.ToString() << std::endl;
            return -1;
        }
        session = nullptr;
    }
    return 0;
}

struct VisParam
{
    VisParam( std::queue< cv::Mat* >* _frame_queue, size_t _batch_size )
    {
        frame_queue = _frame_queue;
        batch_size = _batch_size;
    }
    std::queue< cv::Mat* >* frame_queue;
    size_t batch_size;
};

void* VisualiseThread( void* param_ptr )
{
    VisParam* param = (VisParam*)param_ptr;
    auto frame_queue = param->frame_queue;
    cv::namedWindow( "Video" );
    while( true )
    {
        while( frame_queue->empty() )
        {
        }

        if( frame_queue->front() == nullptr )
        {
            pthread_exit( NULL );
        }

        cv::Mat* frame = frame_queue->front();
        frame_queue->pop();
        if( cvGetWindowHandle( "Video" ) == 0 )
        {
            std::cout << "Window closed. Stopping visualisation" << std::endl;
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

int Detector::ProcMP4( std::string& mp4_path, bool visualise )
{
    if( !session )
    {
        std::cout << "Session is not initialised" << std::endl;
        return -1;
    }
    cv::VideoCapture cap( mp4_path );
    if( !cap.isOpened() )
    {
        std::cout << "Failed to open: " << mp4_path;
        return -1;
    }

    size_t frame_count = (size_t)cap.get( cv::CAP_PROP_FRAME_COUNT );
    std::queue< cv::Mat* > frame_queue;

    pthread_t vis_thread;
    VisParam param( &frame_queue, batch_size );
    if( visualise )
    {
        if( pthread_create( &vis_thread, NULL, VisualiseThread, (void*)&param ) )
        {
            std::cout << "Failed to launch visualisation thread" << std::endl;
            return -1;
        }
    }
    size_t fps = (size_t)cap.get( cv::CAP_PROP_FPS );
    // Aim for 1 batch per second
    size_t drop_frames = fps / batch_size;
    std::vector< cv::Mat* > frames;
    frames.reserve( fps );
    for( size_t i = 0; i < frame_count; i++ )
    {
        cv::Mat* frame = new cv::Mat();
        cap >> *frame;
        if( i % drop_frames == 0 )
        {
            frames.push_back( frame );
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
                return -1;
            }

            if( LogDetection( MP4, output_tensors ) == -1 )
            {
                return -1;
            }

            if( visualise )
            {
                if( VisualiseDetection( frames, output_tensors ) == -1 )
                {
                    return -1;
                }

                for( size_t j = 0; j < frames.size(); j++ )
                {
                    frame_queue.push( frames[ j ] );
                }
                frames.clear();
            }
            auto end = std::chrono::steady_clock::now();
            auto diff = end - start;
            std::cout << std::chrono::duration< double, std::milli >( diff ).count() << " ms" << std::endl;
        }
    }
    frame_queue.push( nullptr );

    if( visualise )
    {
        pthread_join( vis_thread, NULL );
    }

    while( frame_queue.size() != 0 )
    {
        auto frame = frame_queue.front();
        frame_queue.pop();
        if( frame != nullptr )
        {
            delete frame;
        }
    }

    return 0;
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
        std::cout << status.ToString() << std::endl;
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
                cv::rectangle( frame,
                               cv::Point( pixel_min_x, pixel_min_y ),
                               cv::Point( pixel_max_x, pixel_max_y ),
                               cv::Scalar( 0, 255, 0 ),
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
                            std::vector< tensorflow::Tensor >& detection_results,
                            std::string& file_name,
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
                size_t pixel_min_y = frame.rows * box_tensor( i, j, 0 );
                size_t pixel_min_x = frame.cols * box_tensor( i, j, 1 );
                size_t pixel_max_y = frame.rows * box_tensor( i, j, 2 );
                size_t pixel_max_x = frame.cols * box_tensor( i, j, 3 );
                BoundingBox box;
                box.x_max = pixel_max_x;
                box.y_max = pixel_max_y;
                box.x_min = pixel_min_x;
                box.y_min = pixel_min_y;

                box.label_id = (size_t)box_class;
                box.label = label_map.find( box_class );
                box.confidence = confidence;
                log_data.push_back( box );
            }
        }

        if( database.LogDetection( log_type, log_data, file_name, frame_ids.empty() ? 0 : frame_ids[ i ] ) == -1 )
        {
            return -1;
        }
    }

    return 0;
}
}
