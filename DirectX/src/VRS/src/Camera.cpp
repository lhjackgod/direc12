#include "Camera.h"

const DirectX::XMVECTOR Camera::wordUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

DirectX::XMMATRIX Camera::getViewMatrix()
{
	return DirectX::XMMatrixLookAtLH(m_CameraPos, DirectX::XMVectorSet(0.0f,0.0f,0.0f,0.0f), m_Up);
}

DirectX::XMMATRIX Camera::getPerspective()
{
	return DirectX::XMMatrixPerspectiveFovLH(m_Fov, m_AspectRatio, m_zNear, m_zFar);
}

void Camera::setCameraPos(DirectX::XMVECTOR pos)
{
	m_CameraPos = pos;
}

DirectX::XMVECTOR Camera::getCameraPos()
{
	return m_CameraPos;
}

void Camera::update()
{
	DirectX::XMVECTOR targetPos = DirectX::XMVectorZero();
	m_Forward = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(targetPos, m_CameraPos));
	m_Right = DirectX::XMVector3Cross(wordUp, m_Forward);
	m_Up = DirectX::XMVector3Cross(m_Forward, m_Right);
}
