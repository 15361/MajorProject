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
	/// Functionality for drawing a bitmap with soft edges. 
	/// Code by Dan Byström.
	/// Sourced from: http://danbystrom.se/2008/08/24/soft-edged-images-in-gdi/
	/// </summary>
	public static class SoftEdgeBitmaps {

		private enum ChannelARGB {
			Blue = 0,
			Green = 1,
			Red = 2,
			Alpha = 3
		}

		private static GraphicsPath CreateRoundRect(int x, int y, int width, int height, int radius) {
			GraphicsPath gp = new GraphicsPath();

			if (radius == 0)
			{
				gp.AddRectangle(new Rectangle(x, y, width, height));
				Console.WriteLine("Rectangle");
			}
			else {
				gp.AddLine(x + radius, y, x + width - radius, y);
				gp.AddArc(x + width - radius, y, radius, radius, 270, 90);
				gp.AddLine(x + width, y + radius, x + width, y + height - radius);
				gp.AddArc(x + width - radius, y + height - radius, radius, radius, 0, 90);
				gp.AddLine(x + width - radius, y + height, x + radius, y + height);
				gp.AddArc(x, y + height - radius, radius, radius, 90, 90);
				gp.AddLine(x, y + height - radius, x, y + radius);
				gp.AddArc(x, y, radius, radius, 180, 90);
				gp.CloseFigure();
				Console.WriteLine(gp.ToString());
			}
			return gp;
		}

		private static Brush CreateFluffyBrush(
			Rectangle gp,
			float[] blendPositions,
			float[] blendFactors,
			bool horizontal = false) {
			LinearGradientBrush pgb = new LinearGradientBrush(gp, Color.Black, Color.White, horizontal ? LinearGradientMode.Horizontal : LinearGradientMode.Vertical);
			Blend blend = new Blend();
			blend.Positions = blendPositions;
			blend.Factors = blendFactors;
			pgb.Blend = blend;
//			pgb.CenterColor = Color.Black;
//			pgb.SurroundColors = new Color[] { Color.White };
			return pgb;
		}

		private static void TransferARGBChannel(
			Bitmap source,
			Bitmap dest,
			ChannelARGB sourceChannel,
			ChannelARGB destChannel) {
			if (source.Size != dest.Size)
				throw new ArgumentException();
			Rectangle r = new Rectangle(Point.Empty, source.Size);
			BitmapData bdSrc = source.LockBits(r, ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
			BitmapData bdDst = dest.LockBits(r, ImageLockMode.ReadWrite, PixelFormat.Format32bppArgb);
			unsafe {
				byte* bpSrc = (byte*)bdSrc.Scan0.ToPointer();
				byte* bpDst = (byte*)bdDst.Scan0.ToPointer();
				bpSrc += (int)sourceChannel;
				bpDst += (int)destChannel;
				for (int i = r.Height * r.Width; i > 0; i--) {
					*bpDst = *bpSrc;
					bpSrc += 4;
					bpDst += 4;
				}
			}
			source.UnlockBits(bdSrc);
			dest.UnlockBits(bdDst);
		}

		/// <summary>
		/// Returns a Bitmap with soft edges based on the specified Bitmap and target dimensions.
		/// </summary>
		/// <param name="bmpOriginal"></param>
		/// <param name="targetSize"></param>
		/// <returns></returns>
		public static Bitmap MakeSoftEdgeBitmap(Bitmap bmpOriginal, Size targetSize = default(Size)) {
			if (targetSize.IsEmpty) targetSize = bmpOriginal.Size;

			Bitmap bmpFluffy = new Bitmap(bmpOriginal, targetSize);
			Rectangle r = new Rectangle(Point.Empty, bmpFluffy.Size);

			using (Bitmap bmpMask = new Bitmap(r.Width, r.Height))
			using (Graphics g = Graphics.FromImage(bmpMask))
//			using (GraphicsPath path = CreateRoundRect(
//				r.X, r.Y,
//				r.Width, r.Height,
//				Math.Min(r.Width, r.Height) / 5))
			using (Brush brush = CreateFluffyBrush(
				r,
				new float[] { 0.0f, 0.15f, 0.2f, 0.5f, 0.8f, 0.85f, 1.0f },
				new float[] { 0.0f, 0.1f, 0.9f, 1.0f, 0.9f, 0.1f, 0.0f })) {
                        using (Brush opaque = CreateFluffyBrush(
                                r,
				new float[] { 0.0f, 0.15f, 0.2f, 0.5f, 0.8f, 0.85f, 1.0f },
                                new float[] { 0.0f, 0.1f, 0.9f, 1.0f, 0.9f, 0.1f, 0.0f }, true)) {
				g.FillRectangle(Brushes.White, r);
				g.SmoothingMode = SmoothingMode.HighQuality;
				PointF top_left = new PointF(0f, 0f);
				PointF top_right = new PointF((float)r.Width, 0f);
                                PointF bottom_left = new PointF(0f, (float)r.Height);
                                PointF bottom_right = new PointF((float)r.Width, (float)r.Height);
                                PointF centre = new PointF((float)r.Width / 2, (float)r.Height / 2);
				g.FillPolygon(brush, new PointF[] { top_left, top_right, centre });
                                g.FillPolygon(brush, new PointF[] { bottom_left, bottom_right, centre });
                                g.FillPolygon(opaque, new PointF[] { top_left, bottom_left, centre });
                                g.FillPolygon(opaque, new PointF[] { top_right, bottom_right, centre });

				Pen pen = new Pen(brush, 10f);
                                g.DrawPolygon(pen, new PointF[] { top_left, top_right, centre });
                                g.DrawPolygon(pen, new PointF[] { bottom_left, bottom_right, centre });
//				g.FillRectangle(opaque, new Rectangle(r.X, r.Y + r.Height / 5, r.Width, r.Height - r.Height * 2 / 5 ));
//				g.DrawPath(new Pen(Brushes.LimeGreen, 10), path);
				TransferARGBChannel(
					bmpMask,
					bmpFluffy,
					ChannelARGB.Blue,
					ChannelARGB.Alpha);
			}
			}

			return bmpFluffy;
		}
	}
}
