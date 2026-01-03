#include "linkedgroups.h"

#include "iselection.h"
#include "scenelib.h"
#include "scenegraph.h"
#include "instancelib.h"
#include "map.h"
#include "brushnode.h"
#include "patch.h"
#include "ientity.h"
#include "eclasslib.h"
#include "math/matrix.h"
#include "stream/stringstream.h"
#include "string/string.h"
#include "stringio.h"
#include "preferences.h"
#include "../plugins/entity/origin.h"
#include "../plugins/entity/rotation.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
const char kLinkedGroupIdKey[] = "_tb_linked_group_id";
const char kLinkedGroupTransformKey[] = "_tb_transformation";

const Vector3 kAnglesIdentity( 0, 0, 0 );

bool use_quake1_angles_bug(){
	return g_pGameDescription != nullptr && g_pGameDescription->mGameType == "q1";
}

void normalise_angles( Vector3& angles ){
	angles[0] = float_mod( angles[0], 360 );
	angles[1] = float_mod( angles[1], 360 );
	angles[2] = float_mod( angles[2], 360 );
}

void default_angles( Vector3& angles ){
	angles = kAnglesIdentity;
}

void read_angle( Vector3& angles, const char* value ){
	if ( !string_parse_float( value, angles[2] ) ) {
		default_angles( angles );
	}
	else
	{
		angles[0] = 0;
		angles[1] = 0;
		normalise_angles( angles );
	}
}

void read_group_angle( Vector3& angles, const char* value ){
	if( string_equal( value, "-1" ) ) {
		angles = Vector3( 0, -90, 0 );
	}
	else if( string_equal( value, "-2" ) ) {
		angles = Vector3( 0, 90, 0 );
	}
	else {
		read_angle( angles, value );
	}
}

void read_angles( Vector3& angles, const char* value ){
	if ( !string_parse_vector3( value, angles ) ) {
		default_angles( angles );
	}
	else
	{
		const bool quake1Bug = use_quake1_angles_bug();
		angles = Vector3( angles[2], quake1Bug ? -angles[0] : angles[0], angles[1] );
		normalise_angles( angles );
	}
}

void write_angle_value( float angle, Entity* entity ){
	if ( angle == 0 ) {
		entity->setKeyValue( "angle", "" );
	}
	else
	{
		const auto value = StringStream<64>( angle );
		entity->setKeyValue( "angle", value );
	}
}

void write_angles( const Vector3& angles, Entity* entity ){
	if ( angles == kAnglesIdentity ) {
		entity->setKeyValue( "angle", "" );
		entity->setKeyValue( "angles", "" );
	}
	else if ( angles[0] == 0 && angles[1] == 0 ) {
		entity->setKeyValue( "angles", "" );
		write_angle_value( angles[2], entity );
	}
	else
	{
		const bool quake1Bug = use_quake1_angles_bug();
		const auto value = StringStream<64>( quake1Bug ? -angles[1] : angles[1], ' ', angles[2], ' ', angles[0] );
		entity->setKeyValue( "angle", "" );
		entity->setKeyValue( "angles", value );
	}
}

Matrix4 matrix4_rotation_for_euler_xyz_degrees_quantised( const Vector3& angles ){
	if ( angles[0] == 0 && angles[1] == 0 ) {
		return matrix4_rotation_for_z_degrees( angles[2] );
	}
	if ( angles[0] == 0 && angles[2] == 0 ) {
		return matrix4_rotation_for_y_degrees( angles[1] );
	}
	if ( angles[1] == 0 && angles[2] == 0 ) {
		return matrix4_rotation_for_x_degrees( angles[0] );
	}
	return matrix4_rotation_for_euler_xyz_degrees( angles );
}

bool matrix4_affine_inverse_safe( const Matrix4& matrix, Matrix4& inverse ){
	const double det =
	    matrix[0] * ( matrix[5] * matrix[10] - matrix[9] * matrix[6] )
	    - matrix[1] * ( matrix[4] * matrix[10] - matrix[8] * matrix[6] )
	    + matrix[2] * ( matrix[4] * matrix[9] - matrix[8] * matrix[5] );
	if ( std::fabs( det ) < 1e-12 ) {
		return false;
	}
	inverse = matrix4_affine_inverse( matrix );
	return true;
}

std::string generate_link_id(){
	static std::mt19937 rng( std::random_device{}() );
	static std::uniform_int_distribution<int> dist( 0, 15 );
	static const char hex[] = "0123456789abcdef";

	const int groups[] = { 8, 4, 4, 4, 12 };
	std::string id;
	id.reserve( 36 );
	for ( int g = 0; g < 5; ++g )
	{
		if ( g != 0 ) {
			id.push_back( '-' );
		}
		for ( int i = 0; i < groups[g]; ++i )
		{
			id.push_back( hex[dist( rng )] );
		}
	}
	return id;
}

bool parse_transform( const char* value, Matrix4& transform ){
	float elements[16];
	if ( !string_parse_vector( value, elements, elements + 16 ) ) {
		return false;
	}

	transform = Matrix4(
	               elements[0],
	               elements[4],
	               elements[8],
	               elements[12],
	               elements[1],
	               elements[5],
	               elements[9],
	               elements[13],
	               elements[2],
	               elements[6],
	               elements[10],
	               elements[14],
	               elements[3],
	               elements[7],
	               elements[11],
	               elements[15]
	           );
	return true;
}

Matrix4 read_transform( const Entity& entity ){
	const char* value = entity.getKeyValue( kLinkedGroupTransformKey );
	if ( string_empty( value ) ) {
		return g_matrix4_identity;
	}

	Matrix4 transform;
	if ( parse_transform( value, transform ) ) {
		return transform;
	}
	return g_matrix4_identity;
}

void write_transform( Entity& entity, const Matrix4& transform ){
	if ( matrix4_equal_epsilon( transform, g_matrix4_identity, 0.0001f ) ) {
		entity.setKeyValue( kLinkedGroupTransformKey, "" );
		return;
	}

	StringOutputStream stream( 256 );
	stream << transform.xx() << ' ' << transform.yx() << ' ' << transform.zx() << ' ' << transform.tx() << ' '
	       << transform.xy() << ' ' << transform.yy() << ' ' << transform.zy() << ' ' << transform.ty() << ' '
	       << transform.xz() << ' ' << transform.yz() << ' ' << transform.zz() << ' ' << transform.tz() << ' '
	       << transform.xw() << ' ' << transform.yw() << ' ' << transform.zw() << ' ' << transform.tw();

	entity.setKeyValue( kLinkedGroupTransformKey, stream );
}

Matrix4 strip_translation( const Matrix4& matrix ){
	Matrix4 result( matrix );
	result.xw() = 0;
	result.yw() = 0;
	result.zw() = 0;
	result.tx() = 0;
	result.ty() = 0;
	result.tz() = 0;
	result.tw() = 1;
	return result;
}

Matrix4 orthonormalize_rotation( const Matrix4& matrix ){
	Vector3 x = vector3_normalised( matrix.x().vec3() );
	Vector3 y = vector3_normalised( matrix.y().vec3() );
	Vector3 z = vector3_cross( x, y );
	if ( vector3_length( z ) < 1e-6f ) {
		return g_matrix4_identity;
	}
	z = vector3_normalised( z );
	y = vector3_cross( z, x );

	Matrix4 result( g_matrix4_identity );
	result.x().vec3() = x;
	result.y().vec3() = y;
	result.z().vec3() = z;
	return result;
}

const char* rotation_key_for_entity( const Entity& entity ){
	if ( entity.hasKeyValue( "light_rotation" ) ) {
		return "light_rotation";
	}
	if ( entity.hasKeyValue( "rotation" ) ) {
		return "rotation";
	}
	return 0;
}

bool entity_supports_angles( const Entity& entity ){
	if ( entity.hasKeyValue( "light_rotation" )
	  || entity.hasKeyValue( "rotation" )
	  || entity.hasKeyValue( "angles" )
	  || entity.hasKeyValue( "angle" ) ) {
		return true;
	}
	const EntityClass& eclass = entity.getEntityClass();
	return eclass.has_angles || eclass.has_angles_key || eclass.has_direction_key;
}

void transform_entity( Entity& entity, const Matrix4& transform ){
	Vector3 origin;
	read_origin( origin, entity.getKeyValue( "origin" ) );
	write_origin( matrix4_transformed_point( transform, origin ), &entity, "origin" );

	if ( entity_supports_angles( entity ) ) {
		const Matrix4 deltaRotation = strip_translation( transform );
		if ( !matrix4_equal_epsilon( deltaRotation, g_matrix4_identity, 0.0001f ) ) {
			Matrix4 currentRotation = g_matrix4_identity;
			if ( const char* rotationKey = rotation_key_for_entity( entity ) ) {
				Float9 rotation;
				read_rotation( rotation, entity.getKeyValue( rotationKey ) );
				currentRotation = rotation_toMatrix( rotation );
			}
			else{
				Vector3 angles;
				if ( entity.hasKeyValue( "angles" ) ) {
					read_angles( angles, entity.getKeyValue( "angles" ) );
				}
				else if ( entity.hasKeyValue( "angle" ) ) {
					if ( entity.getEntityClass().has_direction_key ) {
						read_group_angle( angles, entity.getKeyValue( "angle" ) );
					}
					else
					{
						read_angle( angles, entity.getKeyValue( "angle" ) );
					}
				}
				else
				{
					default_angles( angles );
				}
				currentRotation = matrix4_rotation_for_euler_xyz_degrees_quantised( angles );
			}

			const Matrix4 updatedRotation = orthonormalize_rotation( matrix4_multiplied_by_matrix4( deltaRotation, currentRotation ) );
			if ( const char* rotationKey = rotation_key_for_entity( entity ) ) {
				Float9 rotation;
				rotation_fromMatrix( rotation, updatedRotation );
				write_rotation( rotation, &entity, rotationKey );
			}
			else
			{
				const Vector3 angles = vector3_snapped_to_zero( matrix4_get_rotation_euler_xyz_degrees( updatedRotation ), ANGLEKEY_SMALLEST );
				write_angles( angles, &entity );
			}
		}
	}

	if ( !string_empty( entity.getKeyValue( kLinkedGroupIdKey ) ) ) {
		const Matrix4 updated = matrix4_premultiplied_by_matrix4( read_transform( entity ), transform );
		write_transform( entity, updated );
	}
}

bool node_has_instance( scene::Node& node ){
	scene::Instantiable* instantiable = Node_getInstantiable( node );
	if ( instantiable == 0 ) {
		return false;
	}

	bool hasInstance = false;
	class HasInstanceVisitor : public scene::Instantiable::Visitor
	{
		bool& m_hasInstance;
	public:
		explicit HasInstanceVisitor( bool& hasInstance )
			: m_hasInstance( hasInstance ){
		}
		void visit( scene::Instance& instance ) const override {
			(void)instance;
			m_hasInstance = true;
		}
	};

	instantiable->forEachInstance( HasInstanceVisitor( hasInstance ) );
	return hasInstance;
}

bool is_linked_group( scene::Node& node ){
	if ( !node_is_group( node ) ) {
		return false;
	}
	Entity* entity = Node_getEntity( node );
	if ( entity == 0 ) {
		return false;
	}
	return !string_empty( entity->getKeyValue( kLinkedGroupIdKey ) );
}

std::string get_link_id( scene::Node& node ){
	Entity* entity = Node_getEntity( node );
	if ( entity == 0 ) {
		return std::string();
	}
	const char* value = entity->getKeyValue( kLinkedGroupIdKey );
	return string_empty( value ) ? std::string() : std::string( value );
}

void clear_link_keys( Entity& entity ){
	entity.setKeyValue( kLinkedGroupIdKey, "" );
	entity.setKeyValue( kLinkedGroupTransformKey, "" );
}

void ensure_transform_key( scene::Node& node ){
	Entity* entity = Node_getEntity( node );
	if ( entity == 0 ) {
		return;
	}
	if ( !string_empty( entity->getKeyValue( kLinkedGroupTransformKey ) ) ) {
		return;
	}
	TransformNode* transform = Node_getTransformNode( node );
	if ( transform == 0 ) {
		return;
	}
	write_transform( *entity, transform->localToParent() );
}

void collect_direct_children( scene::Node& group, std::vector<scene::Node*>& children ){
	scene::Traversable* traversable = Node_getTraversable( group );
	if ( traversable == 0 ) {
		return;
	}

	class ChildCollector final : public scene::Traversable::Walker
	{
		std::vector<scene::Node*>& m_children;
	public:
		explicit ChildCollector( std::vector<scene::Node*>& children )
			: m_children( children ){
		}
		bool pre( scene::Node& node ) const override {
			m_children.push_back( &node );
			return false;
		}
	};

	traversable->traverse( ChildCollector( children ) );
}

std::vector<NodeSmartReference> clone_children( scene::Node& group ){
	std::vector<scene::Node*> children;
	collect_direct_children( group, children );

	std::vector<NodeSmartReference> clones;
	clones.reserve( children.size() );
	for ( scene::Node* child : children )
	{
		NodeSmartReference clone( Node_Clone( *child ) );
		clones.push_back( clone );
	}
	return clones;
}

void transform_clone( scene::Node& node, const Matrix4& transform ){
	if ( Brush* brush = Node_getBrush( node ) ) {
		brush->transform( transform );
		brush->planeChanged();
		return;
	}
	if ( Patch* patch = Node_getPatch( node ) ) {
		patch->transform( transform );
		patch->freezeTransform();
		return;
	}
	if ( Entity* entity = Node_getEntity( node ) ) {
		transform_entity( *entity, transform );
	}
}

void replace_children( scene::Node& group, const std::vector<NodeSmartReference>& clones ){
	scene::Traversable* traversable = Node_getTraversable( group );
	if ( traversable == 0 ) {
		return;
	}

	std::vector<scene::Node*> existing;
	collect_direct_children( group, existing );

	for ( scene::Node* child : existing )
	{
		traversable->erase( *child );
	}

	for ( const auto& clone : clones )
	{
		traversable->insert( clone.get() );
	}
}

std::unordered_map<std::string, std::vector<scene::Node*> > collect_linked_groups_by_id(){
	std::unordered_map<std::string, std::vector<scene::Node*> > groups;

	class LinkedGroupCollector final : public scene::Graph::Walker
	{
		std::unordered_map<std::string, std::vector<scene::Node*> >& m_groups;
	public:
		explicit LinkedGroupCollector( std::unordered_map<std::string, std::vector<scene::Node*> >& groups )
			: m_groups( groups ){
		}
		bool pre( const scene::Path& path, scene::Instance& instance ) const override {
			(void)instance;
			scene::Node& node = path.top().get();
			if ( !node_is_group( node ) ) {
				return true;
			}
			Entity* entity = Node_getEntity( node );
			if ( entity == 0 ) {
				return true;
			}
			const char* linkId = entity->getKeyValue( kLinkedGroupIdKey );
			if ( string_empty( linkId ) ) {
				return true;
			}
			m_groups[linkId].push_back( &node );
			return true;
		}
	};

	GlobalSceneGraph().traverse( LinkedGroupCollector( groups ) );
	return groups;
}

void select_node_instances( scene::Node& node, bool selected ){
	scene::Instantiable* instantiable = Node_getInstantiable( node );
	if ( instantiable == 0 ) {
		return;
	}

	class SelectVisitor final : public scene::Instantiable::Visitor
	{
		bool m_selected;
	public:
		explicit SelectVisitor( bool selected )
			: m_selected( selected ){
		}
		void visit( scene::Instance& instance ) const override {
			Instance_setSelected( instance, m_selected );
		}
	};

	instantiable->forEachInstance( SelectVisitor( selected ) );
}

std::vector<scene::Node*> collect_selected_groups( bool& hasNonGroup ){
	hasNonGroup = false;
	std::unordered_set<scene::Node*> groups;

	class SelectedGroupCollector final : public SelectionSystem::Visitor
	{
		std::unordered_set<scene::Node*>& m_groups;
		bool& m_hasNonGroup;
	public:
		SelectedGroupCollector( std::unordered_set<scene::Node*>& groups, bool& hasNonGroup )
			: m_groups( groups ), m_hasNonGroup( hasNonGroup ){
		}
		void visit( scene::Instance& instance ) const override {
			scene::Node& node = instance.path().top();
			if ( node_is_group( node ) ) {
				m_groups.insert( &node );
			}
			else{
				m_hasNonGroup = true;
			}
		}
	};

	GlobalSelectionSystem().foreachSelected( SelectedGroupCollector( groups, hasNonGroup ) );
	return std::vector<scene::Node*>( groups.begin(), groups.end() );
}

void update_linked_groups_from_source( scene::Node& sourceGroup, const std::vector<scene::Node*>& linkedGroups ){
	Entity* sourceEntity = Node_getEntity( sourceGroup );
	if ( sourceEntity == 0 ) {
		return;
	}

	Matrix4 sourceTransform = read_transform( *sourceEntity );
	Matrix4 sourceInverse;
	if ( !matrix4_affine_inverse_safe( sourceTransform, sourceInverse ) ) {
		globalErrorStream() << "Linked groups update skipped: group transformation not invertible\n";
		return;
	}

	for ( scene::Node* targetGroup : linkedGroups )
	{
		if ( targetGroup == &sourceGroup ) {
			continue;
		}
		Entity* targetEntity = Node_getEntity( *targetGroup );
		if ( targetEntity == 0 ) {
			continue;
		}

		const Matrix4 targetTransform = read_transform( *targetEntity );
		const Matrix4 delta = matrix4_multiplied_by_matrix4( targetTransform, sourceInverse );

		std::vector<NodeSmartReference> clones = clone_children( sourceGroup );
		for ( auto& clone : clones )
		{
			transform_clone( clone.get(), delta );
		}
		replace_children( *targetGroup, clones );
	}
}

bool g_commandActive = false;
bool g_updating = false;
int g_transformDepth = 0;
std::unordered_set<scene::Node*> g_dirtyGroups;
std::unordered_set<scene::Node*> g_transformingGroups;
std::unordered_map<scene::Node*, Matrix4> g_transformStart;
}


static void LinkedGroups_OnCommandStart_impl(){
	if ( g_commandActive ) {
		return;
	}
	g_commandActive = true;
	g_dirtyGroups.clear();
}

static void LinkedGroups_OnCommandFinish_impl(){
	if ( !g_commandActive ) {
		return;
	}

	if ( g_updating ) {
		g_dirtyGroups.clear();
		g_commandActive = false;
		return;
	}

	if ( g_dirtyGroups.empty() ) {
		g_commandActive = false;
		return;
	}

	g_updating = true;

	std::vector<scene::Node*> dirtyGroups( g_dirtyGroups.begin(), g_dirtyGroups.end() );
	g_dirtyGroups.clear();

	std::unordered_map<std::string, std::vector<scene::Node*> > dirtyById;
	for ( scene::Node* group : dirtyGroups )
	{
		if ( group == 0 ) {
			continue;
		}
		if ( !node_has_instance( *group ) ) {
			continue;
		}
		if ( !is_linked_group( *group ) ) {
			continue;
		}
		const std::string linkId = get_link_id( *group );
		if ( !linkId.empty() ) {
			dirtyById[linkId].push_back( group );
		}
	}

	const auto allGroups = collect_linked_groups_by_id();
	for ( const auto& [linkId, groups] : dirtyById )
	{
		if ( groups.size() != 1 ) {
			globalErrorStream() << "Linked groups update skipped: multiple groups modified for link id '" << linkId << "'\n";
			continue;
		}

		auto allIt = allGroups.find( linkId );
		if ( allIt == allGroups.end() ) {
			continue;
		}
		if ( allIt->second.size() < 2 ) {
			continue;
		}

		update_linked_groups_from_source( *groups.front(), allIt->second );
	}

	g_updating = false;
	g_commandActive = false;
}

static void LinkedGroups_MarkGroupChanged_impl( scene::Node& node );

static void LinkedGroups_MarkNodeChanged_impl( scene::Node& node ){
	if ( !g_commandActive || g_updating ) {
		return;
	}
	if ( g_transformingGroups.find( &node ) != g_transformingGroups.end() ) {
		return;
	}

	scene::Instantiable* instantiable = Node_getInstantiable( node );
	if ( instantiable == 0 ) {
		return;
	}

	class GroupFromInstance final : public scene::Instantiable::Visitor
	{
	public:
		void visit( scene::Instance& instance ) const override {
			const scene::Path& path = instance.path();
			for ( std::size_t i = path.size(); i-- > 0; )
			{
				scene::Node& current = path[i].get();
				if ( node_is_group( current ) ) {
					LinkedGroups_MarkGroupChanged_impl( current );
					break;
				}
			}
		}
	};

	instantiable->forEachInstance( GroupFromInstance() );
}

static void LinkedGroups_MarkGroupChanged_impl( scene::Node& node ){
	if ( !g_commandActive || g_updating ) {
		return;
	}
	if ( !is_linked_group( node ) ) {
		return;
	}
	if ( g_transformingGroups.find( &node ) != g_transformingGroups.end() ) {
		return;
	}
	g_dirtyGroups.insert( &node );
}

static void LinkedGroups_BeginTransform_impl( const std::vector<scene::Node*>& groups ){
	if ( groups.empty() ) {
		return;
	}

	if ( g_transformDepth++ != 0 ) {
		return;
	}

	g_transformingGroups.clear();
	g_transformStart.clear();
	for ( scene::Node* group : groups )
	{
		if ( group == 0 ) {
			continue;
		}
		if ( !is_linked_group( *group ) ) {
			continue;
		}
		TransformNode* transform = Node_getTransformNode( *group );
		if ( transform == 0 ) {
			continue;
		}
		g_transformingGroups.insert( group );
		g_transformStart[group] = transform->localToParent();
	}
}

static void LinkedGroups_EndTransform_impl(){
	if ( g_transformDepth == 0 ) {
		return;
	}
	if ( --g_transformDepth != 0 ) {
		return;
	}

	for ( const auto& [group, startTransform] : g_transformStart )
	{
		if ( group == 0 ) {
			continue;
		}
		if ( !node_has_instance( *group ) ) {
			continue;
		}
		if ( !is_linked_group( *group ) ) {
			continue;
		}
		TransformNode* transform = Node_getTransformNode( *group );
		if ( transform == 0 ) {
			continue;
		}

		Matrix4 inverseStart;
		if ( !matrix4_affine_inverse_safe( startTransform, inverseStart ) ) {
			continue;
		}

		const Matrix4 delta = matrix4_multiplied_by_matrix4( transform->localToParent(), inverseStart );
		if ( matrix4_equal_epsilon( delta, g_matrix4_identity, 0.0001f ) ) {
			continue;
		}

		Entity* entity = Node_getEntity( *group );
		if ( entity == 0 ) {
			continue;
		}
		const Matrix4 updated = matrix4_premultiplied_by_matrix4( read_transform( *entity ), delta );
		write_transform( *entity, updated );
	}

	g_transformingGroups.clear();
	g_transformStart.clear();
}

static void LinkedGroups_CreateLinkedDuplicate_impl(){
	if ( GlobalSelectionSystem().Mode() != SelectionSystem::ePrimitive ) {
		globalErrorStream() << "Create linked duplicate: select a group in primitive mode\n";
		return;
	}

	bool hasNonGroup = false;
	std::vector<scene::Node*> groups = collect_selected_groups( hasNonGroup );
	if ( hasNonGroup || groups.size() != 1 ) {
		globalErrorStream() << "Create linked duplicate: select exactly one group\n";
		return;
	}

	scene::Node& group = *groups.front();
	if ( !node_is_group( group ) ) {
		globalErrorStream() << "Create linked duplicate: selection is not a group\n";
		return;
	}

	Entity* entity = Node_getEntity( group );
	if ( entity == 0 ) {
		return;
	}

	std::string linkId = get_link_id( group );
	const bool assignLinkId = linkId.empty();
	if ( linkId.empty() ) {
		linkId = generate_link_id();
	}

	UndoableCommand undo( "createLinkedDuplicate" );

	if ( assignLinkId ) {
		entity->setKeyValue( kLinkedGroupIdKey, linkId.c_str() );
	}
	ensure_transform_key( group );

	const scene::Path& path = GlobalSelectionSystem().ultimateSelected().path();
	NodeSmartReference clone( Node_Clone( group ) );
	Node_getTraversable( path.parent().get() )->insert( clone );
	Map_gatherNamespaced( clone );
	Map_mergeClonedNames( false );

	Entity* cloneEntity = Node_getEntity( clone.get() );
	if ( cloneEntity != 0 ) {
		cloneEntity->setKeyValue( kLinkedGroupIdKey, linkId.c_str() );
		ensure_transform_key( clone.get() );
	}

	GlobalSelectionSystem().setSelectedAll( false );
	select_node_instances( clone.get(), true );
}

static void LinkedGroups_SelectLinkedGroups_impl(){
	if ( GlobalSelectionSystem().Mode() != SelectionSystem::ePrimitive ) {
		globalErrorStream() << "Select linked groups: selection must be groups\n";
		return;
	}

	bool hasNonGroup = false;
	std::vector<scene::Node*> groups = collect_selected_groups( hasNonGroup );
	if ( hasNonGroup || groups.empty() ) {
		globalErrorStream() << "Select linked groups: selection must be groups\n";
		return;
	}

	const auto allGroups = collect_linked_groups_by_id();
	for ( scene::Node* group : groups )
	{
		const std::string linkId = get_link_id( *group );
		auto it = allGroups.find( linkId );
		if ( linkId.empty() || it == allGroups.end() || it->second.size() < 2 ) {
			globalErrorStream() << "Select linked groups: selection must be linked\n";
			return;
		}
	}

	std::unordered_set<scene::Node*> toSelect;
	for ( scene::Node* group : groups )
	{
		const std::string linkId = get_link_id( *group );
		auto it = allGroups.find( linkId );
		if ( it == allGroups.end() ) {
			continue;
		}
		for ( scene::Node* linked : it->second )
		{
			toSelect.insert( linked );
		}
	}

	GlobalSelectionSystem().setSelectedAll( false );
	for ( scene::Node* node : toSelect )
	{
		select_node_instances( *node, true );
	}
}

static void LinkedGroups_SeparateSelectedLinkedGroups_impl(){
	if ( GlobalSelectionSystem().Mode() != SelectionSystem::ePrimitive ) {
		globalErrorStream() << "Separate linked groups: selection must be groups\n";
		return;
	}

	bool hasNonGroup = false;
	std::vector<scene::Node*> selectedGroups = collect_selected_groups( hasNonGroup );
	if ( hasNonGroup || selectedGroups.empty() ) {
		globalErrorStream() << "Separate linked groups: selection must be groups\n";
		return;
	}

	std::unordered_set<scene::Node*> selectedSet( selectedGroups.begin(), selectedGroups.end() );
	const auto allGroups = collect_linked_groups_by_id();

	for ( scene::Node* group : selectedGroups )
	{
		if ( get_link_id( *group ).empty() ) {
			globalErrorStream() << "Separate linked groups: selection must be linked groups\n";
			return;
		}
	}

	UndoableCommand undo( "separateLinkedGroups" );

	for ( const auto& [linkId, groups] : allGroups )
	{
		std::vector<scene::Node*> selectedInSet;
		for ( scene::Node* group : groups )
		{
			if ( selectedSet.find( group ) != selectedSet.end() ) {
				selectedInSet.push_back( group );
			}
		}

		if ( selectedInSet.empty() ) {
			continue;
		}

		if ( selectedInSet.size() == groups.size() ) {
			for ( scene::Node* group : groups )
			{
				Entity* entity = Node_getEntity( *group );
				if ( entity != 0 ) {
					clear_link_keys( *entity );
				}
			}
			continue;
		}

		if ( selectedInSet.size() == 1 ) {
			Entity* entity = Node_getEntity( *selectedInSet.front() );
			if ( entity != 0 ) {
				clear_link_keys( *entity );
			}
		}
		else
		{
			const std::string newLinkId = generate_link_id();
			for ( scene::Node* group : selectedInSet )
			{
				Entity* entity = Node_getEntity( *group );
				if ( entity != 0 ) {
					entity->setKeyValue( kLinkedGroupIdKey, newLinkId.c_str() );
					ensure_transform_key( *group );
				}
			}
		}

		if ( groups.size() - selectedInSet.size() == 1 ) {
			for ( scene::Node* group : groups )
			{
				if ( selectedSet.find( group ) != selectedSet.end() ) {
					continue;
				}
				Entity* entity = Node_getEntity( *group );
				if ( entity != 0 ) {
					clear_link_keys( *entity );
				}
			}
		}
	}
}

#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"

class LinkedGroupsSystemImpl final : public LinkedGroupsSystem
{
public:
	void onCommandStart() override {
		LinkedGroups_OnCommandStart_impl();
	}
	void onCommandFinish() override {
		LinkedGroups_OnCommandFinish_impl();
	}
	void markNodeChanged( scene::Node& node ) override {
		LinkedGroups_MarkNodeChanged_impl( node );
	}
	void markGroupChanged( scene::Node& node ) override {
		LinkedGroups_MarkGroupChanged_impl( node );
	}
	void beginTransform( const std::vector<scene::Node*>& groups ) override {
		LinkedGroups_BeginTransform_impl( groups );
	}
	void endTransform() override {
		LinkedGroups_EndTransform_impl();
	}
	void createLinkedDuplicate() override {
		LinkedGroups_CreateLinkedDuplicate_impl();
	}
	void selectLinkedGroups() override {
		LinkedGroups_SelectLinkedGroups_impl();
	}
	void separateSelectedLinkedGroups() override {
		LinkedGroups_SeparateSelectedLinkedGroups_impl();
	}
};

class LinkedGroupsAPI
{
	LinkedGroupsSystemImpl m_system;
public:
	typedef LinkedGroupsSystem Type;
	STRING_CONSTANT( Name, "*" );

	LinkedGroupsSystem* getTable(){
		return &m_system;
	}
};

typedef SingletonModule<LinkedGroupsAPI> LinkedGroupsModule;
typedef Static<LinkedGroupsModule> StaticLinkedGroupsModule;
StaticRegisterModule staticRegisterLinkedGroups( StaticLinkedGroupsModule::instance() );
