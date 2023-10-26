#pragma once

#include "VideoDestination.h"
#include <d3d11.h>
#include <opencv2/core/ocl.hpp>

namespace OpenVCR
{
	class OPEN_VCR_API WindowVideoDestination : public VideoDestination
	{
	public:
		WindowVideoDestination();
		virtual ~WindowVideoDestination();

		void SetWindowHandle(HWND windowHandle);
		HWND GetWindowHandle();

		virtual bool PowerOn(Error& error) override;
		virtual bool PowerOff(Error& error) override;
		virtual bool AddFrame(Frame& frame, Error& error) override;

	private:
		HWND windowHandle;
		int windowWidth;
		int windowHeight;
		ID3D11Device* device;
		IDXGISwapChain* swapChain;
		ID3D11DeviceContext* deviceContext;
		ID3D11Texture2D* backBufferTexture;
		ID3D11Texture2D* frameTexture;
		ID3D11RenderTargetView* renderTargetView;
		cv::ocl::Context* openCLContext;
	};
}