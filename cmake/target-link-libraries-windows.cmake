target_link_libraries(EnvironmentMapping glad glfw3dll)

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
   target_link_libraries(EnvironmentMapping FreeImaged opencv_cored opencv_imgprocd opencv_imgcodecsd opencv_highguid)
else()
   target_link_libraries(EnvironmentMapping FreeImage opencv_core opencv_imgproc opencv_imgcodecs opencv_highgui)
endif()