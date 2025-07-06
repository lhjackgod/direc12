#pragma once
class Camera
{
public:
    Camera() = default;

    Camera(float width, float height, float zNear, float zFar, DirectX::XMVECTOR pos)
        : m_CameraPos(pos), m_AspectRatio(width / height), m_zNear(zNear), m_zFar(zFar)
    {
        update();
    }

    ~Camera() = default;
    Camera(const Camera&) = default;
    DirectX::XMMATRIX getViewMatrix();
    DirectX::XMMATRIX getPerspective();
    void setCameraPos(DirectX::XMVECTOR pos);
    DirectX::XMVECTOR getCameraPos();

private:
    const static DirectX::XMVECTOR wordUp;
    DirectX::XMVECTOR m_Up;
    DirectX::XMVECTOR m_Right;
    DirectX::XMVECTOR m_Forward;
    DirectX::XMMATRIX m_PerspectiveMatrix;
    DirectX::XMMATRIX m_ViewMatrix;
    DirectX::XMVECTOR m_CameraPos;
    float m_Yaw = -90.0f;
    float m_Pitch = 0.0f;
    float m_AspectRatio;
    float m_Fov = DirectX::XMConvertToRadians(90.0f);
    float m_zNear;
    float m_zFar;
    void update();
};
