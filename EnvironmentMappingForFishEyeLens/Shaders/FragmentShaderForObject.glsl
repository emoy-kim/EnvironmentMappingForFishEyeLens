#version 330

uniform sampler2D BaseTexture;
uniform vec3 LightColor;

in vec3 eye_position;
in vec3 eye_normal;
in vec3 eye_light_position;

in vec3 world_position;
in vec3 world_normal;

in mat4 view_mat;

layout (location = 0) out vec4 final_color;

const float c_zero = 0.0;
const float c_one = 1.0;

vec3 N_eye;

vec3 calculateLighting()
{
   vec3 computed_color = vec3(c_zero, c_zero, c_zero);
   float ndotl;
   float ndoth;
   float att_factor;
   vec3 att_dist;
   vec3 distance_attenuation_factors = vec3(0.005, 0.005, 0.01);
   att_factor = c_one;
   
   vec3 L = eye_light_position - eye_position;

   att_dist.x = c_one;
   att_dist.z = dot( L, L );
   att_dist.y = sqrt( att_dist.z );
   att_factor = c_one / dot( att_dist, distance_attenuation_factors );

   L = normalize( L );

   if (att_factor > c_zero) {
      vec3 H = normalize( L - normalize( eye_position ).xyz );
      ndoth = max( c_zero, dot( N_eye, H ) );
      if (ndoth > c_zero) {
         computed_color += pow( ndoth, 128 ) * LightColor;
      }
   }

   return computed_color * att_factor;
}

void main()
{
   //final_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
   
   N_eye = normalize( eye_normal );
   vec3 computed_color = calculateLighting();

   vec3 L = normalize( eye_light_position - eye_position );
   vec3 V = normalize( -eye_position );
   vec3 R = normalize( reflect( -L, N_eye ) );
   //vec3 O = vec3( c_zero, c_zero, c_zero );
   computed_color += LightColor * 0.7529f * max( dot( N_eye, L ), 0.0f ); // diffuse reflection
   /////////////////////////////////////////////////////////////////

   // EC
#if 0
   vec4 p = vec4(world_position, 1.0f);          
   vec3 e = normalize( vec3(view_mat * p) );     // EC position
   mat4 tinv = transpose( inverse( view_mat ) );
   vec3 N = normalize( vec3(tinv * vec4(world_normal, 1.0f)) );

   vec3 r = reflect( e, N );

   vec2 texcoor;
   vec3 texcolor;

   if (dot( r, N ) >= 0.0f) {
      vec3 t = view_mat[3].xyz; 
      vec3 a = e - t;

      float A = dot(r, r);
      float B = dot(r, a);
      float C = dot(a, a) - 1.0f;

      float m1 = (sqrt(B * B - C) - B);
      float m2 = (-sqrt(B * B - C) - B);

      vec3 nodal1 = m1 * r + e;
      vec3 nodal2 = m2 * r + e;
      const float pi = 3.14159f;
   
      nodal1 = vec3(inverse(view_mat) * vec4(nodal1, 1.0f));
      nodal2 = vec3(inverse(view_mat) * vec4(nodal2, 1.0f));
   
      if (m1 >= 0.0f && nodal1.y >= 0.0f) {
         texcoor.x = (atan(-nodal1.y, nodal1.x)) / pi;   
         texcoor.y = acos(-nodal1.z) / pi;

         texcolor = texture2D(BaseTexture, texcoor).xyz; 
         final_color = vec4(computed_color + texcolor, 1.0f);
      }
      else if (m2 >= 0.0f && nodal2.y >= 0.0f) {
      
         texcoor.x = (atan(-nodal2.y, nodal2.x)) / pi; 
         texcoor.y = acos(-nodal2.z) / pi;

         texcolor = texture2D(BaseTexture, texcoor).xyz; 
         final_color = vec4(computed_color + texcolor, 1.0f);
      }
      else {
         final_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
      }
   }
   else {
      final_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
   }

#else
   // WC
   mat4 RofMv;
   RofMv[0] = vec4(view_mat[0].x, view_mat[1].x, view_mat[2].x, 0.0f); 
   RofMv[1] = vec4(view_mat[0].y, view_mat[1].y, view_mat[2].y, 0.0f); 
   RofMv[2] = vec4(view_mat[0].z, view_mat[1].z, view_mat[2].z, 0.0f); 
   RofMv[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

   mat4 TofMv;
   TofMv = RofMv * view_mat;
   vec3 eye_position = vec3(-TofMv[3].x, -TofMv[3].y, -TofMv[3].z);
   vec3 I = normalize(world_position - eye_position);
   vec3 N = normalize(world_normal);
   vec3 r = reflect(I, N);

   vec2 texcoor;
   vec3 texcolor;

   if (dot( r, N ) >= 0.0f) {
      float A = dot(r, r);
      float B = dot(r, world_position);
      float C = dot(world_position, world_position) - 1.0f;

      float m1 = (sqrt(B * B - C) - B);
      float m2 = (-sqrt(B * B - C) - B);

      vec3 nodal1 = m1 * r + world_position;
      vec3 nodal2 = m2 * r + world_position;
      const float pi = 3.14159f;

      if (m1 >= 0.0f && nodal1.y >= 0.0f) {
         texcoor.x = (atan(-nodal1.y, nodal1.x)) / pi; 
         texcoor.y = acos(-nodal1.z) / pi;

         texcolor = texture2D(BaseTexture, texcoor).xyz; 
         final_color = vec4(computed_color + texcolor, 1.0f);
      }
      else if (m2 >= 0.0f && nodal2.y >= 0.0f) {
         texcoor.x = (atan(-nodal2.y, nodal2.x)) / pi; 
         texcoor.y = acos(-nodal2.z) / pi;

         texcolor = texture2D(BaseTexture, texcoor).xyz; 
         final_color = vec4(computed_color + texcolor, 1.0f);
      }
      else {
         final_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
      }
   }
   else {
      final_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
   }
#endif
}