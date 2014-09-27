#version 330

in vec4 ciPosition;
uniform mat4 ciModelViewProjection;
out vec4 vColor;

void main(){
    gl_Position = ciModelViewProjection * ciPosition;
    //vColor = vec4(1,0,0,0);
}