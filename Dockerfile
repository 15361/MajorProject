# Use an official tensorflow runtime as a parent image
FROM tensorflow/tensorflow:latest

# Set the working directory to /FaceDetection
WORKDIR /FaceDetection

# Copy the current directory contents into the container at /FaceDetection
ADD . /FaceDetection

# Install any needed packages
RUN apt-get update
RUN apt-get install -y git build-essential cmake clang-3.8 clang++-3.8 clang-format-3.6 curl

# Unzip ssd model
RUN tar -xzf ssd_frozen_model.tar.gz

# Install OpenCV
RUN apt-get install -y libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
RUN apt-get install -y python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev
RUN git clone https://github.com/Itseez/opencv.git
RUN cd opencv && mkdir build && cd build && cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local .. && make -j4 && make install

# Install Tensorflow
RUN git clone https://github.com/tensorflow/tensorflow 
RUN apt-get install -y openjdk-8-jdk
RUN add-apt-repository ppa:webupd8team/java && apt-get update && apt-get install oracle-java8-installer
RUN echo "deb [arch=amd64] http://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list && curl https://bazel.build/bazel-release.pub.gpg | apt-key add -
RUN apt-get update && apt-get install bazel && apt-get upgrade bazel
RUN cd tensorflow && bazel build -c opt //tensorflow:libtensorflow_cc.so
RUN cp tensorflow/bazel-bin/tensorflow/libtensorflow_cc.so /usr/local/lib
RUN mkdir /usr/local/include/tensorflow
RUN cd tensorflow/ && cp -r bazel-genfiles /usr/local/include/tensorflow/ && cp -r tensorflow /usr/local/include/tensorflow/ && cp -r third_party /usr/local/include/tensorflow/

# Tensorflow dependencies
RUN cd tensorflow && tensorflow/contrib/makefile/download_dependencies.sh

# Protobuf
RUN mkdir /tmp/proto
RUN cd tensorflow/tensorflow/contrib/makefile/downloads/protobuf/ && ./autogen.sh && ./configure --prefix=/tmp/proto/ && make -j4 && make install
RUN cp /tmp/proto/lib/libprotobuf.so /usr/local/lib && cp -r /tmp/proto/include/* /usr/local/include

# Eigen
RUN mkdir /tmp/eigen
RUN cd tensorflow/tensorflow/contrib/makefile/downloads/eigen && mkdir build && cd build && cmake -DCMAKE_INSTALL_PREFIX=/tmp/eigen/ ../ && make -j4 && make install
RUN cp -r /tmp/eigen/include/eigen3/* /usr/local/include

# Nsync
RUN cp -r tensorflow/contrib/makefile/downloads/nsync/public/* /usr/local/include/nsync


# Run FaceDetection.py when the container launches
CMD ["bash"]
