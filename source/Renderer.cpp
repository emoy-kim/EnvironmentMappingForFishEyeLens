#include "Renderer.h"

RendererGL::RendererGL() : 
   Window( nullptr ), DrawMovingObject( false ), FrameWidth( 1920 ), FrameHeight( 1080 ), ActivatedLightIndex( 0 ),
   TigerIndex( 0 ), TigerRotationAngle( 0 ), EnvironmentRadius( 10.0f ), ClickedPoint( -1, -1 ),
   MainCamera( std::make_unique<CameraGL>() ), ObjectShader( std::make_unique<ShaderGL>() ),
   EnvironmentShader( std::make_unique<ShaderGL>() ), EnvironmentObject( std::make_unique<ObjectGL>() ),
   CowObject( std::make_unique<ObjectGL>() ), Lights( std::make_unique<LightGL>() ),
   LightFinder( std::make_unique<LightPosition>() ), LongitudeLatitudeMapper( std::make_unique<LongitudeLatitudeMapping>() )
{
   Renderer = this;

   initialize();
   printOpenGLInformation();
}

void RendererGL::printOpenGLInformation()
{
   std::cout << "****************************************************************\n";
   std::cout << " - GLFW version supported: " << glfwGetVersionString() << "\n";
   std::cout << " - OpenGL renderer: " << glGetString( GL_RENDERER ) << "\n";
   std::cout << " - OpenGL version supported: " << glGetString( GL_VERSION ) << "\n";
   std::cout << " - OpenGL shader version supported: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << "\n";
   std::cout << "****************************************************************\n\n";
}

void RendererGL::initialize()
{
   if (!glfwInit()) {
      std::cout << "Cannot Initialize OpenGL...\n";
      return;
   }
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
   glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   Window = glfwCreateWindow( FrameWidth, FrameHeight, "Main Camera", nullptr, nullptr );
   glfwMakeContextCurrent( Window );

   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return;
   }
   
   registerCallbacks();
   
   glEnable( GL_DEPTH_TEST );
   glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

   MainCamera->updateWindowSize( FrameWidth, FrameHeight );

   const std::string shader_directory_path = std::string(CMAKE_SOURCE_DIR) + "/shaders";
   ObjectShader->setShader(
      std::string(shader_directory_path + "/BasicPipeline.vert").c_str(),
      std::string(shader_directory_path + "/BasicPipeline.frag").c_str()
   );
   EnvironmentShader->setShader(
      std::string(shader_directory_path + "/EnvironmentMapping.vert").c_str(),
      std::string(shader_directory_path + "/EnvironmentMapping.frag").c_str()
   );
}

void RendererGL::error(int error, const char* description) const
{
   puts( description );
}

void RendererGL::errorWrapper(int error, const char* description)
{
   Renderer->error( error, description );
}

void RendererGL::cleanup(GLFWwindow* window)
{
   glfwSetWindowShouldClose( window, GLFW_TRUE );
}

void RendererGL::cleanupWrapper(GLFWwindow* window)
{
   Renderer->cleanup( window );
}

void RendererGL::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (action != GLFW_PRESS) return;

   switch (key) {
      case GLFW_KEY_UP:
         MainCamera->moveForward();
         break;
      case GLFW_KEY_DOWN:
         MainCamera->moveBackward();
         break;
      case GLFW_KEY_LEFT:
         MainCamera->moveLeft();
         break;
      case GLFW_KEY_RIGHT:
         MainCamera->moveRight();
         break;
      case GLFW_KEY_W:
         MainCamera->moveUp();
         break;
      case GLFW_KEY_S:
         MainCamera->moveDown();
         break;
      case GLFW_KEY_I:
         MainCamera->resetCamera();
         break;
      case GLFW_KEY_L:
         Lights->toggleLightSwitch();
         ActivatedLightIndex = 0;
         Lights->activateLight( 0 );
         std::cout << "Light Turned " << (Lights->isLightOn() ? "On!\n" : "Off!\n");
         break;
      case GLFW_KEY_ENTER:
         Lights->deactivateLight( ActivatedLightIndex++ );
         if (ActivatedLightIndex == Lights->getTotalLightNum()) ActivatedLightIndex = 0;
         Lights->activateLight( ActivatedLightIndex );
         break;
      case GLFW_KEY_SPACE:
         DrawMovingObject = !DrawMovingObject;
         break;
      case GLFW_KEY_P: {
         const glm::vec3 pos = MainCamera->getCameraPosition();
         std::cout << "Camera Position: " << pos.x << ", " << pos.y << ", " << pos.z << "\n";
      } break;
      case GLFW_KEY_Q:
      case GLFW_KEY_ESCAPE:
         cleanupWrapper( window );
         break;
      default:
         return;
   }
}

void RendererGL::keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   Renderer->keyboard( window, key, scancode, action, mods );
}

void RendererGL::cursor(GLFWwindow* window, double xpos, double ypos)
{
   if (MainCamera->getMovingState()) {
      const auto x = static_cast<int>(round( xpos ));
      const auto y = static_cast<int>(round( ypos ));
      const int dx = x - ClickedPoint.x;
      const int dy = y - ClickedPoint.y;
      MainCamera->moveForward( -dy );
      MainCamera->rotateAroundWorldY( -dx );

      if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) {
         MainCamera->pitch( -dy );
      }

      ClickedPoint.x = x;
      ClickedPoint.y = y;
   }
}

void RendererGL::cursorWrapper(GLFWwindow* window, double xpos, double ypos)
{
   Renderer->cursor( window, xpos, ypos );
}

void RendererGL::mouse(GLFWwindow* window, int button, int action, int mods)
{
   if (button == GLFW_MOUSE_BUTTON_LEFT) {
      const bool moving_state = action == GLFW_PRESS;
      if (moving_state) {
         double x, y;
         glfwGetCursorPos( window, &x, &y );
         ClickedPoint.x = static_cast<int>(round( x ));
         ClickedPoint.y = static_cast<int>(round( y ));
      }
      MainCamera->setMovingState( moving_state );
   }
}

void RendererGL::mouseWrapper(GLFWwindow* window, int button, int action, int mods)
{
   Renderer->mouse( window, button, action, mods );
}

void RendererGL::mousewheel(GLFWwindow* window, double xoffset, double yoffset) const
{
   if (yoffset >= 0.0) MainCamera->zoomIn();
   else MainCamera->zoomOut();
}

void RendererGL::mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset)
{
   Renderer->mousewheel( window, xoffset, yoffset );
}

void RendererGL::reshape(GLFWwindow* window, int width, int height) const
{
   MainCamera->updateWindowSize( width, height );
   glViewport( 0, 0, width, height );
}

void RendererGL::reshapeWrapper(GLFWwindow* window, int width, int height)
{
   Renderer->reshape( window, width, height );
}

void RendererGL::registerCallbacks() const
{
   glfwSetErrorCallback( errorWrapper );
   glfwSetWindowCloseCallback( Window, cleanupWrapper );
   glfwSetKeyCallback( Window, keyboardWrapper );
   glfwSetCursorPosCallback( Window, cursorWrapper );
   glfwSetMouseButtonCallback( Window, mouseWrapper );
   glfwSetScrollCallback( Window, mousewheelWrapper );
   glfwSetFramebufferSizeCallback( Window, reshapeWrapper );
}

void RendererGL::setEnvironmentObject(const cv::Mat& texture)
{
   const std::string sample_directory_path = std::string(CMAKE_SOURCE_DIR) + "/samples";
   const std::string object_path = sample_directory_path + "/objects/hemisphere.glbin";
   std::ifstream file(object_path, std::ios::in | std::ios::binary);
   int vertex_num, face_num, material_info_num;
   file.read( reinterpret_cast<char*>(&vertex_num), sizeof(int) );
   file.read( reinterpret_cast<char*>(&face_num), sizeof(int) );
   file.read( reinterpret_cast<char*>(&material_info_num), sizeof(int) );
   
   std::vector<float> vertices(vertex_num);
   for (int i = 0; i < vertex_num; ++i) {
      file.read( reinterpret_cast<char*>(&vertices[i]), sizeof(float) );
   }
   std::vector<short> faces(face_num);
   for (int i = 0; i < face_num; ++i) {
      file.read( reinterpret_cast<char*>(&faces[i]), sizeof(short) );
   }
   file.close();

   std::vector<glm::vec3> hemisphere_vertices;
   for (int i = 0; i < face_num - 96; ++i) {
      const int face_index = faces[i] * 8;
      hemisphere_vertices.emplace_back( 
         vertices[face_index],
         vertices[face_index + 1], 
         vertices[face_index + 2] 
      );
   }

   EnvironmentObject->setObject( GL_TRIANGLES, hemisphere_vertices, texture );
   EnvironmentObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void RendererGL::setMovingTigerObjects(const cv::Mat& texture)
{
   MovingTigerObjects.clear();
   const std::string sample_directory_path = std::string(CMAKE_SOURCE_DIR) + "/samples";
   const std::string object_path = sample_directory_path + "/objects/tiger";
   for (int t = 0; t < 12; ++t) {
      std::ifstream file(object_path + std::to_string( t ) + ".txt");
      int polygon_num;
      file >> polygon_num;

      const int vertex_num = polygon_num * 3;
      std::vector<glm::vec3> tiger_vertices(vertex_num);
      std::vector<glm::vec3> tiger_normals(vertex_num);
      std::vector<glm::vec2> tiger_textures_not_used(vertex_num);
      for (int i = 0; i < polygon_num; ++i) {
         int triangle_vertex_num;
         file >> triangle_vertex_num;
         for (int v = 0; v < triangle_vertex_num; ++v) {
            const int index = i * triangle_vertex_num + v;
            file >> tiger_vertices[index].x >> tiger_vertices[index].y >> tiger_vertices[index].z;
            file >> tiger_normals[index].x >> tiger_normals[index].y >> tiger_normals[index].z;
            file >> tiger_textures_not_used[index].x >> tiger_textures_not_used[index].y;
         }
      }
      file.close();

      MovingTigerObjects.emplace_back( std::make_unique<ObjectGL>() );
      MovingTigerObjects[t]->setObject( GL_TRIANGLES, tiger_vertices, tiger_normals, texture );
      MovingTigerObjects[t]->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
   }
}

void RendererGL::setCowObject(const cv::Mat& texture)
{
   const std::string sample_directory_path = std::string(CMAKE_SOURCE_DIR) + "/samples";
   const std::string object_path = sample_directory_path + "/objects/cow.txt";
   std::ifstream file(object_path);
   int polygon_num;
   file >> polygon_num;

   const int vertex_num = polygon_num * 3;
   std::vector<glm::vec3> cow_vertices(vertex_num);
   std::vector<glm::vec3> cow_normals(vertex_num);
   for (int i = 0; i < polygon_num; ++i) {
      int triangle_vertex_num;
      file >> triangle_vertex_num;
      for (int v = 0; v < triangle_vertex_num; ++v) {
         const int index = i * triangle_vertex_num + v;
         file >> cow_vertices[index].x >> cow_vertices[index].y >> cow_vertices[index].z;
         file >> cow_normals[index].x >> cow_normals[index].y >> cow_normals[index].z;
      }
   }
   file.close();

   CowObject->setObject( GL_TRIANGLES, cow_vertices, cow_normals, texture );
   CowObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void RendererGL::findLightsAndGetTexture(cv::Mat& texture, const cv::Mat& fisheye, int light_num_to_find)
{
   LongitudeLatitudeMapper->convertFisheye( texture, fisheye );
   
   std::vector<cv::Point> light_points;
   LightFinder->estimateLightPositions( light_points, texture, light_num_to_find );

   const float color_scale = 1.0f / 255.0f;
   const float width_scale = static_cast<float>(CV_PI) / static_cast<float>(fisheye.cols - 1);
   const float height_scale = static_cast<float>(CV_PI) / static_cast<float>(fisheye.rows - 1);
   const glm::vec4 ambient_color(1.0f, 1.0f, 1.0f, 1.0f);
   const glm::vec4 specular_color(0.9f, 0.9f, 0.9f, 1.0f);
   for (const auto& light : light_points) {
      const auto& color = fisheye.at<cv::Vec3b>(light.y, light.x);
      const glm::vec4 diffuse_color(
         static_cast<float>(color[2]) * color_scale, 
         static_cast<float>(color[1]) * color_scale, 
         static_cast<float>(color[0]) * color_scale,
         1.0f
      );
      const glm::vec4 light_position(
         -sin( light.y * height_scale ) * cos( light.x * width_scale ),
         sin( light.y * height_scale ) * sin( light.x * width_scale ),
         -cos( light.y * height_scale ),
         1.0f
      );
      Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );
      Lights->deactivateLight( Lights->getTotalLightNum() - 1 );
   }
   Lights->activateLight( 0 );
}

void RendererGL::drawEnvironment(float scale_factor) const
{
   glUseProgram( EnvironmentShader->getShaderProgram() );

   const glm::mat4 to_world = scale( glm::mat4(1.0f), glm::vec3(scale_factor, scale_factor, scale_factor) );
   EnvironmentShader->transferBasicTransformationUniforms( to_world, MainCamera.get(), true );
   EnvironmentObject->transferUniformsToShader( EnvironmentShader.get() );

   glBindTextureUnit( 0, EnvironmentObject->getTextureID( 0 ) );
   glBindVertexArray( EnvironmentObject->getVAO() );
   glDrawArrays( EnvironmentObject->getDrawMode(), 0, EnvironmentObject->getVertexNum() );
}

void RendererGL::drawMovingTiger(float scale_factor, float theta)
{
   glUseProgram( ObjectShader->getShaderProgram() );

   const glm::mat4 to_world =
      rotate( glm::mat4(1.0f), glm::radians( theta ), glm::vec3(0.0f, 1.0f, 0.0f) ) * 
      translate( glm::mat4(1.0f), glm::vec3(EnvironmentRadius * 0.6f, 2.0f, 0.0f) ) *
      rotate( glm::mat4(1.0f), glm::radians( 180.0f ), glm::vec3(0.0f, 1.0f, 0.0f) ) * 
      rotate( glm::mat4(1.0f), glm::radians( -90.0f ), glm::vec3(1.0f, 0.0f, 0.0f) ) * 
      scale( glm::mat4(1.0f), glm::vec3(scale_factor, scale_factor, scale_factor) );
   ObjectShader->transferBasicTransformationUniforms( to_world, MainCamera.get(), true );
   glUniform1f( ObjectShader->getLocation( "EnvironmentRadius" ), EnvironmentRadius );
   const glm::vec3 activated_light_position = Lights->getLightPosition( ActivatedLightIndex );
   glUniform3fv( ObjectShader->getLocation( "ActivatedLightPosition" ), 1, &activated_light_position[0] );
   MovingTigerObjects[TigerIndex]->transferUniformsToShader( ObjectShader.get() );
   Lights->transferUniformsToShader( ObjectShader.get() );

   glBindTextureUnit( 0, MovingTigerObjects[TigerIndex]->getTextureID( 0 ) );
   glBindVertexArray( MovingTigerObjects[TigerIndex]->getVAO() );
   glDrawArrays( MovingTigerObjects[TigerIndex]->getDrawMode(), 0, MovingTigerObjects[TigerIndex]->getVertexNum() );
}

void RendererGL::drawCow(float scale_factor)
{
   glUseProgram( ObjectShader->getShaderProgram() );

   const glm::mat4 to_world = 
      translate( glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 0.0f) ) *
      rotate( glm::mat4(1.0f), glm::radians( -90.0f ), glm::vec3(0.0f, 1.0f, 0.0f) ) * 
      scale( glm::mat4(1.0f), glm::vec3(scale_factor, scale_factor, scale_factor) );
   ObjectShader->transferBasicTransformationUniforms( to_world, MainCamera.get(), true );
   glUniform1f( ObjectShader->getLocation( "EnvironmentRadius" ), EnvironmentRadius );
   const glm::vec3 activated_light_position = Lights->getLightPosition( ActivatedLightIndex );
   glUniform3fv( ObjectShader->getLocation( "ActivatedLightPosition" ), 1, &activated_light_position[0] );
   CowObject->transferUniformsToShader( ObjectShader.get() );
   Lights->transferUniformsToShader( ObjectShader.get() );

   glBindTextureUnit( 0, CowObject->getTextureID( 0 ) );
   glBindVertexArray( CowObject->getVAO() );
   glDrawArrays( CowObject->getDrawMode(), 0, CowObject->getVertexNum() );
}

void RendererGL::render()
{
   glClear( OPENGL_COLOR_BUFFER_BIT | OPENGL_DEPTH_BUFFER_BIT );

   drawEnvironment( EnvironmentRadius );
   if (DrawMovingObject) drawMovingTiger( 0.03f, static_cast<float>(TigerRotationAngle) );
   else drawCow( 3.0f );

   glBindVertexArray( 0 );
   glUseProgram( 0 );
}

void RendererGL::update()
{
   if (DrawMovingObject) {
      TigerIndex++;
      if (TigerIndex == 12) TigerIndex = 0;
      TigerRotationAngle += 3;
      if (TigerRotationAngle == 360) TigerRotationAngle = 0;
   }
}

void RendererGL::play(const cv::Mat& fisheye)
{
   if (glfwWindowShouldClose( Window )) initialize();

   cv::Mat texture;
   findLightsAndGetTexture( texture, fisheye );
   setEnvironmentObject( texture );
   setMovingTigerObjects( texture );
   setCowObject( texture );
   EnvironmentShader->setUniformLocations( 0 );
   ObjectShader->setUniformLocations( Lights->getTotalLightNum() );
   ObjectShader->addUniformLocation( "EnvironmentRadius" );
   ObjectShader->addUniformLocation( "ActivatedLightPosition" );

   const double update_time = 0.1;
   double last = glfwGetTime(), time_delta = 0.0;
   while (!glfwWindowShouldClose( Window )) {
      const double now = glfwGetTime();
      time_delta += now - last;
      last = now;
      if (time_delta >= update_time) {
         update();
         time_delta -= update_time;
      }

      render();

      glfwSwapBuffers( Window );
      glfwPollEvents();
   }
   glfwDestroyWindow( Window );
}