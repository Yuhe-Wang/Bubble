#pragma once
#ifndef __glMath_h__
#define __glMath_h__
#include <math.h>
namespace GLMath
{
	struct Point
	{
		float x,y,z;
	};
	typedef Point Vector;
	void crossProduct(const Vector& v1,const Vector& v2, Vector & v)
	{
		v.x=v1.y * v2.z - v1.z * v2.y;
		v.y=v1.z * v2.x - v1.x * v2.z;
		v.z=v1.x * v2.y - v1.y * v2.x;
	}
	inline float sqrt(const Vector& v)
	{
		return ::sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
	}
}
#endif