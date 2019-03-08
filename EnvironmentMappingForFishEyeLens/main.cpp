#include "EnvironmentMapping.h"

int main()
{
   const Mat image = imread("Samples/fisheye/sky.jpg");
   
   EnvironmentMapping EnvironmentMapper;
   EnvironmentMapper.play( image );

   return 0;
}