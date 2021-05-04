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


#if LIGHT_QUALITY
// Interpolates automatically from vertex shader.
varying vec3 Color;
#endif


#if LIGHT_QUALITY >= 2

uniform vec3 AmbientLight;
uniform vec3 AmbientColor;
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

#endif


void main( void )
{
	// Apply interpolated texture coordinates.
	gl_FragColor = texture2D( Texture, gl_TexCoord[0].st );
	
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
	gl_FragColor.a *= Alpha;
	
	// Apply Death Star distance fog.
	gl_FragColor.rgb *= clamp( 65536.0 - gl_FragCoord.z * 65536.0, 0.0, 1.0 );
}
