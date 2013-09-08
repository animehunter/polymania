IN vec3 in_Position;
IN vec4 in_Color;
OUT vec4 inout_Color;

uniform mat4 projection;
uniform mat4 modelview;

uniform float camx;
uniform float camy;

void main()
{
   inout_Color = in_Color;
   gl_Position = projection*modelview*vec4(in_Position.x*(cos(camx*3.142)+2), in_Position.y*(sin(camy*3.142)+2), in_Position.z-2, 1.0);
}
