#include "EnvironmentMapping.h"


//------------------------------------------------------------------
//
// Camera Class
//
//------------------------------------------------------------------

CameraGL::CameraGL() : 
   CameraGL(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f))
{   
}

CameraGL::CameraGL(
   const vec3& cam_position,
   const vec3& view_reference_position,
   const vec3& view_up_vector,
   float fov,
   float near_plane,
   float far_plane
) : 
   ZoomSensitivity( 2.0f ), MoveSensitivity( 5.0f ), RotationSensitivity( 0.01f ), IsMoving( false ),
   AspectRatio( 0.0f ), InitFOV( fov ), NearPlane( near_plane ), FarPlane( far_plane ), 
   InitCamPos( cam_position ), InitRefPos( view_reference_position ), InitUpVec( view_up_vector ), 
   FOV( fov ), CamPos( cam_position ), RefPos( view_reference_position ), UpVec( view_up_vector ),
   ViewMatrix( lookAt( CamPos, RefPos, UpVec ) ), ProjectionMatrix( mat4(1.0f) )
{
}

bool CameraGL::getMovingState() const
{
   return IsMoving;
}

void CameraGL::setMovingState(bool is_moving)
{
   IsMoving = is_moving;
}

void CameraGL::pitch(int angle)
{
   const vec3 u_axis(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
   ViewMatrix = glm::rotate( ViewMatrix, static_cast<float>(angle) * RotationSensitivity, u_axis );
}

void CameraGL::yaw(int angle)
{
   const vec3 v_axis(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
   ViewMatrix = glm::rotate( ViewMatrix, static_cast<float>(angle) * RotationSensitivity, v_axis );
}

void CameraGL::moveForward()
{
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * vec3( 0.0f, 0.0f, -1.0f ) );
}

void CameraGL::moveBackward()
{
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * vec3( 0.0f, 0.0f, 1.0f ) );
}

void CameraGL::moveLeft()
{
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * vec3( 1.0f, 0.0f, 0.0f ) );
}

void CameraGL::moveRight()
{
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * vec3( -1.0f, 0.0f, 0.0f ) );
}

void CameraGL::moveUp()
{
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * vec3( 0.0f, -1.0f, 0.0f ) );
}

void CameraGL::moveDown()
{
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * vec3( 0.0f, 1.0f, 0.0f ) );
}

void CameraGL::zoomIn()
{
   if (FOV > 0.0f) {
      FOV -= ZoomSensitivity;
      ProjectionMatrix = perspective( radians( FOV ), AspectRatio, NearPlane, FarPlane );
   }
}

void CameraGL::zoomOut()
{
   if (FOV < 90.0f) {
      FOV += ZoomSensitivity;
      ProjectionMatrix = perspective( radians( FOV ), AspectRatio, NearPlane, FarPlane );
   }
}

void CameraGL::resetCamera()
{
   ViewMatrix = lookAt( InitCamPos, InitRefPos, InitUpVec );
   ProjectionMatrix = perspective( radians( InitFOV ), AspectRatio, NearPlane, FarPlane );
}

void CameraGL::updateWindowSize(int width, int height)
{
   AspectRatio = static_cast<float>(width) / height;
   ProjectionMatrix = perspective( radians( FOV ), AspectRatio, NearPlane, FarPlane );
}


//------------------------------------------------------------------
//
// Object Class
//
//------------------------------------------------------------------

ObjectGL::ObjectGL() : ObjVAO( 0 ), ObjVBO( 0 ), DrawMode( 0 ), TextureID( 0 ), VerticesCount( 0 ), Colors{}
{
}

void ObjectGL::prepareVertexBuffer(const int& n_bytes_per_vertex)
{
   glGenBuffers( 1, &ObjVBO );
   glBindBuffer( GL_ARRAY_BUFFER, ObjVBO );
   glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * DataBuffer.size(), DataBuffer.data(), GL_STATIC_DRAW );
   glBindBuffer( GL_ARRAY_BUFFER, 0 );

   glGenVertexArrays( 1, &ObjVAO );
   glBindVertexArray( ObjVAO );
   glBindBuffer( GL_ARRAY_BUFFER, ObjVBO );
   glVertexAttribPointer( VertexLoc, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, bufferOffset( 0 ) );
   glEnableVertexAttribArray( VertexLoc );
}

void ObjectGL::prepareTexture(const Mat& texture)
{
   glGenTextures( 1, &TextureID );
   glActiveTexture( GL_TEXTURE0 + TextureID );
   glBindTexture( GL_TEXTURE_2D, TextureID );

   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, texture.cols, texture.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, texture.data );
   
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
}

void ObjectGL::setObject(
   GLenum draw_mode, 
   const vec3& color, 
   const vector<vec3>& vertices,
   const Mat& texture
)
{
   DrawMode = draw_mode;
   Colors = { color.r, color.g, color.b };
   for (const auto& vertex : vertices) {
      DataBuffer.push_back( vertex.x );
      DataBuffer.push_back( vertex.y );
      DataBuffer.push_back( vertex.z );
      VerticesCount++;
   }
   
   const int n_bytes_per_vertex = 3 * sizeof(GLfloat);
   prepareVertexBuffer( n_bytes_per_vertex );
   prepareTexture( texture );
}

void ObjectGL::prepareNormal(const int& n_bytes_per_vertex) const
{
   glVertexAttribPointer( NormalLoc, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, bufferOffset( 3 * sizeof(GLfloat) ) );
   glEnableVertexAttribArray( NormalLoc );	
}

void ObjectGL::setObject(
   GLenum draw_mode, 
   const vec3& color, 
   const vector<vec3>& vertices,
   const vector<vec3>& normals,
   const Mat& texture
)
{
   DrawMode = draw_mode;
   Colors = { color.r, color.g, color.b };
   for (uint i = 0; i < vertices.size(); ++i) {
      DataBuffer.push_back( vertices[i].x );
      DataBuffer.push_back( vertices[i].y );
      DataBuffer.push_back( vertices[i].z );
      DataBuffer.push_back( normals[i].x );
      DataBuffer.push_back( normals[i].y );
      DataBuffer.push_back( normals[i].z );
      VerticesCount++;
   }
   
   const int n_bytes_per_vertex = 6 * sizeof(GLfloat);
   prepareVertexBuffer( n_bytes_per_vertex );
   prepareNormal( n_bytes_per_vertex );
   prepareTexture( texture );
}


//------------------------------------------------------------------
//
// Shader Class
//
//------------------------------------------------------------------

ShaderGL::ShaderGL() : 
   ShaderProgram( 0 ), MVPLocation( 0 ), WorldLocation( 0 ), ViewLocation( 0 ), ProjectLocation( 0 ),
   ColorLocation( 0 ), TextureLocation( 0 ), LightLocation( 0 ), LightColorLocation( 0 )
{
}

void ShaderGL::readShaderFile(string& shader_contents, const char* shader_path) const
{
   ifstream file(shader_path, ios::in);
   if (!file.is_open()) {
      cout << "Cannot Open Shader File: " << shader_path << endl;
      return;
   }

   string line;
   while (!file.eof()) {
      getline( file, line );
      shader_contents.append( line + "\n" );
   }
   file.close();
}

void ShaderGL::setShader(const char* vertex_shader_path, const char* fragment_shader_path)
{
   string vertex_contents, fragment_contents;
   readShaderFile( vertex_contents, vertex_shader_path );
   readShaderFile( fragment_contents, fragment_shader_path );

   const GLuint vertex_shader = glCreateShader( GL_VERTEX_SHADER );
   const GLuint fragment_shader = glCreateShader( GL_FRAGMENT_SHADER );
   const char* vertex_source = vertex_contents.c_str();
   const char* fragment_source = fragment_contents.c_str();
   glShaderSource( vertex_shader, 1, &vertex_source, nullptr );
   glShaderSource( fragment_shader, 1, &fragment_source, nullptr );
   glCompileShader( vertex_shader );
   glCompileShader( fragment_shader );

   ShaderProgram = glCreateProgram();
   glAttachShader( ShaderProgram, vertex_shader );
   glAttachShader( ShaderProgram, fragment_shader );
   glLinkProgram( ShaderProgram );

   MVPLocation = glGetUniformLocation( ShaderProgram, "ModelViewProjectionMatrix" );
   WorldLocation = glGetUniformLocation( ShaderProgram, "WorldMatrix" );
   ViewLocation = glGetUniformLocation( ShaderProgram, "ViewMatrix" );
   ProjectLocation = glGetUniformLocation( ShaderProgram, "ProjectMatrix" );

   ColorLocation = glGetUniformLocation( ShaderProgram, "PrimitiveColor" );
   TextureLocation = glGetUniformLocation( ShaderProgram, "BaseTexture" );

   LightLocation = glGetUniformLocation( ShaderProgram, "LightPosition" );
   LightColorLocation = glGetUniformLocation( ShaderProgram, "LightColor" );

   glDeleteShader( vertex_shader );
   glDeleteShader( fragment_shader );
}


//------------------------------------------------------------------
//
// Renderer Class
//
//------------------------------------------------------------------

EnvironmentMapping* EnvironmentMapping::Renderer = nullptr;
EnvironmentMapping::EnvironmentMapping(const int& light_num_to_find) : 
   Window( nullptr ), TurnLightOn( false ), LightNum( light_num_to_find ), LightIndex( 0 )
{
   Renderer = this;

   initialize();
   printOpenGLInformation();
}

EnvironmentMapping::~EnvironmentMapping()
{
   glfwTerminate();
}

void EnvironmentMapping::printOpenGLInformation() const
{
   cout << "****************************************************************" << endl;
   cout << " - GLFW version supported: " << glfwGetVersionString() << endl;
   cout << " - GLEW version supported: " << glewGetString( GLEW_VERSION ) << endl;
   cout << " - OpenGL renderer: " << glGetString( GL_RENDERER ) << endl;
   cout << " - OpenGL version supported: " << glGetString( GL_VERSION ) << endl;
   cout << " - OpenGL shader version supported: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << endl  ;
   cout << "****************************************************************" << endl << endl;
}

void EnvironmentMapping::initializeOpenGL(const int& width, const int& height)
{
   if (!glfwInit()) {
      cout << "Cannot Initialize OpenGL..." << endl;
      return;
   }
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
   glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   Window = glfwCreateWindow( width, height, "Environment Mapping", nullptr, nullptr );
   glfwMakeContextCurrent( Window );
   glewInit();
   
   glEnable( GL_DEPTH_TEST );
   glClearColor( 0.0f, 1.0f, 1.0f, 1.0f );

   MainCamera.updateWindowSize( width, height );
}

void EnvironmentMapping::setEnvironmentObject(const Mat& texture, const float& scale)
{
   ifstream file("Samples/objects/hemisphere.glbin", ios::in | ios::binary);
   int vertex_num, face_num, material_info_num;
   file.read( reinterpret_cast<char*>(&vertex_num), sizeof(int) );
   file.read( reinterpret_cast<char*>(&face_num), sizeof(int) );
   file.read( reinterpret_cast<char*>(&material_info_num), sizeof(int) );
   
   vector<float> vertices(vertex_num);
   for (int i = 0; i < vertex_num; ++i) {
      file.read( reinterpret_cast<char*>(&vertices[i]), sizeof(float) );
   }
   vector<short> faces(face_num);
   for (int i = 0; i < face_num; ++i) {
      file.read( reinterpret_cast<char*>(&faces[i]), sizeof(short) );
   }
   file.close();

   vector<vec3> hemisphere_vertices;
   for (int i = 0; i < face_num - 96; ++i) {
      const int face_index = faces[i] * 8;
      hemisphere_vertices.emplace_back( 
         vertices[face_index] * scale,
         vertices[face_index + 1] * scale, 
         vertices[face_index + 2] * scale 
      );
   }

   const vec3 environment_color = { 1.0f, 1.0f, 1.0f };
   EnvironmentObject.setObject( GL_TRIANGLES, environment_color, hemisphere_vertices, texture );
}

void EnvironmentMapping::setCowObject(const Mat& texture, const float& scale)
{
   ifstream file("Samples/objects/cow.txt");
   int polygon_num;
   file >> polygon_num;

   const int vertex_num = polygon_num * 3;
   vector<vec3> cow_vertices(vertex_num);
   vector<vec3> cow_normals(vertex_num);
   for (int i = 0; i < polygon_num; ++i) {
      int triangle_vertex_num;
      file >> triangle_vertex_num;
      for (int v = 0; v < triangle_vertex_num; ++v) {
         const int index = i * triangle_vertex_num + v;
         file >> cow_vertices[index].x >> cow_vertices[index].y >> cow_vertices[index].z;
         file >> cow_normals[index].x >> cow_normals[index].y >> cow_normals[index].z;
         cow_vertices[index] *= scale;
      }
   }
   file.close();

   const vec3 cow_color = { 1.0f, 1.0f, 1.0f };
   CowObject.setObject( GL_TRIANGLES, cow_color, cow_vertices, cow_normals, texture );
}

void EnvironmentMapping::setMovingTigerObject(const Mat& texture, const float& scale)
{
   
}

void EnvironmentMapping::initialize()
{
   const int width = 1920;
   const int height = 1080;
   initializeOpenGL( width, height );
   registerCallbacks();

   EnvironmentShader.setShader( 
      "Shaders/VertexShaderForEnvironment.glsl",
      "Shaders/FragmentShaderForEnvironment.glsl" 
   );
   ObjectShader.setShader(
      "Shaders/VertexShaderForObject.glsl",
      "Shaders/FragmentShaderForObject.glsl" 
   );
}

void EnvironmentMapping::error(int error, const char* description) const
{
   puts( description );
}

void EnvironmentMapping::errorWrapper(int error, const char* description)
{
   Renderer->error( error, description );
}

void EnvironmentMapping::cleanup(GLFWwindow* window)
{
   glDeleteProgram( EnvironmentShader.ShaderProgram );
   glDeleteProgram( ObjectShader.ShaderProgram );
   glDeleteVertexArrays( 1, &EnvironmentObject.ObjVAO );
   glDeleteBuffers( 1, &EnvironmentObject.ObjVBO );
   glDeleteVertexArrays( 1, &CowObject.ObjVAO );
   glDeleteBuffers( 1, &CowObject.ObjVBO );
   glDeleteVertexArrays( 1, &MovingTigerObject.ObjVAO );
   glDeleteBuffers( 1, &MovingTigerObject.ObjVBO );

   glfwSetWindowShouldClose( window, GLFW_TRUE );
}

void EnvironmentMapping::cleanupWrapper(GLFWwindow* window)
{
   Renderer->cleanup( window );
}

void EnvironmentMapping::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (action != GLFW_PRESS) return;

   switch (key) {
      case GLFW_KEY_UP:
         MainCamera.moveForward();
         break;
      case GLFW_KEY_DOWN:
         MainCamera.moveBackward();
         break;
      case GLFW_KEY_LEFT:
         MainCamera.moveLeft();
         break;
      case GLFW_KEY_RIGHT:
         MainCamera.moveRight();
         break;
      case GLFW_KEY_W:
         MainCamera.moveUp();
         break;
      case GLFW_KEY_S:
         MainCamera.moveDown();
         break;
      case GLFW_KEY_I:
         MainCamera.resetCamera();
         break;
      case GLFW_KEY_L:
         TurnLightOn = !TurnLightOn;
         LightIndex = 0;
         break;
      case GLFW_KEY_ENTER:
         LightIndex++;
         if (LightIndex == LightNum) LightIndex = 0;
         break;
      case GLFW_KEY_Q:
      case GLFW_KEY_ESCAPE:
         cleanupWrapper( window );
         break;
      default:
         return;
   }
}

void EnvironmentMapping::keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   Renderer->keyboard( window, key, scancode, action, mods );
}

void EnvironmentMapping::cursor(GLFWwindow* window, double xpos, double ypos)
{
   if (MainCamera.getMovingState()) {
      const int dx = static_cast<int>(round( xpos )) - ClickedPoint.x;
      const int dy = static_cast<int>(round( ypos )) - ClickedPoint.y;
      MainCamera.pitch( dy );
      MainCamera.yaw( dx );
   }
}

void EnvironmentMapping::cursorWrapper(GLFWwindow* window, double xpos, double ypos)
{
   Renderer->cursor( window, xpos, ypos );
}

void EnvironmentMapping::mouse(GLFWwindow* window, int button, int action, int mods)
{
   if (button == GLFW_MOUSE_BUTTON_LEFT) {
      const bool moving_state = action == GLFW_PRESS;
      if (moving_state) {
         double x, y;
         glfwGetCursorPos( window, &x, &y );
         ClickedPoint = Point(static_cast<int>(round( x )), static_cast<int>(round( y )));
      }
      MainCamera.setMovingState( moving_state );
   }
}

void EnvironmentMapping::mouseWrapper(GLFWwindow* window, int button, int action, int mods)
{
   Renderer->mouse( window, button, action, mods );
}

void EnvironmentMapping::mousewheel(GLFWwindow* window, double xoffset, double yoffset)
{
   if (yoffset >= 0.0) {
      MainCamera.zoomIn();
   }
   else {
      MainCamera.zoomOut();
   }
}

void EnvironmentMapping::mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset)
{
   Renderer->mousewheel( window, xoffset, yoffset );
}

void EnvironmentMapping::reshape(GLFWwindow* window, int width, int height)
{
   MainCamera.updateWindowSize( width, height );
   glViewport( 0, 0, width, height );
}

void EnvironmentMapping::reshapeWrapper(GLFWwindow* window, int width, int height)
{
   Renderer->reshape( window, width, height );
}

void EnvironmentMapping::registerCallbacks() const
{
   glfwSetErrorCallback( errorWrapper );
   glfwSetWindowCloseCallback( Window, cleanupWrapper );
   glfwSetKeyCallback( Window, keyboardWrapper );
   glfwSetCursorPosCallback( Window, cursorWrapper );
   glfwSetMouseButtonCallback( Window, mouseWrapper );
   glfwSetScrollCallback( Window, mousewheelWrapper );
   glfwSetFramebufferSizeCallback( Window, reshapeWrapper );
}

void EnvironmentMapping::findLightsAndGetTexture(Mat& texture, const Mat& fisheye)
{
   LongitudeLatitudeMapper.convertFisheye( texture, fisheye );

   vector<Point> light_points;
   LightFinder.estimateLightPositions( light_points, texture, LightNum );

   LightColors.clear();
   LightPositions.clear();
   const float color_scale = 1.0f / 255.0f;
   const float width_scale = static_cast<float>(CV_PI) / static_cast<float>(fisheye.cols - 1);
   const float height_scale = static_cast<float>(CV_PI) / static_cast<float>(fisheye.rows - 1);
   for (const auto& light : light_points) {
      const auto& color = fisheye.at<Vec3b>(light.y, light.x);
      LightColors.emplace_back( color[2] * color_scale, color[1] * color_scale, color[0] * color_scale );
      LightPositions.emplace_back(
         -sin( light.y * height_scale ) * cos( light.x * width_scale ),
         sin( light.y * height_scale ) * sin( light.x * width_scale ),
         -cos( light.y * height_scale )
      );
   }
}

void EnvironmentMapping::setObjects(const Mat& texture)
{
   if (EnvironmentObject.ObjVAO != 0) {
      glDeleteVertexArrays( 1, &EnvironmentObject.ObjVAO );
      glDeleteBuffers( 1, &EnvironmentObject.ObjVBO );
   }
   setEnvironmentObject( texture, 1.0f );

   if (CowObject.ObjVAO != 0) {
      glDeleteVertexArrays( 1, &CowObject.ObjVAO );
      glDeleteBuffers( 1, &CowObject.ObjVBO );
   }
   setCowObject( texture, 1.0f );

   if (MovingTigerObject.ObjVAO != 0) {
      glDeleteVertexArrays( 1, &MovingTigerObject.ObjVAO );
      glDeleteBuffers( 1, &MovingTigerObject.ObjVBO );
   }
   setMovingTigerObject( texture, 1.0f );
}

void EnvironmentMapping::drawEnvironment()
{
   glUseProgram( EnvironmentShader.ShaderProgram );
   
   const mat4 model_view_projection = MainCamera.ProjectionMatrix * MainCamera.ViewMatrix;
   glUniformMatrix4fv( EnvironmentShader.MVPLocation, 1, GL_FALSE, &model_view_projection[0][0] );
   glUniform1i( EnvironmentShader.TextureLocation, EnvironmentObject.TextureID );
   
   glBindVertexArray( EnvironmentObject.ObjVAO );
   glUniform3fv( EnvironmentShader.ColorLocation, 1, value_ptr( EnvironmentObject.Colors ) );
   glDrawArrays( EnvironmentObject.DrawMode, 0, EnvironmentObject.VerticesCount );
}

void EnvironmentMapping::drawCow()
{
   glUseProgram( ObjectShader.ShaderProgram );

   const mat4 to_world = glm::rotate( mat4(1.0f), radians( -90.0f ), vec3(0.0f, 1.0f, 0.0f) );
   glUniformMatrix4fv( ObjectShader.WorldLocation, 1, GL_FALSE, &to_world[0][0] );
   glUniformMatrix4fv( ObjectShader.ViewLocation, 1, GL_FALSE, &MainCamera.ViewMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.ProjectLocation, 1, GL_FALSE, &MainCamera.ProjectionMatrix[0][0] );

   vec3 light_color(0.0f), light_position(0.0f);
   if (TurnLightOn) {
      light_color = LightColors[LightIndex];
      light_position = LightPositions[LightIndex];
   }
   glUniform3fv( ObjectShader.LightLocation, 1, &light_position[0] );
   glUniform3fv( ObjectShader.LightColorLocation, 1, &light_color[0] );
   glUniform1i( ObjectShader.TextureLocation, CowObject.TextureID );

   glBindVertexArray( CowObject.ObjVAO );
   glUniform3fv( ObjectShader.ColorLocation, 1, value_ptr( CowObject.Colors ) );
   glDrawArrays( CowObject.DrawMode, 0, CowObject.VerticesCount );
}

void EnvironmentMapping::render()
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   drawEnvironment();
   drawCow();
   // drawMovingTiger();

   glBindVertexArray( 0 );
   glUseProgram( 0 );
}

void EnvironmentMapping::play(const Mat& fisheye)
{
   if (glfwWindowShouldClose( Window )) initialize();

   Mat texture;
   findLightsAndGetTexture( texture, fisheye );
   setObjects( texture );
   
   while (!glfwWindowShouldClose( Window )) {
      render();

      glfwPollEvents();
      glfwSwapBuffers( Window );
   }
   glfwDestroyWindow( Window );
}