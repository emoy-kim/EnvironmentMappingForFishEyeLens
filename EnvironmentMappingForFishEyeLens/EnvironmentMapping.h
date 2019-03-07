#pragma once

#include <OpenGLLinker.h>
#include "LongitudeLatitudeMapping.h"
#include "LightPosition.h"

using namespace glm;

class CameraGL
{
   const float ZoomSensitivity;
   const float MoveSensitivity;
   const float RotationSensitivity;
   bool IsMoving;
   float AspectRatio;
   float InitFOV;
   float NearPlane, FarPlane;
   vec3 InitCamPos, InitRefPos, InitUpVec;

public:
   float FOV;
   vec3 CamPos, RefPos, UpVec;
   mat4 ViewMatrix, ProjectionMatrix;

   CameraGL();
   CameraGL(
      const vec3& cam_position,
      const vec3& view_reference_position,
      const vec3& view_up_vector,
      float fov = 30.0f,
      float near_plane = 0.1f,
      float far_plane = 10000.0f
   );

   bool getMovingState() const;
   void setMovingState(bool is_moving);
   void pitch(int angle);
   void yaw(int angle);
   void moveForward();
   void moveBackward();
   void moveLeft();
   void moveRight();
   void moveUp();
   void moveDown();
   void zoomIn();
   void zoomOut();
   void resetCamera();
   void updateWindowSize(int width, int height);
};

class ObjectGL
{
   vector<GLfloat> DataBuffer; // 3 for vertex, 3 for normal, and 2 for texture
   
   void prepareVertexBuffer(const int& n_bytes_per_vertex);
   void prepareNormal(const int& n_bytes_per_vertex) const;
   void prepareTexture(const Mat& texture);
   GLvoid* bufferOffset(int offset) const { return reinterpret_cast<GLvoid *>(offset); }

public:
   enum LayoutLocation { VertexLoc=0, NormalLoc, TextureLoc };

   GLuint ObjVAO, ObjVBO;
   GLenum DrawMode;
   GLuint TextureID;
   GLsizei VerticesCount;
   vec3 Colors;

   ObjectGL();

   void setObject(
      GLenum draw_mode, 
      const vec3& color,
      const vector<vec3>& vertices,
      const Mat& texture
   );

   void setObject(
      GLenum draw_mode, 
      const vec3& color, 
      const vector<vec3>& vertices,
      const vector<vec3>& normals,
      const Mat& texture
   );
};

class ShaderGL
{
   void readShaderFile(string& shader_contents, const char* shader_path) const;

public:
   GLuint ShaderProgram;
   GLint MVPLocation, WorldLocation, ViewLocation, ProjectLocation;
   GLint ColorLocation, TextureLocation;
   GLint LightLocation, LightColorLocation;
   
   ShaderGL();

   void setShader(const char* vertex_shader_path, const char* fragment_shader_path);
};

class EnvironmentMapping
{
   static EnvironmentMapping* Renderer;
   LongitudeLatitudeMapping LongitudeLatitudeMapper;
   GLFWwindow* Window;

   Point ClickedPoint;

   CameraGL MainCamera;
   ShaderGL EnvironmentShader;
   ObjectGL EnvironmentObject;
   
   ShaderGL ObjectShader;
   ObjectGL CowObject;
   ObjectGL MovingTigerObject;

   bool TurnLightOn;
   int LightNum;
   int LightIndex;
   LightPosition LightFinder;
   vector<vec3> LightColors;
   vector<vec3> LightPositions;

   void registerCallbacks() const;
   void initializeOpenGL(const int& width, const int& height);
   void setEnvironmentObject(const Mat& texture, const float& scale);
   void setCowObject(const Mat& texture, const float& scale);
   void setMovingTigerObject(const Mat& texture, const float& scale);
   void initialize();

   void printOpenGLInformation() const;

   void error(int error, const char* description) const;
   void cleanup(GLFWwindow* window);
   void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
   void cursor(GLFWwindow* window, double xpos, double ypos);
   void mouse(GLFWwindow* window, int button, int action, int mods);
   void mousewheel(GLFWwindow* window, double xoffset, double yoffset);
   void reshape(GLFWwindow* window, int width, int height);
   static void errorWrapper(int error, const char* description);
   static void cleanupWrapper(GLFWwindow* window);
   static void keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
   static void cursorWrapper(GLFWwindow* window, double xpos, double ypos);
   static void mouseWrapper(GLFWwindow* window, int button, int action, int mods);
   static void mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset);
   static void reshapeWrapper(GLFWwindow* window, int width, int height);

   void findLightsAndGetTexture(Mat& texture, const Mat& fisheye);
   void setObjects(const Mat& texture);
   void drawEnvironment();
   void drawCow();
   void render();


public:
   EnvironmentMapping(const int& light_num_to_find = 5);
   ~EnvironmentMapping();

   void play(const Mat& fisheye);
};