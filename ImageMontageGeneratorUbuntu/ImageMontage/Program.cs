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
            for (int i = 0; i < 100; i++)
            {
                GenerateCollage collage_generator = new GenerateCollage();
                collage_generator.face_directory_path = args[0];
                collage_generator.non_face_directory_path = args[1];
                collage_generator.output_directory_path = args[2];
                collage_generator.Generate(rand.Next(1, 20), true);
                collage_generator.Save("Collage" + i.ToString());
                Console.WriteLine("Generating " + i.ToString() + "th image");
            }
		}
	}
}
