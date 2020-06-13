#include "LightPosition.h"

LightPosition::LightPosition() : LightNum( 0 )
{
   
}

int LightPosition::getNextHighestPowerOf2(int number)
{ 
   uint value = static_cast<uint>(number) - 1;
   value |= value >> 1;
   value |= value >> 2;
   value |= value >> 4;
   value |= value >> 8;
   value |= value >> 16;
   return static_cast<int>(value + 1);
}

void LightPosition::drawLightPosition(cv::Mat& image, const cv::Point& light_position) const
{
   cv::circle( image, light_position, 3, cv::Scalar(0, 255, 255), -1 );
}

void LightPosition::drawBlockLine(cv::Mat& image, const cv::Point& start, const cv::Point& end) const
{
   cv::line( image, start, end, cv::Scalar(0, 255, 0), 1 );
}

void LightPosition::adjustIntensities(const cv::Mat& longitude_latitude)
{
   cv::Mat gray;
   cv::cvtColor( longitude_latitude, gray, cv::COLOR_BGR2GRAY );

   AdjustedIntensities.create( longitude_latitude.size(), CV_32FC1 );
   const double scale = 1.0 / static_cast<double>(gray.rows - 1);
   for (int j = 0; j < gray.rows; ++j) {
      const auto adjuster = static_cast<float>(sin( static_cast<double>(j) * scale * CV_PI ));
      const auto* gray_ptr = gray.ptr<uchar>(j);
      auto* adjusted_ptr = AdjustedIntensities.ptr<float>(j);
      for (int i = 0; i < gray.cols; ++i) {
         adjusted_ptr[i] = adjuster * static_cast<float>(gray_ptr[i]);
      }
   }
}

void LightPosition::calculateDeltaXDividingIntensityInHalf(
   int& dx,
   const cv::Mat& adjusted, 
   const cv::Range& row_range,
   float half_intensity
) const
{
   float half_sum_from_x = 0.0f;
   while (half_sum_from_x < half_intensity && dx < adjusted.cols) {
      dx++;
      for (int j = row_range.start; j < row_range.end; ++j) {
         half_sum_from_x += adjusted.at<float>(j, dx);
      }
   }
}

void LightPosition::calculateDeltaYDividingIntensityInHalf(
   int& dy,
   const cv::Mat& adjusted, 
   const cv::Range& col_range,
   float half_intensity
) const
{
   float half_sum_from_y = 0.0f;
   while (half_sum_from_y < half_intensity && dy < adjusted.rows) {
      dy++;
      const auto* adjusted_ptr = adjusted.ptr<float>(dy);
      for (int i = col_range.start; i < col_range.end; ++i) {
         half_sum_from_y += adjusted_ptr[i];
      }
   }
}

void LightPosition::medianCut(cv::Mat& longitude_latitude, const cv::Rect& block, int iteration)
{
   if (block.x >= longitude_latitude.cols || block.y >= longitude_latitude.rows || block.width == 0 || block.height == 0) return;

   int dx = -1, dy = -1;
   const cv::Mat adjusted_block = AdjustedIntensities(block);
   const float half_intensity = static_cast<float>(cv::sum( adjusted_block )[0]) * 0.5f;

   if (iteration == 0) {
      calculateDeltaXDividingIntensityInHalf( dx, adjusted_block, { 0, adjusted_block.rows }, half_intensity );
      calculateDeltaYDividingIntensityInHalf( dy, adjusted_block, { 0, adjusted_block.cols }, half_intensity );

      const cv::Point light_position(block.x + dx, block.y + dy);
      const float intensity = AdjustedIntensities.at<float>(light_position.y, light_position.x);
      LightInfos[intensity] = light_position;
      drawLightPosition( longitude_latitude, light_position );
   }
   else if (block.width > block.height) {
      calculateDeltaXDividingIntensityInHalf( dx, adjusted_block, { 0, adjusted_block.rows }, half_intensity );
      drawBlockLine( longitude_latitude, { dx, block.tl().y }, { dx, block.br().y } );
      
      const cv::Rect left_block(block.x, block.y, dx, block.height);
      const cv::Rect right_block(block.x + dx, block.y, block.width - dx, block.height);
      medianCut( longitude_latitude, left_block, iteration - 1 );
      medianCut( longitude_latitude, right_block, iteration - 1 );
   }
   else {
      calculateDeltaYDividingIntensityInHalf( dy, adjusted_block, { 0, adjusted_block.cols }, half_intensity );
      drawBlockLine( longitude_latitude, { block.tl().x, dy }, { block.br().x, dy } );

      const cv::Rect top_block(block.x, block.y, block.width, dy);
      const cv::Rect bottom_block(block.x, block.y + dy, block.width, block.height - dy);
      medianCut( longitude_latitude, top_block, iteration - 1 );
      medianCut( longitude_latitude, bottom_block, iteration - 1 );
   }
}

float LightPosition::calculateVariance(
   const cv::Mat& adjusted, 
   const cv::Point& center, 
   const cv::Range& col_range, 
   const cv::Range& row_range
) const
{
   float variance = 0.0f;
   for (int j = row_range.start; j < row_range.end; ++j) {
      const auto* adjusted_ptr = adjusted.ptr<float>(j);
      for (int i = col_range.start; i < col_range.end; ++i) {
         variance += adjusted_ptr[i] * ((i - center.x) * (i - center.x) + (j - center.y) * (j - center.y));
      }
   }
   return sqrt( variance / (col_range.size() * row_range.size()) );
}

void LightPosition::calculateDeltaXMinimizingVariance(
   int& dx,
   cv::Point& left_prev_point,
   cv::Point& right_prev_point,
   const cv::Mat& adjusted, 
   float total_intensity
) const
{
   float left_total = 0.0f;
   float min_of_max_variance = std::numeric_limits<float>::max();
   for (int x = 0; x < adjusted.cols - 1; ++x) {
      for (int j = 0; j < adjusted.rows; ++j) {
         left_total += adjusted.at<float>(j, x);
      }

      int left_dx = -1, left_dy = -1;
      const cv::Range left_col_range(0, x + 1);
      const cv::Range left_row_range(0, adjusted.rows);
      const float left_half_intensity = left_total * 0.5f;
      calculateDeltaXDividingIntensityInHalf( left_dx, adjusted, left_row_range, left_half_intensity );
      calculateDeltaYDividingIntensityInHalf( left_dy, adjusted, left_col_range, left_half_intensity );
      const float max_left_variance = calculateVariance( adjusted, { left_dx, left_dy }, left_col_range, left_row_range );

      int right_dx = x, right_dy = -1;
      const cv::Range right_col_range(x + 1, adjusted.cols);
      const cv::Range right_row_range(0, adjusted.rows);
      const float right_half_intensity = (total_intensity - left_total) * 0.5f;
      calculateDeltaXDividingIntensityInHalf( right_dx, adjusted, right_row_range, right_half_intensity );
      calculateDeltaYDividingIntensityInHalf( right_dy, adjusted, right_col_range, right_half_intensity );
      const float max_right_variance = calculateVariance( adjusted, { right_dx, right_dy }, right_col_range, right_row_range );

      const float max_variance = std::max( max_left_variance, max_right_variance );
      if (min_of_max_variance > max_variance) {
         min_of_max_variance = max_variance;
         dx = x;
         left_prev_point = { left_dx, right_dx };
         right_prev_point = { right_dx, right_dy };
      }
   }
}

void LightPosition::calculateDeltaYMinimizingVariance(
   int& dy,
   cv::Point& top_prev_point,
   cv::Point& bottom_prev_point,
   const cv::Mat& adjusted, 
   float total_intensity
) const
{
   float top_total = 0.0f;
   float min_of_max_variance = std::numeric_limits<float>::max();
   for (int y = 0; y < adjusted.rows - 1; ++y) {
      const auto* adjusted_ptr = adjusted.ptr<float>(y);
      for (int i = 0; i < adjusted.cols; ++i) {
         top_total += adjusted_ptr[i];
      }

      int top_dx = -1, top_dy = -1;
      const cv::Range top_col_range(0, adjusted.cols);
      const cv::Range top_row_range(0, y + 1);
      const float top_half_intensity = top_total * 0.5f;
      calculateDeltaXDividingIntensityInHalf( top_dx, adjusted, top_row_range, top_half_intensity );
      calculateDeltaYDividingIntensityInHalf( top_dy, adjusted, top_col_range, top_half_intensity );
      const float max_left_variance = calculateVariance( adjusted, { top_dx, top_dy }, top_col_range, top_row_range );

      int bottom_dx = -1, bottom_dy = y;
      const cv::Range bottom_col_range(0, adjusted.cols);
      const cv::Range bottom_row_range(y + 1, adjusted.rows);
      const float bottom_half_intensity = (total_intensity - top_total) * 0.5f;
      calculateDeltaXDividingIntensityInHalf( bottom_dx, adjusted, bottom_row_range, bottom_half_intensity );
      calculateDeltaYDividingIntensityInHalf( bottom_dy, adjusted, bottom_col_range, bottom_half_intensity );
      const float max_right_variance = calculateVariance( adjusted, { bottom_dx, bottom_dy }, bottom_col_range, bottom_row_range );

      const float max_variance = std::max( max_left_variance, max_right_variance );
      if (min_of_max_variance > max_variance) {
         min_of_max_variance = max_variance;
         dy = y;
         top_prev_point = { top_dx, top_dy };
         bottom_prev_point = { bottom_dx, bottom_dy };
      }
   }
}

void LightPosition::varianceCut(cv::Mat& longitude_latitude, cv::Point& prev_point, const cv::Rect& block, int iteration)
{
   if (block.x >= longitude_latitude.cols || block.y >= longitude_latitude.rows || block.width == 0 || block.height == 0) return;

   int dx, dy;
   cv::Point left_prev_point, right_prev_point;
   cv::Point top_prev_point, bottom_prev_point;
   const cv::Mat adjusted_block = AdjustedIntensities(block);
   const float total_intensity = static_cast<float>(cv::sum( adjusted_block )[0]);

   if (iteration == 0) {
      if (prev_point.x == 0 || prev_point.y == 0) {
         calculateDeltaXMinimizingVariance( dx, left_prev_point, right_prev_point, adjusted_block, total_intensity );
         calculateDeltaYMinimizingVariance( dy, top_prev_point, bottom_prev_point, adjusted_block, total_intensity );
         prev_point.x = dx;
         prev_point.y = dy;
      }

      const cv::Point light_position = prev_point;
      const float intensity = AdjustedIntensities.at<float>(light_position.y, light_position.x);
      LightInfos[intensity] = light_position;
      drawLightPosition( longitude_latitude, light_position );
   }
   else if (block.width > block.height) {
      calculateDeltaXMinimizingVariance( dx, left_prev_point, right_prev_point, adjusted_block, total_intensity );
      drawBlockLine( longitude_latitude, { dx, block.tl().y }, { dx, block.br().y } );
      
      const cv::Rect left_block(block.x, block.y, dx, block.height);
      const cv::Rect right_block(block.x + dx, block.y, block.width - dx, block.height);
      varianceCut( longitude_latitude, left_prev_point, left_block, iteration - 1 );
      varianceCut( longitude_latitude, right_prev_point, right_block, iteration - 1 );
   }
   else {
      calculateDeltaYMinimizingVariance( dy, top_prev_point, bottom_prev_point, adjusted_block, total_intensity );
      drawBlockLine( longitude_latitude, { block.tl().x, dy }, { block.br().x, dy } );

      const cv::Rect top_block(block.x, block.y, block.width, dy);
      const cv::Rect bottom_block(block.x, block.y + dy, block.width, block.height - dy);
      varianceCut( longitude_latitude, top_prev_point, top_block, iteration - 1 );
      varianceCut( longitude_latitude, bottom_prev_point, bottom_block, iteration - 1 );
   }
}

void LightPosition::estimateLightPositions(
   std::vector<cv::Point>& light_points,
   const cv::Mat& longitude_latitude, 
   int light_num_to_find, 
   bool use_median_cut
)
{
   std::cout << ">> Find Light Positions...\n";
   LightInfos.clear();
   LightNum = getNextHighestPowerOf2( light_num_to_find );
   const int iteration = LightNum == 0 ? 0 : static_cast<int>(log2( LightNum ));

   adjustIntensities( longitude_latitude );

   cv::Mat result = longitude_latitude.clone();
   if (use_median_cut) medianCut( result, { 0, 0, result.cols, result.rows }, iteration );

   light_points.clear();
   for (auto it = LightInfos.begin(); it != std::next( LightInfos.begin(), light_num_to_find ); ++it) {
      light_points.emplace_back( it->second );
   }
   cv::imshow( "Light Positions", result );
   std::cout << ">> Finding Done.\n\n";
}