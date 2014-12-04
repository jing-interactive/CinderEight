#version 110
uniform float	resolution;
const float center = 0.5;
const float width = 0.02;
uniform sampler2D	uVideoTex;
void main(void)
{
	// calculate glowing line strips based on texture coordinate
	float f = fract( resolution * gl_TexCoord[0].s );
	float d = abs(center - f);
	float strips = clamp(width / d, 0.0, 1.0);

	// calculate fade based on texture coordinate
	float fade = gl_TexCoord[0].y;

	// calculate output color
    gl_FragColor.rgb = vec3(gl_Color.r,texture2D(uVideoTex,gl_TexCoord[0].xy).gb);// * strips;// * fade;
	gl_FragColor.a = 1.0;
}