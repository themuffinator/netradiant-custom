#pragma once

#include "generic/vector.h"

class Entity;

extern const char* const kEntityBrowserMimeType;
extern const char* const kSoundBrowserMimeType;

bool AssetDrop_handleEntityClass( const char* classname, const Vector3& point );
bool AssetDrop_handleSoundPath( const char* soundPath, const Vector3& point );
