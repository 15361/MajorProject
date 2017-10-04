add_library(Logging Logging.cc)

add_library(Detection Detection.cc)
target_link_libraries(Detection ${OPENCV_LIBS} tensorflow_cc pthread Logging)


