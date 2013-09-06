IN vec3 pos;
IN vec4 color;
OUT vec4 colorIn;

void main()
{
   colorIn = color;
   gl_Position = vec4(pos, 1.0);
}
