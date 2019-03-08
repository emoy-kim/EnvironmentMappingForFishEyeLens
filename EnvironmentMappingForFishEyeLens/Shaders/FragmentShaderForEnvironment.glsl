#version 330

uniform sampler2D BaseTexture;             

in vec4 color;                             
in vec2 tex_coord;                         

layout (location = 0) out vec4 final_color;

void main() 
{
   final_color = texture( BaseTexture, tex_coord );
}