uniform sampler2D color_texture;

uniform sampler2D tex0;

varying vec3 v;
varying vec3 N;


uniform vec3 lightDirection;
uniform vec4 ambientLight;

uniform float lightLength;
uniform float minLight;

void main() {


    float lambertFactor = max( dot( lightDirection, N ), minLight);
    lambertFactor = 1.0 - (lambertFactor/2.0);
    
    //gl_FragColor= vec4(lambertFactor,lambertFactor,lambertFactor,1);
   gl_FragColor= vec4(ambientLight.x * lambertFactor,ambientLight.y* lambertFactor,ambientLight.z* lambertFactor,ambientLight.w);

}