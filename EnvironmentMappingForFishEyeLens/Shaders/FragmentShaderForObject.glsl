#version 330

uniform sampler2D BaseTexture;
uniform float EnvironmentRadius;
uniform vec3 LightColor;

in vec3 position_in_wc;
in vec3 normal_in_wc;
in vec3 eye_position_in_wc;

in vec3 light_position_in_ec;
in vec3 position_in_ec;
in vec3 normal_in_ec;

layout (location = 0) out vec4 final_color;

vec3 calculateLightingEquation()
{
   vec3 light_vector = light_position_in_ec - position_in_ec;

   vec3 distance_factors;
   distance_factors.x = 1.0;
   distance_factors.z = dot( light_vector, light_vector );
   distance_factors.y = sqrt( distance_factors.z );
   
   vec3 attenuation_factors = vec3(0.005, 0.005, 0.01);
   float attenuation_effect = min( 1.0 / dot( distance_factors, attenuation_factors ), 1.0 );

   light_vector = normalize( light_vector );

   vec3 computed_color = vec3(0.0);
   vec3 normal = normalize( normal_in_ec );
   if (attenuation_effect > 0.0) {
      vec3 halfway_vector = normalize( light_vector - normalize( position_in_ec ).xyz );
      float n_dot_h = max( 0.0, dot( normal, halfway_vector ) );
      computed_color += LightColor * pow( n_dot_h, 128 );
      computed_color *= attenuation_effect;
   }

   const float diffuse_reflection_coefficient = 0.7529f;
   computed_color += LightColor * diffuse_reflection_coefficient * max( dot( normal, light_vector ), 0.0 );
   return computed_color;
}

vec3 calculateReflectedTextureInWC()
/*
   Hemisphere Equation: x^2 + y^2 + z^2 = 1, 0 <= y <= 1

   Reflected View Vector: (x-xw)/rx = (y-yw)/ry = (z-zw)/rz
    * object point x in wc = (xw, yw, zw)
    * normalized reflected r = (rx, ry, rz)

   Intersection Point: (rx * t + xw, ry * t + yw, rz * t + zw), some real t >= 0

   Solve Equation: (rx * t + xw)^2 + (ry * t + yw)^2 + (rz * t + zw)^2 = 1
    * t1 = -(r dot x) + sqrt((r dot x)^2 - (x^2 - 1))
    * t2 = -(r dot x) - sqrt((r dot x)^2 - (x^2 - 1))
*/
{
   const float pi = 3.14159f;
   vec3 view_vector = normalize( position_in_wc - eye_position_in_wc );
   vec3 normal = normalize( normal_in_wc );
   vec3 reflected = normalize(reflect( view_vector, normal ));

   vec3 reflected_texture = vec3(0.0);
   float radius = EnvironmentRadius;
   if (dot( reflected, normal ) >= 0.0) {
      float r_dot_x = dot( reflected, position_in_wc );
      float d = sqrt( r_dot_x * r_dot_x - (dot( position_in_wc, position_in_wc ) - radius * radius) );
      float t1 = -r_dot_x + d;
      float t2 = -r_dot_x - d;

      vec3 point1 = t1 * reflected + position_in_wc;
      vec3 point2 = t2 * reflected + position_in_wc;
      vec3 intersection_point;
      if (t1 >= 0.0 && point1.y >= 0.0) intersection_point = point1 / radius;
      else if (t2 >= 0.0 && point2.y >= 0.0) intersection_point = point2 / radius;
      else return reflected_texture;

      vec2 texture_point;
      texture_point.x = atan( -intersection_point.y, intersection_point.x ) / pi; 
      texture_point.y = acos( -intersection_point.z ) / pi;
      reflected_texture = texture( BaseTexture, texture_point ).stp; 
   }
   return reflected_texture;
}

void main()
{
   final_color = vec4(calculateLightingEquation() + calculateReflectedTextureInWC(), 1.0);
}