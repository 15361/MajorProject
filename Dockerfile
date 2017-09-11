# Use an official tensorflow runtime as a parent image
FROM tensorflow/tensorflow:latest

# Set the working directory to /FaceDetection
WORKDIR /FaceDetection

# Copy the current directory contents into the container at /FaceDetection
ADD . /FaceDetection

# Install any needed packages
RUN apt-get update
RUN apt-get install -y wget python-pip python-dev protobuf-compiler python-pil python-lxml git

# Install Object Recognition Framework
RUN git clone https://github.com/tensorflow/models.git
RUN pip install tensorflow
RUN pip install jupyter
RUN pip install matplotlib
RUN cd models && protoc object_detection/protos/*.proto --python_out=.
RUN cd models && export PYTHONPATH=$PYTHONPATH:`pwd`:`pwd`/slim

# Install frozen model
RUN cd models && wget http://download.tensorflow.org/models/object_detection/ssd_mobilenet_v1_coco_11_06_2017.tar.gz
RUN cd models && tar -xzvf ssd_mobilenet_v1_coco_11_06_2017.tar.gz

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
RUN cd 101_ObjectCategories && FILES=$(ls -d */*); for FILE in $FILES; do mv $FILE /FaceDetection/101_sorted/$(echo "$FILE" | sed "s/\//-/g"); done
RUN rm -rf lfw 101_ObjectCategories

# Get FDDB Datasets
RUN wget http://vis-www.cs.umass.edu/fddb/FDDB-folds.tgz
RUN tar -xvf FDDB-folds.tgz
RUN wget http://tamaraberg.com/faceDataset/originalPics.tar.gz
RUN mkdir FDDB-pics
RUN tar -xvf originalPics.tar.gz FDDB-pics/

# Install split data
RUN wget http://vis-www.cs.umass.edu/lfw/peopleDevTrain.txt
RUN wget http://vis-www.cs.umass.edu/lfw/peopleDevTest.txt



# Run FaceDetection.py when the container launches
CMD ["bash"]
