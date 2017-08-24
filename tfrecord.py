import sys, os
import tensorflow as tf
from PIL import Image

from object_detection.utils import dataset_util


flags = tf.app.flags
flags.DEFINE_string('output_path', '', 'Path to output TFRecord')
flags.DEFINE_string('input_path', '', 'Path to input jpg')
FLAGS = flags.FLAGS


def create_tf_example(_filename, _encoded_image_data, _width, _height):
  height = _height # Image height
  width = _width # Image width
  filename = _filename # Filename of the image. Empty if image is not from file
  encoded_image_data = _encoded_image_data # Encoded image bytes
  image_format = b'jpeg' # or b'png'

  xmins = [0] # List of normalized left x coordinates in bounding box (1 per box)
  xmaxs = [1] # List of normalized right x coordinates in bounding box
             # (1 per box)
  ymins = [0] # List of normalized top y coordinates in bounding box (1 per box)
  ymaxs = [1] # List of normalized bottom y coordinates in bounding box
             # (1 per box)
  classes_text = ['face'] # List of string class name of bounding box (1 per box)
  classes = [1] # List of integer class id of bounding box (1 per box)

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


def main(_):
  print "Output path: " + str(FLAGS.output_path) + " Input path: " + str(FLAGS.input_path)
  writer = tf.python_io.TFRecordWriter(FLAGS.output_path)

  files = []

  for file in os.listdir(FLAGS.input_path):
    jpgfile = Image.open(FLAGS.input_path + "/" + file)
	
    tf_example = create_tf_example(file, list(jpgfile.getdata()), jpgfile.size[0], jpgfile.size[1])
    writer.write(tf_example.SerializeToString())

  writer.close()


if __name__ == '__main__':
  tf.app.run()