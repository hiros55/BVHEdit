#include "Quaternion.hpp"
#include"Matrix.hpp"
#include <cmath>
#include<iostream>

//引数の符号を返す
template<typename T>
T sign(T a) {
	return a > 0 ? 1 : -1;
}

//ゼロクォータニオン 
Quaternion& Quaternion::zero()
{
	this->ary[0] = this->ary[1] = this->ary[2] = this->ary[3] = 0.0;
	return *this;
}
//クォータニオン設定
Quaternion& Quaternion::loadquat(float w, float x, float y, float z)
{
	this->ary[0] = w;
	this->ary[1] = x;
	this->ary[2] = y;
	this->ary[3] = z;
	return *this;
}
//クォータニオン 和
Quaternion& Quaternion::add(const Quaternion* b, const Quaternion* c)
{
	this->ary[0] = b->ary[0] + c->ary[0];
	this->ary[1] = b->ary[1] + c->ary[1];
	this->ary[2] = b->ary[2] + c->ary[2];
	this->ary[3] = b->ary[3] + c->ary[3];

	return *this;
}
//クォータニオン 差
Quaternion& Quaternion::sub(const Quaternion* b, const Quaternion* c)
{
	this->ary[0] = b->ary[0] - c->ary[0];
	this->ary[1] = b->ary[1] - c->ary[1];
	this->ary[2] = b->ary[2] - c->ary[2];
	this->ary[3] = b->ary[3] - c->ary[3];

	return *this;
}
//クォータニオン 積
Quaternion& Quaternion::multiply(const Quaternion* b, const Quaternion* c)
{
	this->ary[0] = b->ary[0] * c->ary[0] - b->ary[1] * c->ary[1] - b->ary[2] * c->ary[2] - b->ary[3] * c->ary[3];
	this->ary[1] = b->ary[0] * c->ary[1] + b->ary[1] * c->ary[0] - b->ary[2] * c->ary[3] + b->ary[3] * c->ary[2];
	this->ary[2] = b->ary[0] * c->ary[2] + b->ary[1] * c->ary[3] + b->ary[2] * c->ary[0] - b->ary[3] * c->ary[1];
	this->ary[3] = b->ary[0] * c->ary[3] - b->ary[1] * c->ary[2] + b->ary[2] * c->ary[1] + b->ary[3] * c->ary[0];

	return *this;
}
//クォータニオン スカラ
Quaternion& Quaternion::scala(float s)
{
	this->ary[0] = s * this->ary[0];
	this->ary[1] = s * this->ary[1];
	this->ary[2] = s * this->ary[2];
	this->ary[3] = s * this->ary[3];

	return *this;
}
//クォータニオン ノルム
const float Quaternion::sqrnorm()
{
	return this->ary[0] * this->ary[0] + this->ary[1] * this->ary[1] + this->ary[2] * this->ary[2] + this->ary[3] * this->ary[3];
}
//オイラー角からクォータニオンへ変換
Quaternion& Quaternion::EulerAnglesToQuaternion(float vAxisx, float vAxisy, float vAxisz, float Angle) {
	float pi = 3.141593f;
	float radian = Angle * (pi / 180.0f) / 2.0f;
	float s = sin(radian);
	this->ary[0] = cos(radian);
	this->ary[1] = vAxisx * s;
	this->ary[2] = vAxisy * s;
	this->ary[3] = vAxisz * s;
	return *this;
}
//行列からクォータニオン変換
void Quaternion::MatirxToQuaternion(Matrix* m) {
	float* q = new float[4];
	float r;
	q[0] = sqrt(m->array[0] + m->array[5] + m->array[10] + 1.0f) / 2.0f;
	q[1] = sqrt(m->array[0] - m->array[5] - m->array[10] + 1.0f) / 2.0f;
	q[2] = sqrt(-m->array[0] + m->array[5] - m->array[10] + 1.0f) / 2.0f;
	q[3] = sqrt(-m->array[0] - m->array[5] + m->array[10] + 1.0f) / 2.0f;
	float max = q[0];

	for (int i = 1; i < 4; i++) {
		if (max < q[i]) {
			max = q[i];
		}
	}

	if (max == 0) {//w > 0 かつ 最大値がw
		q[1] *= sign(m->array[6] - m->array[9]);
		q[2] *= sign(m->array[8] - m->array[2]);
		q[3] *= sign(m->array[1] - m->array[4]);
	}
	else if (max == 1) {//x > 0 かつ 最大値がx
		q[0] *= sign(m->array[6] - m->array[9]);
		q[2] *= sign(m->array[1] + m->array[4]);
		q[3] *= sign(m->array[8] + m->array[2]);
	}
	else if (max == 2) {//y > 0 かつ 最大値がy
		q[0] *= sign(m->array[8] - m->array[2]);
		q[1] *= sign(m->array[1] + m->array[4]);
		q[3] *= sign(m->array[6] - m->array[9]);
	}
	else if (max == 3) {//z > 0かつ 最大値がz
		q[0] *= sign(m->array[1] - m->array[4]);
		q[1] *= sign(m->array[8] + m->array[2]);
		q[2] *= sign(m->array[6] + m->array[9]);
	}
	else {
		std::cout << "Error:MatirxToQuaternion 変換できません" << std::endl;
	}

	this->loadquat(q[0], q[1], q[2], q[3]);
	r = this->norm();
	this->scala(1 / r);
}
//クォータニオンから行列へ変換
Quaternion& Quaternion::QuaternionToMatrix(Matrix* m) {

	float x2 = this->ary[1] * this->ary[1];
	float y2 = this->ary[2] * this->ary[2];
	float z2 = this->ary[3] * this->ary[3];
	float yz = this->ary[2] * this->ary[3];
	float xz = this->ary[1] * this->ary[3];
	float xy = this->ary[1] * this->ary[2];
	float wx = this->ary[0] * this->ary[1];
	float wy = this->ary[0] * this->ary[2];
	float wz = this->ary[0] * this->ary[3];

	m->array[0] = 1.0f - 2.0f * (y2 + z2);
	m->array[1] = 2.0f * (xy + wz);
	m->array[2] = 2.0f * (xz - wy);
	m->array[3] = 0.0f;
	m->array[4] = 2.0f * (xy - wz);
	m->array[5] = 1.0f - 2.0f * (x2 + z2);
	m->array[6] = 2.0f * (yz + wx);
	m->array[7] = 0.0f;
	m->array[8] = 2.0f * (xz + wy);
	m->array[9] = 2.0f * (yz - wx);
	m->array[10] = 1.0f - 2.0f * (x2 + y2);
	m->array[11] = 0.0f;
	m->array[12] = 0.0f;
	m->array[13] = 0.0f;
	m->array[14] = 0.0f;
	m->array[15] = 1.0f;
	return *this;
}