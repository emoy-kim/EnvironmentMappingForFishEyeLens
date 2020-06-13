#include "Renderer.h"

int main()
{
   const std::string sample_directory_path = std::string(CMAKE_SOURCE_DIR) + "/samples";
   const cv::Mat image = cv::imread( sample_directory_path + "/fisheye/sky.jpg" );

   RendererGL renderer;
   renderer.play( image );
   return 0;
}