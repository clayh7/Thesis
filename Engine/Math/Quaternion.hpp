#pragma once

#include "Engine/Math/Vector3f.hpp"


//-------------------------------------------------------------------------------------------------
class Euler;
class Matrix4f;


//-------------------------------------------------------------------------------------------------
// I've tried to make this class work, but something keeps messing up
// Currently I believe it's broken, and I'll fix it the next time I need it
class Quaternion
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static Quaternion ZERO;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	double w;
	double x;
	double y;
	double z;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Quaternion( );
	Quaternion( double real, Vector3f const & img );
	Quaternion( double realw, double imgx, double imgy, double imgz );
	Quaternion( Euler const & eulerAngles );
	Quaternion( Matrix4f const & mat );

	Vector3f GetComplex( ) const;
	double GetReal( ) const;
	Quaternion GetConjugate( void ) const;

	//return The quaternion q such that q * (*this) == (*this) * q
	//== [ 0 0 0 1 ]<sup>T</sup>.
	Quaternion Inverse( ) const;

	//return The quaternion product (*this) x @p rhs.
	Quaternion Product( Quaternion const & rhs ) const;

	//return The quaternion product (*this) x rhs.
	Quaternion operator*( Quaternion const & rhs ) const;

	//return The quaternion (*this) * s.
	Quaternion operator*( double s ) const;

	//Produces the sum of this quaternion and rhs.
	Quaternion operator+( Quaternion const & rhs ) const;

	//Produces the difference of this quaternion and rhs.
	Quaternion operator-( Quaternion const & rhs ) const;

	//Unary negation.
	Quaternion operator-( ) const;

	//return The quaternion (*this) / s.
	Quaternion operator/( double s ) const;

	double normSquared( ) const;
	double norm( ) const;

	//Computes the rotation matrix represented by a unit
	//quaternion.
	Matrix4f GetRotationMatrix( ) const;

	//Computes the quaternion that is equivalent to a given
	//euler angle rotation.
	void MakeEuler( Euler const & euler );

	//return Euler angles in roll-pitch-yaw order.
	Euler GetEuler( ) const;
};