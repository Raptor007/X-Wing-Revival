#if BLASTPOINTS > 0
#undef BLASTPOINTS
#define BLASTPOINTS 0
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


// Material properties:
uniform sampler2D Texture;
uniform float Alpha;
uniform vec3 AmbientColor;

#if LIGHT_QUALITY
// Interpolates automatically from vertex shader.
varying vec3 Color;
#endif


#if LIGHT_QUALITY >= 2

uniform vec3 AmbientLight;
uniform vec3 DiffuseColor;
uniform vec3 SpecularColor;
uniform float Shininess;

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

uniform vec3 CamPos;
varying vec3 WorldPos;
varying vec3 WorldNormal;

#if DIRECTIONAL_LIGHTS || POINT_LIGHTS
float directional_light( vec3 normal, vec3 light_dir, float wrap_around )
{
	// Wrap-around 0 is true Lambert lighting, where illumination ends at 90 degrees.
	// Wrap-around 1 means 90-degree surfaces receive 50% illumination.
	
	return max( (dot( normal, light_dir ) + wrap_around) / (1.0 + wrap_around), 0.0 );
}
#endif

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

#elif BLASTPOINTS > 0 // && (LIGHT_QUALITY < 2)
varying vec3 WorldPos;
#endif


#if BLASTPOINTS > 0

#if BLASTPOINT_QUALITY >= 1
uniform vec3 BlastPoint[ BLASTPOINTS ];
uniform float BlastRadius[ BLASTPOINTS ];
#else
varying float BlastDarken;
#endif

#endif


void main( void )
{
	// Apply interpolated texture coordinates.
	gl_FragColor = texture2D( Texture, gl_TexCoord[0].st );
	
	#if BLASTPOINTS > 0
		#if BLASTPOINT_QUALITY >= 1
			float darken = 1.0;
			
			float blast_radius = 0.0;
			for( int i = 0; i < BLASTPOINTS; i ++ )
			{
				float dist = length( BlastPoint[ i ] - WorldPos );
				float dark = dist / max( BlastRadius[ i ], dist );
				#if BLASTPOINT_QUALITY >= 3
					darken = min( darken * pow( dark, 0.375 ), dark );
				#else
					darken = min( darken, dark );
				#endif
				blast_radius = max( blast_radius, BlastRadius[ i ] );
			}
			
			// Avoid darkening screens and glowing engines.
			darken = min( 1.0, max( darken, length(AmbientColor) * 10.0 ) );
		#else
			float darken = BlastDarken;
		#endif
		
		#if BLASTPOINT_QUALITY >= 2
			// Adjust darkness curve by blast radius.
			darken = pow( darken, max( 1., min( 0.2, blast_radius * 0.0625 ) ) );
			
			#if LIGHT_QUALITY >= 2
				float shininess = Shininess * 0.5 + 0.5 * darken;
				#undef SHININESS
				#define SHININESS shininess
			#endif
		#endif
		
		gl_FragColor.rgb *= 0.25 + 0.75 * darken;
	#endif
	
	#if LIGHT_QUALITY >= 2
		vec3 world_normal = normalize( WorldNormal );
		vec3 vec_to_cam = normalize( WorldPos - CamPos );
		vec3 diffuse = vec3( 0.0, 0.0, 0.0 );
		vec3 specular = vec3( 0.0, 0.0, 0.0 );
		
		#if DIRECTIONAL_LIGHTS > 0
			specular += pow( directional_light( vec_to_cam, reflect(DirectionalLight0Dir,world_normal), DirectionalLight0WrapAround ), SHININESS ) * DirectionalLight0Color;
		#endif
		#if DIRECTIONAL_LIGHTS > 1
			specular += pow( directional_light( vec_to_cam, reflect(DirectionalLight1Dir,world_normal), DirectionalLight1WrapAround ), SHININESS ) * DirectionalLight1Color;
		#endif
		#if DIRECTIONAL_LIGHTS > 2
			specular += pow( directional_light( vec_to_cam, reflect(DirectionalLight2Dir,world_normal), DirectionalLight2WrapAround ), SHININESS ) * DirectionalLight2Color;
		#endif
		#if DIRECTIONAL_LIGHTS > 3
			specular += pow( directional_light( vec_to_cam, reflect(DirectionalLight3Dir,world_normal), DirectionalLight3WrapAround ), SHININESS ) * DirectionalLight3Color;
		#endif
		#if POINT_LIGHTS > 0
			vec3 light_vec = PointLight0Pos - WorldPos;
			diffuse += point_light( world_normal, light_vec, 0.0, PointLight0Radius ) * PointLight0Color;
			specular += point_light_specular( world_normal, light_vec, vec_to_cam, 0.0, PointLight0Radius, SHININESS ) * PointLight0Color;
		#endif
		#if POINT_LIGHTS > 1
			light_vec = PointLight1Pos - WorldPos;
			diffuse += point_light( world_normal, light_vec, 0.0, PointLight1Radius ) * PointLight1Color;
			specular += point_light_specular( world_normal, light_vec, vec_to_cam, 0.0, PointLight1Radius, SHININESS ) * PointLight1Color;
		#endif
		#if POINT_LIGHTS > 2
			light_vec = PointLight2Pos - WorldPos;
			diffuse += point_light( world_normal, light_vec, 0.0, PointLight2Radius ) * PointLight2Color;
			specular += point_light_specular( world_normal, light_vec, vec_to_cam, 0.0, PointLight2Radius, SHININESS ) * PointLight2Color;
		#endif
		#if POINT_LIGHTS > 3
			light_vec = PointLight3Pos - WorldPos;
			diffuse += point_light( world_normal, light_vec, 0.0, PointLight3Radius ) * PointLight3Color;
			specular += point_light_specular( world_normal, light_vec, vec_to_cam, 0.0, PointLight3Radius, SHININESS ) * PointLight3Color;
		#endif
		
		#if BLASTPOINTS > 0
			specular *= darken;
		#endif
		
		// Apply per-pixel calculated color.
		gl_FragColor.rgb *= Color + (DIFFUSE_COLOR*diffuse);
		
		// This acts like a white specular texture to allow highlights on dark surfaces.
		gl_FragColor.rgb += SPECULAR_COLOR*specular;
		gl_FragColor.a = max( max( gl_FragColor.a, specular.r ), max( specular.g, specular.b ) );
		
	#elif LIGHT_QUALITY
		// Apply per-vertex interpolated color.
		gl_FragColor.rgb *= Color;
	#endif
	
	// Apply material alpha.
	#if (BLASTPOINTS > 0) && (BLASTPOINT_QUALITY >= 2)
		gl_FragColor.a *= pow( Alpha, darken );
	#else
		gl_FragColor.a *= Alpha;
	#endif
}
