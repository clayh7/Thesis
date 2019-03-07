#pragma once

#include <vector>
#include <string>
#include "Engine/Math/Matrix4f.hpp"


//-------------------------------------------------------------------------------------------------
class Joint
{
public:
	std::string name;
	int parentIndex;
	Matrix4f modelToBoneSpace;
	Matrix4f boneToModelSpace;
};


//-------------------------------------------------------------------------------------------------
class IBinaryReader;
class IBinaryWriter;


//-------------------------------------------------------------------------------------------------
class Skeleton
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	// 1: Initial Version
	static uint32_t const FILE_VERSION = 1;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	std::vector<std::string> m_names;
	std::vector<int> m_parentIndicies;
	std::vector<Matrix4f> m_modelToBoneSpace;
	std::vector<Matrix4f> m_boneToModeSpace;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	~Skeleton( );
	void AddJoint( const char * jointName, int parentJointIndex, Matrix4f const &initialBoneToModelMatrix );
	void ReadFromFile( std::string const & filename );
	void WriteToFile( std::string const & filename ) const;

	std::string const & GetName( int index ) const;
	Vector3f GetJointPosition( int index ) const;
	bool GetBonePositions( int index, Vector3f * out_position0, Vector3f * out_position1 ) const;
	int GetJointCount( ) const;
	int GetLastAddJointIndex( ) const;
	int GetJointIndexFromName( std::string const & name ) const;
	Joint GetJoint( int const idx ) const;
	std::vector<Matrix4f> GetCurrentMatricies( ) const;

	void SetJointWorldTransform( int jointIndex, Matrix4f const & worldTransform );

private:
	void ReadFromStream( IBinaryReader & reader );
	void WriteToStream( IBinaryWriter & writer ) const;
};