#pragma once

#include "Shader.h"

class LightGL
{
public:
   LightGL();
   ~LightGL() = default;

   [[nodiscard]] bool isLightOn() const;
   void toggleLightSwitch();

   void addLight(
      const glm::vec4& light_position,
      const glm::vec4& ambient_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
      const glm::vec4& diffuse_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
      const glm::vec4& specular_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
      const glm::vec3& spotlight_direction = glm::vec3(0.0f, 0.0f, -1.0f),
      float spotlight_exponent = 0.0f,
      float spotlight_cutoff_angle_in_degree = 180.0f,
      const glm::vec3& attenuation_factor = glm::vec3(1.0f, 0.0f, 0.0f)
   );

   void setGlobalAmbientColor(const glm::vec4& global_ambient_color);
   void setAmbientColor(const glm::vec4& ambient_color, int light_index);
   void setDiffuseColor(const glm::vec4& diffuse_color, int light_index);
   void setSpecularColor(const glm::vec4& specular_color, int light_index);
   void setSpotlightDirection(const glm::vec3& spotlight_direction, int light_index);
   void setSpotlightExponent(float spotlight_exponent, int light_index);
   void setSpotlightCutoffAngle(float spotlight_cutoff_angle_in_degree, int light_index);
   void setAttenuationFactor(const glm::vec3& attenuation_factor, int light_index);
   void setLightPosition(const glm::vec4& light_position, int light_index);
   void activateLight(int light_index);
   void deactivateLight(int light_index);
   void transferUniformsToShader(const ShaderGL* shader);
   [[nodiscard]] int getTotalLightNum() const { return TotalLightNum; }
   [[nodiscard]] glm::vec3 getLightPosition(int light_index) const
   {
      return glm::vec3{ Positions[light_index].x, Positions[light_index].y, Positions[light_index].z };
   }

private:
   bool TurnLightOn;
   int TotalLightNum;
   glm::vec4 GlobalAmbientColor;
   std::vector<bool> IsActivated;
   std::vector<glm::vec4> Positions;
   std::vector<glm::vec4> AmbientColors;
   std::vector<glm::vec4> DiffuseColors;
   std::vector<glm::vec4> SpecularColors;
   std::vector<glm::vec3> SpotlightDirections;
   std::vector<float> SpotlightExponents;
   std::vector<float> SpotlightCutoffAngles;
   std::vector<glm::vec3> AttenuationFactors;
};