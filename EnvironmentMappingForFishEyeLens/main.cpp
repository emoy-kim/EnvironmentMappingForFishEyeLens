#include "EnvironmentMapping.h"

int main()
{
   Mat image = imread("Samples/fisheye/sky.jpg");
   //Mat image = imread("Samples/mirrorball/hall.jpg");

   EnvironmentMapping EnvironmentMapper;
   EnvironmentMapper.play( image );

   return 0;
}