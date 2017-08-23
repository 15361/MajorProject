# Use an official tensorflow runtime as a parent image
FROM tensorflow/tensorflow:latest

# Set the working directory to /FaceDetection
WORKDIR /FaceDetection

# Copy the current directory contents into the container at /FaceDetection
ADD . /FaceDetection

# Install any needed packages
RUN apt-get update
RUN apt-get install -y wget python-pip python-dev protobuf-compiler python-pil python-lxml

# Download and Extract Data Sets
RUN wget http://vis-www.cs.umass.edu/lfw/lfw.tgz
RUN wget http://www.vision.caltech.edu/Image_Datasets/Caltech101/101_ObjectCategories.tar.gz
RUN tar -xzf lfw.tgz
RUN tar -xzf 101_ObjectCategories.tar.gz
RUN rm -rf 101_ObjectCategories/face*
RUN rm lfw.tgz 101_ObjectCategories.tar.gz
RUN mkdir lfw_sorted
RUN mkdir 101_sorted
RUN mv $(ls -d lfw/*/*) lfw_sorted/
RUN cd 101_ObjectCategories
RUN mv $(FILES=$(ls -d */*); for FILE in $FILES; do echo "$FILE" | sed "s/\//-/g"; done) /FaceDetection/101_sorted/
RUN cd /FaceDetection
RUN rm -rf lfw 101_ObjectCategories

#Install Object Recognition Framework
RUN git clone https://github.com/tensorflow/models.git
RUN pip install tensorflow
RUN pip install jupyter
RUN pip install matplotlib
RUN cd models
RUN protoc object_detection/protos/*.proto --python_out=.
RUN export PYTHONPATH=$PYTHONPATH:`pwd`:`pwd`/slim



# Run FaceDetection.py when the container launches
CMD ["bash"]