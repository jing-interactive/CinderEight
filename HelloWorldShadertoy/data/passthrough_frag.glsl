#version 110
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform sampler2D iChannel0;          // input channel. XX = 2D/Cube
uniform vec3      iMouse;

void main (void){
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	gl_FragColor = texture2D( iChannel0, uv );
}
