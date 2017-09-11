using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;

namespace ImageCollage
{
    class GenerateCollage
    {
        public ImageCollage Collage { get; private set; }

        public string[] face_files = null;
        public string[] non_face_files = null;


        public GenerateCollage()
        {
            Collage = new ImageCollage();
            Collage.Width = 4000f;
            Collage.Height = 4000f;
            Collage.BackColor = Color.Black;
            Collage.InitialArrangement = InitialArrangement.Random;
            Collage.SoftEdges = true;
        }

        public Bitmap Bitmap { get; private set; }

        public string face_directory_path = "./lfw/";
        public string non_face_directory_path = "./cifar/";
        public string output_directory_path = "./collage/";

        public void ReadImages(int image_count, bool have_faces)
        {
            Random rand = new Random(Guid.NewGuid().GetHashCode());
            if (!System.IO.Directory.Exists(face_directory_path) || !System.IO.Directory.Exists(non_face_directory_path))
            {
                Console.WriteLine("Invalid Source Directory");
                throw new ArgumentException();
            }
/*
            string[] face_files = null;
            string[] non_face_files = null;
            try
            {
                face_files = System.IO.Directory.GetFiles(face_directory_path);
                non_face_files = System.IO.Directory.GetFiles(non_face_directory_path);
            }
            catch (Exception e)
            {

                Console.WriteLine(e.Message);
                return;
            }
*/
            Collage.Images.Add(new ImageInfo(true, face_files[rand.Next(face_files.Length)], (float)rand.Next(50, 100)));
            for (int i = 1; i < image_count; i++)
            {
                if (rand.Next(2) == 0)
                {
                    Collage.Images.Add(new ImageInfo(true, face_files[rand.Next(face_files.Length)], (float)rand.Next(50, 100)));
                }
                else
                {
                    Collage.Images.Add(new ImageInfo(false, non_face_files[rand.Next(non_face_files.Length)], (float)rand.Next(50, 100)));
                }
            }
            Collage.background_path = non_face_files[rand.Next(non_face_files.Length)];
        }

        public void Generate(int image_count, bool have_faces)
        {
            Collage.Images.Clear();
//            Console.Write("Selecting Images\r");
            ReadImages(image_count, have_faces);
//            Console.Write("Arranging Images\r");
            Collage.Arrange();
//            Console.Write("Generating Collage\r");
            Bitmap = Collage.Render();
        }

        public void Save(string file_name)
        {
//            Console.Write("Saving Image\r");

            Bitmap.Save(output_directory_path + file_name + ".jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
//            Console.Write("Saving Metadata\r");
            SaveMetadata(file_name);
//            Console.Write("Saving Complete\r");
        }

        public void SaveMetadata(string filename)
        {
            List<string> metadata = new List<string>();
            metadata.Add("{");
            foreach (ImageInfo image in Collage.Images)
            {
                metadata.Add("{ f: " + image.Face.ToString() + " x:" + image.finalBounds.X + " y:" + image.finalBounds.Y + " w:" + image.finalBounds.Width + " h:" + image.finalBounds.Height + " }");
            }
            metadata.Add("}");
            try
            {
                System.IO.File.WriteAllLines(output_directory_path + filename + ".txt", metadata.ToArray());
            }
            catch (Exception e) 
            { 
                Console.WriteLine(e.Message); 
                return;
            }
        }
    }
}
