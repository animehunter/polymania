IN vec3 pos;
IN vec4 color;
OUT vec4 colorIn;

uniform mat4 projection;
uniform mat4 modelview;

uniform float camx;
uniform float camy;

void main()
{
   colorIn = color;
   gl_Position = projection*modelview*vec4(pos.x*(cos(camx*3.142)+2), pos.y*(sin(camy*3.142)+2), pos.z-2, 1.0);
}
