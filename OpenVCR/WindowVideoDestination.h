#pragma once

#include "IODevice.h"
#include <d3d11.h>

namespace OpenVCR
{
	class OPEN_VCR_API WindowVideoDestination : public IODevice
	{
	public:
		WindowVideoDestination();
		virtual ~WindowVideoDestination();

		void SetWindowHandle(HWND windowHandle);
		HWND GetWindowHandle();

		enum class RenderMode
		{
			STRETCH,
			MAINTAIN_ASPECT_RATIO
		};

		void SetRenderMode(RenderMode renderMode);
		RenderMode GetRenderMode();

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, bool& moved, Error& error) override;

		// Call this when the window size changes to update the swap-chain and backbuffer, etc.
		bool WindowSizeChanged(Error& error);

		void SetSourceName(const std::string& givenName) { *this->sourceName = givenName; }
		const std::string& GetSourceName() const { return *this->sourceName; }

	private:
		bool CreateFrameTexture(Error& error);
		bool SetupRenderTargetViewAndViewport(Error& error);
		void RenderFrameIntoTexture(cv::Mat& frame, cv::Mat& texture);

		std::string* sourceName;
		HWND windowHandle;
		int windowWidth;
		int windowHeight;
		RenderMode renderMode;
		ID3D11Device* device;
		IDXGISwapChain* swapChain;
		ID3D11DeviceContext* deviceContext;
		ID3D11Texture2D* backBufferTexture;
		ID3D11Texture2D* frameTexture;
		ID3D11RenderTargetView* renderTargetView;
	};
}