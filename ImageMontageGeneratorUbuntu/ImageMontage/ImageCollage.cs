using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.Drawing.Drawing2D;
using System.Threading;

namespace ImageCollage {

	/// <summary>
	/// Represents an image montage and the layout logic used to construct it.
	/// </summary>
	public class ImageCollage {

        /// <summary>
        /// The minimum threshold for absolute displacement in an iteration
        /// before the arrangement is regarded as stable.
        /// </summary>
		const float MIN_DISPLACEMENT = 10f;
        /// <summary>
        /// The magnitude of the force that pulls the images towards the
        /// center of the screen.
        /// </summary>
		const float PULLING_FORCE = 10f;
        /// <summary>
        /// The amount by which the velocity of each image is reduced in the
        /// following cycle.
        /// </summary>
		const float DAMPING = 0.5f;
        /// <summary>
        /// The amount by which the original force decays when a collision
        /// occurs between two images.
        /// </summary>
		const float ABSORBANCE = 0.5f;
        /// <summary>
        /// The maximum number of iterations before the algorithm will
        /// terminate. If this number is reached before the arrangement
        /// achieves stability, other constants may need to be tweaked.
        /// </summary>
		const int MAX_ITERATIONS = 200;

		/// <summary>
		/// Gets the collection of images that make up the montage.
		/// </summary>
		public List<ImageInfo> Images { get; private set; }
		/// <summary>
		/// Gets or sets the width of the canvas.
		/// </summary>
		public float Width { get; set; }
		/// <summary>
		/// Gets or sets the height of the canvas.
		/// </summary>
		public float Height { get; set; }
        /// <summary>
        /// Gets or sets the size of the canvas.
        /// </summary>
        public SizeF Size {
            get {
                return new SizeF(Width, Height);
            }
            set {
                Width = value.Width;
                Height = value.Height;
            }
        }
		/// <summary>
		/// Gets or sets the background color.
		/// </summary>
		public Color BackColor { get; set; }


        public string background_path { get; set; }
		/// <summary>
		/// Gets or sets a value indicating whether to render the images with soft edges.
		/// </summary>
		public bool SoftEdges { get; set; }
        /// <summary>
        /// Gets or sets the type of initial arrangement for the images.
        /// </summary>
        public InitialArrangement InitialArrangement { get; set; }

		/// <summary>
		/// Constructor.
		/// </summary>
		public ImageCollage() {
			Images = new List<ImageInfo>();
			Width = 1600;
			Height = 1200;
			BackColor = Color.Transparent;
			SoftEdges = true;
		}

		/// <summary>
		/// Calculates the image layout using a force-based approach.
		/// </summary>
		public void Arrange() {
			// reset image details
			Init();

			// even image distribution
			Distribute();

			PointF singularity = new PointF(Width/2, Height/2);

			int iteration = 0;
			float totalDisplacement;

			do {
				totalDisplacement = 0;
				iteration++;

				foreach (ImageInfo image in Images) {

					// calculate pulling force
					Vector netForce = new Vector(PULLING_FORCE, Vector.GetBearingAngle(image.Center, singularity));

					// calculate new velocity
					Vector oldVelocity = image.Velocity;
					image.Velocity = Vector.Add(Vector.Multiply(oldVelocity, DAMPING), netForce);

					// translate position
					PointF oldPosition = image.Location;
					Vector currentPosition = new Vector(Vector.CalcDistance(Point.Empty, image.Center), Vector.GetBearingAngle(Point.Empty, image.Center));
					image.Center = Vector.Add(currentPosition, image.Velocity).ToLocation();

					ImageInfo collidesWith;
					if (BoundsOverlap(image, out collidesWith)) {
						if (collidesWith != null) {
							// collision! bounce back in the opposite direction
							Vector bounceBack = new Vector(-image.Velocity.Magnitude, Vector.GetBearingAngle(image.Center, collidesWith.Center));
							netForce = Vector.Add(Vector.Multiply(netForce, ABSORBANCE), bounceBack);
							image.Velocity = Vector.Add(Vector.Multiply(oldVelocity, DAMPING), netForce);

							// try sliding along edges (given that we can't move to the target position)
							RectangleF tryX = new RectangleF(image.Location.X, oldPosition.Y, image.Width, image.Height);
							RectangleF tryY = new RectangleF(oldPosition.X, image.Location.Y, image.Width, image.Height);

							if (!ProposedBoundsOverlap(image, tryX))
								image.Location = tryX.Location;
							else if (!ProposedBoundsOverlap(image, tryY))
								image.Location = tryY.Location;
							else
								image.Location = oldPosition;
						}
					}

					totalDisplacement += Math.Abs(Vector.CalcDistance(image.Location, oldPosition));
				}
			}
			while ((totalDisplacement > MIN_DISPLACEMENT) && (iteration < MAX_ITERATIONS));
		}

		/// <summary>
		/// Returns a RectangleF representing the total bounds of the montage.
		/// </summary>
		/// <returns></returns>
		public RectangleF CalcTotalArea() {
			RectangleF total = RectangleF.Empty;

			foreach (ImageInfo image in Images) {
				if (total == RectangleF.Empty)
					total = image.Bounds;
				else
					total = RectangleF.Union(total, image.Bounds);
			}

			return total;
		}

		/// <summary>
		/// Determines whether the bounds of an image overlap any other images, and outputs the first collision.
		/// </summary>
		/// <param name="x"></param>
		/// <param name="collidesWith"></param>
		/// <returns></returns>
		private bool BoundsOverlap(ImageInfo x, out ImageInfo collidesWith) {
			bool found = false;

			RectangleF xBounds = x.Bounds;
			collidesWith = null;

			RectangleF overallBounds = new RectangleF(0, 0, Width, Height);
			if (!overallBounds.Contains(xBounds)) return true;

			foreach (ImageInfo y in Images) {
				if (x == y) continue;

				RectangleF yBounds = y.Bounds;
				if (xBounds.IntersectsWith(yBounds)) {
					collidesWith = y;
					return true;
				}
			}

			return found;
		}

		/// <summary>
		/// Determines whether the proposed bounds of an image overlaps with any other images.
		/// </summary>
		/// <param name="x"></param>
		/// <param name="proposedBounds"></param>
		/// <returns></returns>
		private bool ProposedBoundsOverlap(ImageInfo x, RectangleF proposedBounds) {
			bool found = false;

			RectangleF overallBounds = new RectangleF(0, 0, Width, Height);
			if (!overallBounds.Contains(proposedBounds)) return true;

			foreach (ImageInfo y in Images) {
				if (x == y) continue;

				RectangleF yBounds = y.Bounds;
				if (proposedBounds.IntersectsWith(yBounds)) return true;
			}

			return found;
		}

		/// <summary>
		/// Randomly distributes the images to provide their initial positions.
		/// </summary>
		private void Distribute() {
			ImageInfo dontCare;
            Random rnd = new Random(System.Guid.NewGuid().GetHashCode());

            if (InitialArrangement == InitialArrangement.Random) {
                foreach (ImageInfo image in Images) {
                    do {
                        image.X = rnd.Next(0, (int)Width);
                        image.Y = rnd.Next(0, (int)Height);
                    }
                    while (BoundsOverlap(image, out dontCare));
                }
            }
            else if (InitialArrangement == InitialArrangement.Uniform) {
                int n = Images.Count;

                int[] shuffleOrder = Enumerable.Range(0, n).ToArray<int>();
                for (int j = 0; j < n; j++) {
                    int rand_index = rnd.Next(j, n);
                    int ndx = shuffleOrder[rand_index];
                    shuffleOrder[rand_index] = shuffleOrder[j];
                    shuffleOrder[j] = ndx;
                }

                int i = 0;
                int divs = (int)Math.Ceiling(Math.Sqrt(n));
                for (int y = 0; y < divs; y++) {
                    for (int x = 0; x < divs; x++) {
                        ImageInfo image = Images[shuffleOrder[i]];

                        image.Center = new PointF((0.5f * x) * (Width / divs), (0.5f * y) * (Height / divs));

                        i++;
                        if (i == n) break;
                    }

                    if (i == n) break;
                }
            }
		}

        /// <summary>
        /// Renders the montage image as a Bitmap.
        /// </summary>
        /// <returns></returns>
        public Bitmap Render() {
            return RenderInternal(false);
        }

		/// <summary>
		/// Renders the montage image as a Bitmap.
		/// </summary>
		/// <returns></returns>
		private Bitmap RenderInternal(bool preview) {
			Bitmap bmp = new Bitmap((int)Width, (int)Height);

			RectangleF totalArea = CalcTotalArea();

			float scaleBy = Math.Min(Width / totalArea.Width, Height / totalArea.Height);

            using (Graphics g = Graphics.FromImage(bmp)) {
				g.Clear(BackColor);
                if (!preview)
                {
                    using (Bitmap back = new Bitmap(background_path))
                    {
                        g.DrawImage(back, new RectangleF(0, 0, Width, Height));
                    }
                }

				foreach (ImageInfo info in Images) {
					RectangleF bounds = info.Bounds;

					bounds.X -= totalArea.X;
					bounds.Y -= totalArea.Y;

					bounds.X *= scaleBy;
					bounds.Y *= scaleBy;
					bounds.Width *= scaleBy;
					bounds.Height *= scaleBy;

					bounds.X += (Width - (totalArea.Width * scaleBy)) / 2;
					bounds.Y += (Height - (totalArea.Height * scaleBy)) / 2;
					
					info.finalBounds = bounds;
                    if (preview) {
                        Rectangle integral = new Rectangle(new Point((int)bounds.X, (int)bounds.Y), bounds.Size.ToSize());
                        g.FillRectangle(Brushes.DarkGray, integral);
                        g.DrawRectangle(Pens.Gray, integral);
                    }
                    else {
                        using (Bitmap img = new Bitmap(info.Path)) {
													if (SoftEdges) {
															using (Bitmap soft = SoftEdgeBitmaps.MakeSoftEdgeBitmap(img, bounds.Size.ToSize())) {
																	g.DrawImage(soft, bounds);
															}
													}
													else {
															g.DrawImage(img, bounds);
													}
                        }
                    }
				}
			}

			return bmp;
		}

        /// <summary>
        /// Renders a preview of the layout of the montage image as a Bitmap.
        /// </summary>
        /// <returns></returns>
        public Bitmap Preview() {
            return RenderInternal(true);
        }

		/// <summary>
		/// Initialises the canvas.
		/// </summary>
		private void Init() {
			foreach (ImageInfo image in Images) {
				image.Reset();
			}
		}
	}

    /// <summary>
    /// Represents the different types of initial arrangement.
    /// </summary>
    public enum InitialArrangement {
        /// <summary>
        /// The initial layout is completely randomised. Checks are carried out to prevent overlapping images.
        /// </summary>
        Random,
        /// <summary>
        /// The initial layout conforms to a uniform grid, however the order of the image collection is shuffled.
        /// </summary>
        Uniform
    }

    /// <summary>
    /// Represents the different methods that can be used to set a tile's size.
    /// </summary>
    public enum TileSize {
        /// <summary>
        /// Calculates an intermediate size such that tile area is mostly uniform.
        /// </summary>
        Averaged,
        /// <summary>
        /// Scales the tiles such that variation in the horizontal dimension is preserved.
        /// </summary>
        PreserveHorizontal,
        /// <summary>
        /// Scales the tiles such that variation in the vertical dimension is preserved.
        /// </summary>
        PreserveVertical,
    }

	/// <summary>
	/// Represents an image in the montage.
	/// </summary>
	public class ImageInfo {

	public RectangleF finalBounds;


        /// <summary>
        /// Gets the path to the image.
        /// </summary>
        public bool Face { get; private set; }

        /// <summary>
        /// Gets the path to the image.
        /// </summary>
        public string Path { get; private set; }
		/// <summary>
		/// Gets the original width of the image.
		/// </summary>
		public float OriginalWidth { get; private set; }
		/// <summary>
		/// Gets the original height of the image.
		/// </summary>
		public float OriginalHeight { get; private set; }
		/// <summary>
		/// Calculates the width of the image given its new height (maintaining the aspect ratio).
		/// </summary>
		/// <param name="height"></param>
		/// <returns></returns>
		public float CalcWidth(float height) {
			float aspect = OriginalWidth / OriginalHeight;
			return height * aspect;
		}
		/// <summary>
		/// Calculates the height of the image given its new width (maintaining the aspect ratio).
		/// </summary>
		/// <param name="width"></param>
		/// <returns></returns>
		public float CalcHeight(float width) {
			float aspect = OriginalWidth / OriginalHeight;
			return width / aspect;
		}
		/// <summary>
		/// Gets or sets the height of the image on the canvas.
		/// </summary>
		public float Height { get; set; }
		/// <summary>
		/// Gets or sets the width of the image on the canvas.
		/// </summary>
		public float Width { get; set; }
		/// <summary>
		/// Gets or sets the horizontal position of the image on the canvas.
		/// </summary>
		public float X { get; set; }
		/// <summary>
		/// Gets or sets the vertical position of the image on the canvas.
		/// </summary>
		public float Y { get; set; }
		/// <summary>
		/// Gets or sets the relative size of the image compared to the others on the canvas.
		/// </summary>
		private float RelativeSize { get; set; }
		/// <summary>
		/// Gets or sets the top-left coordinate of the image on the canvas.
		/// </summary>
		public PointF Location {
			get {
				return new PointF(X, Y);
			}
			set {
				X = value.X;
				Y = value.Y;
			}
		}
		/// <summary>
		/// Gets or sets the location of the center of the image on the canvas.
		/// </summary>
		public PointF Center {
			get {
				return new PointF(X + (Width/2), Y + (Height/2));
			}
			set {
				X = value.X - Width / 2;
				Y = value.Y - Height / 2;
			}
		}
		/// <summary>
		/// Gets or sets the bounds of the image on the canvas.
		/// </summary>
		public RectangleF Bounds {
			get {
				return new RectangleF(X, Y, Width, Height);
			}
			set {
				X = value.X;
				Y = value.Y;
				Width = value.Width;
				Height = value.Height;
			}
		}
		/// <summary>
		/// Gets or sets the velocity of the image during the simulation.
		/// </summary>
		public Vector Velocity { get; set; }
        /// <summary>
        /// Gets or sets the method used to set the size of the image tile.
        /// </summary>
        public TileSize TileSize { get; set; }

        /// <summary>
        /// Creates a new instance of the ImageInfo class using the specified image file and relative size.
        /// </summary>
        /// <param name="face"></param>
        /// <param name="path"></param>
        /// <param name="relativeSize"></param>
        public ImageInfo(bool face, string path, float relativeSize = 100f, TileSize tileSize = TileSize.Averaged) {
            Face = face;
            RelativeSize = relativeSize / 2;
			Path = path;
            TileSize = tileSize;

            Size sz = ImageHeader.GetDimensions(Path);
            OriginalWidth = sz.Width;
            OriginalHeight = sz.Height;

			Reset();
		}

		/// <summary>
		/// Resets the properties of the image back to the defaults.
		/// </summary>
		public void Reset() {
			// reset position and velocity
            X = Y = 0f;
            Velocity = new Vector(0, 0);

            // images will be of a uniform size if we take the average of the
            // resulting sizes when either the width or height is fixed
			float w1 = RelativeSize;
			float h1 = CalcHeight(w1);
			float h2 = RelativeSize;
			float w2 = CalcWidth(h2);

            if (TileSize == TileSize.Averaged) {
                Width = (w1 + w2) / 2;
                Height = (h1 + h2) / 2;
            }
            else if (TileSize == TileSize.PreserveHorizontal) {
                Width = w2;
                Height = h2;
            }
            else if (TileSize == TileSize.PreserveVertical) {
                Width = w1;
                Height = h1;
            }
		}
	}
}
