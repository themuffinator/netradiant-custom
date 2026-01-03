#pragma once

#include "ientity.h"
#include "generic/callback.h"

#include <cstring>
#include <vector>

namespace scene
{
class Node;
}

void LinkedGroups_OnCommandStart();
void LinkedGroups_OnCommandFinish();

void LinkedGroups_MarkNodeChanged( scene::Node& node );
void LinkedGroups_MarkGroupChanged( scene::Node& node );

void LinkedGroups_BeginTransform( const std::vector<scene::Node*>& groups );
void LinkedGroups_EndTransform();

void LinkedGroups_CreateLinkedDuplicate();
void LinkedGroups_SelectLinkedGroups();
void LinkedGroups_SeparateSelectedLinkedGroups();

class LinkedGroupsEntityObserver final : public Entity::Observer
{
	scene::Node& m_node;
	bool m_suppress;

	static bool isLinkedGroupsKey( const char* key ){
		return key != 0
		    && ( std::strcmp( key, "_tb_linked_group_id" ) == 0
		      || std::strcmp( key, "_tb_transformation" ) == 0 );
	}

	void valueChanged( const char* value ){
		(void)value;
		if ( !m_suppress ) {
			LinkedGroups_MarkNodeChanged( m_node );
		}
	}
	typedef MemberCaller<LinkedGroupsEntityObserver, void(const char*), &LinkedGroupsEntityObserver::valueChanged> ValueChangedCaller;
public:
	explicit LinkedGroupsEntityObserver( scene::Node& node )
		: m_node( node ), m_suppress( false ){
	}

	void attach( Entity& entity ){
		m_suppress = true;
		entity.attach( *this );
		m_suppress = false;
	}
	void detach( Entity& entity ){
		m_suppress = true;
		entity.detach( *this );
		m_suppress = false;
	}

	void insert( const char* key, EntityKeyValue& value ) override {
		if ( isLinkedGroupsKey( key ) ) {
			return;
		}
		(void)key;
		value.attach( ValueChangedCaller( *this ) );
		if ( !m_suppress ) {
			LinkedGroups_MarkNodeChanged( m_node );
		}
	}
	void erase( const char* key, EntityKeyValue& value ) override {
		if ( isLinkedGroupsKey( key ) ) {
			return;
		}
		(void)key;
		value.detach( ValueChangedCaller( *this ) );
		if ( !m_suppress ) {
			LinkedGroups_MarkNodeChanged( m_node );
		}
	}
	void clear() override {
	}
};
