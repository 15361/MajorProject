#include <opencv2/opencv.hpp>

void ProcFrame(cv::Mat frame)
{

}

int main(int argc, char** argv)
{
	std::cout << "OpenCV version : " << CV_VERSION << std::endl;
	std::cout << "Major version : " << CV_MAJOR_VERSION << std::endl;
	std::cout << "Minor version : " << CV_MINOR_VERSION << std::endl;
	std::cout << "Subminor version : " << CV_SUBMINOR_VERSION << std::endl;

	if(argc != 3)
	{
		std::cout << "Usage: " << argv[0] << " infile outfile" << std::endl;
		return -1;
	}
	char* infile = argv[1];
	char* outfile = argv[2];

    cv::VideoCapture cap(infile); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
    {
    	std::cout << "Failed to open: " << infile;
        return -1;
    }

    cv::VideoWriter write(outfile, cap.get(cv::CAP_PROP_FOURCC), cap.get(cv::CAP_PROP_FPS), cv::Size(cap.get(cv::CAP_PROP_FRAME_WIDTH), cap.get(cv::CAP_PROP_FRAME_HEIGHT)));
    if(!write.isOpened())
    {
    	std::cout << "Failed to open: " << outfile;
        return -1;
    }

    cv::Mat edges;
    cv::namedWindow("Video", 1);
    size_t frames = (size_t)cap.get(cv::CAP_PROP_FRAME_COUNT);
    for(size_t i = 0; i < frames; i++)
    {
    	cv::Mat frame;
        cap >> frame; // get a new frame from camera
        ProcFrame(frame);
    	if(cvGetWindowHandle("Video") == 0)
    	{
    		std::cout << "Window closed" << std::endl;
    		return 0;
    	}
        cv::imshow("Video", frame);
        write.write(frame);
        if(cv::waitKey(30) >= 0) break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
