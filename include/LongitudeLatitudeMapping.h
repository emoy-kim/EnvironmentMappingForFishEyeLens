/*
 * Author: Emoy Kim
 * E-mail: emoy.kim_AT_gmail.com
 * 
 * This code is a free software; it can be freely used, changed and redistributed.
 * If you use any version of the code, please reference the code.
 * 
 */

#pragma once

#include "_Common.h"

class LongitudeLatitudeMapping
{
public:
   LongitudeLatitudeMapping() = default;
   ~LongitudeLatitudeMapping() = default;

   static void convertFisheye(cv::Mat& converted, const cv::Mat& fisheye);
   static void convertMirrorball(cv::Mat& converted, const cv::Mat& mirrorball);

private:
   static void getBilinearInterpolatedColor(cv::Vec3b& bgr_color, const cv::Mat& image, const cv::Vec2d& point);
   static void getTextureCoordinates(cv::Point2d& texture_point, const cv::Point& image_point, const cv::Size& image_size);
   
   static void getSphereCoordinatesForFisheye(cv::Point3d& on_sphere, const cv::Point2d& longitude_latitude);
   static void getFisheyeCoordinatesFromSphere(cv::Point2d& fisheye_point, const cv::Point3d& on_sphere);

   static void getSphereCoordinatesForMirrorball(cv::Point3d& on_sphere, const cv::Point2d& longitude_latitude);
   static void getMirrorballCoordinatesFromSphere(cv::Point2d& mirrorball_point, const cv::Point3d& on_sphere);
};