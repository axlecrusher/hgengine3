#ifndef MERCURYWINDOW_H
#define MERCURYWINDOW_H

#include <global.h>
//#include <MercuryUtil.h>
//#include <MercuryStringUtil.h>
#include <list>
#include <Callback.h>
#include <functional>

class MercuryWindow
{
public:
	struct Dimensions
	{
		Dimensions(uint16_t w = 0, uint16_t h = 0)
		: width(w), height(h)
		{}
		uint16_t width, height;
	};

	MercuryWindow(const MString& title, MercuryWindow::Dimensions d, int bits, int depthBits, bool fullscreen);
	virtual ~MercuryWindow();

	inline static MercuryWindow* MakeWindow(Dimensions d) {
		MercuryWindow::m_windowInstance = genWindowClbk(d);
		return GetCurrentWindow();
	}

	virtual bool SwapBuffers() = 0;
	virtual bool PumpMessages() = 0;
	
	inline static MercuryWindow* GetCurrentWindow()
	{
		return MercuryWindow::m_windowInstance;
	}
	
	virtual void* GetProcAddress(const MString& x) = 0;
	
	inline uint16_t CurrentWidth() const { return m_currentDimensions.width; }
	inline uint16_t CurrentHeight() const { return m_currentDimensions.height; }

	inline bool InFocus() const { return m_inFocus; }

	bool m_close;
	void ScanForKeystrokes();

	void ToggleFullscreen(bool fullscreen);

protected:
	virtual void TryFullscreen() = 0;

//	static MercuryWindow* (*genWindowClbk)(void);
	static std::function<MercuryWindow*(Dimensions d)> genWindowClbk;
//	static Callback0R< MercuryWindow* > genWindowClbk;
	static MercuryWindow* m_windowInstance;
	
	MString m_title;

	Dimensions m_currentDimensions;
	Dimensions m_requestedDimenstions;

	uint8_t m_bits, m_depthBits;
	bool m_fullscreen;

	int m_iLastMouseX;
	int m_iLastMouseY;

	bool m_inFocus;

	bool m_altEnter;

};

#endif

/* 
 * Copyright (c) 2008, Joshua Allen
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
