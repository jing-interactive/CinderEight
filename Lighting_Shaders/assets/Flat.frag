#version 150

in vec4 vEyePosition;
in vec3 vEyeNormal;

out vec4  oColor;

uniform vec4 lightPosition;

void main (){
    vec3 position = vEyePosition.xyz;
    vec3 normal = normalize(vEyeNormal);
    vec3 lightVector = vec3(normalize (lightPosition - vEyePosition));
    vec3 diffuse = vec3(max (dot(lightVector, normal), 0.0));
    oColor = vec4(diffuse,1.);
}