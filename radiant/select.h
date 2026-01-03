/*
   Copyright (C) 1999-2006 Id Software, Inc. and contributors.
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
 */

#pragma once

#include "generic/vector.h"
#include <string>

enum class TextureFindMatchMode
{
	Exact = 0,
	Contains,
	StartsWith,
	EndsWith,
	Wildcard,
	Regex,
};

enum class TextureReplaceMode
{
	ReplaceFull = 0,
	ReplaceMatch,
};

enum class TextureFindScope
{
	All = 0,
	Selected,
	SelectedFaces,
};

enum class TextureShaderFilter
{
	Any = 0,
	DefaultOnly,
	RealOnly,
};

enum class TextureUsageFilter
{
	Any = 0,
	InUseOnly,
	NotInUse,
};

enum class EntityFindScope
{
	All = 0,
	Selected,
};

struct TextureFindReplaceOptions
{
	std::string find;
	std::string replace;
	std::string includeFilter;
	std::string excludeFilter;
	std::string surfaceFlagsRequire;
	std::string surfaceFlagsExclude;
	std::string contentFlagsRequire;
	std::string contentFlagsExclude;
	TextureFindMatchMode matchMode = TextureFindMatchMode::Exact;
	TextureReplaceMode replaceMode = TextureReplaceMode::ReplaceFull;
	TextureFindScope scope = TextureFindScope::All;
	TextureShaderFilter shaderFilter = TextureShaderFilter::Any;
	TextureUsageFilter usageFilter = TextureUsageFilter::Any;
	bool caseSensitive = false;
	bool matchNameOnly = false;
	bool autoPrefix = true;
	bool visibleOnly = true;
	bool includeBrushes = true;
	bool includePatches = true;
	int minWidth = 0;
	int maxWidth = 0;
	int minHeight = 0;
	int maxHeight = 0;
};

struct EntityFindReplaceOptions
{
	std::string find;
	std::string replace;
	std::string keyFilter;
	std::string classFilter;
	TextureFindMatchMode matchMode = TextureFindMatchMode::Exact;
	TextureReplaceMode replaceMode = TextureReplaceMode::ReplaceFull;
	EntityFindScope scope = EntityFindScope::All;
	bool caseSensitive = false;
	bool visibleOnly = true;
	bool searchKeys = false;
	bool searchValues = true;
	bool replaceKeys = false;
	bool replaceValues = true;
	bool includeWorldspawn = false;
};

void Select_GetBounds( Vector3& mins, Vector3& maxs );

void Select_Delete();
void deleteSelection();

void Scene_BrushPatchSelectByShader( const char *shader );
void Select_FacesAndPatchesByShader( const char *shader );

void Select_EntitiesByKeyValue( const char* key, const char* value );

void Select_ConnectedEntities( bool targeting, bool targets, bool focus );


void Select_SetShader( const char* shader );
void Select_SetShader_Undo( const char* shader );

void Select_SetTexdef( const class TextureProjection& projection, bool setBasis = true, bool resetBasis = false );
void Select_SetTexdef( const float* hShift, const float* vShift, const float* hScale, const float* vScale, const float* rotation );

void Select_SetFlags( const class ContentsFlagsValue& flags );

void Select_ProjectTexture( const class texdef_t& texdef, const Vector3* direction );
void Select_ProjectTexture( const class TextureProjection& projection, const Vector3& normal );
void Select_FitTexture( float horizontal = 1, float vertical = 1, bool only_dimension = false );
void FindReplaceTextures( const TextureFindReplaceOptions& options );
void FindReplaceEntities( const EntityFindReplaceOptions& options );

void Select_ShowAllHidden();
void Select_registerCommands();

// updating workzone to a given brush (depends on current view)

void Selection_construct();
void Selection_destroy();


struct select_workzone_t
{
	// defines the boundaries of the current work area
	// is used to guess brushes and drop points third coordinate when creating from 2D view
	Vector3 d_work_min, d_work_max;

	select_workzone_t() :
		d_work_min(-64.0f,-64.0f,-64.0f ),
		d_work_max( 64.0f, 64.0f, 64.0f ){
	}
};

const select_workzone_t& Select_getWorkZone();
