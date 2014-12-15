#version 150

uniform float		uTexOffset;
uniform sampler2D	uLeftTex;
uniform sampler2D	uRightTex;
const float two_pi = 6.2831853;
const float tenLogBase10 = 3.0102999566398; // 10.0 / log(10.0);
uniform float time;
uniform bool isSphere;

in vec2	ciTexCoord0;
in vec4 ciPosition;
in vec3 ciColor;
in vec3		ciNormal;
uniform mat4 ciModelViewProjection;
uniform mat3	ciNormalMatrix;


out vec3 vColor;

out VertexData {
	vec4 position;
	vec3 normal;
	vec4 color;
	vec2 texCoord;
} vVertexOut;

void main(void)
{
    // retrieve texture coordinate and offset it to scroll the texture
    vec2 coord = ciTexCoord0.st + vec2(-0.25, uTexOffset);
    
    // retrieve the FFT from left and right texture and average it
    float fft = max(0.0001, mix( texture( uLeftTex, coord ).r, texture( uRightTex, coord ).r, 0.5));
    
    // convert to decibels
    float decibels = tenLogBase10 * log( fft );
    
    // offset the vertex based on the decibels and create a cylinder

    float r = decibels;
    vec4 vertex = ciPosition;
    
    if (!isSphere) {
        vertex.y -= time*r * cos(ciTexCoord0.t * two_pi);
        vertex.z += time * r * sin(ciTexCoord0.t * two_pi);
    }
    //vertex  = vertex * (1 + 0.05 * decibels);
    vec3 normal = ciNormalMatrix * ciNormal;
    
    vertex.xyz += 0.01*normal*decibels;
    vVertexOut.texCoord = ciTexCoord0;
    vVertexOut.color.rgb = ciColor;
    
    gl_Position = ciModelViewProjection * vertex;
}