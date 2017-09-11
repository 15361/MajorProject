using System;
using System.Collections.Generic;
using System.Linq;

namespace ImageCollage {
	static class Program {
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		static void Main(string[] args) {
            Random rand = new Random(Guid.NewGuid().GetHashCode());
            if (args.Length != 3)
            {
                Console.WriteLine("Usage: program.exe face_dir non_face_dir output_dir");
                throw new ArgumentException();
            }

            string[] face_files = null;
            string[] non_face_files = null;
            try
            {
                face_files = System.IO.Directory.GetFiles(args[0]);
                non_face_files = System.IO.Directory.GetFiles(args[1]);
            }
            catch (Exception e)
            {

                Console.WriteLine(e.Message);
                return;
            }


            for (int i = 0; i < 100; i++)
            {
                GenerateCollage collage_generator = new GenerateCollage();
                collage_generator.face_directory_path = args[0];
                collage_generator.non_face_directory_path = args[1];
                collage_generator.output_directory_path = args[2];
		        collage_generator.face_files = face_files;
                collage_generator.non_face_files = non_face_files;
                collage_generator.Generate(rand.Next(1, 20), true);
                collage_generator.Save("Collage" + i.ToString());
                Console.Write("Generating " + i.ToString() + "th image\r");
            }
		}
	}
}
