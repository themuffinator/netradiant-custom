/// ============================================================================
/*
Copyright (C) 2026 VibeRadiant contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
*/
/// ============================================================================

attribute vec4		attr_TexCoord0;

varying vec2		var_tex_diffuse;
varying vec3		var_vertex;
varying vec3		var_normal;
varying vec3		var_lightcoord;

void	main()
{
	gl_Position = ftransform();

	var_vertex = gl_Vertex.xyz;
	var_normal = gl_Normal.xyz;
	var_tex_diffuse = (gl_TextureMatrix[0] * attr_TexCoord0).st;
	var_lightcoord = (gl_TextureMatrix[3] * gl_Vertex).xyz;
}
