#pragma once

#include <OpenCVLinker.h>
#include <map>

using namespace std;
using namespace cv;

class LightPosition
{
   int LightNum;
   Mat AdjustedIntensities;
   map<float, Point> LightInfos;

   int getNextHighestPowerOf2(const int& number) const;

   void drawLightPosition(Mat& image, const Point& light_position) const;
   void drawBlockLine(Mat& image, const Point& start, const Point& end) const;

   void adjustIntensities(const Mat& longitude_latitude);

   void calculateDeltaXDividingIntensityInHalf(
      int& dx,
      const Mat& adjusted, 
      const Range& row_range,
      const float& half_intensity
   ) const;
   void calculateDeltaYDividingIntensityInHalf(
      int& dy,
      const Mat& adjusted, 
      const Range& col_range,
      const float& half_intensity
   ) const;
   void medianCut(Mat& longitude_latitude, const Rect& block, const int& iteration);

   float calculateVariance(const Mat& adjusted, const Point& center, const Range& col_range, const Range& row_range) const;
   void calculateDeltaXMinimizingVariance(
      int& dx,
      Point& left_prev_point,
      Point& right_prev_point,
      const Mat& adjusted, 
      const float& total_intensity
   ) const;
   void calculateDeltaYMinimizingVariance(
      int& dy,
      Point& top_prev_point,
      Point& bottom_prev_point,
      const Mat& adjusted, 
      const float& total_intensity
   ) const;
   void varianceCut(Mat& longitude_latitude, Point& prev_point, const Rect& block, const int& iteration);


public:
   LightPosition();

   void estimateLightPositions(
      vector<Point>& light_points,  
      const Mat& longitude_latitude,
      const int& light_num_to_find,
      const bool& use_median_cut = true
   );
};