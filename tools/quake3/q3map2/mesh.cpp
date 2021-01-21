/* -------------------------------------------------------------------------------

   Copyright (C) 1999-2007 id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

   ----------------------------------------------------------------------------------

   This code has been altered significantly from its original form, to support
   several games based on the Quake III Arena engine, in the form of "Q3Map2."

   ------------------------------------------------------------------------------- */



/* marker */
#define MESH_C



/* dependencies */
#include "q3map2.h"



/*
   LerpDrawVert()
   returns an 50/50 interpolated vert
 */

void LerpDrawVert( bspDrawVert_t *a, bspDrawVert_t *b, bspDrawVert_t *out ){
	int k;


	out->xyz[ 0 ] = 0.5 * ( a->xyz[ 0 ] + b->xyz[ 0 ] );
	out->xyz[ 1 ] = 0.5 * ( a->xyz[ 1 ] + b->xyz[ 1 ] );
	out->xyz[ 2 ] = 0.5 * ( a->xyz[ 2 ] + b->xyz[ 2 ] );

	out->st[ 0 ] = 0.5 * ( a->st[ 0 ] + b->st[ 0 ] );
	out->st[ 1 ] = 0.5 * ( a->st[ 1 ] + b->st[ 1 ] );

	for ( k = 0; k < MAX_LIGHTMAPS; k++ )
	{
		out->lightmap[ k ][ 0 ] = 0.5f * ( a->lightmap[ k ][ 0 ] + b->lightmap[ k ][ 0 ] );
		out->lightmap[ k ][ 1 ] = 0.5f * ( a->lightmap[ k ][ 1 ] + b->lightmap[ k ][ 1 ] );
		out->color[ k ][ 0 ] = ( a->color[ k ][ 0 ] + b->color[ k ][ 0 ] ) >> 1;
		out->color[ k ][ 1 ] = ( a->color[ k ][ 1 ] + b->color[ k ][ 1 ] ) >> 1;
		out->color[ k ][ 2 ] = ( a->color[ k ][ 2 ] + b->color[ k ][ 2 ] ) >> 1;
		out->color[ k ][ 3 ] = ( a->color[ k ][ 3 ] + b->color[ k ][ 3 ] ) >> 1;
	}

	/* ydnar: added normal interpolation */
	out->normal[ 0 ] = 0.5f * ( a->normal[ 0 ] + b->normal[ 0 ] );
	out->normal[ 1 ] = 0.5f * ( a->normal[ 1 ] + b->normal[ 1 ] );
	out->normal[ 2 ] = 0.5f * ( a->normal[ 2 ] + b->normal[ 2 ] );

	/* if the interpolant created a bogus normal, just copy the normal from a */
	if ( VectorNormalize( out->normal, out->normal ) == 0 ) {
		VectorCopy( a->normal, out->normal );
	}
}



/*
   LerpDrawVertAmount()
   returns a biased interpolated vert
 */

void LerpDrawVertAmount( bspDrawVert_t *a, bspDrawVert_t *b, float amount, bspDrawVert_t *out ){
	int k;


	out->xyz[ 0 ] = a->xyz[ 0 ] + amount * ( b->xyz[ 0 ] - a->xyz[ 0 ] );
	out->xyz[ 1 ] = a->xyz[ 1 ] + amount * ( b->xyz[ 1 ] - a->xyz[ 1 ] );
	out->xyz[ 2 ] = a->xyz[ 2 ] + amount * ( b->xyz[ 2 ] - a->xyz[ 2 ] );

	out->st[ 0 ] = a->st[ 0 ] + amount * ( b->st[ 0 ] - a->st[ 0 ] );
	out->st[ 1 ] = a->st[ 1 ] + amount * ( b->st[ 1 ] - a->st[ 1 ] );

	for ( k = 0; k < MAX_LIGHTMAPS; k++ )
	{
		out->lightmap[ k ][ 0 ] = a->lightmap[ k ][ 0 ] + amount * ( b->lightmap[ k ][ 0 ] - a->lightmap[ k ][ 0 ] );
		out->lightmap[ k ][ 1 ] = a->lightmap[ k ][ 1 ] + amount * ( b->lightmap[ k ][ 1 ] - a->lightmap[ k ][ 1 ] );
		out->color[ k ][ 0 ] = a->color[ k ][ 0 ] + amount * ( b->color[ k ][ 0 ] - a->color[ k ][ 0 ] );
		out->color[ k ][ 1 ] = a->color[ k ][ 1 ] + amount * ( b->color[ k ][ 1 ] - a->color[ k ][ 1 ] );
		out->color[ k ][ 2 ] = a->color[ k ][ 2 ] + amount * ( b->color[ k ][ 2 ] - a->color[ k ][ 2 ] );
		out->color[ k ][ 3 ] = a->color[ k ][ 3 ] + amount * ( b->color[ k ][ 3 ] - a->color[ k ][ 3 ] );
	}

	out->normal[ 0 ] = a->normal[ 0 ] + amount * ( b->normal[ 0 ] - a->normal[ 0 ] );
	out->normal[ 1 ] = a->normal[ 1 ] + amount * ( b->normal[ 1 ] - a->normal[ 1 ] );
	out->normal[ 2 ] = a->normal[ 2 ] + amount * ( b->normal[ 2 ] - a->normal[ 2 ] );

	/* if the interpolant created a bogus normal, just copy the normal from a */
	if ( VectorNormalize( out->normal, out->normal ) == 0 ) {
		VectorCopy( a->normal, out->normal );
	}
}


void FreeMesh( mesh_t *m ) {
	free( m->verts );
	free( m );
}

void PrintMesh( mesh_t *m ) {
	int i, j;

	for ( i = 0 ; i < m->height ; i++ ) {
		for ( j = 0 ; j < m->width ; j++ ) {
			Sys_Printf( "(%5.2f %5.2f %5.2f) "
						, m->verts[i * m->width + j].xyz[0]
						, m->verts[i * m->width + j].xyz[1]
						, m->verts[i * m->width + j].xyz[2] );
		}
		Sys_Printf( "\n" );
	}
}


mesh_t *CopyMesh( mesh_t *mesh ) {
	mesh_t  *out;
	int size;

	out = safe_malloc( sizeof( *out ) );
	out->width = mesh->width;
	out->height = mesh->height;

	size = out->width * out->height * sizeof( *out->verts );
	out->verts = safe_malloc( size );
	memcpy( out->verts, mesh->verts, size );

	return out;
}


/*
   TransposeMesh()
   returns a transposed copy of the mesh, freeing the original
 */

mesh_t *TransposeMesh( mesh_t *in ) {
	int w, h;
	mesh_t      *out;

	out = safe_malloc( sizeof( *out ) );
	out->width = in->height;
	out->height = in->width;
	out->verts = safe_malloc( out->width * out->height * sizeof( bspDrawVert_t ) );

	for ( h = 0 ; h < in->height ; h++ ) {
		for ( w = 0 ; w < in->width ; w++ ) {
			out->verts[ w * in->height + h ] = in->verts[ h * in->width + w ];
		}
	}

	FreeMesh( in );

	return out;
}

void InvertMesh( mesh_t *in ) {
	int w, h;
	bspDrawVert_t temp;

	for ( h = 0 ; h < in->height ; h++ ) {
		for ( w = 0 ; w < in->width / 2 ; w++ ) {
			temp = in->verts[ h * in->width + w ];
			in->verts[ h * in->width + w ] = in->verts[ h * in->width + in->width - 1 - w ];
			in->verts[ h * in->width + in->width - 1 - w ] = temp;
		}
	}
}

/*
   =================
   MakeMeshNormals

   =================
 */
void MakeMeshNormals( mesh_t in ){
	int i, j, k, dist;
	vec3_t normal;
	vec3_t sum;
	int count;
	vec3_t base;
	vec3_t delta;
	int x, y;
	bspDrawVert_t   *dv;
	vec3_t around[8], temp;
	bool good[8];
	bool wrapWidth, wrapHeight;
	float len;
	int neighbors[8][2] =
	{
		{0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}, {-1,1}
	};


	wrapWidth = false;
	for ( i = 0 ; i < in.height ; i++ ) {
		VectorSubtract( in.verts[i * in.width].xyz,
						in.verts[i * in.width + in.width - 1].xyz, delta );
		len = VectorLength( delta );
		if ( len > 1.0 ) {
			break;
		}
	}
	if ( i == in.height ) {
		wrapWidth = true;
	}

	wrapHeight = false;
	for ( i = 0 ; i < in.width ; i++ ) {
		VectorSubtract( in.verts[i].xyz,
						in.verts[i + ( in.height - 1 ) * in.width].xyz, delta );
		len = VectorLength( delta );
		if ( len > 1.0 ) {
			break;
		}
	}
	if ( i == in.width ) {
		wrapHeight = true;
	}


	for ( i = 0 ; i < in.width ; i++ ) {
		for ( j = 0 ; j < in.height ; j++ ) {
			count = 0;
			dv = &in.verts[j * in.width + i];
			VectorCopy( dv->xyz, base );
			for ( k = 0 ; k < 8 ; k++ ) {
				VectorClear( around[k] );
				good[k] = false;

				for ( dist = 1 ; dist <= 3 ; dist++ ) {
					x = i + neighbors[k][0] * dist;
					y = j + neighbors[k][1] * dist;
					if ( wrapWidth ) {
						if ( x < 0 ) {
							x = in.width - 1 + x;
						}
						else if ( x >= in.width ) {
							x = 1 + x - in.width;
						}
					}
					if ( wrapHeight ) {
						if ( y < 0 ) {
							y = in.height - 1 + y;
						}
						else if ( y >= in.height ) {
							y = 1 + y - in.height;
						}
					}

					if ( x < 0 || x >= in.width || y < 0 || y >= in.height ) {
						break;                  // edge of patch
					}
					VectorSubtract( in.verts[y * in.width + x].xyz, base, temp );
					if ( VectorNormalize( temp, temp ) == 0 ) {
						continue;               // degenerate edge, get more dist
					}
					else {
						good[k] = true;
						VectorCopy( temp, around[k] );
						break;                  // good edge
					}
				}
			}

			VectorClear( sum );
			for ( k = 0 ; k < 8 ; k++ ) {
				if ( !good[k] || !good[( k + 1 ) & 7] ) {
					continue;   // didn't get two points
				}
				CrossProduct( around[( k + 1 ) & 7], around[k], normal );
				if ( VectorNormalize( normal, normal ) == 0 ) {
					continue;
				}
				VectorAdd( normal, sum, sum );
				count++;
			}
			if ( count == 0 ) {
//Sys_Printf("bad normal\n");
				count = 1;
			}
			VectorNormalize( sum, dv->normal );
		}
	}
}

/*
   PutMeshOnCurve()
   drops the aproximating points onto the curve
   ydnar: fixme: make this use LerpDrawVert() rather than this complicated mess
 */

void PutMeshOnCurve( mesh_t in ) {
	int i, j, l, m;
	float prev, next;


	// put all the aproximating points on the curve
	for ( i = 0 ; i < in.width ; i++ ) {
		for ( j = 1 ; j < in.height ; j += 2 ) {
			for ( l = 0 ; l < 3 ; l++ ) {
				prev = ( in.verts[j * in.width + i].xyz[l] + in.verts[( j + 1 ) * in.width + i].xyz[l] ) * 0.5;
				next = ( in.verts[j * in.width + i].xyz[l] + in.verts[( j - 1 ) * in.width + i].xyz[l] ) * 0.5;
				in.verts[j * in.width + i].xyz[l] = ( prev + next ) * 0.5;

				/* ydnar: interpolating st coords */
				if ( l < 2 ) {
					prev = ( in.verts[j * in.width + i].st[l] + in.verts[( j + 1 ) * in.width + i].st[l] ) * 0.5;
					next = ( in.verts[j * in.width + i].st[l] + in.verts[( j - 1 ) * in.width + i].st[l] ) * 0.5;
					in.verts[j * in.width + i].st[l] = ( prev + next ) * 0.5;

					for ( m = 0; m < MAX_LIGHTMAPS; m++ )
					{
						prev = ( in.verts[j * in.width + i].lightmap[ m ][l] + in.verts[( j + 1 ) * in.width + i].lightmap[ m ][l] ) * 0.5;
						next = ( in.verts[j * in.width + i].lightmap[ m ][l] + in.verts[( j - 1 ) * in.width + i].lightmap[ m ][l] ) * 0.5;
						in.verts[j * in.width + i].lightmap[ m ][l] = ( prev + next ) * 0.5;
					}
				}
			}
		}
	}

	for ( j = 0 ; j < in.height ; j++ ) {
		for ( i = 1 ; i < in.width ; i += 2 ) {
			for ( l = 0 ; l < 3 ; l++ ) {
				prev = ( in.verts[j * in.width + i].xyz[l] + in.verts[j * in.width + i + 1].xyz[l] ) * 0.5;
				next = ( in.verts[j * in.width + i].xyz[l] + in.verts[j * in.width + i - 1].xyz[l] ) * 0.5;
				in.verts[j * in.width + i].xyz[l] = ( prev + next ) * 0.5;

				/* ydnar: interpolating st coords */
				if ( l < 2 ) {
					prev = ( in.verts[j * in.width + i].st[l] + in.verts[j * in.width + i + 1].st[l] ) * 0.5;
					next = ( in.verts[j * in.width + i].st[l] + in.verts[j * in.width + i - 1].st[l] ) * 0.5;
					in.verts[j * in.width + i].st[l] = ( prev + next ) * 0.5;

					for ( m = 0; m < MAX_LIGHTMAPS; m++ )
					{
						prev = ( in.verts[j * in.width + i].lightmap[ m ][l] + in.verts[j * in.width + i + 1].lightmap[ m ][l] ) * 0.5;
						next = ( in.verts[j * in.width + i].lightmap[ m ][l] + in.verts[j * in.width + i - 1].lightmap[ m ][l] ) * 0.5;
						in.verts[j * in.width + i].lightmap[ m ][l] = ( prev + next ) * 0.5;
					}
				}
			}
		}
	}
}


/*
   =================
   SubdivideMesh

   =================
 */
mesh_t *SubdivideMesh( mesh_t in, float maxError, float minLength ){
	int i, j, k, l;
	bspDrawVert_t prev, next, mid;
	vec3_t prevxyz, nextxyz, midxyz;
	vec3_t delta;
	float len;
	mesh_t out;

	bspDrawVert_t expand[MAX_EXPANDED_AXIS][MAX_EXPANDED_AXIS];


	out.width = in.width;
	out.height = in.height;

	for ( i = 0 ; i < in.width ; i++ ) {
		for ( j = 0 ; j < in.height ; j++ ) {
			expand[j][i] = in.verts[j * in.width + i];
		}
	}

	// horizontal subdivisions
	for ( j = 0 ; j + 2 < out.width ; j += 2 ) {
		// check subdivided midpoints against control points
		for ( i = 0 ; i < out.height ; i++ ) {
			for ( l = 0 ; l < 3 ; l++ ) {
				prevxyz[l] = expand[i][j + 1].xyz[l] - expand[i][j].xyz[l];
				nextxyz[l] = expand[i][j + 2].xyz[l] - expand[i][j + 1].xyz[l];
				midxyz[l] = ( expand[i][j].xyz[l] + expand[i][j + 1].xyz[l] * 2
							  + expand[i][j + 2].xyz[l] ) * 0.25;
			}

			// if the span length is too long, force a subdivision
			if ( VectorLength( prevxyz ) > minLength
				 || VectorLength( nextxyz ) > minLength ) {
				break;
			}

			// see if this midpoint is off far enough to subdivide
			VectorSubtract( expand[i][j + 1].xyz, midxyz, delta );
			len = VectorLength( delta );
			if ( len > maxError ) {
				break;
			}
		}

		if ( out.width + 2 >= MAX_EXPANDED_AXIS ) {
			break;  // can't subdivide any more
		}

		if ( i == out.height ) {
			continue;   // didn't need subdivision
		}

		// insert two columns and replace the peak
		out.width += 2;

		for ( i = 0 ; i < out.height ; i++ ) {
			LerpDrawVert( &expand[i][j], &expand[i][j + 1], &prev );
			LerpDrawVert( &expand[i][j + 1], &expand[i][j + 2], &next );
			LerpDrawVert( &prev, &next, &mid );

			for ( k = out.width - 1 ; k > j + 3 ; k-- ) {
				expand[i][k] = expand[i][k - 2];
			}
			expand[i][j + 1] = prev;
			expand[i][j + 2] = mid;
			expand[i][j + 3] = next;
		}

		// back up and recheck this set again, it may need more subdivision
		j -= 2;

	}

	// vertical subdivisions
	for ( j = 0 ; j + 2 < out.height ; j += 2 ) {
		// check subdivided midpoints against control points
		for ( i = 0 ; i < out.width ; i++ ) {
			for ( l = 0 ; l < 3 ; l++ ) {
				prevxyz[l] = expand[j + 1][i].xyz[l] - expand[j][i].xyz[l];
				nextxyz[l] = expand[j + 2][i].xyz[l] - expand[j + 1][i].xyz[l];
				midxyz[l] = ( expand[j][i].xyz[l] + expand[j + 1][i].xyz[l] * 2
							  + expand[j + 2][i].xyz[l] ) * 0.25;
			}

			// if the span length is too long, force a subdivision
			if ( VectorLength( prevxyz ) > minLength
				 || VectorLength( nextxyz ) > minLength ) {
				break;
			}
			// see if this midpoint is off far enough to subdivide
			VectorSubtract( expand[j + 1][i].xyz, midxyz, delta );
			len = VectorLength( delta );
			if ( len > maxError ) {
				break;
			}
		}

		if ( out.height + 2 >= MAX_EXPANDED_AXIS ) {
			break;  // can't subdivide any more
		}

		if ( i == out.width ) {
			continue;   // didn't need subdivision
		}

		// insert two columns and replace the peak
		out.height += 2;

		for ( i = 0 ; i < out.width ; i++ ) {
			LerpDrawVert( &expand[j][i], &expand[j + 1][i], &prev );
			LerpDrawVert( &expand[j + 1][i], &expand[j + 2][i], &next );
			LerpDrawVert( &prev, &next, &mid );

			for ( k = out.height - 1 ; k > j + 3 ; k-- ) {
				expand[k][i] = expand[k - 2][i];
			}
			expand[j + 1][i] = prev;
			expand[j + 2][i] = mid;
			expand[j + 3][i] = next;
		}

		// back up and recheck this set again, it may need more subdivision
		j -= 2;

	}

	// collapse the verts

	out.verts = &expand[0][0];
	for ( i = 1 ; i < out.height ; i++ ) {
		memmove( &out.verts[i * out.width], expand[i], out.width * sizeof( bspDrawVert_t ) );
	}

	return CopyMesh( &out );
}



/*
   IterationsForCurve() - ydnar
   given a curve of a certain length, return the number of subdivision iterations
   note: this is affected by subdivision amount
 */

int IterationsForCurve( float len, int subdivisions ){
	int iterations, facets;


	/* calculate the number of subdivisions */
	for ( iterations = 0; iterations < 3; iterations++ )
	{
		facets = subdivisions * 16 * pow( 2, iterations );
		if ( facets >= len ) {
			break;
		}
	}

	/* return to caller */
	return iterations;
}


/*
   SubdivideMesh2() - ydnar
   subdivides each mesh quad a specified number of times
 */

mesh_t *SubdivideMesh2( mesh_t in, int iterations ){
	int i, j, k;
	bspDrawVert_t prev, next, mid;
	mesh_t out;

	bspDrawVert_t expand[ MAX_EXPANDED_AXIS ][ MAX_EXPANDED_AXIS ];


	/* initial setup */
	out.width = in.width;
	out.height = in.height;
	for ( i = 0; i < in.width; i++ )
	{
		for ( j = 0; j < in.height; j++ )
			expand[ j ][ i ] = in.verts[ j * in.width + i ];
	}

	/* keep chopping */
	for ( ; iterations > 0; iterations-- )
	{
		/* horizontal subdivisions */
		for ( j = 0; j + 2 < out.width; j += 4 )
		{
			/* check size limit */
			if ( out.width + 2 >= MAX_EXPANDED_AXIS ) {
				break;
			}

			/* insert two columns and replace the peak */
			out.width += 2;
			for ( i = 0; i < out.height; i++ )
			{
				LerpDrawVert( &expand[ i ][ j ], &expand[ i ][ j + 1 ], &prev );
				LerpDrawVert( &expand[ i ][ j + 1 ], &expand[ i ][ j + 2 ], &next );
				LerpDrawVert( &prev, &next, &mid );

				for ( k = out.width - 1 ; k > j + 3; k-- )
					expand [ i ][ k ] = expand[ i ][ k - 2 ];
				expand[ i ][ j + 1 ] = prev;
				expand[ i ][ j + 2 ] = mid;
				expand[ i ][ j + 3 ] = next;
			}

		}

		/* vertical subdivisions */
		for ( j = 0; j + 2 < out.height; j += 4 )
		{
			/* check size limit */
			if ( out.height + 2 >= MAX_EXPANDED_AXIS ) {
				break;
			}

			/* insert two columns and replace the peak */
			out.height += 2;
			for ( i = 0; i < out.width; i++ )
			{
				LerpDrawVert( &expand[ j ][ i ], &expand[ j + 1 ][ i ], &prev );
				LerpDrawVert( &expand[ j + 1 ][ i ], &expand[ j + 2 ][ i ], &next );
				LerpDrawVert( &prev, &next, &mid );

				for ( k = out.height - 1; k > j  +  3; k-- )
					expand[ k ][ i ] = expand[ k - 2 ][ i ];
				expand[ j + 1 ][ i ] = prev;
				expand[ j + 2 ][ i ] = mid;
				expand[ j + 3 ][ i ] = next;
			}
		}
	}

	/* collapse the verts */
	out.verts = &expand[ 0 ][ 0 ];
	for ( i = 1; i < out.height; i++ )
		memmove( &out.verts[ i * out.width ], expand[ i ], out.width * sizeof( bspDrawVert_t ) );

	/* return to sender */
	return CopyMesh( &out );
}







/*
   ================
   ProjectPointOntoVector
   ================
 */
void ProjectPointOntoVector( vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj ){
	vec3_t pVec, vec;

	VectorSubtract( point, vStart, pVec );
	VectorSubtract( vEnd, vStart, vec );
	VectorNormalize( vec, vec );
	// project onto the directional vector for this segment
	VectorMA( vStart, DotProduct( pVec, vec ), vec, vProj );
}

/*
   ================
   RemoveLinearMeshColumsRows
   ================
 */
mesh_t *RemoveLinearMeshColumnsRows( mesh_t *in ) {
	int i, j, k;
	float len, maxLength;
	vec3_t proj, dir;
	mesh_t out;

	bspDrawVert_t expand[MAX_EXPANDED_AXIS][MAX_EXPANDED_AXIS];


	out.width = in->width;
	out.height = in->height;

	for ( i = 0 ; i < in->width ; i++ ) {
		for ( j = 0 ; j < in->height ; j++ ) {
			expand[j][i] = in->verts[j * in->width + i];
		}
	}

	for ( j = 1 ; j < out.width - 1; j++ ) {
		maxLength = 0;
		for ( i = 0 ; i < out.height ; i++ ) {
			ProjectPointOntoVector( expand[i][j].xyz, expand[i][j - 1].xyz, expand[i][j + 1].xyz, proj );
			VectorSubtract( expand[i][j].xyz, proj, dir );
			len = VectorLength( dir );
			if ( len > maxLength ) {
				maxLength = len;
			}
		}
		if ( maxLength < 0.1 ) {
			out.width--;
			for ( i = 0 ; i < out.height ; i++ ) {
				for ( k = j; k < out.width; k++ ) {
					expand[i][k] = expand[i][k + 1];
				}
			}
			j--;
		}
	}
	for ( j = 1 ; j < out.height - 1; j++ ) {
		maxLength = 0;
		for ( i = 0 ; i < out.width ; i++ ) {
			ProjectPointOntoVector( expand[j][i].xyz, expand[j - 1][i].xyz, expand[j + 1][i].xyz, proj );
			VectorSubtract( expand[j][i].xyz, proj, dir );
			len = VectorLength( dir );
			if ( len > maxLength ) {
				maxLength = len;
			}
		}
		if ( maxLength < 0.1 ) {
			out.height--;
			for ( i = 0 ; i < out.width ; i++ ) {
				for ( k = j; k < out.height; k++ ) {
					expand[k][i] = expand[k + 1][i];
				}
			}
			j--;
		}
	}
	// collapse the verts
	out.verts = &expand[0][0];
	for ( i = 1 ; i < out.height ; i++ ) {
		memmove( &out.verts[i * out.width], expand[i], out.width * sizeof( bspDrawVert_t ) );
	}

	return CopyMesh( &out );
}



/*
   =================
   SubdivideMeshQuads
   =================
 */
mesh_t *SubdivideMeshQuads( mesh_t *in, float minLength, int maxsize, int *widthtable, int *heighttable ){
	int i, j, k, w, h, maxsubdivisions, subdivisions;
	vec3_t dir;
	float length, maxLength, amount;
	mesh_t out;
	bspDrawVert_t expand[MAX_EXPANDED_AXIS][MAX_EXPANDED_AXIS];

	out.width = in->width;
	out.height = in->height;

	for ( i = 0 ; i < in->width ; i++ ) {
		for ( j = 0 ; j < in->height ; j++ ) {
			expand[j][i] = in->verts[j * in->width + i];
		}
	}

	if ( maxsize > MAX_EXPANDED_AXIS ) {
		Error( "SubdivideMeshQuads: maxsize > MAX_EXPANDED_AXIS" );
	}

	// horizontal subdivisions

	maxsubdivisions = ( maxsize - in->width ) / ( in->width - 1 );

	for ( w = 0, j = 0 ; w < in->width - 1; w++, j += subdivisions + 1 ) {
		maxLength = 0;
		for ( i = 0 ; i < out.height ; i++ ) {
			VectorSubtract( expand[i][j + 1].xyz, expand[i][j].xyz, dir );
			length = VectorLength( dir );
			if ( length > maxLength ) {
				maxLength = length;
			}
		}

		subdivisions = (int) ( maxLength / minLength );
		if ( subdivisions > maxsubdivisions ) {
			subdivisions = maxsubdivisions;
		}

		widthtable[w] = subdivisions + 1;
		if ( subdivisions <= 0 ) {
			continue;
		}

		out.width += subdivisions;

		for ( i = 0 ; i < out.height ; i++ ) {
			for ( k = out.width - 1 ; k > j + subdivisions; k-- ) {
				expand[i][k] = expand[i][k - subdivisions];
			}
			for ( k = 1; k <= subdivisions; k++ )
			{
				amount = (float) k / ( subdivisions + 1 );
				LerpDrawVertAmount( &expand[i][j], &expand[i][j + subdivisions + 1], amount, &expand[i][j + k] );
			}
		}
	}

	maxsubdivisions = ( maxsize - in->height ) / ( in->height - 1 );

	for ( h = 0, j = 0 ; h < in->height - 1; h++, j += subdivisions + 1 ) {
		maxLength = 0;
		for ( i = 0 ; i < out.width ; i++ ) {
			VectorSubtract( expand[j + 1][i].xyz, expand[j][i].xyz, dir );
			length = VectorLength( dir );
			if ( length  > maxLength ) {
				maxLength = length;
			}
		}

		subdivisions = (int) ( maxLength / minLength );
		if ( subdivisions > maxsubdivisions ) {
			subdivisions = maxsubdivisions;
		}

		heighttable[h] = subdivisions + 1;
		if ( subdivisions <= 0 ) {
			continue;
		}

		out.height += subdivisions;

		for ( i = 0 ; i < out.width ; i++ ) {
			for ( k = out.height - 1 ; k > j + subdivisions; k-- ) {
				expand[k][i] = expand[k - subdivisions][i];
			}
			for ( k = 1; k <= subdivisions; k++ )
			{
				amount = (float) k / ( subdivisions + 1 );
				LerpDrawVertAmount( &expand[j][i], &expand[j + subdivisions + 1][i], amount, &expand[j + k][i] );
			}
		}
	}

	// collapse the verts
	out.verts = &expand[0][0];
	for ( i = 1 ; i < out.height ; i++ ) {
		memmove( &out.verts[i * out.width], expand[i], out.width * sizeof( bspDrawVert_t ) );
	}

	return CopyMesh( &out );
}
