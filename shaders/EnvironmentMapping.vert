#version 460

uniform mat4 ModelViewProjectionMatrix;
uniform vec3 PrimitiveColor;

layout (location = 0) in vec4 v_position;

out vec4 color;
out vec2 tex_coord;

void main() 
{                        
   const float pi = 3.14159f;
   tex_coord.x = atan( -v_position.y, v_position.x ) / pi;
   tex_coord.y = acos( -v_position.z ) / pi;

   color = vec4( PrimitiveColor, 1.0f );                  
   gl_Position =  ModelViewProjectionMatrix * v_position; 
}