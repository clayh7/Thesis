#pragma once


//-------------------------------------------------------------------------------------------------
#if !defined( __TOOLS_FBX__ )
#define __TOOLS_FBX__

#include <string>
#include <map>

void FBXListScene( std::string const &filename );

#endif


//-------------------------------------------------------------------------------------------------
#include "Engine/Math/Matrix4f.hpp"


//-------------------------------------------------------------------------------------------------
class MeshBuilder;
class Skeleton;
class Motion;


//-------------------------------------------------------------------------------------------------
class SceneImport
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	std::vector<MeshBuilder*> m_meshes;
	std::vector<Skeleton*> m_skeletons;
	std::vector<Motion*> m_motion;
	//No idea how to forward declare a FbxNode* ...

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	~SceneImport( )
	{
		//destroy all meshes
	}
};


//-------------------------------------------------------------------------------------------------
SceneImport * FBXLoadSceneFromFile( std::string const & fbxFilename, Matrix4f const & engineBasis, bool isEngineBasisRightHanded, Matrix4f const & transform = Matrix4f::IDENTITY );