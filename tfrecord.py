import sys, os
import io
import tensorflow as tf
from PIL import Image

from object_detection.utils import dataset_util


flags = tf.app.flags
flags.DEFINE_string('output_path', '', 'Path to output directory for TFRecord')
flags.DEFINE_string('input_path', '', 'Path to input jpg directory')
flags.DEFINE_string('train_file', '', 'Path to training split txt')
flags.DEFINE_string('test_file', '', 'Path to test split txt')
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

def processFile(writer, f):
	for line in f:
		entry_info = line.split("\t")
		if len(entry_info) < 2:
			print "Processing " + line.rstrip() + " entries"
			continue
		name = entry_info[0]
		image_id = int(entry_info[1])
		file = name + "_" + format(image_id, '04') + ".jpg"
		sys.stdout.write("Processing image " + file + "                   \r")
		sys.stdout.flush()
		with tf.gfile.GFile(os.path.join(FLAGS.input_path, file), 'rb') as fid:
			enc_data = fid.read()
		jpgfile = Image.open(io.BytesIO(enc_data))
		tf_example = create_tf_example(file, enc_data, jpgfile.size[0], jpgfile.size[1])
		writer.write(tf_example.SerializeToString())

def main(_):
  print "Output path: " + str(FLAGS.output_path) + " Input path: " + str(FLAGS.input_path)

  with open(FLAGS.train_file, "r") as f:
    print "Reading training split from " + str(FLAGS.train_file)
    writer = tf.python_io.TFRecordWriter(os.path.join(FLAGS.output_path, "lfw_train.record"))
    processFile(writer, f)
    writer.close()
	
  with open(FLAGS.test_file, "r") as f:
    print "Reading training split from " + str(FLAGS.test_file)
    writer = tf.python_io.TFRecordWriter(os.path.join(FLAGS.output_path, "lfw_test.record"))
    processFile(writer, f)
    writer.close()


if __name__ == '__main__':
  tf.app.run()