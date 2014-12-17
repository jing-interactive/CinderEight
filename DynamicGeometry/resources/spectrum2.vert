#version 150

uniform float		uTexOffset;
uniform sampler2D	uLeftTex;
uniform sampler2D	uRightTex;
const float two_pi = 6.2831853;
const float pi = 3.1415927;
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
    vec2 coord = isSphere ? ciTexCoord0.ts : ciTexCoord0.st + vec2(-0.25, uTexOffset);
    
    // retrieve the FFT from left and right texture and average it
    float fft = max(0.0001, mix( texture( uLeftTex, coord ).r, texture( uRightTex, coord ).r, 0.5));
    
    // convert to decibels
    float decibels = tenLogBase10 * log( fft );
    
    // offset the vertex based on the decibels and create a cylinder

    float r = 1.0 + 0.01 * decibels;
    vec4 vertexCyl = ciPosition;
    vec4 vertexSphere = ciPosition;
    
    if (!isSphere) {
        float cosTwoPi = cos(ciTexCoord0.t * two_pi);
        vertexCyl.y = r * cosTwoPi;
        
        float sinTwoPi = sin(ciTexCoord0.t * two_pi);
        vertexCyl.z = r * sinTwoPi;
        
        //sphere http://mathworld.wolfram.com/Sphere.html
        float sinPi = sin(ciTexCoord0.s * pi);
        vertexSphere.x = r * cosTwoPi * sinPi;
        vertexSphere.y = r * sinTwoPi * sinPi;
        vertexSphere.z = r * cos(ciTexCoord0.s * pi);
    }
    
    vec4 vertex = mix(vertexSphere, vertexCyl, clamp(1.0 - time, 0.0, 1.0));
    vVertexOut.texCoord = ciTexCoord0;
    vVertexOut.color.rgb = ciColor;
    
    gl_Position = ciModelViewProjection * vertex;
}