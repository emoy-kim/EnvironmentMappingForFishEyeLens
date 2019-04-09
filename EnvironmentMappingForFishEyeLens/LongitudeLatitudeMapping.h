/*
 * Author: Emoy Kim
 * E-mail: emoy.kim_AT_gmail.com
 * 
 * This code is a free software; it can be freely used, changed and redistributed.
 * If you use any version of the code, please reference the code.
 * 
 */

#pragma once

#include <OpenCVLinker.h>

using namespace std;
using namespace cv;

class LongitudeLatitudeMapping
{
   
   void getBilinearInterpolatedColor(Vec3b& bgr_color, const Mat& image, const Vec2d& point) const;
   void getTextureCoordinates(Point2d& texture_point, const Point& image_point, const Size& image_size) const;
   
   void getSphereCoordinatesForFisheye(Point3d& on_sphere, const Point2d& longitude_latitude) const;
   void getFisheyeCoordinatesFromSphere(Point2d& fisheye_point, const Point3d& on_sphere) const;

   void getSphereCoordinatesForMirrorball(Point3d& on_sphere, const Point2d& longitude_latitude) const;
   void getMirrorballCoordinatesFromSphere(Point2d& mirrorball_point, const Point3d& on_sphere) const;

public:
   void convertFisheye(Mat& converted, const Mat& fisheye) const;
   void convertMirrorball(Mat& converted, const Mat& mirrorball) const;
};