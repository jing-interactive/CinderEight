
varying vec3 v;
varying vec3 N;


uniform vec3 lightDirection;
uniform vec4 ambientLight;

void main()
{

		float NdotL;
		vec4 diffuse, ambient, globalAmbient;


    v = vec3(gl_ModelViewMatrix * gl_Vertex);
	N = normalize(gl_NormalMatrix * gl_Normal);

//	lightDir = normalize(vec3(gl_LightSource[0].position));
//	
//	lightDir.x = 1.0;
//	lightDir.y = 0.0;
//	lightDir.z = 1.0;


	diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
		NdotL = max(dot(N, lightDirection),0.0);//max(dot(N, lightDir), 0.0);
		diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
		ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
		
		gl_FrontColor = NdotL * diffuse  + ambient;
		//gl_FrontColor.x = N;//0.1 + (dot(N, lightDirection) / 2.0);
//		gl_FrontColor.y = 0.0;
//		gl_FrontColor.z = 0.0;
//		
	//	gl_FrontColor = (vec4(gl_Normal,1.0));

	gl_TexCoord[0] = gl_MultiTexCoord0;
    
	gl_Position = ftransform();

    
}