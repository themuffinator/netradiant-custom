/// ============================================================================
/*
Copyright (C) 2026 VibeRadiant contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
*/
/// ============================================================================

uniform sampler2D	u_diffusemap;
uniform vec3		u_light_origin;
uniform vec3		u_light_color;

varying vec2		var_tex_diffuse;
varying vec3		var_vertex;
varying vec3		var_normal;
varying vec3		var_lightcoord;

void	main()
{
	vec3 normal = normalize( var_normal );
	vec3 L = normalize( u_light_origin - var_vertex );
	float ndotl = max( dot( normal, L ), 0.0 );

	vec3 base = texture2D( u_diffusemap, var_tex_diffuse ).rgb;

	vec3 to_center = var_lightcoord - vec3( 0.5, 0.5, 0.5 );
	float dist = length( to_center ) * 2.0;
	float atten = clamp( 1.0 - dist, 0.0, 1.0 );

	vec3 color = base * u_light_color * ndotl * atten;
	gl_FragColor = vec4( color, 1.0 );
}
