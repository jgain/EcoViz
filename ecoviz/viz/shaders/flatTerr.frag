#version 330 

in vec4 colour;

out vec4 col;

 // trivial shader to flat shade triangle

void main(void)
{
   col = colour;
//   if (gl_FrontFacing == false)
 //     col = vec4(1.0, 0.0, 0.0, 1.0);
}
