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
flags.DEFINE_string('metadata_path', '', 'Path to input metadata directory')
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

  number_entries = int(next(metadata))
  for i in range(0, number_entries):
    next_line = next(metadata).strip('\n')

    data = [ float(entry) for entry in next_line.split(' ')[:-2] ]
    centre_x = data[3]
    centre_y = data[4]

    r_major = data[0] * math.cos(data[2] * math.pi / 180)
    r_minor = data[1] * math.cos(data[2] * math.pi / 180)

    x = float(centre_x - r_minor)
    y = float(centre_y - r_major)
    w = float(r_minor * 2)
    h = float(r_major * 2)

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
	files = os.listdir(FLAGS.metadata_path)
	random.shuffle(files)
	for metadata_file in files:
		if "ellipseList" not in metadata_file:
			continue

		with open(os.path.join(FLAGS.metadata_path, metadata_file)) as metadata:
			for file in metadata:
				filepath = os.path.join(FLAGS.input_path, file.strip('\n') + ".jpg")
				if "/" not in file:
					continue
				with tf.gfile.GFile(filepath, 'rb') as fid:
					enc_data = fid.read()
				jpgfile = Image.open(io.BytesIO(enc_data))

				tf_example = create_tf_example(filepath, enc_data, jpgfile.size[0], jpgfile.size[1], metadata)
				writer.write(tf_example.SerializeToString())

def main(_):
	print "Output path: " + str(FLAGS.output_path) + " Input path: " + str(FLAGS.input_path)

	train_name = "lfw_train.record" if not FLAGS.eval else "lfw_test.record"
	writer = tf.python_io.TFRecordWriter(os.path.join(FLAGS.output_path, train_name))
	processFiles(writer)
	writer.close()


if __name__ == '__main__':
  tf.app.run()
