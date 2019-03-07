#include "Engine/Utils/FBXUtils.hpp"

//-------------------------------------------------------------------------------------------------
//For TOOLS only
#if defined(TOOLS_BUILD)
#include <fbxsdk.h>
#pragma comment(lib, "libfbxsdk-md.lib")

#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Math/Vector4i.hpp"
#include "Engine/Math/Vector4f.hpp"
#include "Engine/Math/Matrix4fStack.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"
#include "Engine/RenderSystem/Motion.hpp"
#include "Engine/RenderSystem/Skeleton.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
class SkinWeight
{
public:
	Vector4i indices;
	Vector4f weights;

public:
	SkinWeight( )
		: indices( )
		, weights( )
	{

	}
};


//-------------------------------------------------------------------------------------------------
static std::string const GetAttributeTypeName( FbxNodeAttribute::EType type )
{
	switch ( type )
	{
	case FbxNodeAttribute::eUnknown: return "unidentified";
	case FbxNodeAttribute::eNull: return "null";
	case FbxNodeAttribute::eMarker: return "marker";
	case FbxNodeAttribute::eSkeleton: return "skeleton";
	case FbxNodeAttribute::eMesh: return "mesh";
	case FbxNodeAttribute::eNurbs: return "nurbs";
	case FbxNodeAttribute::ePatch: return "patch";
	case FbxNodeAttribute::eCamera: return "camera";
	case FbxNodeAttribute::eCameraStereo: return "stereo";
	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
	case FbxNodeAttribute::eLight: return "light";
	case FbxNodeAttribute::eOpticalReference: return "optical reference";
	case FbxNodeAttribute::eOpticalMarker: return "marker";
	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
	case FbxNodeAttribute::eBoundary: return "boundary";
	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
	case FbxNodeAttribute::eShape: return "shape";
	case FbxNodeAttribute::eLODGroup: return "lodgroup";
	case FbxNodeAttribute::eSubDiv: return "subdiv";
	default: return "unknown";
	}
}


//-------------------------------------------------------------------------------------------------
static void PrintAttribute( FbxNodeAttribute *attribute, int depth )
{
	if ( nullptr == attribute )
	{
		return;
	}

	FbxNodeAttribute::EType type = attribute->GetAttributeType( );

	std::string const typeName = GetAttributeTypeName( type );
	std::string const attributeName = attribute->GetName( );
	g_ConsoleSystem->AddLog( Stringf( "%*s- type='%s', name='%s'", (depth + 1)*4, " ", &typeName[0], &attributeName[0] ), Color::WHITE );
}


//-------------------------------------------------------------------------------------------------
static void PrintNode( FbxNode *node, int depth )
{
	g_ConsoleSystem->AddLog( Stringf( "%*sNode [%s]", (depth + 1)*4, " ", node->GetName( ) ), Color::WHITE );
	for ( int attributeIndex = 0; attributeIndex < node->GetNodeAttributeCount( ); ++attributeIndex )
	{
		PrintAttribute( node->GetNodeAttributeByIndex( attributeIndex ), depth );
	}

	for ( int childIndex = 0; childIndex < node->GetChildCount( ); ++childIndex )
	{
		PrintNode( node->GetChild( childIndex ), depth + 1 );
	}
}


//-------------------------------------------------------------------------------------------------
void FBXListScene( std::string const &filename )
{
	FbxManager *fbxManager = FbxManager::Create( );
	if ( nullptr == fbxManager )
	{
		g_ConsoleSystem->AddLog( "Could not create fbx manager.", Color::RED );
		return;
	}

	FbxIOSettings *ioSettings = FbxIOSettings::Create( fbxManager, IOSROOT );
	fbxManager->SetIOSettings( ioSettings );

	//Create Importer
	FbxImporter *importer = FbxImporter::Create( fbxManager, "" );
	bool result = importer->Initialize( &filename[0], -1, fbxManager->GetIOSettings() );

	if ( result )
	{
		//We have imported the FBX
		FbxScene *scene = FbxScene::Create( fbxManager, "" );
		bool importSuccess = importer->Import( scene );
		if ( importSuccess )
		{
			FbxNode *root = scene->GetRootNode( );
			PrintNode( root, 0 );
		}

		FBX_SAFE_DESTROY( scene );
	}
	else
	{
		g_ConsoleSystem->AddLog( Stringf( "Could not import scene: %s", &filename[0] ), Color::RED );
	}

	FBX_SAFE_DESTROY( importer );
	FBX_SAFE_DESTROY( ioSettings );
	FBX_SAFE_DESTROY( fbxManager );
}


//-------------------------------------------------------------------------------------------------
static Vector3f ToEngineVector3f( FbxVector4 const &fbxVector4 )
{
	return Vector3f( ( float ) fbxVector4.mData[0], ( float ) fbxVector4.mData[1], ( float ) fbxVector4.mData[2] );
}


//-------------------------------------------------------------------------------------------------
static Color ToEngineColor( FbxColor const &fbxColor )
{
	unsigned char r = ( unsigned char ) ( fbxColor.mRed * 255 );
	unsigned char g = ( unsigned char ) ( fbxColor.mGreen * 255 );
	unsigned char b = ( unsigned char ) ( fbxColor.mBlue * 255 );
	unsigned char a = ( unsigned char ) ( fbxColor.mAlpha * 255 );
	return Color( r, g, b, a );
}



//-------------------------------------------------------------------------------------------------
static bool GetPosition( Vector3f *out_position, Matrix4f transform, FbxMesh *mesh, int polyIndex, int vertIndex )
{
	FbxVector4 fbxPosition;
	int controlIndex = mesh->GetPolygonVertex( polyIndex, vertIndex );
	fbxPosition = mesh->GetControlPointAt( controlIndex );

	Vector4f newPosition( Vector4f( ToEngineVector3f( fbxPosition ), 1 ) * transform );
	*out_position = newPosition.XYZ( );
	return true;
}


//-------------------------------------------------------------------------------------------------
template <typename ElemType, typename VarType>
static bool GetObjectFromElement( FbxMesh *mesh,
	int polyIndex,
	int vertIndex,
	ElemType *elem,
	VarType *out_var )
{
	if ( nullptr == elem )
	{
		return false;
	}
	
	switch ( elem->GetMappingMode() )
	{

	case FbxGeometryElement::eByControlPoint:
	{
		int controlIndex = mesh->GetPolygonVertex( polyIndex, vertIndex );
		switch ( elem->GetReferenceMode( ) )
		{

		case FbxGeometryElement::eDirect:
			if ( controlIndex < elem->GetDirectArray( ).GetCount( ) )
			{
				*out_var = elem->GetDirectArray( ).GetAt( controlIndex );
				return true;
			}
			break;

		case FbxGeometryElement::eIndexToDirect:
			if ( controlIndex < elem->GetIndexArray( ).GetCount( ) )
			{
				int index = elem->GetIndexArray( ).GetAt( controlIndex );
				*out_var = elem->GetDirectArray( ).GetAt( index );
				return true;
			}
			break;

		default:
			break;
		}
		break;
	}

	case FbxGeometryElement::eByPolygonVertex:
	{
		int directVertexIndex = ( polyIndex * 3 ) + vertIndex;
		switch ( elem->GetReferenceMode( ) )
		{

		case FbxGeometryElement::eDirect:
			if ( directVertexIndex < elem->GetDirectArray( ).GetCount( ) )
			{
				*out_var = elem->GetDirectArray( ).GetAt( directVertexIndex );
				return true;
			}
			break;

		case FbxGeometryElement::eIndexToDirect:
			if ( directVertexIndex < elem->GetIndexArray( ).GetCount( ) )
			{
				int index = elem->GetIndexArray( ).GetAt( directVertexIndex );
				*out_var = elem->GetDirectArray( ).GetAt( index );
				return true;
			}
			break;

		}
		break;
	}

	default:
		ERROR_AND_DIE( "Couldn't fine Geometry Element Type" );
		break;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
static bool GetUV( Vector2f *out_uv,
	FbxMesh *mesh,
	int polyIndex,
	int vertIndex,
	int uvIndex )
{
	FbxVector2 uv;
	FbxGeometryElementUV *uvs = mesh->GetElementUV( uvIndex );
	if ( GetObjectFromElement( mesh, polyIndex, vertIndex, uvs, &uv ) )
	{
		*out_uv = Vector2f( ( float ) uv.mData[0], ( float ) uv.mData[1] );
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
static bool GetColor( Color *out_color, FbxMesh *mesh, int polyIndex, int vertIndex )
{
	FbxColor color;
	FbxGeometryElementVertexColor *colorElement = mesh->GetElementVertexColor( );
	if ( GetObjectFromElement( mesh, polyIndex, vertIndex, colorElement, &color ) )
	{
		*out_color = ToEngineColor( color );
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
static bool GetNormal( Vector3f *out_normal,
	Matrix4f const &transform,
	FbxMesh *mesh,
	int polyIndex,
	int vertIndex,
	int normalIndex )
{
	FbxVector4 normal;
	FbxGeometryElementNormal *normals = mesh->GetElementNormal( normalIndex );
	if ( GetObjectFromElement( mesh, polyIndex, vertIndex, normals, &normal ) )
	{
		Vector3f tempNormal = ToEngineVector3f( normal );
		Vector4f vecNormal = Vector4f( tempNormal, 0.0f ) * transform;
		*out_normal = vecNormal.XYZ( );
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
static void ImportVertex( MeshBuilder * meshBuilder, Matrix4f const & transform, FbxMesh * mesh, int polyIndex, int vertIndex, std::vector<SkinWeight> const & skinWeights )
{
	Vector3f normal;
	if ( GetNormal( &normal, transform, mesh, polyIndex, vertIndex, 0 ) )
	{
		meshBuilder->SetNormal( normal );

		//#TODO: Remove once I have a VertexType that supports only Normals
		//#TODO: Make this part of the post process step
		Vector3f bitangent = Vector3f( 1.f, 0.f, 0.f );
		if ( normal == bitangent )
		{
			bitangent = Vector3f( 0.f, 1.f, 0.f );
		}
		Vector3f tangent = CrossProduct( bitangent, normal );
		bitangent = CrossProduct( normal, tangent );

		meshBuilder->SetTangent( tangent );
		meshBuilder->SetBitangent( bitangent );
	}

	Color vertexColor;
	if ( GetColor( &vertexColor, mesh, polyIndex, vertIndex ) )
	{
		meshBuilder->SetColor( vertexColor );
	}

	Vector2f uv;
	if ( GetUV( &uv, mesh, polyIndex, vertIndex, 0 ) ) //#TODO: support importing 0 and 1
	{
		meshBuilder->SetUV0( uv );
	}

	// Set Bone Weights
	unsigned int controlIndex = mesh->GetPolygonVertex( polyIndex, vertIndex );
	if( controlIndex < skinWeights.size( ) )
	{
		meshBuilder->SetBoneWeights( skinWeights[controlIndex].indices, skinWeights[controlIndex].weights );
		//meshBuilder->renormalize_skin_weights( ); // just to be safe
	}
	else
	{
		meshBuilder->ClearBoneWeights( );
	}

	Vector3f position;
	if ( GetPosition( &position, transform, mesh, polyIndex, vertIndex ) )
	{
		meshBuilder->AddVertex( position );
	}
}


//-------------------------------------------------------------------------------------------------
static Vector4f ToEngineVector4( FbxDouble4 const &vec4 )
{
	return Vector4f( ( float ) vec4.mData[0], ( float ) vec4.mData[1], ( float ) vec4.mData[2], ( float ) vec4.mData[3] );
}


//-------------------------------------------------------------------------------------------------
static Matrix4f ToEngineMatrix4( FbxMatrix const &fbxMat )
{
	Vector4f row1 = ToEngineVector4( fbxMat.mData[0] );
	Vector4f row2 = ToEngineVector4( fbxMat.mData[1] );
	Vector4f row3 = ToEngineVector4( fbxMat.mData[2] );
	Vector4f row4 = ToEngineVector4( fbxMat.mData[3] );
	return Matrix4f( row1, row2, row3, row4 );
}


//-------------------------------------------------------------------------------------------------
static Matrix4f GetGeometricTransform( FbxNode *node )
{
	Matrix4f ret = Matrix4f::IDENTITY;

	if ( ( node != nullptr ) && ( node->GetNodeAttribute( ) != nullptr ) )
	{
		FbxVector4 const geo_trans = node->GetGeometricTranslation( FbxNode::eSourcePivot );
		FbxVector4 const geo_rot = node->GetGeometricRotation( FbxNode::eSourcePivot );
		FbxVector4 const geo_scale = node->GetGeometricScaling( FbxNode::eSourcePivot );

		FbxMatrix geo_mat;
		geo_mat.SetTRS( geo_trans, geo_rot, geo_scale );

		ret = ToEngineMatrix4( geo_mat );
	}

	return ret;
}


//-------------------------------------------------------------------------------------------------
// Check if a mesh contains skin weights
static bool HasSkinWeights( FbxMesh const *mesh )
{
	int deformerCount = mesh->GetDeformerCount( FbxDeformer::eSkin );
	return ( deformerCount > 0 );
}


//-------------------------------------------------------------------------------------------------
static void AddHighestWeight( SkinWeight * skinWeights, int jointIndex, float weight )
{
	float lowestWeight = skinWeights->weights.x;
	if( skinWeights->weights.y < lowestWeight )
	{
		lowestWeight = skinWeights->weights.y;
	}
	if( skinWeights->weights.z < lowestWeight )
	{
		lowestWeight = skinWeights->weights.z;
	}
	if( skinWeights->weights.w < lowestWeight )
	{
		lowestWeight = skinWeights->weights.w;
	}

	if( lowestWeight < weight )
	{
		if( skinWeights->weights.x == lowestWeight )
		{
			skinWeights->weights.x = weight;
			skinWeights->indices.x = jointIndex;
		}
		else if( skinWeights->weights.y == lowestWeight )
		{
			skinWeights->weights.y = weight;
			skinWeights->indices.y = jointIndex;
		}
		else if( skinWeights->weights.z == lowestWeight )
		{
			skinWeights->weights.z = weight;
			skinWeights->indices.z = jointIndex;
		}
		else if( skinWeights->weights.w == lowestWeight )
		{
			skinWeights->weights.w = weight;
			skinWeights->indices.w = jointIndex;
		}
	}
}


//-------------------------------------------------------------------------------------------------
static int GetJointIndexForNode( std::map<int, FbxNode*> & jointIndexToNode, FbxNode const * node )
{
	for( auto jointNodeMapIter = jointIndexToNode.begin( ); jointNodeMapIter != jointIndexToNode.end( ); ++jointNodeMapIter )
	{
		if( jointNodeMapIter->second == node )
		{
			return jointNodeMapIter->first;
		}
	}
	return -1;
}


//-------------------------------------------------------------------------------------------------
static void Normalize( SkinWeight * skinWeight )
{
	float totalWeight = skinWeight->weights.x + skinWeight->weights.y + skinWeight->weights.z + skinWeight->weights.w;
	if( totalWeight > 0.f )
	{
		skinWeight->weights = skinWeight->weights * ( 1.f / totalWeight );
	}
	else
	{
		skinWeight->weights = Vector4f( 1.f, 0.f, 0.f, 0.f );
	}
}


//-------------------------------------------------------------------------------------------------
static void GetSkinWeights( std::vector<SkinWeight> * skinWeights, FbxMesh const * mesh, std::map<int, FbxNode*> & jointIndexToNode )
{
	for( size_t i = 0; i < skinWeights->size( ); ++i )
	{
		(*skinWeights)[i].indices = Vector4i( 0 );
		(*skinWeights)[i].weights = Vector4f( 0.f );
	}

	int deformerCount = mesh->GetDeformerCount( FbxDeformer::eSkin );
	ASSERT_RECOVERABLE(  deformerCount == 1, "Only one deformer supported." );
	for( int deformerIndex = 0; deformerIndex < deformerCount; ++deformerIndex )
	{
		FbxSkin * skin = (FbxSkin*) mesh->GetDeformer( deformerIndex, FbxDeformer::eSkin );

		if( nullptr == skin )
		{
			continue;
		}

		// Clusters are a link between this skin object, bones, 
		// and the verts that bone affects.
		int clusterCount = skin->GetClusterCount( );
		for( int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex )
		{
			FbxCluster * cluster = skin->GetCluster( clusterIndex );
			FbxNode const * linkNode = cluster->GetLink( );

			// Not associated with a bone - ignore it, we
			// don't care about it. 
			if( nullptr == linkNode )
			{
				continue;
			}

			int jointIndex = GetJointIndexForNode( jointIndexToNode, linkNode ); // you guys should have something like this
			if( jointIndex == -1 ) //Invalid Index
			{
				continue;
			}

			int * controlPointIndices = cluster->GetControlPointIndices( );
			int indexCount = cluster->GetControlPointIndicesCount( );
			double * weights = cluster->GetControlPointWeights( );

			for( int index = 0; index < indexCount; ++index )
			{
				int controlIndex = controlPointIndices[index];
				double weight = weights[index];

				SkinWeight * skinWeight = &( ( *skinWeights )[controlIndex] );
				AddHighestWeight( skinWeight, jointIndex, (float) weight );
			}
		}
	}

	for( unsigned int skinWeightIndex = 0; skinWeightIndex < skinWeights->size( ); ++skinWeightIndex )
	{
		Normalize( &( ( *skinWeights )[skinWeightIndex] ) );
		// Renormalize all the skin weights
		// If skin_weights were never added - make sure
		// you set it's weights to 1, 0, 0, 0
	}
}


//-------------------------------------------------------------------------------------------------
static void ImportMesh( SceneImport * import, FbxMesh * mesh, Matrix4fStack * matrixStack, std::map<int, FbxNode*> & jointIndexToNode )
{
	MeshBuilder *meshBuilder = new MeshBuilder( );

	ASSERT_RECOVERABLE(  mesh->IsTriangleMesh( ), "Not a triangle." );
	if ( !mesh->IsTriangleMesh( ) )
	{
		return;
	}

	meshBuilder->Begin( );

	Matrix4f geoTransform = GetGeometricTransform( mesh->GetNode() );
	matrixStack->Push( geoTransform );

	int controlPointCount = mesh->GetControlPointsCount( );

	// NEW STUFF IS HERE!  Before we import any vertices
	// figure out our weights for all vertices
	std::vector<SkinWeight> skinWeights;
	skinWeights.resize( controlPointCount );
	if( HasSkinWeights( mesh ) )
	{
		GetSkinWeights( &skinWeights, mesh, jointIndexToNode );
	}
	else
	{
		FbxNode * node = mesh->GetNode( );
		FbxNode * parent = node->GetParent( );
		while( parent != nullptr )
		{
			int jointIndex = GetJointIndexForNode( jointIndexToNode, parent );
			if( jointIndex != -1 )
			{
				for( size_t i = 0; i < skinWeights.size( ); ++i )
				{
					skinWeights[i].indices = Vector4i( jointIndex, 0, 0, 0 );
					skinWeights[i].weights = Vector4f( 1.f, 0.f, 0.f, 0.f );
				}
				break;
			}
			else
			{
				parent = parent->GetParent( );
			}
		}
		if( parent == nullptr )
		{
			for( size_t i = 0; i < skinWeights.size( ); ++i )
			{
				skinWeights[i].indices = Vector4i( 0 );
				skinWeights[i].weights = Vector4f( 1.f, 0.f, 0.f, 0.f );
			}
		}
		// walk tree up till node associated with joint. 

		// Find first parent node that has a joint associated with it
		// all vertices (fully weighted)
		// all skin_weights = indices{ joint_idx, 0, 0, 0 }  weights{ 1.0f, 0.0f, 0.0f, 0.0f };
	}

	//#TODO: Set up material also

	meshBuilder->SetUV0( Vector2f( 0.f ) );
	Matrix4f transform = matrixStack->GetTop( );
	int polyCount = mesh->GetPolygonCount( );
	for ( int polyIndex = 0; polyIndex < polyCount; ++polyIndex )
	{
		int vertCount = mesh->GetPolygonSize( polyIndex );
		ASSERT_RECOVERABLE(  vertCount == 3, "Should only be 3 verts in a triangle." );
		for ( int vertIndex = 0; vertIndex < vertCount; ++vertIndex )
		{
			ImportVertex( meshBuilder, transform, mesh, polyIndex, vertIndex, skinWeights );
		}
	}

	matrixStack->Pop( );

	meshBuilder->End( );
	import->m_meshes.push_back( meshBuilder );
}


//-------------------------------------------------------------------------------------------------
static void ImportNodeAttribute( SceneImport *import, FbxNodeAttribute *attribute, Matrix4fStack * matrixStack, std::map<int, FbxNode*> & jointIndexToNode )
{
	//Be safe
	if ( nullptr == attribute )
	{
		return;
	}
	
	switch ( attribute->GetAttributeType() )
	{
	case FbxNodeAttribute::eMesh:
		ImportMesh( import, ( FbxMesh* ) attribute, matrixStack, jointIndexToNode );
		break;

	default:
		break;
	}
}


//-------------------------------------------------------------------------------------------------
static Matrix4f GetNodeTransform( FbxNode *node )
{
	FbxMatrix fbxLocalMatrix = node->EvaluateLocalTransform( );
	return ToEngineMatrix4( fbxLocalMatrix );
}


//-------------------------------------------------------------------------------------------------
static void ImportMeshes( SceneImport * import, FbxNode * node, Matrix4fStack * matrixStack, std::map<int, FbxNode*> & jointIndexToNode )
{
	//Be safe
	if ( nullptr == node )
	{
		return;
	}

	Matrix4f nodeLocalTransform = GetNodeTransform( node );
	matrixStack->Push( nodeLocalTransform );

	// We want to load meshes
	int attributeCount = node->GetNodeAttributeCount( );
	for ( int attributeIndex = 0; attributeIndex < attributeCount; ++attributeIndex )
	{
		ImportNodeAttribute( import, node->GetNodeAttributeByIndex( attributeIndex ), matrixStack, jointIndexToNode );
	}

	//Import Children
	int childCount = node->GetChildCount( );
	for ( int childIndex = 0; childIndex < childCount; ++childIndex )
	{
		ImportMeshes( import, node->GetChild( childIndex ), matrixStack, jointIndexToNode );
	}

	matrixStack->Pop( );
}


//---------------------------------------------------------------------------------------------
static Skeleton * ImportSkeleton( SceneImport * import, Matrix4fStack * matStack, Skeleton * skeleton, FbxSkeleton * fbxSkeleton, int parentJointIndex )
{
	Skeleton * returnSkeleton = nullptr;
	if ( fbxSkeleton->IsSkeletonRoot( ) )
	{
		//THIS IS NEW SKELETON
		returnSkeleton = new Skeleton( );
		import->m_skeletons.push_back( returnSkeleton );
	}
	else
	{
		returnSkeleton = skeleton;
		//No orphans
		ASSERT_RECOVERABLE(  returnSkeleton != nullptr, "Trying to add a child with no parent" );
	}

	Matrix4f geoTransform = GetGeometricTransform( fbxSkeleton->GetNode( ) );

	matStack->Push( geoTransform );
	Matrix4f modelSpace = matStack->GetTop( );
	returnSkeleton->AddJoint( fbxSkeleton->GetNode( )->GetName( ), parentJointIndex, modelSpace );
	matStack->Pop( );

	return returnSkeleton;
}


//---------------------------------------------------------------------------------------------
static void ImportSkeletons( SceneImport * import, FbxNode * node, Matrix4fStack * matStack, Skeleton * skeleton, int parentJointIndex, std::map<int,FbxNode*> * jointIndexToNode )
{
	if ( node == nullptr )
	{
		return;
	}

	Matrix4f mat = GetNodeTransform( node );
	matStack->Push( mat );

	//Walk attributes, looking for doot doots
	int attributeCount = node->GetNodeAttributeCount( );
	for ( int attribIndex = 0; attribIndex < attributeCount; ++attribIndex )
	{
		FbxNodeAttribute *attrib = node->GetNodeAttributeByIndex( attribIndex );
		if ( attrib != nullptr && ( attrib->GetAttributeType( ) == FbxNodeAttribute::eSkeleton ) ) //Also make it only meshs in the other function
		{
			//So we have skeleton
			FbxSkeleton * fbxSkeleton = ( FbxSkeleton* ) attrib;
			Skeleton * newSkeleton = ImportSkeleton( import, matStack, skeleton, fbxSkeleton, parentJointIndex );

			//newSkeleton will either be the same skeleton passed,
			//or a new skeleton, or no skeleton if it was a bad node.
			//If we got something back - it is what we pass on to the
			//next generation.
			if ( newSkeleton != nullptr )
			{
				skeleton = newSkeleton;
				parentJointIndex = skeleton->GetLastAddJointIndex( );

				//Add joint to FbxNode mapping
				jointIndexToNode->insert( std::pair<int, FbxNode*>( parentJointIndex, node ) );

			}
		}
	}

	//Do the rest of the tree
	int childCount = node->GetChildCount( );
	for ( int childIndex = 0; childIndex < childCount; ++childIndex )
	{
		ImportSkeletons( import, node->GetChild( childIndex ), matStack, skeleton, parentJointIndex, jointIndexToNode );
	}

	matStack->Pop( );
}


//-------------------------------------------------------------------------------------------------
uint32_t GetSkeletonCount( SceneImport * import )
{
	return import->m_skeletons.size( );
}


//-------------------------------------------------------------------------------------------------
Skeleton * GetSkeleton( SceneImport * import, uint32_t skeletonIndex )
{
	return import->m_skeletons[skeletonIndex];
}


//-------------------------------------------------------------------------------------------------
static Matrix4f GetNodeWorldTransformAtTime( FbxNode *node, FbxTime time, Matrix4f const &importTransform )
{
	if ( node == nullptr )
	{
		return Matrix4f::IDENTITY;
	}

	FbxMatrix fbxMat = node->EvaluateGlobalTransform( time );
	Matrix4f engineMatrix = ToEngineMatrix4( fbxMat );
	return engineMatrix * importTransform;
}


//-------------------------------------------------------------------------------------------------
void ImportMotions( SceneImport * import, Matrix4f const & importTransform, FbxScene * scene, float framerate, std::map<int,FbxNode*> & jointIndexToNode )
{
	// Get number of animations
	int animationCount = scene->GetSrcObjectCount<FbxAnimStack>( );
	if ( 0 == animationCount )
	{
		return;
	}

	uint32_t skeletonCount = GetSkeletonCount( import );
	if ( 0 == skeletonCount )
	{
		return;
	}
	// #TODO: Only supporting one skeleton for now - update when needed.
	ASSERT_RECOVERABLE(  skeletonCount == 1, "Only support for importing one" );

	// Timing information for animation in this scene
	FbxGlobalSettings & settings = scene->GetGlobalSettings( );
	FbxTime::EMode timeMode = settings.GetTimeMode( );
	double scene_framerate;
	if ( timeMode == FbxTime::eCustom )
	{
		scene_framerate = settings.GetCustomFrameRate( );
	}
	else
	{
		scene_framerate = FbxTime::GetFrameRate( timeMode );
	}
	
	Skeleton *skeleton = GetSkeleton( import, 0 );

	// Time between frames
	FbxTime advance;
	advance.SetSecondDouble( ( double ) ( 1.0f / framerate ) );

	for ( int animIndex = 0; animIndex < animationCount; ++animIndex )
	{
		// Import Motions
		FbxAnimStack *anim = scene->GetSrcObject<FbxAnimStack>( );
		if ( nullptr == anim )
		{
			continue;
		}

		// Get duration of this specific animation.
		FbxTime startTime = anim->LocalStart;
		FbxTime endTime = anim->LocalStop;
		FbxTime duration = endTime - startTime;

		scene->SetCurrentAnimationStack( anim );

		std::string motionName = anim->GetName( );
		float timeSpan = ( float ) duration.GetSecondDouble( );

		Motion * newMotion = new Motion( motionName, timeSpan, framerate, skeleton );

		int jointCount = skeleton->GetJointCount( );
		for ( int jointIndex = 0; jointIndex < jointCount; ++jointIndex )
		{
			FbxNode *node = jointIndexToNode[jointIndex];

			// Extracting World Position
			// Local, you would need to grab parent as well

			FbxTime eval_time = FbxTime( 0 );
			for ( uint32_t frameIndex = 0; frameIndex < ( uint32_t ) newMotion->GetFrameCount( ); ++frameIndex)
			{
				Matrix4f boneTransform = GetNodeWorldTransformAtTime( node, eval_time, importTransform );
				newMotion->SetTransform( jointIndex, frameIndex, boneTransform );

				eval_time += advance;
			}
		}

		import->m_motion.push_back( newMotion );
	}
}


//-------------------------------------------------------------------------------------------------
static void TriangulateScene( FbxScene *scene )
{
	FbxGeometryConverter converter( scene->GetFbxManager( ) );
	converter.Triangulate( scene, true ); //true - replaces nodes
}


//-------------------------------------------------------------------------------------------------
// static void Finalize( SceneImport * import )
// {
// 	for ( MeshBuilder * mb : import->m_meshes )
// 	{
// 		mb->MeshReduction( );
// 	}
// }


//-------------------------------------------------------------------------------------------------
static void ImportScene( SceneImport * import, FbxScene * scene, Matrix4fStack * matrixStack )
{
	StopWatch importTimer;

	//Make everything triangles
	TriangulateScene( scene );
	importTimer.PrintLap( "Triangulate Time" );

	//Import all meshes
	FbxNode *root = scene->GetRootNode( );

	std::map<int, FbxNode*> jointIndexToNode;
	ImportSkeletons( import, root, matrixStack, nullptr, -1, &jointIndexToNode );
	importTimer.PrintLap( "Import Skeletons Time" );

	ImportMeshes( import, root, matrixStack, jointIndexToNode );
	importTimer.PrintLap( "Import Meshes Time" );

	ImportMotions( import, matrixStack->GetTop(), scene, 30, jointIndexToNode );
	importTimer.PrintLap( "Import Motions Time" );

	//Finalize( import );
	//importTimer.PrintLap( "Finalize Time" );

	importTimer.PrintTime( "Total Time" );
}


//-------------------------------------------------------------------------------------------------
static Matrix4f GetSceneBasis( FbxScene *scene )
{
	fbxsdk::FbxAxisSystem axisSystem = scene->GetGlobalSettings( ).GetAxisSystem( );
	
	FbxAMatrix mat;
	axisSystem.GetMatrix( mat );

	return ToEngineMatrix4( mat );
}


//-------------------------------------------------------------------------------------------------
SceneImport* FBXLoadSceneFromFile( std::string const &fbxFilename, Matrix4f const &engineBasis, bool isEngineBasisRightHanded, Matrix4f const &transform /*= Matrix4f::IDENTITY*/ )
{
	FbxManager *fbxManager = FbxManager::Create( );
	if ( nullptr == fbxManager )
	{
		g_ConsoleSystem->AddLog( "Could not create fbx manager.", Color::RED );
		return nullptr;
	}

	FbxIOSettings *ioSettings = FbxIOSettings::Create( fbxManager, IOSROOT );
	fbxManager->SetIOSettings( ioSettings );

	//Create Importer
	FbxImporter *importer = FbxImporter::Create( fbxManager, "" );
	bool result = importer->Initialize( &fbxFilename[0], -1, fbxManager->GetIOSettings( ) );
	if ( !result )
	{
		g_ConsoleSystem->AddLog( Stringf( "Could not initialize scene: %s", &fbxFilename[0] ), Color::RED );
		return nullptr;
	}

	//We have imported the FBX
	FbxScene *scene = FbxScene::Create( fbxManager, "" );
	bool importSuccess = importer->Import( scene );
	if ( !importSuccess )
	{
		g_ConsoleSystem->AddLog( Stringf( "Could not import scene: %s", &fbxFilename[0] ), Color::RED );
	}

	SceneImport *import = new SceneImport( );
	Matrix4fStack matrixStack;

	matrixStack.Push( transform );
	matrixStack.Push( engineBasis );

	//Set up our initial transforms
	Matrix4f sceneBasis = GetSceneBasis( scene );
	sceneBasis.Transpose( ); //Get Inverse
	if ( !isEngineBasisRightHanded )
	{
		Vector3f forward = sceneBasis.GetForward( );
		sceneBasis.SetForward( -forward );
	}
	
	matrixStack.Push( sceneBasis );

	//The Actual Work
	ImportScene( import, scene, &matrixStack );

	FBX_SAFE_DESTROY( scene );
	FBX_SAFE_DESTROY( importer );
	FBX_SAFE_DESTROY( ioSettings );
	FBX_SAFE_DESTROY( fbxManager );

	return import;
}


//-------------------------------------------------------------------------------------------------
//For not TOOLS
#else

#include <string>
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/Core/EngineCommon.hpp"

//-------------------------------------------------------------------------------------------------
void FBXListScene( std::string const & filename )
{
	UNREFERENCED( filename );
	g_ConsoleSystem->AddLog( "TOOLS_BUILD does not exist.", Color::RED );
}


//-------------------------------------------------------------------------------------------------
SceneImport * FBXLoadSceneFromFile( std::string const * fbxFilename )
{
	UNREFERENCED( fbxFilename );
	g_ConsoleSystem->AddLog( "TOOLS_BUILD does not exist.", Color::RED );
	return nullptr;
}

#endif