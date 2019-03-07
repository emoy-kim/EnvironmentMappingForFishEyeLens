#version 330

uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectMatrix;
uniform vec3 PrimitiveColor;
uniform vec3 LightPosition;

layout (location = 0) in vec4 v_position;
layout (location = 1) in vec4 v_normal;

out vec3 world_position;
out vec3 world_normal;

out vec3 eye_position;
out vec3 eye_normal;
out vec3 eye_light_position;

out mat4 view_mat;

void main()
{
   vec4 wc_position = WorldMatrix * v_position;
   vec4 wc_normal = transpose( inverse( WorldMatrix ) ) * v_normal;

   vec4 ec_position = ViewMatrix * wc_position;
   vec4 ec_normal = transpose( inverse( ViewMatrix ) ) * wc_normal;

   const float light_distance = 10.0f;
   eye_light_position = vec3(ViewMatrix * vec4(light_distance * LightPosition, 1.0));

   world_position = vec3(wc_position);
   world_normal = vec3(wc_normal);
   eye_position = vec3(ec_position);
   eye_normal = vec3(ec_normal);
   view_mat = ViewMatrix;

   gl_Position = ProjectMatrix * ViewMatrix * wc_position;
}