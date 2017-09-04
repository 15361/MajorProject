using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;

namespace ImageCollage {

	public struct Vector {

		private float mMagnitude;

		public float Magnitude {
			get { return mMagnitude; }
			set { mMagnitude = value; }
		}

		private float mDirection;

		public float Direction {
			get { return mDirection; }
			set { mDirection = value; }
		}

		public Vector(float magnitude, float direction) {
			mMagnitude = magnitude;
			mDirection = direction;

			if (mMagnitude < 0) {
				mMagnitude = -mMagnitude;
				mDirection = (180 + mDirection) % 360;
			}
			if (mDirection < 0) mDirection = (360 + mDirection);
		}

		public static Vector Add(Vector a, Vector b) {
			double aX = a.Magnitude * Math.Cos((Math.PI / 180.0) * (double)a.Direction);
			double aY = a.Magnitude * Math.Sin((Math.PI / 180.0) * (double)a.Direction);

			double bX = b.Magnitude * Math.Cos((Math.PI / 180.0) * (double)b.Direction);
			double bY = b.Magnitude * Math.Sin((Math.PI / 180.0) * (double)b.Direction);

			aX += bX;
			aY += bY;

			double magnitude = Math.Sqrt(Math.Pow(aX, 2) + Math.Pow(aY, 2));
			if (Double.IsNaN(magnitude) || Double.IsInfinity(magnitude)) System.Diagnostics.Debugger.Break();

			double direction;
			if (magnitude == 0)
				direction = 0;
			else
				direction = (180.0 / Math.PI) * Math.Atan2(aY, aX);

			if (Double.IsNaN(direction) || Double.IsInfinity(direction)) System.Diagnostics.Debugger.Break();

			return new Vector((float)magnitude, (float)direction);
		}

		public static Vector Multiply(Vector a, float b) {
			return new Vector(a.Magnitude * b, a.Direction);
		}

		public PointF ToLocation() {
			float aX = mMagnitude * (float)Math.Cos((Math.PI / 180.0) * (double)mDirection);
			float aY = mMagnitude * (float)Math.Sin((Math.PI / 180.0) * (double)mDirection);

			return new PointF(aX, aY);
		}

		public override string ToString() {
			return mMagnitude.ToString("N5") + " " + mDirection.ToString("N2") + "°";
		}

		public static float GetBearingAngle(PointF start, PointF end) {
			PointF half = new PointF(start.X + ((end.X - start.X) / 2), start.Y + ((end.Y - start.Y) / 2));

			double diffX = (double)(half.X - start.X);
			double diffY = (double)(half.Y - start.Y);

			if (diffY != 0)
				return (float)(Math.Atan2(diffY, diffX) * (180.0 / Math.PI));
			else
				return (diffX < 0) ? 180f : 0f;
		}

		public static float CalcDistance(PointF a, PointF b) {
			double xDist = (a.X - b.X);
			double yDist = (a.Y - b.Y);
			return (float)Math.Sqrt(Math.Pow(xDist, 2) + Math.Pow(yDist, 2));
		}

	}

}
