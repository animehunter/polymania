IN vec3 pos;
IN vec4 color;
OUT vec4 colorIn;

uniform mat4 projection;
uniform mat4 modelview;

void main()
{
   colorIn = color;
   gl_Position = projection*modelview*vec4(pos, 1.0);
}
