#ifdef GL_ES
precision highp float;
#endif


// Shader variable:
uniform vec3 AmbientLight;


void main( void )
{
	gl_FragColor = vec4( AmbientLight, 1.0 );
}
