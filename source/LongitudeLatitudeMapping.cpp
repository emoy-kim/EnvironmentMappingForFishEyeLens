#include "LongitudeLatitudeMapping.h"

void LongitudeLatitudeMapping::getBilinearInterpolatedColor(
   cv::Vec3b& bgr_color, 
   const cv::Mat& image, 
   const cv::Vec2d& point
)
{
   const int x0 = static_cast<int>(floor( point(0) ));
   const int y0 = static_cast<int>(floor( point(1) ));
   const int x1 = std::min( x0 + 1, image.cols - 1 );
   const int y1 = std::min( y0 + 1, image.rows - 1 );
   const auto tx = point(0) - static_cast<double>(x0);
   const auto ty = point(1) - static_cast<double>(y0);

   const auto* curr_row = image.ptr<cv::Vec3b>(y0);
   const auto* next_row = image.ptr<cv::Vec3b>(y1);
   bgr_color = static_cast<cv::Vec3b>(
      static_cast<cv::Vec3d>(curr_row[x0]) * (1.0 - tx) * (1.0 - ty) + 
      static_cast<cv::Vec3d>(curr_row[x1]) * tx * (1.0 - ty) + 
      static_cast<cv::Vec3d>(next_row[x0]) * (1.0 - tx) * ty + 
      static_cast<cv::Vec3d>(next_row[x1]) * tx * ty
   );
}

void LongitudeLatitudeMapping::getTextureCoordinates(
   cv::Point2d& texture_point, 
   const cv::Point& image_point, 
   const cv::Size& image_size
)
{
   texture_point.x = image_point.x / static_cast<double>(image_size.width - 1);
   texture_point.y = 1.0 - image_point.y / static_cast<double>(image_size.height - 1);
}

void LongitudeLatitudeMapping::getSphereCoordinatesForFisheye(
   cv::Point3d& on_sphere, 
   const cv::Point2d& longitude_latitude
)
// The sphere coordinates is right-handed system, whose y-axis is up-vector and -z-axis view-vector.
// When the point is on +y-axis, theta is 0 radian. When the point is on -x-axis, phi is 0 radian.
// The range of phi is [0, PI].
{
   const double phi = longitude_latitude.x * CV_PI;
   const double theta = longitude_latitude.y * CV_PI;
   const double sin_phi = sin( phi );
   const double cos_phi = cos( phi );
   const double sin_theta = sin( theta );
   const double cos_theta = cos( theta );
   on_sphere.x = -sin_theta * cos_phi;
   on_sphere.y = cos_theta;
   on_sphere.z = -sin_theta * sin_phi;
}

void LongitudeLatitudeMapping::getFisheyeCoordinatesFromSphere(
   cv::Point2d& fisheye_point, 
   const cv::Point3d& on_sphere
)
// fisheye_point's range is [-1, 1]. It means that fisheye image which is circle has range of [-1, 1].
// Image point from 90 degree of incidence projected on the perimeter of circle whose radius is 1.
{
   const double fisheye_angle = atan2( 
      sqrt( on_sphere.x * on_sphere.x + on_sphere.y * on_sphere.y ), 
      abs( on_sphere.z )
   );
   const double radius = fisheye_angle * 2.0 / CV_PI;
   const double rotation_angle_on_image_plane = atan2( on_sphere.y, on_sphere.x );
   fisheye_point.x = radius * cos( rotation_angle_on_image_plane );
   fisheye_point.y = radius * sin( rotation_angle_on_image_plane );
}

void LongitudeLatitudeMapping::convertFisheye(cv::Mat& converted, const cv::Mat& fisheye)
{
   std::cout << ">> Convert Fisheye Image to Longitude-Latitude Image...\n";
   converted = cv::Mat::zeros( fisheye.size(), fisheye.type() );
   for (int j = 0; j < converted.rows; ++j) {
      auto* converted_ptr = converted.ptr<cv::Vec3b>(j);
      for (int i = 0; i < converted.cols; ++i) {
         cv::Point2d texture_point;
         getTextureCoordinates( texture_point, { i, j }, converted.size() );

         cv::Point3d on_sphere;
         getSphereCoordinatesForFisheye( on_sphere, texture_point );

         cv::Point2d fisheye_point;
         getFisheyeCoordinatesFromSphere( fisheye_point, on_sphere );
         if (fisheye_point.x * fisheye_point.x + fisheye_point.y * fisheye_point.y > 1.0) continue;

         cv::Point2d fisheye_image_point;
         fisheye_image_point.x = (fisheye_point.x + 1.0) * 0.5 * fisheye.cols;
         fisheye_image_point.y = (fisheye_point.y + 1.0) * 0.5 * fisheye.rows;

         if (fisheye_image_point.x < 0 || fisheye_image_point.x >= fisheye.cols ||
             fisheye_image_point.y < 0 || fisheye_image_point.y >= fisheye.rows) continue;

         cv::Vec3b bgr_color;
         getBilinearInterpolatedColor( bgr_color, fisheye, fisheye_image_point );
         converted_ptr[i] = bgr_color;
      }
   }
   std::cout << ">> Converting Done.\n\n";
}

void LongitudeLatitudeMapping::getSphereCoordinatesForMirrorball(
   cv::Point3d& on_sphere, 
   const cv::Point2d& longitude_latitude
)
// The sphere coordinates is right-handed system, whose y-axis is up-vector and -z-axis view-vector.
// When the point is on +y-axis, theta is 0 radian. When the point is on +x-axis, phi is 0 radian.
// The range of phi is [0, 2PI].
{
   const double phi = longitude_latitude.x * CV_2PI;
   const double theta = longitude_latitude.y * CV_PI;
   const double sin_phi = sin( phi );
   const double cos_phi = cos( phi );
   const double sin_theta = sin( theta );
   const double cos_theta = cos( theta );
   on_sphere.x = sin_theta * cos_phi;
   on_sphere.y = cos_theta;
   on_sphere.z = -sin_theta * sin_phi;
}

void LongitudeLatitudeMapping::getMirrorballCoordinatesFromSphere(
   cv::Point2d& mirrorball_point, 
   const cv::Point3d& on_sphere
)
{
   const double coef = 1.0 / sqrt(2.0 * (1.0 + on_sphere.z));
   mirrorball_point.x = on_sphere.x * coef;
   mirrorball_point.y = on_sphere.y * coef;
}


void LongitudeLatitudeMapping::convertMirrorball(cv::Mat& converted, const cv::Mat& mirrorball)
{
   converted = cv::Mat::zeros( mirrorball.rows, mirrorball.cols * 2, mirrorball.type() );
   for (int j = 0; j < converted.rows; ++j) {
      auto* converted_ptr = converted.ptr<cv::Vec3b>(j);
      for (int i = 0; i < converted.cols; ++i) {
         cv::Point2d texture_point;
         getTextureCoordinates( texture_point, { i, j }, converted.size() );

         cv::Point3d on_sphere;
         getSphereCoordinatesForMirrorball( on_sphere, texture_point );

         cv::Point2d mirrorball_point;
         getMirrorballCoordinatesFromSphere( mirrorball_point, on_sphere );
         if (mirrorball_point.x * mirrorball_point.x + mirrorball_point.y * mirrorball_point.y > 1.0) continue;

         cv::Point2d mirrorball_image_point;
         mirrorball_image_point.x = (mirrorball_point.x + 1.0) * 0.5 * mirrorball.cols;
         mirrorball_image_point.y = (mirrorball_point.y + 1.0) * 0.5 * mirrorball.rows;

         if (mirrorball_image_point.x < 0 || mirrorball_image_point.x >= mirrorball.cols ||
             mirrorball_image_point.y < 0 || mirrorball_image_point.y >= mirrorball.rows) continue;

         cv::Vec3b bgr_color;
         getBilinearInterpolatedColor( bgr_color, mirrorball, mirrorball_image_point );
         converted_ptr[i] = bgr_color;
      }
   }
}