/*
 * Author: Emoy Kim
 * E-mail: emoy.kim_AT_gmail.com
 * 
 * This code is a free software; it can be freely used, changed and redistributed.
 * If you use any version of the code, please reference the code.
 * 
 * 
 * NOTE:
 *   The algorithm is based on median-cut[1] and variance-cut[2].
 *   
 *   
 * [1] http://gl.ict.usc.edu/Research/MedianCut/
 * [2] http://gl.ict.usc.edu/Research/VarianceMin/
 * 
 */

#pragma once

#include "_Common.h"

class LightPosition
{
public:
   LightPosition();
   ~LightPosition() = default;

   void estimateLightPositions(
      std::vector<cv::Point>& light_points,  
      const cv::Mat& longitude_latitude,
      int light_num_to_find,
      bool use_median_cut = true
   );

private:
   int LightNum;
   cv::Mat AdjustedIntensities;
   std::map<float, cv::Point> LightInfos;

   static int getNextHighestPowerOf2(int number);

   void drawLightPosition(cv::Mat& image, const cv::Point& light_position) const;
   void drawBlockLine(cv::Mat& image, const cv::Point& start, const cv::Point& end) const;

   void adjustIntensities(const cv::Mat& longitude_latitude);

   void calculateDeltaXDividingIntensityInHalf(
      int& dx,
      const cv::Mat& adjusted, 
      const cv::Range& row_range,
      float half_intensity
   ) const;
   void calculateDeltaYDividingIntensityInHalf(
      int& dy,
      const cv::Mat& adjusted, 
      const cv::Range& col_range,
      float half_intensity
   ) const;
   void medianCut(cv::Mat& longitude_latitude, const cv::Rect& block, int iteration);

   [[nodiscard]] float calculateVariance(
      const cv::Mat& adjusted, 
      const cv::Point& center, 
      const cv::Range& col_range, 
      const cv::Range& row_range
   ) const;
   void calculateDeltaXMinimizingVariance(
      int& dx,
      cv::Point& left_prev_point,
      cv::Point& right_prev_point,
      const cv::Mat& adjusted, 
      float total_intensity
   ) const;
   void calculateDeltaYMinimizingVariance(
      int& dy,
      cv::Point& top_prev_point,
      cv::Point& bottom_prev_point,
      const cv::Mat& adjusted, 
      float total_intensity
   ) const;
   void varianceCut(cv::Mat& longitude_latitude, cv::Point& prev_point, const cv::Rect& block, int iteration);
};