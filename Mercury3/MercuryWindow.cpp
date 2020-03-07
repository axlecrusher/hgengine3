#include "MercuryWindow.h"
//#include "MercuryMessageManager.h"

#include <HgInput.h>

MercuryWindow::MercuryWindow(const MString& title, MercuryWindow::Dimensions d, int bits, int depthBits, bool fullscreen)
	:m_title(title), m_requestedDimenstions(d), m_bits(bits), m_depthBits(depthBits), m_fullscreen(fullscreen),
	m_iLastMouseX(0),m_iLastMouseY(0),m_inFocus(false), m_close(false), m_altEnter(false)
{
//	MESSAGEMAN.GetValue( "Input.CursorGrabbed" )->SetBool( true );
}

MercuryWindow::~MercuryWindow()
{
}

void MercuryWindow::ScanForKeystrokes() {
	using namespace ENGINE::INPUT;

	bool alt_enter = ScanForKeyCombo(KeyCodes::KEY_ALT, KeyCodes::KEY_ENTER);
	if (alt_enter && !m_altEnter) {
		ToggleFullscreen(!m_fullscreen);
	}
	m_altEnter = alt_enter; //remember last combo so that we can filter repeats

	if (ScanForKeyCombo(KeyCodes::KEY_ALT, KeyCodes::KEY_F4)) {
		m_close = true;
	}
}

void MercuryWindow::ToggleFullscreen(bool fullscreen) {
	m_fullscreen = fullscreen;
	TryFullscreen();
}

MercuryWindow* MercuryWindow::m_windowInstance;

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
