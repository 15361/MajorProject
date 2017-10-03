#pragma once
#include <opencv2/opencv.hpp>
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/platform/env.h>
#include <pthread.h>
#include <chrono>

#include "Database.h"

namespace MajorProject
{
class Detector
{
public:
    Detector( Database& _database )
        : session( nullptr )
        , confidence_threshold( 0.5 )
        , label_map( { { 1, "face" } } )
        , database( _database )
        , batch_size( 1 )
    {
    }

    ~Detector()
    {
        CloseSession();
    }

    /*
     * @InitSession	Creates a new session and loads model from .pb
     *
     * @param model_path	Path to .pb file to load the model form
     *
     * @return	-1 on failure, 0 otherwise
     */
    int InitSession( std::string& model_path );

    /*
     * @CloseSession	Closes session
     *
     * @return	-1 on failure, 0 otherwise
     */
    int CloseSession();

    /*
     * @ProcMP4	Detects objects in an MP4 file
     *
     * @param mp4_path	Path to .mp4 file to process
     *
     * @return	-1 on failure, 0 otherwise
     */
    int ProcMP4( std::string& mp4_path, bool visualise = false );

    /*
     * @ProcJPG	Detects objects in an JPEG file
     *
     * @param mp4_path	Path to .jpg file to process
     *
     * @return	-1 on failure, 0 otherwise
     */
    int ProcJPG( std::string& image_path, bool visualise = false )
    {
        return 0;
    }


    /*
     * @SetConfidenceThreshold	Sets the level of confidence for a positive identification. Between 0 and 1
     */
    void SetConfidenceThreshold( double _confidence )
    {
        confidence_threshold = _confidence;
    }

    /*
     * @SetConfidenceThreshold	Sets label map. Label map maps output id ( positive integer >=1) to corresponding label
     * string
     */
    void SetLabelMap( std::map< size_t, std::string >&& _label_map )
    {
        label_map = _label_map;
    }

    /*
     * @SetDatabase	Sets logging database for recording detected faces
     */
    void SetDatabase( Database& _database )
    {
        database = _database;
    }

    void SetBatchSize( size_t _batch_size )
    {
        batch_size = _batch_size;
    }

private:
    tensorflow::Tensor CreateTensor( std::vector< cv::Mat* >& frame );

    int DetectObjects( tensorflow::Tensor& image_tensor, std::vector< tensorflow::Tensor >& outputs );

    int VisualiseDetection( std::vector< cv::Mat* >& frame, std::vector< tensorflow::Tensor >& detection_results );

    int LogDetection( LogType log_type,
                      std::vector< tensorflow::Tensor >& detection_results,
                      std::string& file_name,
                      std::vector< size_t > frame_ids );

    tensorflow::Session* session;
    double confidence_threshold;
    std::map< size_t, std::string > label_map;
    Database database;
    size_t batch_size;
};
}
