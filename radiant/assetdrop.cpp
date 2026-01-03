#include "assetdrop.h"

#include <limits>

#include "entity.h"
#include "eclasslib.h"
#include "ientity.h"
#include "ieclass.h"
#include "iscenegraph.h"
#include "scenelib.h"
#include "iundo.h"
#include "math/aabb.h"
#include "math/vector.h"
#include "grid.h"
#include "string/string.h"

const char* const kEntityBrowserMimeType = "application/x-viberadiant-entityclass";
const char* const kSoundBrowserMimeType = "application/x-viberadiant-soundpath";

namespace {
class EntityAtPointFinder final : public scene::Graph::Walker
{
	const Vector3& m_point;
	mutable Entity* m_bestEntity = nullptr;
	mutable float m_bestDistance2 = std::numeric_limits<float>::max();

public:
	explicit EntityAtPointFinder( const Vector3& point )
		: m_point( point ){
	}

	bool pre( const scene::Path& path, scene::Instance& instance ) const override {
		if ( !Node_isEntity( path.top() ) ) {
			return true;
		}

		Entity* entity = Node_getEntity( path.top() );
		if ( entity == nullptr ) {
			return false;
		}

		if ( classname_equal( entity->getClassName(), "worldspawn" ) ) {
			return false;
		}

		AABB bounds = instance.worldAABB();
		const float margin = std::max( 8.0f, GetGridSize() );
		bounds.extents += Vector3( margin, margin, margin );

		if ( !aabb_intersects_point( bounds, m_point ) ) {
			return false;
		}

		const float distance2 = vector3_length_squared( m_point - bounds.origin );
		if ( distance2 < m_bestDistance2 ) {
			m_bestDistance2 = distance2;
			m_bestEntity = entity;
		}

		return false;
	}

	Entity* bestEntity() const {
		return m_bestEntity;
	}
};

Entity* findEntityAtPoint( const Vector3& point ){
	EntityAtPointFinder finder( point );
	GlobalSceneGraph().traverse( finder );
	return finder.bestEntity();
}

Vector3 snappedPoint( const Vector3& point ){
	Vector3 snapped = point;
	vector3_snap( snapped, GetSnapGridSize() );
	return snapped;
}

bool createTargetSpeakerAtPoint( const Vector3& point, const char* soundPath ){
	EntityClass* entityClass = GlobalEntityClassManager().findOrInsert( "target_speaker", true );
	if ( entityClass == nullptr ) {
		return false;
	}

	NodeSmartReference node( GlobalEntityCreator().createEntity( entityClass ) );
	Node_getTraversable( GlobalSceneGraph().root() )->insert( node );

	scene::Path entitypath( makeReference( GlobalSceneGraph().root() ) );
	entitypath.push( makeReference( node.get() ) );
	scene::Instance& instance = findInstance( entitypath );

	if ( Transformable* transform = Instance_getTransformable( instance ) ) {
		transform->setType( TRANSFORM_PRIMITIVE );
		transform->setTranslation( point );
		transform->freezeTransform();
	}

	if ( Entity* entity = Node_getEntity( node ) ) {
		entity->setKeyValue( "noise", soundPath );
	}

	GlobalSelectionSystem().setSelectedAll( false );
	Instance_setSelected( instance, true );
	return true;
}
} // namespace

bool AssetDrop_handleEntityClass( const char* classname, const Vector3& point ){
	if ( string_empty( classname ) ) {
		return false;
	}

	Entity_createFromSelection( classname, snappedPoint( point ) );
	return true;
}

bool AssetDrop_handleSoundPath( const char* soundPath, const Vector3& point ){
	if ( string_empty( soundPath ) ) {
		return false;
	}

	const Vector3 snapped = snappedPoint( point );
	UndoableCommand undo( "entityAssignSound" );

	if ( Entity* entity = findEntityAtPoint( snapped ) ) {
		entity->setKeyValue( "noise", soundPath );
		return true;
	}

	return createTargetSpeakerAtPoint( snapped, soundPath );
}
