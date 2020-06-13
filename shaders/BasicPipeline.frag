#version 460

#define MAX_LIGHTS 32

struct LightInfo
{
   int LightSwitch;
   vec4 Position;
   vec4 AmbientColor;
   vec4 DiffuseColor;
   vec4 SpecularColor;
   vec3 SpotlightDirection;
   float SpotlightExponent;
   float SpotlightCutoffAngle;
   vec3 AttenuationFactors;
};
uniform LightInfo Lights[MAX_LIGHTS];

struct MateralInfo {
   vec4 EmissionColor;
   vec4 AmbientColor;
   vec4 DiffuseColor;
   vec4 SpecularColor;
   float SpecularExponent;
};
uniform MateralInfo Material;

layout (binding = 0) uniform sampler2D BaseTexture;
uniform int UseTexture;
uniform float EnvironmentRadius;

uniform int UseLight;
uniform int LightNum;
uniform vec4 GlobalAmbient;

uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

in vec3 position_in_wc;
in vec3 normal_in_wc;
in vec3 eye_position_in_wc;

in vec3 light_position_in_ec;
in vec3 position_in_ec;
in vec3 normal_in_ec;

in vec2 tex_coord;

layout (location = 0) out vec4 final_color;

const float zero = 0.0f;
const float one = 1.0f;

bool IsPointLight(in vec4 light_position)
{
   return light_position.w != zero;
}

float getAttenuation(in vec3 light_vector, in int light_index)
{
   vec3 distance_scale;
   distance_scale.x = one;
   distance_scale.z = dot( light_vector, light_vector );
   distance_scale.y = sqrt( distance_scale.z );
   return one / dot( distance_scale, Lights[light_index].AttenuationFactors );
}

float getSpotlightFactor(in vec3 normalized_light_vector, in int light_index)
{
   if (Lights[light_index].SpotlightCutoffAngle >= 180.0f) return one;

   vec4 direction_in_ec = 
      transpose( inverse( ViewMatrix * WorldMatrix ) ) * 
      vec4(Lights[light_index].SpotlightDirection, 1.0f);
   vec3 normalized_direction = normalize( direction_in_ec.xyz );
   float cutoff_angle = clamp( Lights[light_index].SpotlightCutoffAngle, zero, 90.0f );
   float factor = dot( -normalized_light_vector, normalized_direction );
   return factor >= cos( radians( cutoff_angle ) ) ? pow( factor, Lights[light_index].SpotlightExponent ) : zero;
}

vec4 calculateLightingEquation()
{
   vec4 color = Material.EmissionColor + GlobalAmbient * Material.AmbientColor;

   for (int i = 0; i < LightNum; ++i) {
      if (Lights[i].LightSwitch == 0) continue;
      
      vec4 light_position_in_ec = ViewMatrix * Lights[i].Position;
      
      float final_effect_factor = one;
      vec3 light_vector = light_position_in_ec.xyz - position_in_ec;
      if (IsPointLight( light_position_in_ec )) {
         float attenuation = getAttenuation( light_vector, i );

         light_vector = normalize( light_vector );
         float spotlight_factor = getSpotlightFactor( light_vector, i );
         final_effect_factor = attenuation * spotlight_factor;
      }
      else light_vector = normalize( light_position_in_ec.xyz );
   
      if (final_effect_factor <= zero) continue;

      vec4 local_color = Lights[i].AmbientColor * Material.AmbientColor;

      float diffuse_intensity = max( dot( normal_in_ec, light_vector ), zero );
      local_color += diffuse_intensity * Lights[i].DiffuseColor * Material.DiffuseColor;

      vec3 halfway_vector = normalize( light_vector - normalize( position_in_ec ) );
      float specular_intensity = max( dot( normal_in_ec, halfway_vector ), zero );
      local_color += 
         pow( specular_intensity, Material.SpecularExponent ) * 
         Lights[i].SpecularColor * Material.SpecularColor;

      color += local_color;
   }
   return color;
}

vec3 calculateReflectedTextureInWC()
/*
   Hemisphere Equation: x^2 + y^2 + z^2 = 1, 0 <= y <= 1

   Reflected View Vector: (x-xw)/rx = (y-yw)/ry = (z-zw)/rz
    -> object point x in wc = (xw, yw, zw)
    -> normalized reflected r = (rx, ry, rz)

   Intersection Point: (rx * t + xw, ry * t + yw, rz * t + zw), some real t >= 0

   Solve Equation: (rx * t + xw)^2 + (ry * t + yw)^2 + (rz * t + zw)^2 = 1
    -> t1 = -(r dot x) + sqrt((r dot x)^2 - (x^2 - 1))
    -> t2 = -(r dot x) - sqrt((r dot x)^2 - (x^2 - 1))
*/
{
   const float pi = 3.14159f;
   vec3 view_vector = normalize( position_in_wc - eye_position_in_wc );
   vec3 normal = normalize( normal_in_wc );
   vec3 reflected = normalize(reflect( view_vector, normal ));

   vec3 reflected_texture = vec3(zero);
   float radius = EnvironmentRadius;
   if (dot( reflected, normal ) >= zero) {
      float r_dot_x = dot( reflected, position_in_wc );
      float d = sqrt( r_dot_x * r_dot_x - (dot( position_in_wc, position_in_wc ) - radius * radius) );
      float t1 = -r_dot_x + d;
      float t2 = -r_dot_x - d;

      vec3 point1 = t1 * reflected + position_in_wc;
      vec3 point2 = t2 * reflected + position_in_wc;
      vec3 intersection_point;
      if (t1 >= zero && point1.y >= zero) intersection_point = point1 / radius;
      else if (t2 >= zero && point2.y >= zero) intersection_point = point2 / radius;
      else return reflected_texture;

      vec2 texture_point;
      texture_point.x = atan( -intersection_point.y, intersection_point.x ) / pi; 
      texture_point.y = acos( -intersection_point.z ) / pi;
      reflected_texture = texture( BaseTexture, texture_point ).rgb; 
   }
   return reflected_texture;
}

void main()
{
   if (UseTexture == 0) final_color = vec4(one);
   else final_color = texture( BaseTexture, tex_coord );

   if (UseLight != 0) {
      final_color *= calculateLightingEquation();
   }
   else final_color *= Material.DiffuseColor;

   final_color.xyz += calculateReflectedTextureInWC();
}