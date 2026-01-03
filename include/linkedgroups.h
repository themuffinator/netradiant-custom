#pragma once

#include "ientity.h"
#include "generic/callback.h"

#include <cstring>
#include <vector>

namespace scene
{
class Node;
}

class LinkedGroupsSystem
{
public:
	INTEGER_CONSTANT( Version, 1 );
	STRING_CONSTANT( Name, "linkedgroups" );

	virtual void onCommandStart() = 0;
	virtual void onCommandFinish() = 0;
	virtual void markNodeChanged( scene::Node& node ) = 0;
	virtual void markGroupChanged( scene::Node& node ) = 0;
	virtual void beginTransform( const std::vector<scene::Node*>& groups ) = 0;
	virtual void endTransform() = 0;
	virtual void createLinkedDuplicate() = 0;
	virtual void selectLinkedGroups() = 0;
	virtual void separateSelectedLinkedGroups() = 0;
};

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<LinkedGroupsSystem> GlobalLinkedGroupsModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<LinkedGroupsSystem> GlobalLinkedGroupsModuleRef;

inline LinkedGroupsSystem& GlobalLinkedGroupsSystem(){
	return GlobalLinkedGroupsModule::getTable();
}

inline void LinkedGroups_OnCommandStart(){
	GlobalLinkedGroupsSystem().onCommandStart();
}
inline void LinkedGroups_OnCommandFinish(){
	GlobalLinkedGroupsSystem().onCommandFinish();
}
inline void LinkedGroups_MarkNodeChanged( scene::Node& node ){
	GlobalLinkedGroupsSystem().markNodeChanged( node );
}
inline void LinkedGroups_MarkGroupChanged( scene::Node& node ){
	GlobalLinkedGroupsSystem().markGroupChanged( node );
}
inline void LinkedGroups_BeginTransform( const std::vector<scene::Node*>& groups ){
	GlobalLinkedGroupsSystem().beginTransform( groups );
}
inline void LinkedGroups_EndTransform(){
	GlobalLinkedGroupsSystem().endTransform();
}
inline void LinkedGroups_CreateLinkedDuplicate(){
	GlobalLinkedGroupsSystem().createLinkedDuplicate();
}
inline void LinkedGroups_SelectLinkedGroups(){
	GlobalLinkedGroupsSystem().selectLinkedGroups();
}
inline void LinkedGroups_SeparateSelectedLinkedGroups(){
	GlobalLinkedGroupsSystem().separateSelectedLinkedGroups();
}

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
