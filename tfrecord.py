import sys, os
import io
import math
import tensorflow as tf
from PIL import Image
import random

from object_detection.utils import dataset_util
import json

flags = tf.app.flags
flags.DEFINE_string('output_path', '', 'Path to output directory for TFRecord')
flags.DEFINE_string('input_path', '', 'Path to input jpg directory')
flags.DEFINE_boolean('eval', False, 'Whether it is eval job or not')
FLAGS = flags.FLAGS


def create_tf_example(_filename, _encoded_image_data, _width, _height, _metadata):
  height = _height # Image height
  width = _width # Image width
#  if height != 300 or width != 300:
#    print "Invalid Image dimensions! " + str(width) + "x" + str(height)
#    exit()

  filename = _filename # Filename of the image. Empty if image is not from file
  encoded_image_data = _encoded_image_data # Encoded image bytes
  image_format = b'jpeg' # or b'png'
  metadata = _metadata

  xmins = [] # List of normalized left x coordinates in bounding box (1 per box)
  xmaxs = [] # List of normalized right x coordinates in bounding box
             # (1 per box)
  ymins = [] # List of normalized top y coordinates in bounding box (1 per box)
  ymaxs = [] # List of normalized bottom y coordinates in bounding box
             # (1 per box)

  classes_text = []  # List of string class name of bounding box (1 per box)
  classes = [] # List of integer class id of bounding box (1 per box)

  for line in metadata:
    if len(line) <= 2:
       continue
    entries = [ e[2:] for e in line.split(' ')[1:-1] ] # drop leading letter and colon
    if entries[0] == "False":
       continue
    x = float(entries[1])
    y = float(entries[2])
    w = float(entries[3])
    h = float(entries[4])

    if math.isnan(x) or math.isnan(y) or math.isnan(w) or math.isnan(h) or height is 0 or width is 0:
	print "NAN!"
	exit()

    xmins.append(x / float(width))
    xmaxs.append((x + w) / float(width))

    ymins.append(y / float(height))
    ymaxs.append((y + h) / float(height))

    classes_text.append('face')
    classes.append(1)


  tf_example = tf.train.Example(features=tf.train.Features(feature={
      'image/height': dataset_util.int64_feature(height),
      'image/width': dataset_util.int64_feature(width),
      'image/filename': dataset_util.bytes_feature(filename),
      'image/source_id': dataset_util.bytes_feature(filename),
      'image/encoded': dataset_util.bytes_feature(encoded_image_data),
      'image/format': dataset_util.bytes_feature(image_format),
      'image/object/bbox/xmin': dataset_util.float_list_feature(xmins),
      'image/object/bbox/xmax': dataset_util.float_list_feature(xmaxs),
      'image/object/bbox/ymin': dataset_util.float_list_feature(ymins),
      'image/object/bbox/ymax': dataset_util.float_list_feature(ymaxs),
      'image/object/class/text': dataset_util.bytes_list_feature(classes_text),
      'image/object/class/label': dataset_util.int64_list_feature(classes),
  }))
  return tf_example

def processFiles(writer):
	files = os.listdir(FLAGS.input_path)
	random.shuffle(files)
	for file in files:
		if file[-3:] == "txt":
			continue
		with tf.gfile.GFile(os.path.join(FLAGS.input_path, file), 'rb') as fid:
			enc_data = fid.read()
		jpgfile = Image.open(io.BytesIO(enc_data))

		with open(FLAGS.input_path + "/" + file[:-3] + "txt") as metadata:
			tf_example = create_tf_example(file, enc_data, jpgfile.size[0], jpgfile.size[1], metadata)
		writer.write(tf_example.SerializeToString())

def main(_):
	print "Output path: " + str(FLAGS.output_path) + " Input path: " + str(FLAGS.input_path)

	train_name = "lfw_train.record" if not FLAGS.eval else "lfw_test.record"
	writer = tf.python_io.TFRecordWriter(os.path.join(FLAGS.output_path, train_name))
	processFiles(writer)
	writer.close()


if __name__ == '__main__':
  tf.app.run()
