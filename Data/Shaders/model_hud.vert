#ifdef GL_ES
precision highp float;
#endif


// Camera position:
uniform vec3 CamPos;

// Model properties:
uniform vec3 Pos;
uniform vec3 XVec;
uniform vec3 YVec;
uniform vec3 ZVec;


void main( void )
{
	// Calculate the vertex in worldspace and apply it to the rendering matrices.
	vec4 world_vertex = vec4( Pos.x + dot(gl_Vertex.xyz,XVec), Pos.y + dot(gl_Vertex.xyz,YVec), Pos.z + dot(gl_Vertex.xyz,ZVec), gl_Vertex.w );
	gl_Position = gl_ModelViewProjectionMatrix * world_vertex;
}
