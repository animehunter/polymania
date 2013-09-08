IN vec4 colorIn;

uniform float camx;
uniform float camy;

void main()
{
   vec4 newColor = colorIn;
   newColor.r *= clamp(abs(camx), 0.0, 1.0);
   newColor.g *= clamp(abs(camy), 0.0, 1.0);
   newColor.b *= clamp(abs((camx+camy)*0.5), 0.0, 1.0);
   fragColor = newColor;
}
