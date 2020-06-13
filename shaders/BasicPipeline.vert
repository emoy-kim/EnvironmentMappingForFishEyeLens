#version 460

uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelViewProjectionMatrix;

uniform vec3 ActivatedLightPosition;

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_tex_coord;

out vec3 position_in_wc;
out vec3 normal_in_wc;
out vec3 eye_position_in_wc;

out vec3 position_in_ec;
out vec3 normal_in_ec;
out vec3 light_position_in_ec;

out vec2 tex_coord;

void main()
{   
   vec4 w_position = WorldMatrix * vec4(v_position, 1.0f);
   vec4 w_normal = transpose( inverse( WorldMatrix ) ) * vec4(v_normal, 1.0f);
   position_in_wc = w_position.xyz;
   normal_in_wc = w_normal.xyz;
   eye_position_in_wc = inverse( ViewMatrix )[3].xyz;
   
   vec4 e_position = ViewMatrix * w_position;
   vec4 e_normal = transpose( inverse( ViewMatrix ) ) * w_normal;
   position_in_ec = e_position.xyz;
   normal_in_ec = e_normal.xyz;

   tex_coord = v_tex_coord;
   
   const float light_distance = 10.0f;
   light_position_in_ec = vec3(ViewMatrix * vec4(light_distance * ActivatedLightPosition, 1.0));

   gl_Position = ModelViewProjectionMatrix * vec4(v_position, 1.0f);
}