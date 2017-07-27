# Use an official tensorflow runtime as a parent image
FROM tensorflow:latest

# Set the working directory to /FaceDetection
WORKDIR /FaceDetection

# Copy the current directory contents into the container at /FaceDetection
ADD . /FaceDetection

# Install any needed packages specified in requirements.txt
RUN apt-get install -y curl cmake

# Run FaceDetection.py when the container launches
CMD ["bash"]