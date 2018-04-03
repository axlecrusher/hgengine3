#include <Win32Window.h>
//#include <Windowsx.h>
//#include <GLHeaders.h>
//#include <MercuryInput.h>
//#include <MercuryMessageManager.h>
#include <Wingdi.h>

/*
#define GL_GLEXT_PROTOTYPES
#include <glcorearb.h>
#include <glext.h>
*/
#include <glew.h>

extern "C" {
#include <HgInput.h>
}

uint8_t GlobalMouseGrabbed_Set = 1;

uint8_t stereo_view;

//LRESULT CALLBACK WindowCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); //Window callback
Callback0R< MercuryWindow* > MercuryWindow::genWindowClbk(Win32Window::GenWin32Window); //Register window generation callback
//MercuryWindow* MercuryWindow::genWindowClbk = (void*)Win32Window::GenWin32Window;
bool ACTIVE = false;

MercuryWindow* Win32Window::GenWin32Window()
{
	if (stereo_view)
		return new Win32Window("Mercury3", 1280, 480, 24, 16, false);
	return new Win32Window("Mercury3", 640, 480, 24, 16, false);
}

LPCTSTR StringToLPCTSTR(const MString & s)
{
	size_t length = s.length();
	LPCTSTR str = new WCHAR[length+1];
	for (size_t i=0;i<length; ++i) ((WCHAR*)str)[i]=s[i];
	((WCHAR*)str)[length]=0;
	return str;
}

Win32Window::Win32Window(const MString& title, int width, int height, int bits, int depthBits, bool fullscreen)
	:m_hwnd(NULL), m_hdc(NULL), m_hglrc(NULL), m_hInstance(NULL), m_className(NULL), m_windowAtom(NULL), m_winTitle(NULL),m_cX(0),
	m_cY(0),MercuryWindow(title, width, height, bits, depthBits, fullscreen)
{
	m_className = (WCHAR*)StringToLPCTSTR("Mercury Render Window");
	m_winTitle = (WCHAR*)StringToLPCTSTR(title);

	GenWinClass();
	GenWindow();
	GenPixelType();
	SetPixelType();
	CreateRenderingContext();
}

Win32Window::~Win32Window()
{
	wglMakeCurrent(NULL, NULL);

	if (m_hglrc)
	{
		wglDeleteContext(m_hglrc);
		m_hglrc = NULL;
	}

	if (m_hwnd)
	{
		::DestroyWindow( m_hwnd );
		m_hwnd = NULL;
	}
	if ( !UnregisterClass((LPCWSTR)m_windowAtom,m_hInstance) )
	{
		m_hInstance = NULL;
	}
}

void Win32Window::GenWinClass()
{
	m_hInstance = GetModuleHandle(NULL);
	m_wndclass.cbSize = sizeof(WNDCLASSEX);
	m_wndclass.hIconSm = NULL;
	m_wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	m_wndclass.lpfnWndProc = (WNDPROC)Win32Window::WindowCallback;
	m_wndclass.cbClsExtra = 0;
	m_wndclass.cbWndExtra = 0;
	m_wndclass.hInstance = m_hInstance;
	m_wndclass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	m_wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	m_wndclass.hbrBackground = NULL;
	m_wndclass.lpszMenuName = NULL;
	m_wndclass.lpszClassName = m_className;
}

static Win32Window* windowInstances[1] = { 0 };

void Win32Window::GenWindow()
{
	DWORD dwExStyle; // Window Extended Style
	DWORD dwStyle;	// Window Style
	RECT rect;

	m_windowAtom = RegisterClassEx( &m_wndclass );

	if (m_windowAtom == NULL)
	{
		printf("Error registering window class, code %d", GetLastError() );
		throw(5);
	}

	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	dwStyle = WS_OVERLAPPEDWINDOW;

	rect.left=(long)0;
	rect.right=(long)m_width;
	rect.top=(long)0;
	rect.bottom=(long)m_height;	

	AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);

	m_hwnd = CreateWindowEx(
		dwExStyle,
		(LPCWSTR)m_windowAtom,
		m_winTitle,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle,
		0, 0,
		m_width,
		m_height,
		NULL,
		NULL,
		m_hInstance,
		NULL);

	windowInstances[0] = this;

	if (m_hwnd == NULL)
	{
		printf("Error creating window, code %d", GetLastError() );
		throw(5);
	}

	ShowWindow(m_hwnd,SW_SHOW);
	SetForegroundWindow(m_hwnd);					// Slightly Higher Priority
	SetFocus(m_hwnd);								// Sets Keyboard Focus To The Window

	//Resize windows so it's actually as big as we want, not minus borders.s
	GetClientRect( m_hwnd, &rect );
	int diffx = m_width - rect.right;
	int diffy = m_height - rect.bottom;
	SetWindowPos( m_hwnd, HWND_TOP, 0, 0, diffx + m_width, diffy + m_height, 0 );

//	wglSwapInterval(0);
}

void Win32Window::SetPixelType()
{
	int pixelFormat;
	m_hdc = GetDC(m_hwnd);

	if ( m_hdc == NULL )
	{
		printf("Error getting a device handle, code %d", GetLastError() );
		throw(5);
	}

	pixelFormat = ChoosePixelFormat(m_hdc, &m_pfd);
	if (pixelFormat == 0)
	{
		printf("Error choosing pixel format, code %d", GetLastError() );
		throw(5);
	}

	if( !SetPixelFormat(m_hdc, pixelFormat, &m_pfd) )
	{
		printf("Failed setting pixel format, code %d", GetLastError() );
		throw(5);
	}
}

void Win32Window::CreateRenderingContext()
{
	m_hglrc = wglCreateContext(m_hdc);

	if (m_hglrc == NULL)
	{
		printf("Failed to create rendering context, code %d", GetLastError() );
		throw(5);
	}

	if ( !wglMakeCurrent(m_hdc, m_hglrc) )
	{
		printf("Failed to set current rendering context, code %d", GetLastError() );
		throw(5);
	}
}

void Win32Window::GenPixelType()
{
	memset(&m_pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	m_pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	m_pfd.nVersion = 1;
	m_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	m_pfd.iPixelType = PFD_TYPE_RGBA;
	m_pfd.cColorBits = m_bits;
	m_pfd.cDepthBits = m_depthBits;
	m_pfd.iLayerType = PFD_MAIN_PLANE;
}

bool Win32Window::SwapBuffers()
{
//	glEnable(GL_NORMALIZE); //deprecated
//	glEnable (GL_BLEND); 
//	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glFinish();

	return (::SwapBuffers( m_hdc )==TRUE);
}

bool Win32Window::PumpMessages()
{
	static int lastx, lasty;
	static bool bWasCursorVisible;
	MSG message;

	if ( InFocus() != ACTIVE )
	{
		m_inFocus = ACTIVE;
//		ShowCursor(!m_inFocus);
		
		if( GlobalMouseGrabbed_Set )
		{
			RECT rect;
			GetWindowRect(m_hwnd, &rect);
			SetCursorPos( rect.left + m_width/2, rect.top + m_height/2 );
		}
		
	}

	while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
	{
		RECT rect;
		GetClientRect( m_hwnd, &rect );
		m_width = rect.right;
		m_height = rect.bottom;

		if ( InFocus() )
		{
			switch( message.message )
			{
			case WM_KEYDOWN:
				{
					if ( IsKeyRepeat(message.lParam) ) break;
//					printf("event %d\n", ConvertScancode(message.lParam));
					uint16_t x = ConvertScancode(message.lParam);
					KeyDownMap[x] = 1;
//					KeyboardInput::ProcessKeyInput( ConvertScancode( message.lParam ), true, false);
				}
				break;
			case WM_KEYUP:
				{
					if ( IsKeyRepeat(message.lParam) ) break;
					uint16_t x = ConvertScancode(message.lParam);
					KeyDownMap[x] = 0;
//					KeyboardInput::ProcessKeyInput( ConvertScancode( message.lParam ), false, false);
				}
				break;
			case WM_MOUSEMOVE:
				{
					POINT pos;
					RECT rect;
					GetWindowRect(m_hwnd, &rect);
					GetCursorPos(&pos);
					ScreenToClient(m_hwnd, &pos );
//					bool left = GetBit(message.wParam,MK_LBUTTON);
//					bool right = GetBit(message.wParam,MK_RBUTTON);
//					bool center = GetBit(message.wParam,MK_MBUTTON);
					bool su = 0;
					bool sd = 0;
					int px = pos.x;// - rect.left - borderx;
					int py = pos.y;// - rect.top - bordery;

					if( GlobalMouseGrabbed_Set )
					{
						int x, y;
						x = m_width/2 - px;
						y = m_height/2 - py;


						if (x!=0 || y!=0) //prevent recursive XWarp
						{
							m_iLastMouseX = x;
							m_iLastMouseY = y;
							MOUSE_INPUT.dx += x;
							MOUSE_INPUT.dy += y;
//							printf("mouse %d %d\n", MOUSE_INPUT.dx, MOUSE_INPUT.dy);
							//							MouseInput::ProcessMouseInput(x, y, left, right, center, su, sd, true);
							lastx = x; lasty = y;
							
							pos.x = m_width/2;
							pos.y = m_height/2;
							ClientToScreen(m_hwnd, &pos );
							SetCursorPos( pos.x, pos.y);
							if( bWasCursorVisible )
							{
								bWasCursorVisible = false;
								ShowCursor( false );
							}
						}
					}
					else
					{
						MOUSE_INPUT.dx += m_iLastMouseX - px;
						MOUSE_INPUT.dy += m_iLastMouseY - py;

						m_iLastMouseX = px;
						m_iLastMouseY = py;
						lastx = px; lasty = py;

//						printf("mouse %d %d\n", MOUSE_INPUT.dx, MOUSE_INPUT.dy);

//						MouseInput::ProcessMouseInput(px, py, left, right, center, su, sd, true);
						if( !bWasCursorVisible )
						{
							ShowCursor( true );
							bWasCursorVisible = true;
						}
					}
				}
				break;
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
//				MouseInput::ProcessMouseInput( lastx, lasty, GetBit(message.wParam,MK_LBUTTON), 
//					GetBit(message.wParam,MK_RBUTTON), GetBit(message.wParam,MK_MBUTTON), 0, 0, false);
				break;
			case 0x020A: //Not-too-well-documented mouse wheel.
			{
				short cx = short(message.wParam>>16);
//				MouseInput::ProcessMouseInput( lastx, lasty, GetBit(message.wParam,MK_LBUTTON), 
//					GetBit(message.wParam,MK_RBUTTON), GetBit(message.wParam,MK_MBUTTON), (cx>0), (cx<0), false);
//				MouseInput::ProcessMouseInput( lastx, lasty, GetBit(message.wParam,MK_LBUTTON), 
//					GetBit(message.wParam,MK_RBUTTON), GetBit(message.wParam,MK_MBUTTON), 0, 0, false);
				break;
			}
			}
		}
		TranslateMessage(&message);				// Translate The Message
		DispatchMessage(&message);				// Dispatch The Message
	}
	return true;
}

void* Win32Window::GetProcAddress(const MString& x)
{
	return wglGetProcAddress( x.c_str() );
}

bool Win32Window::IsKeyRepeat(uint32_t c)
{
//	printf("count %d\n", (c&65535));
	return (c&65535) > 1;
}

uint16_t Win32Window::ConvertScancode( uint32_t scanin )
{
//	Specifies the scan code. The value depends on the OEM.
	scanin = (scanin>>16)&511;
	switch( scanin )
	{
	case 1: return 27;      //esc
	case 0: return '0';
	case 41: return 97;     //`
	case 14: return 8;      //backspace
	case 87: return 292;    //F11
	case 88: return 293;    //F12
	case 12: return 45;     //-
	case 13: return 61;     //=
	case 43: return 92;     //backslash
	case 15: return 9;      //tab
	case 58: return 15;     //Caps lock
	case 42: return 160;    //[lshift]
	case 54: return 161;    //[rshift]

	case 30: return 'a';
	case 48: return 'b';
	case 46: return 'c';
	case 32: return 'd';
	case 18: return 'e';
	case 33: return 'f';
	case 34: return 'g';
	case 35: return 'h';
	case 23: return 'i';
	case 36: return 'j';
	case 37: return 'k';	
	case 38: return 'l';	
	case 50: return 'm';	
	case 49: return 'n';	
	case 24: return 'o';	
	case 25: return 'p';	
	case 16: return 'q';	
	case 19: return 'r';	
	case 31: return 's';	
	case 20: return 't';	
	case 22: return 'u';	
	case 47: return 'v';	
	case 17: return 'w';	
	case 45: return 'x';	
	case 21: return 'y';	
	case 44: return 'z';	

	case 39: return 59;	//;
	case 40: return 39;	//'
	case 51: return 44;	//,
	case 52: return 46;	//.
	case 53: return 47;	// /

	case 328: return 273;	//arrow keys: up
	case 331: return 276;	//arrow keys: left
	case 333: return 275;	//arrow keys: right
	case 336: return 274;	//arrow keys: down
//STOPPED HERE
	case 29: return 162;	//left ctrl
	case 347: return 91;	//left super (aka win)
	case 64: return 164;	//left alt
	case 57: return 32;	//space bar
	case 108: return 165;	//right alt
	case 134: return 91;	//right super (aka win)
	case 349: return 93;	//menu
	case 285: return 268;	//right control
	
	case 107: return 316;	//Print Screen
	//case 78: scroll lock
	case 127: return 19;	//Pause
	case 118: return 277;	//Insert
	case 110: return 278;	//Home
	case 112: return 280;	//Page Up
	case 119: return 127;	//Delete
	case 115: return 279;	//End
	case 117: return 181;	//Page Down
	
	//case 77: Num Lock (not mapped)
	case 106: return 267;	//Keypad /
	case 63: return 268;	//Keypad *
	case 82: return 269;	//Keypad -
	case 79: return 263;	//Keypad 7
	case 80: return 264;	//Keypad 8
	case 81: return 265;	//Keypad 9
	case 86: return 270;	//Keypad +
	case 83: return 260;	//Keypad 4
	case 84: return 261;	//Keypad 5
	case 85: return 262;	//Keypad 6
//	case 87: return 257;	//Keypad 1
//	case 88: return 258;	//Keypad 2
	case 89: return 259;	//Keypad 3
//	case 36:		//Enter
	case 104: return 13;	//Keypad enter
	case 90: return 260;	//Keypad 0
	case 91: return 266;	//Keypad .
	
	default:
		// numbers
		if( scanin >= 10 && scanin <= 18 )
			return scanin + ( (short)'1' - 10 );
		// f1 -- f10
		if( scanin >= 67 && scanin <= 76 )
			return scanin + ( 282 - 67 );
		return scanin;
	}
}

LRESULT CALLBACK Win32Window::WindowCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Win32Window* window = NULL;

	for (uint8_t i = 0; i < 1; i++) {

		if (windowInstances[i]!=NULL && windowInstances[i]->m_hwnd == hWnd) {
			window = windowInstances[i];
			break;
		}
	}

	switch (uMsg)
	{
	case WM_CLOSE:
		if (window) window->m_close = true;
		break;
	case WM_ACTIVATE:
		ACTIVE = LOWORD(wParam)!=WA_INACTIVE;
//		return 0;
		break;
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

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
