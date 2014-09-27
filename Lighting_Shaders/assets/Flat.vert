#version 150

in vec4 ciPosition;
in vec3 ciNormal;

uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;
uniform mat4 ciModelViewProjection;

out vec4    vEyePosition;
out vec3    vEyeNormal;

void main (){
    vEyePosition = ciModelView * ciPosition;
    vEyeNormal = ciNormalMatrix * ciNormal;
    gl_Position = ciModelViewProjection * ciPosition;
}

