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
bazel build -c opt //tensorflow:libtensorflow_cc.so   
sudo cp bazel-bin/tensorflow/libtensorflow.so /usr/local/lib  

mkdir /usr/local/include/tensorflow
cp -r bazel-genfiles/ /usr/local/include/tensorflow/
cp -r tensorflow /usr/local/include/tensorflow/
cp -r third_party /usr/local/include/tensorflow/
```

Install tensorflow dependencies


*Protobuf*
```
mkdir /tmp/proto
tensorflow/contrib/makefile/download_dependencies.sh
cd tensorflow/contrib/makefile/downloads/protobuf/
./autogen.sh
./configure --prefix=/tmp/proto/
make
make install

cp /tmp/proto/lib/libprotobuf.so /usr/local/lib
cp -r /tmp/proto/include/* /usr/local/include
```

*Eigen*
```
mkdir /tmp/eigen
cd tensorflow/contrib/makefile/downloads/eigen
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/tmp/eigen/ ../
make install

cp -r /tmp/eigen/include/eigen3/* /usr/local/include
```

*Others*
```
cp -r tensorflow/contrib/makefile/downloads/nsync/public/* /usr/local/include/nsync
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
./bin/StreamingDemo ../video/in.mp4 ../video/out.mp4 ../path/to/unzipped/model.pb
```
