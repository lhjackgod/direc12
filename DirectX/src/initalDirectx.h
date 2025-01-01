#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <vector>
#include <iostream>
#include <stdio.h>

inline std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr) :
		std::runtime_error(HrToString(hr)), m_hr(hr) {}
	HRESULT Error() const { return m_hr; }
private:
	const HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw HrException(hr);
	}
}

void depthBufferType()
{
	DXGI_FORMAT_D32_FLOAT; //32 bits depthbuffer
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT; //32bits depthBuffer (uint)8bits stencile 24padding
	DXGI_FORMAT_D24_UNORM_S8_UINT; //24bits depthBuffer (uint)8bits stencile
}

void multiSample()
{
	DXGI_FORMAT mBackBufferFrambuffer = DXGI_FORMAT_R32G32B32A32_FLOAT;
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = mBackBufferFrambuffer;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	msQualityLevels.SampleCount = 4;
}

void createFence()
{
	
}