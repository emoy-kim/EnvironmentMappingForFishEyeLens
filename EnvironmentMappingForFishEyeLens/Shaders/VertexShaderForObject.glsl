#version 330

uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform vec3 PrimitiveColor;
uniform vec3 LightPosition;

layout (location = 0) in vec4 v_position;
layout (location = 1) in vec4 v_normal;

out vec3 position_in_wc;
out vec3 normal_in_wc;
out vec3 eye_position_in_wc;

out vec3 position_in_ec;
out vec3 normal_in_ec;
out vec3 light_position_in_ec;

void main()
{
   vec4 w_position = WorldMatrix * v_position;
   vec4 w_normal = transpose( inverse( WorldMatrix ) ) * v_normal;
   position_in_wc = w_position.xyz;
   normal_in_wc = w_normal.xyz;
   eye_position_in_wc = inverse( ViewMatrix )[3].xyz;
   
   vec4 e_position = ViewMatrix * w_position;
   vec4 e_normal = transpose( inverse( ViewMatrix ) ) * w_normal;
   position_in_ec = e_position.xyz;
   normal_in_ec = e_normal.xyz;
   
   const float light_distance = 10.0f;
   light_position_in_ec = vec3(ViewMatrix * vec4(light_distance * LightPosition, 1.0));

   gl_Position = ProjectionMatrix * e_position;
}