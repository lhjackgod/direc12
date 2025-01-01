#pragma once
#include <DirectXMath.h>
#include <iostream>



std::ostream& operator << (std::ostream& os, DirectX::FXMMATRIX v)
{
	for (int i = 0; i < 4; i++)
	{
		os << DirectX::XMVectorGetX(v.r[i]) << "\t";
		os << DirectX::XMVectorGetY(v.r[i]) << "\t";
		os << DirectX::XMVectorGetZ(v.r[i]) << "\t";
		os << DirectX::XMVectorGetW(v.r[i]) << "\t";
	}
	return os;
}

void testMOne()
{
	if (!DirectX::XMVerifyCPUSupport())
	{
		std::cout << "direct math not support" << std::endl;
		return;
	}
	DirectX::XMMATRIX A(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 4.0f, 0.0f,
		1.0f, 2.0f, 3.0f, 1.0f);
	DirectX::XMMATRIX B = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX C = A * B;
	DirectX::XMMATRIX D = DirectX::XMMatrixTranspose(A);
	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
	DirectX::XMMATRIX E = DirectX::XMMatrixInverse(&det, A);
	DirectX::XMMATRIX F = A * E;

	std::cout << "A =" << std::endl << A << std::endl;
	std::cout << "B = " << std::endl << B << std::endl;
	std::cout << "C = A * B" << std::endl << C << std::endl;
	std::cout << "D = transpose(A) = " << std::endl << D << std::endl;
	std::cout << "det = determinant(A)" << det << std::endl;
	std::cout << "E = inverse(A) = " << std::endl << E << std::endl;
	std::cout << "F = A * E = " << std::endl << F << std::endl;
}