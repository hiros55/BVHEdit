#pragma once
#include"Matrix.hpp"
#include<cmath>
class Quaternion {
public:
	Quaternion() {};
	~Quaternion() {};
	float ary[4];//w,x,y,z
	Quaternion& zero();
	Quaternion& loadquat(float w, float x, float y, float z);
	Quaternion& loadquat(const Quaternion* a) { return this->loadquat(a->ary[0], a->ary[1], a->ary[2], a->ary[3]); };
	Quaternion& add(const Quaternion* b, const Quaternion* c);
	Quaternion& sub(const Quaternion* b, const Quaternion* c);
	Quaternion& multiply(const Quaternion* b, const Quaternion* c);
	Quaternion& multiply(const Quaternion* a) { const Quaternion* tmp = this; multiply(a, tmp); return *this; };
	Quaternion& scala(float s);
	const float sqrnorm();
	const float norm() { return sqrt(sqrnorm()); };
	Quaternion& EulerAnglesToQuaternion(float vAxisx, float vAxisy, float vAxisz, float Angle);
	void MatirxToQuaternion(Matrix* m);
	Quaternion& QuaternionToMatrix(Matrix* m);
};