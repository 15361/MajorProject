# MajorProject
***
## Build

#### Dependencies

##### OpenCV

Follow instructions at http://docs.opencv.org/3.0-beta/doc/tutorials/introduction/linux_install/linux_install.html to install opencv  


##### Tensorflow

Follow instructions https://www.tensorflow.org/install/install_sources to install tensorflow and setup bazel (You may have to use bazel version 0.5.4)

From tensorflow root directory  
```
bazel build -c opt //tensorflow:libtensorflow.so   
sudo cp bazel-bin/tensorflow/libtensorflow.so /usr/local/lib
```


#### Building Major Project

```
mkdir build
cd build
cmake ..
make -j8
```

#### Running Demo

From build directory  
```
./bin/StreamingDemo in.mp4 out.mp4
```
