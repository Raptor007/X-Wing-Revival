#if BLASTPOINTS > 0
#undef BLASTPOINTS
#endif

#ifndef LIGHT_QUALITY
#define LIGHT_QUALITY 1
#elif LIGHT_QUALITY > 1
#undef LIGHT_QUALITY
#define LIGHT_QUALITY 1
#endif

// ---------------------------------------------------------------------------

#ifdef GL_ES
precision highp float;
#endif


// Shader variables.

#ifndef LIGHT_QUALITY
#define LIGHT_QUALITY 2
#endif

#ifndef DIRECTIONAL_LIGHTS
#define DIRECTIONAL_LIGHTS 4
#endif

#ifndef POINT_LIGHTS
#define POINT_LIGHTS 4
#endif

#ifndef AMBIENT_COLOR
#define AMBIENT_COLOR AmbientColor
#endif

#ifndef DIFFUSE_COLOR
#define DIFFUSE_COLOR DiffuseColor
#endif

#ifndef SPECULAR_COLOR
#define SPECULAR_COLOR SpecularColor
#endif

#ifndef SHININESS
#define SHININESS Shininess
#endif


// Camera position:
uniform vec3 CamPos;

// Model properties:
uniform vec3 Pos;
uniform vec3 XVec;
uniform vec3 YVec;
uniform vec3 ZVec;
uniform vec3 AmbientColor;
uniform vec3 DiffuseColor;
uniform vec3 SpecularColor;
uniform float Shininess;

// World light properties:

uniform vec3 AmbientLight;

#if DIRECTIONAL_LIGHTS > 0
uniform vec3 DirectionalLight0Dir;
uniform vec3 DirectionalLight0Color;
uniform float DirectionalLight0WrapAround;
#endif

#if DIRECTIONAL_LIGHTS > 1
uniform vec3 DirectionalLight1Dir;
uniform vec3 DirectionalLight1Color;
uniform float DirectionalLight1WrapAround;
#endif

#if DIRECTIONAL_LIGHTS > 2
uniform vec3 DirectionalLight2Dir;
uniform vec3 DirectionalLight2Color;
uniform float DirectionalLight2WrapAround;
#endif

#if DIRECTIONAL_LIGHTS > 3
uniform vec3 DirectionalLight3Dir;
uniform vec3 DirectionalLight3Color;
uniform float DirectionalLight3WrapAround;
#endif

#if POINT_LIGHTS > 0
uniform vec3 PointLight0Pos;
uniform vec3 PointLight0Color;
uniform float PointLight0Radius;
#endif

#if POINT_LIGHTS > 1
uniform vec3 PointLight1Pos;
uniform vec3 PointLight1Color;
uniform float PointLight1Radius;
#endif

#if POINT_LIGHTS > 2
uniform vec3 PointLight2Pos;
uniform vec3 PointLight2Color;
uniform float PointLight2Radius;
#endif

#if POINT_LIGHTS > 3
uniform vec3 PointLight3Pos;
uniform vec3 PointLight3Color;
uniform float PointLight3Radius;
#endif


#if LIGHT_QUALITY
// We always do ambient light and directional diffuse per-vertex.

varying vec3 Color;

#if DIRECTIONAL_LIGHTS || (POINT_LIGHTS && (LIGHT_QUALITY < 2))
float directional_light( vec3 normal, vec3 light_dir, float wrap_around )
{
	// Wrap-around 0 is true Lambert lighting, where illumination ends at 90 degrees.
	// Wrap-around 1 means 90-degree surfaces receive 50% illumination.
	
	return max( (dot( normal, light_dir ) + wrap_around) / (1.0 + wrap_around), 0.0 );
}
#endif
#endif

#if LIGHT_QUALITY >= 2
// Provide the fragment shader with the data it needs to operate per-pixel.

varying vec3 WorldPos;
varying vec3 WorldNormal;

#elif LIGHT_QUALITY
// Do the work per-vertex and interpolate for the fragment shader.

#if POINT_LIGHTS
float point_light( vec3 normal, vec3 light_vec, float wrap_around, float radius )
{
	float sqrt_intensity = radius / length(light_vec);
	return directional_light( normal, normalize(light_vec), wrap_around ) * sqrt_intensity * sqrt_intensity;
}

float point_light_specular( vec3 normal, vec3 light_vec, vec3 vec_to_cam, float wrap_around, float radius, float shininess )
{
	float sqrt_intensity = radius / length(light_vec);
	return pow( directional_light( normal, reflect(-1.0*normalize(light_vec),normal), wrap_around ), shininess ) * sqrt_intensity * sqrt_intensity;
}
#endif

#endif  // if LIGHT_QUALITY


#if BLASTPOINTS > 0

#if (BLASTPOINT_QUALITY >= 1) && (LIGHT_QUALITY < 2)
// Provide the fragment shader with the data it needs to operate per-pixel.
varying vec3 WorldPos;
#endif

#if BLASTPOINT_QUALITY <= 0
uniform vec3 BlastPoint[ BLASTPOINTS ];
uniform float BlastRadius[ BLASTPOINTS ];
varying float BlastDarken;
#endif

#endif  // if BLASTPOINTS


void main( void )
{
	// Calculate the vertex in worldspace and apply it to the rendering matrices.
	vec4 world_vertex = vec4( Pos.x + dot(gl_Vertex.xyz,XVec), Pos.y + dot(gl_Vertex.xyz,YVec), Pos.z + dot(gl_Vertex.xyz,ZVec), gl_Vertex.w );
	gl_Position = gl_ModelViewProjectionMatrix * world_vertex;
	
	// Pass along texture coordinates to be interpolated for the fragment shader.
	gl_TexCoord[0] = gl_MultiTexCoord0;
	
	#if BLASTPOINTS > 0
		#if BLASTPOINT_QUALITY >= 1
			#if LIGHT_QUALITY < 2
				// Get the position of each vertex to interpolate in the fragment shader.
				WorldPos = world_vertex.xyz;
			#endif
		#else
			// Low quality blastpoint is done per vertex.
			BlastDarken = 1.0;
			
			float blast_radius = 0.0;
			for( int i = 0; i < BLASTPOINTS; i ++ )
			{
				float dist = length( BlastPoint[ i ] - world_vertex.xyz );
				float dark = dist / max( BlastRadius[ i ], dist );
				#if BLASTPOINT_QUALITY >= 0
					BlastDarken = min( BlastDarken * pow( dark, 0.375 ), dark );
				#else
					BlastDarken = min( BlastDarken, dark );
				#endif
				blast_radius = max( blast_radius, BlastRadius[ i ] );
			}
			
			// Avoid darkening screens and glowing engines.
			BlastDarken = min( 1.0, max( BlastDarken, length(AmbientColor) * 10.0 ) );
		#endif
	#endif
	
	#if LIGHT_QUALITY >= 2
		// We'll do most of the work in the fragment shader, per-pixel.
		
		// Calculate the normal vector in worldspace.
		WorldNormal = normalize( vec3( dot(gl_Normal,XVec), dot(gl_Normal,YVec), dot(gl_Normal,ZVec) ) );
		
		// Get the position of each vertex to interpolate in the fragment shader.
		WorldPos = world_vertex.xyz;
		
		#ifdef FRONT_AND_BACK
			// Invert the normal vector if it is facing away from the camera.
			// This emulates the GL_FRONT_AND_BACK lighting style.
			vec3 vec_to_cam = normalize( CamPos - WorldPos );
			float normal_dot_cam = dot( WorldNormal, vec_to_cam );
			WorldNormal *= normal_dot_cam / abs(normal_dot_cam);
		#endif
		
		// Just do the ambient light and directional diffuse per-vertex.
		vec3 diffuse = AmbientLight;
		#if DIRECTIONAL_LIGHTS > 0
			diffuse += directional_light( WorldNormal, DirectionalLight0Dir, DirectionalLight0WrapAround ) * DirectionalLight0Color;
		#endif
		#if DIRECTIONAL_LIGHTS > 1
			diffuse += directional_light( WorldNormal, DirectionalLight1Dir, DirectionalLight1WrapAround ) * DirectionalLight1Color;
		#endif
		#if DIRECTIONAL_LIGHTS > 2
			diffuse += directional_light( WorldNormal, DirectionalLight2Dir, DirectionalLight2WrapAround ) * DirectionalLight2Color;
		#endif
		#if DIRECTIONAL_LIGHTS > 3
			diffuse += directional_light( WorldNormal, DirectionalLight3Dir, DirectionalLight3WrapAround ) * DirectionalLight3Color;
		#endif
		Color = AMBIENT_COLOR + (DIFFUSE_COLOR*diffuse);
		
	#elif LIGHT_QUALITY
		// We'll do most of the work here, per-vertex.
		
		// Calculate the normal vector in worldspace.
		vec3 world_normal = normalize( vec3( dot(gl_Normal,XVec), dot(gl_Normal,YVec), dot(gl_Normal,ZVec) ) );
		
		// Calculate the normalized vector from the vertex to the camera.
		vec3 vec_to_cam = normalize( CamPos - world_vertex.xyz );
		
		#ifdef FRONT_AND_BACK
			// Invert the normal vector if it is facing away from the camera.
			// This emulates the GL_FRONT_AND_BACK lighting style.
			float normal_dot_cam = dot( world_normal, vec_to_cam );
			world_normal *= normal_dot_cam / abs(normal_dot_cam);
		#endif
		
		// Calculate total color of diffuse and specular lighting.
		vec3 diffuse = AmbientLight;
		vec3 specular = vec3( 0.0, 0.0, 0.0 );
		
		#if DIRECTIONAL_LIGHTS > 0
			diffuse += directional_light( world_normal, DirectionalLight0Dir, DirectionalLight0WrapAround ) * DirectionalLight0Color;
			specular += pow( directional_light( vec_to_cam, reflect(-1.0*DirectionalLight0Dir,world_normal), DirectionalLight0WrapAround ), SHININESS ) * DirectionalLight0Color;
		#endif
		#if DIRECTIONAL_LIGHTS > 1
			diffuse += directional_light( world_normal, DirectionalLight1Dir, DirectionalLight1WrapAround ) * DirectionalLight1Color;
			specular += pow( directional_light( vec_to_cam, reflect(-1.0*DirectionalLight1Dir,world_normal), DirectionalLight1WrapAround ), SHININESS ) * DirectionalLight1Color;
		#endif
		#if DIRECTIONAL_LIGHTS > 2
			diffuse += directional_light( world_normal, DirectionalLight2Dir, DirectionalLight2WrapAround ) * DirectionalLight2Color;
			specular += pow( directional_light( vec_to_cam, reflect(-1.0*DirectionalLight2Dir,world_normal), DirectionalLight2WrapAround ), SHININESS ) * DirectionalLight2Color;
		#endif
		#if DIRECTIONAL_LIGHTS > 3
			diffuse += directional_light( world_normal, DirectionalLight3Dir, DirectionalLight3WrapAround ) * DirectionalLight3Color;
			specular += pow( directional_light( vec_to_cam, reflect(-1.0*DirectionalLight3Dir,world_normal), DirectionalLight3WrapAround ), SHININESS ) * DirectionalLight3Color;
		#endif
		#if POINT_LIGHTS > 0
			vec3 light_vec = PointLight0Pos - world_vertex.xyz;
			diffuse += point_light( world_normal, light_vec, 0.0, PointLight0Radius ) * PointLight0Color;
			specular += point_light_specular( world_normal, light_vec, vec_to_cam, 0.0, PointLight0Radius, SHININESS ) * PointLight0Color;
		#endif
		#if POINT_LIGHTS > 1
			light_vec = PointLight1Pos - world_vertex.xyz;
			diffuse += point_light( world_normal, light_vec, 0.0, PointLight1Radius ) * PointLight1Color;
			specular += point_light_specular( world_normal, light_vec, vec_to_cam, 0.0, PointLight1Radius, SHININESS ) * PointLight1Color;
		#endif
		#if POINT_LIGHTS > 2
			light_vec = PointLight2Pos - world_vertex.xyz;
			diffuse += point_light( world_normal, light_vec, 0.0, PointLight2Radius ) * PointLight2Color;
			specular += point_light_specular( world_normal, light_vec, vec_to_cam, 0.0, PointLight2Radius, SHININESS ) * PointLight2Color;
		#endif
		#if POINT_LIGHTS > 3
			light_vec = PointLight3Pos - world_vertex.xyz;
			diffuse += point_light( world_normal, light_vec, 0.0, PointLight3Radius ) * PointLight3Color;
			specular += point_light_specular( world_normal, light_vec, vec_to_cam, 0.0, PointLight3Radius, SHININESS ) * PointLight3Color;
		#endif
		
		Color = AMBIENT_COLOR + (DIFFUSE_COLOR*diffuse) + (SPECULAR_COLOR*specular);
	#endif
}
