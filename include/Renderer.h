/*
 * Author: Emoy Kim
 * E-mail: emoy.kim_AT_gmail.com
 *
 */

#pragma once

#include "_Common.h"
#include "Light.h"
#include "Object.h"
#include "LongitudeLatitudeMapping.h"
#include "LightPosition.h"

class RendererGL
{
public:
   RendererGL(const RendererGL&) = delete;
   RendererGL(const RendererGL&&) = delete;
   RendererGL& operator=(const RendererGL&) = delete;
   RendererGL& operator=(const RendererGL&&) = delete;


   RendererGL();
   ~RendererGL() = default;

   void play(const cv::Mat& fisheye);

private:
   inline static RendererGL* Renderer = nullptr;
   GLFWwindow* Window;
   bool DrawMovingObject;
   int FrameWidth;
   int FrameHeight;
   int ActivatedLightIndex;
   int TigerIndex;
   int TigerRotationAngle;
   float EnvironmentRadius;
   glm::ivec2 ClickedPoint;
   std::unique_ptr<CameraGL> MainCamera;
   std::unique_ptr<ShaderGL> ObjectShader;
   std::unique_ptr<ShaderGL> EnvironmentShader;
   std::unique_ptr<ObjectGL> EnvironmentObject;
   std::unique_ptr<ObjectGL> CowObject;
   std::vector<std::unique_ptr<ObjectGL>> MovingTigerObjects;
   std::unique_ptr<LightGL> Lights;
   std::unique_ptr<LightPosition> LightFinder;
   std::unique_ptr<LongitudeLatitudeMapping> LongitudeLatitudeMapper;
 
   void registerCallbacks() const;
   void initialize();

   static void printOpenGLInformation();

   void error(int error, const char* description) const;
   void cleanup(GLFWwindow* window);
   void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
   void cursor(GLFWwindow* window, double xpos, double ypos);
   void mouse(GLFWwindow* window, int button, int action, int mods);
   void mousewheel(GLFWwindow* window, double xoffset, double yoffset) const;
   void reshape(GLFWwindow* window, int width, int height) const;
   static void errorWrapper(int error, const char* description);
   static void cleanupWrapper(GLFWwindow* window);
   static void keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
   static void cursorWrapper(GLFWwindow* window, double xpos, double ypos);
   static void mouseWrapper(GLFWwindow* window, int button, int action, int mods);
   static void mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset);
   static void reshapeWrapper(GLFWwindow* window, int width, int height);

   void setEnvironmentObject(const cv::Mat& texture);
   void setMovingTigerObjects(const cv::Mat& texture);
   void setCowObject(const cv::Mat& texture);
   
   void findLightsAndGetTexture(cv::Mat& texture, const cv::Mat& fisheye, int light_num_to_find = 5);
   void drawEnvironment(float scale_factor) const;
   void drawMovingTiger(float scale_factor, float theta);
   void drawCow(float scale_factor);
   void render();
   void update();
};