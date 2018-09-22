#ifndef WIN32WINDOW_H
#define WIN32WINDOW_H

#include <windows.h>
#include <MercuryWindow.h>
//#include <MScopedArray.h>

#include <HgInput.h>

extern bool stereo_view;

///Win32 Window Driver
class Win32Window : public MercuryWindow
{
public:
	Win32Window(const MString& title, int width, int height, int bits, int depthBits, bool fullscreen);
	virtual ~Win32Window();
	virtual bool SwapBuffers();
	virtual bool PumpMessages();

	static MercuryWindow* GenWin32Window();
	virtual void* GetProcAddress(const MString& x);
	//static uint16_t ConvertScancode( size_t scanin );
	static ENGINE::INPUT::KeyCodes ConvertScancode2(size_t virtualKey);
	
private:
	LONG_PTR ChangeStyle(DWORD dwStyle);
	void TryFullscreen();

	bool IsKeyRepeat(size_t c);
	static LRESULT CALLBACK WindowCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void GenWindow();
	void GenWinClass();
	void SetPixelType();
	void CreateRenderingContext();
	void GenPixelType();

	bool ChangeDisplaySettings();

	HWND m_hwnd; //window handle
	HDC m_hdc; //device handle
	PIXELFORMATDESCRIPTOR m_pfd; //pixel format descriptor
	HGLRC m_hglrc; //GL rendering context

	WNDCLASSEX m_wndclass;
	HINSTANCE m_hInstance;
	DWORD m_dwStyle;

	ATOM m_windowAtom;

	wchar_t* m_className;
	wchar_t* m_winTitle;

	//	MScopedArray< WCHAR > m_className;
//	MScopedArray< WCHAR > m_winTitle;

	uint16_t m_cX, m_cY;
};

#endif

/* Copyright (c) 2008, Joshua Allen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *	-	Redistributions of source code must retain the above
 *		copyright notice, this list of conditions and the following disclaimer.
 *	-	Redistributions in binary form must reproduce the above copyright
 *		notice, this list of conditions and the following disclaimer in
 *		the documentation and/or other materials provided with the distribution.
 *	-	Neither the name of the Mercury Engine nor the names of its
 *		contributors may be used to endorse or promote products derived from
 *		this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
