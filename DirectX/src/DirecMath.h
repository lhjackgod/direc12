#pragma once
#include <DirectXMath.h>
#include <iostream>
using namespace DirectX;

std::ostream& operator << (std::ostream& outputStream, DirectX::FXMVECTOR v)
{
	XMFLOAT3 dest;
	XMStoreFloat3(&dest, v);
	outputStream << "( " << dest.x << ", " << dest.y << ", " << dest.z << " )" << std::endl;
	return outputStream;
}

void testOne()
{
	std::cout.setf(std::ios_base::boolalpha);
	if (!XMVerifyCPUSupport())
	{
		std::cout << "directx math not supportd" << std::endl;
		return;
	}
	XMVECTOR p = XMVectorZero();
	XMVECTOR q = XMVectorSplatOne();
	XMVECTOR u = XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
	XMVECTOR v = XMVectorReplicate(-2.0f);
	XMVECTOR w = XMVectorSplatZ(u);

	std::cout << "p == " << p;
	std::cout << "q == " << q;
	std::cout << "u == " << u;
	std::cout << "v == " << v;
	std::cout << "w == " << w;
}

void testTwo()
{
	std::cout.setf(std::ios_base::boolalpha);
	if (!XMVerifyCPUSupport())
	{
		std::cout << "directx math not supportd" << std::endl;
		return;
	}
	XMVECTOR n = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR u = XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
	XMVECTOR v = XMVectorSet(-2.0f, 1.0f, -3.0f, 0.0f);
	XMVECTOR w = XMVectorSet(0.707f, 0.707f, 0.0f, 0.0f);

	XMVECTOR a = u + v;
	XMVECTOR b = u - v;
	XMVECTOR c = 10.0f * u;
	
	XMVECTOR L = XMVector3Length(u);
	XMVECTOR d = XMVector3Normalize(u);
	XMVECTOR s = XMVector3Dot(u, v);
	XMVECTOR e = XMVector3Cross(u, v);

	XMVECTOR projW, perpW;
	XMVector3ComponentsFromNormal(&projW, &perpW, w, n);
	bool equal = XMVector3Equal(projW + perpW, w) != 0;
	bool notEqual = XMVector3NotEqual(projW + perpW, w) != 0;

	XMVECTOR angleVec = XMVector3AngleBetweenVectors(projW, perpW); //get the radiances
	float angleRadius = XMVectorGetX(angleVec);
	float angleDegrees = XMConvertToDegrees(angleRadius); //change to angle

	std::cout << "u = " << u;
	std::cout << "v = " << v;
	std::cout << "w = " << w;
	std::cout << "n = " << n;
	std::cout << "a = u + v = " << a;
	std::cout << "b = u - v = " << b;
	std::cout << "c = 10 * u = " << c;
	std::cout << "d = u / ||u|| = " << d;
	std::cout << "e = u x v = " << e;
	std::cout << "L = ||u|| = " << L;
	std::cout << "s = u.v = " << s;
	std::cout << "projW = " << projW;
	std::cout << "perpW = " << perpW;
	std::cout << "projW + perpW == w" << equal << std::endl;
	std::cout << "projW + perpW! = w" << notEqual << std::endl;
	std::cout << "angle = " << angleDegrees;
}

void testThree()
{
	std::cout.setf(std::ios_base::boolalpha);
	std::cout.precision(8);
	XMVECTOR u = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	XMVECTOR n = XMVector3Normalize(u);

	float LU = XMVectorGetX(XMVector3Length(n));
	std::cout << LU << std::endl; //0.99994

	const float epsilon = 0.001f;
	if (fabs(1.0f - LU) < epsilon)
	{
		std::cout << true << std::endl;
	}
}