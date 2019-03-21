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

bool stereo_view;

//LRESULT CALLBACK WindowCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); //Window callback
Callback0R< MercuryWindow* > MercuryWindow::genWindowClbk(Win32Window::GenWin32Window); //Register window generation callback
//MercuryWindow* MercuryWindow::genWindowClbk = (void*)Win32Window::GenWin32Window;
bool ACTIVE = false;

MercuryWindow* Win32Window::GenWin32Window()
{
	if (stereo_view)
		return new Win32Window("Mercury3", 1280, 480, 24, 16, false);
	return new Win32Window("Mercury3", 800, 600, 24, 16, false);
}

template<typename STRTYPE>
STRTYPE* StringToLPCTSTR(const std::string& s)
{
	size_t length = s.length();
	STRTYPE* str = new STRTYPE[length+1];
	for (size_t i = 0; i < length; ++i) {
		str[i] = s[i];
	}
	str[length]=0;
	return str;
}

Win32Window::Win32Window(const MString& title, int width, int height, int bits, int depthBits, bool fullscreen)
	:m_hwnd(NULL), m_hdc(NULL), m_hglrc(NULL), m_hInstance(NULL), m_className(NULL), m_windowAtom(NULL), m_winTitle(NULL),m_cX(0),
	m_cY(0),MercuryWindow(title, width, height, bits, depthBits, fullscreen)
{
	m_className = StringToLPCTSTR<WCHAR>("Mercury Render Window");
	m_winTitle = StringToLPCTSTR<WCHAR>(title);

	GenWinClass();
	GenWindow();
	GenPixelType();
	SetPixelType();
	CreateRenderingContext();

	if (fullscreen) {
		ChangeDisplaySettings();
	}
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
	//DWORD dwStyle;	// Window Style
	RECT rect;

	m_windowAtom = RegisterClassEx( &m_wndclass );

	if (m_windowAtom == NULL)
	{
		printf("Error registering window class, code %d", GetLastError() );
		throw(5);
	}

	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

	//if (m_fullscreen) {
	//	dwStyle = WS_POPUP; //required for fullscreen
	//}

	rect.left=(long)0;
	rect.right=(long)m_requestedWidth;
	rect.top=(long)0;
	rect.bottom=(long)m_requestedHeight;	

	//store default window style values
	m_dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	m_dwStyle |= WS_OVERLAPPEDWINDOW; //windowed mode

	auto dwStyle = m_dwStyle;

	if (m_fullscreen) {
		dwStyle = WS_POPUP; //required for fullscreen
	}

	AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);

	m_hwnd = CreateWindowEx(
		dwExStyle,
		(LPCWSTR)m_windowAtom,
		m_winTitle,
		dwStyle,
		0, 0,
		m_requestedWidth,
		m_requestedHeight,
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
	int diffx = m_requestedWidth - rect.right;
	int diffy = m_requestedHeight - rect.bottom;
	SetWindowPos( m_hwnd, HWND_TOP, 0, 0, diffx + m_requestedWidth, diffy + m_requestedHeight, 0 );

	GetClientRect(m_hwnd, &rect);
	m_currentWidth = rect.right;
	m_currentHeight = rect.bottom;

//	wglSwapInterval(0);
}

LONG_PTR Win32Window::ChangeStyle(DWORD dwStyle) {
	auto oldStyle = SetWindowLongPtr(m_hwnd, GWL_STYLE, dwStyle);

	ShowWindow(m_hwnd, SW_SHOW);	//needed after SetWindowLongPtr
	SetForegroundWindow(m_hwnd);	// Slightly Higher Priority
	SetFocus(m_hwnd);				// Sets Keyboard Focus To The Window

	return oldStyle;
}

bool Win32Window::ChangeDisplaySettings() {
	DEVMODE screenSettings;
	memset(&screenSettings, 0, sizeof(screenSettings));
	screenSettings.dmSize = sizeof(screenSettings);
	screenSettings.dmPelsWidth = m_requestedWidth;
	screenSettings.dmPelsHeight = m_requestedHeight;
	screenSettings.dmBitsPerPel = m_bits;
	screenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	DWORD dwFlags = 0;
	dwFlags = m_fullscreen ? CDS_FULLSCREEN | dwFlags : dwFlags;

	auto r = ::ChangeDisplaySettings(&screenSettings, dwFlags);

	char* error_msg = nullptr;
	bool ret = true;

	switch (r) {
	case DISP_CHANGE_SUCCESSFUL:
		break;
	case DISP_CHANGE_BADDUALVIEW:
		error_msg = "The settings change was unsuccessful because the system is DualView capable.";
		break;
	case DISP_CHANGE_BADFLAGS:
		error_msg = "An invalid set of flags was passed in.";
		break;
	case DISP_CHANGE_BADMODE:
		error_msg = "The graphics mode is not supported.";
		break;
	case DISP_CHANGE_BADPARAM:
		error_msg = "An invalid parameter was passed in.This can include an invalid flag or combination of flags.";
		break;
	case DISP_CHANGE_FAILED:
		error_msg = "The display driver failed the specified graphics mode.";
		break;
	case DISP_CHANGE_NOTUPDATED:
		error_msg = "Unable to write settings to the registry.";
		break;
	case DISP_CHANGE_RESTART:
		error_msg = "The computer must be restarted for the graphics mode to work.";
		break;
	}
	if (error_msg != nullptr) {
		fprintf(stderr, "Failed to change screen mode: %s\n", error_msg);
		ret = false;
	}

	RECT rect;
	GetClientRect(m_hwnd, &rect);
	m_currentWidth = rect.right;
	m_currentHeight = rect.bottom;

	return ret;
}

void Win32Window::TryFullscreen() {
	DWORD dwStyle = m_dwStyle;
	if (m_fullscreen) {
		dwStyle = WS_POPUP;
	}

	auto oldStyle = ChangeStyle(dwStyle);

	//if (m_fullscreen) {
		bool r = ChangeDisplaySettings();
		if (!r) {
			m_fullscreen = !m_fullscreen;
			ChangeStyle(oldStyle);
		}
	//}
	//maybe chake back to old settings if not fullscreen
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
//	glFinish();

	return (::SwapBuffers( m_hdc )==TRUE);
}

bool Win32Window::PumpMessages()
{
	using namespace ENGINE::INPUT;
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
			SetCursorPos( rect.left + m_currentWidth/2, rect.top + m_currentHeight/2 );
		}
		
	}

	for (int i = 0; i < KeyDownMap_length; ++i) {
		KeyRepeatMap[i] = KeyDownMap[i];
	}

	while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
	{
		if ( InFocus() )
		{
//			printf("message %d\n", message.message);
			switch( message.message )
			{
			//case WM_MINMAXINFO:
			//	break;
			case WM_CHAR:
				//probably useful for typing purposes
				break;
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
				{
					if (IsKeyRepeat(message.lParam)) break;
					const auto x = ConvertScancode2(message.wParam);
					KeyDownMap[x] = true;
					//printf("key: 0x%x\n", message.wParam);
//					KeyboardInput::ProcessKeyInput( ConvertScancode( message.lParam ), true, false);
				}
				break;
			case WM_SYSKEYUP:
			case WM_KEYUP:
				{
					if ( IsKeyRepeat(message.lParam) ) break;
					const auto x = ConvertScancode2(message.wParam);
					KeyDownMap[x] = false;
					KeyRepeatMap[x] = false;
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
						x = m_currentWidth/2 - px;
						y = m_currentHeight/2 - py;


						if (x!=0 || y!=0) //prevent recursive XWarp
						{
							m_iLastMouseX = x;
							m_iLastMouseY = y;
							MOUSE_INPUT.dx += x;
							MOUSE_INPUT.dy += y;
//							printf("mouse %d %d\n", MOUSE_INPUT.dx, MOUSE_INPUT.dy);
							//							MouseInput::ProcessMouseInput(x, y, left, right, center, su, sd, true);
							lastx = x; lasty = y;
							
							pos.x = m_currentWidth/2;
							pos.y = m_currentHeight/2;
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
				KeyDownMap[KeyCodes::KEY_C_MOUSE] = (message.wParam & MK_MBUTTON > 0);
				KeyDownMap[KeyCodes::KEY_L_MOUSE] = (message.wParam & MK_LBUTTON > 0);
				KeyDownMap[KeyCodes::KEY_R_MOUSE] = (message.wParam & MK_RBUTTON > 0);
				//				MouseInput::ProcessMouseInput( lastx, lasty, GetBit(message.wParam,MK_LBUTTON), 
//					GetBit(message.wParam,MK_RBUTTON), GetBit(message.wParam,MK_MBUTTON), 0, 0, false);
				break;
			case WM_MOUSEHWHEEL: //Not-too-well-documented mouse wheel.
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
		if (message.message != WM_SYSKEYDOWN && message.message != WM_SYSKEYUP) {
			DispatchMessage(&message);				// Dispatch The Message
		}
	}
	KeyDownMap[0] = false;
	return true;
}

void* Win32Window::GetProcAddress(const MString& x)
{
	return wglGetProcAddress( x.c_str() );
}

bool Win32Window::IsKeyRepeat(size_t c)
{
//	printf("count %d\n", (c&65535));
	return (c&0xffff) > 1;
}

ENGINE::INPUT::KeyCodes Win32Window::ConvertScancode2(size_t virtualKey)
{
	using namespace ENGINE::INPUT;

	//numbers
	if (virtualKey >= 0x30 && virtualKey <= 0x39) {
		const auto t = virtualKey - 0x30;
		return (KeyCodes)(KeyCodes::KEY_0 + t);
	}

	//letters
	if (virtualKey >= 0x41 && virtualKey <= 0x5A) {
		const auto t = virtualKey - 0x41;
		return (KeyCodes)(KeyCodes::KEY_A + t);
	}

	switch (virtualKey) {
	case VK_LBUTTON:			return KeyCodes::KEY_R_MOUSE;
	case VK_RBUTTON:			return KeyCodes::KEY_L_MOUSE;
	case VK_CANCEL:				break;
	case VK_MBUTTON:			return KeyCodes::KEY_C_MOUSE;
	case VK_XBUTTON1:			break;
	case VK_XBUTTON2:			break;
	case VK_BACK:				return KeyCodes::KEY_BACKSPACE;
	case VK_TAB:				return KeyCodes::KEY_TAB;
	case VK_CLEAR:				return KeyCodes::KEY_CLEAR;
	case VK_RETURN:				return KeyCodes::KEY_ENTER;
	case VK_SHIFT:				return KeyCodes::KEY_SHIFT;
	case VK_CONTROL:			return KeyCodes::KEY_CONTROL;
	case VK_MENU:				return KeyCodes::KEY_ALT;
	case VK_PAUSE:				return KeyCodes::KEY_PAUSE;
	case VK_CAPITAL:			return KeyCodes::KEY_CAPS;
	case VK_ESCAPE:				return KeyCodes::KEY_ESCAPE;
	case VK_CONVERT:			break;
	case VK_NONCONVERT:			break;
	case VK_ACCEPT:				break;
	case VK_MODECHANGE:			break;
	case VK_SPACE:				return KeyCodes::KEY_SPACE;
	case VK_PRIOR:				return KeyCodes::KEY_PAGEUP;
	case VK_NEXT:				return KeyCodes::KEY_PAGEDOWN;
	case VK_END:				return KeyCodes::KEY_END;
	case VK_HOME:				return KeyCodes::KEY_HOME;
	case VK_LEFT:				return KeyCodes::KEY_LEFTARROW;
	case VK_UP:					return KeyCodes::KEY_UPARROW;
	case VK_RIGHT:				return KeyCodes::KEY_RIGHTARROW;
	case VK_DOWN:				return KeyCodes::KEY_DOWNARROW;
	case VK_SELECT:				return KeyCodes::KEY_SELECT;
	case VK_PRINT:				return KeyCodes::KEY_PRINT;
	case VK_EXECUTE:			return KeyCodes::KEY_EXE;
	case VK_SNAPSHOT:			return KeyCodes::KEY_PRINTSCREEN;
	case VK_INSERT:				return KeyCodes::KEY_INSERT;
	case VK_DELETE:				return KeyCodes::KEY_DELETE;
	case VK_HELP:				return KeyCodes::KEY_HELP;
	case VK_LWIN:				return KeyCodes::KEY_LWINDOWS;
	case VK_RWIN:				return KeyCodes::KEY_RWINDOWS;
	case VK_APPS:				break;
	case VK_SLEEP:				break;
	case VK_NUMPAD0:			return KeyCodes::KEY_NUM0;
	case VK_NUMPAD1:			return KeyCodes::KEY_NUM1;
	case VK_NUMPAD2:			return KeyCodes::KEY_NUM2;
	case VK_NUMPAD3:			return KeyCodes::KEY_NUM3;
	case VK_NUMPAD4:			return KeyCodes::KEY_NUM4;
	case VK_NUMPAD5:			return KeyCodes::KEY_NUM5;
	case VK_NUMPAD6:			return KeyCodes::KEY_NUM6;
	case VK_NUMPAD7:			return KeyCodes::KEY_NUM7;
	case VK_NUMPAD8:			return KeyCodes::KEY_NUM8;
	case VK_NUMPAD9:			return KeyCodes::KEY_NUM9;
	case VK_MULTIPLY:			return KeyCodes::KEY_MULTIPLY;
	case VK_ADD:				return KeyCodes::KEY_ADD;
	case VK_SEPARATOR:			break;
	case VK_SUBTRACT:			return KeyCodes::KEY_SUBTRACT;
	case VK_DECIMAL:			return KeyCodes::KEY_DECIMAL;
	case VK_DIVIDE:				return KeyCodes::KEY_DIVIDE;
	case VK_F1:					return KeyCodes::KEY_F1;
	case VK_F2:					return KeyCodes::KEY_F2;
	case VK_F3:					return KeyCodes::KEY_F3;
	case VK_F4:					return KeyCodes::KEY_F4;
	case VK_F5:					return KeyCodes::KEY_F5;
	case VK_F6:					return KeyCodes::KEY_F6;
	case VK_F7:					return KeyCodes::KEY_F7;
	case VK_F8:					return KeyCodes::KEY_F8;
	case VK_F9:					return KeyCodes::KEY_F9;
	case VK_F10:				return KeyCodes::KEY_F10;
	case VK_F11:				return KeyCodes::KEY_F11;
	case VK_F12:				return KeyCodes::KEY_F12;
	case VK_F13:				return KeyCodes::KEY_F13;
	case VK_F14:				return KeyCodes::KEY_F14;
	case VK_F15:				return KeyCodes::KEY_F15;
	case VK_F16:				return KeyCodes::KEY_F16;
	case VK_F17:				return KeyCodes::KEY_F17;
	case VK_F18:				return KeyCodes::KEY_F18;
	case VK_F19:				return KeyCodes::KEY_F19;
	case VK_F20:				return KeyCodes::KEY_F20;
	case VK_F21:				return KeyCodes::KEY_F21;
	case VK_F22:				return KeyCodes::KEY_F22;
	case VK_F23:				return KeyCodes::KEY_F23;
	case VK_F24:				return KeyCodes::KEY_F24;
	case VK_NUMLOCK:			return KeyCodes::KEY_NUMLOCK;
	case VK_SCROLL:				return KeyCodes::KEY_SCROLLLOCK;
	case VK_LSHIFT:				return KeyCodes::KEY_LSHIFT;
	case VK_RSHIFT:				return KeyCodes::KEY_RSHIFT;
	case VK_LCONTROL:			return KeyCodes::KEY_LCONTROL;
	case VK_RCONTROL:			return KeyCodes::KEY_RCONTROL;
	case VK_LMENU:				break;
	case VK_RMENU:				break;
	case VK_BROWSER_BACK:		break;
	case VK_BROWSER_FORWARD:	break;
	case VK_BROWSER_REFRESH:	break;
	case VK_BROWSER_STOP:		break;
	case VK_BROWSER_SEARCH:		break;
	case VK_BROWSER_FAVORITES:	break;
	case VK_BROWSER_HOME:		break;
	case VK_VOLUME_MUTE:		break;
	case VK_VOLUME_DOWN:		break;
	case VK_VOLUME_UP:			break;
	case VK_MEDIA_NEXT_TRACK:	break;
	case VK_MEDIA_PREV_TRACK:	break;
	case VK_MEDIA_STOP:			break;
	case VK_MEDIA_PLAY_PAUSE:	break;
	case VK_LAUNCH_MAIL:		break;
	case VK_LAUNCH_MEDIA_SELECT:break;
	case VK_LAUNCH_APP1:		break;
	case VK_LAUNCH_APP2:		break;
	case VK_OEM_1:				return KeyCodes::KEY_SEMICOLON;
	case VK_OEM_PLUS:			return KeyCodes::KEY_PLUS;
	case VK_OEM_COMMA:			return KeyCodes::KEY_COMMA;
	case VK_OEM_MINUS:			return KeyCodes::KEY_MINUS;
	case VK_OEM_PERIOD:			return KeyCodes::KEY_PERIOD;
	case VK_OEM_2:				return KeyCodes::KEY_QUESTIONMARK;
	case VK_OEM_3:				return KeyCodes::KEY_TILDE;
	case VK_OEM_4:				return KeyCodes::KEY_L_BRACKET;
	case VK_OEM_5:				return KeyCodes::KEY_PIPE;
	case VK_OEM_6:				return KeyCodes::KEY_R_BRACKET;
	case VK_OEM_7:				return KeyCodes::KEY_QUOTE;
	case VK_OEM_8:				break;
	case VK_OEM_102:			break;
	case VK_PROCESSKEY:			break;
	case VK_ATTN:				break;
	case VK_CRSEL:				break;
	case VK_EXSEL:				break;
	case VK_EREOF:				break;
	case VK_PLAY:				break;
	case VK_ZOOM:				break;
	case VK_NONAME:				break;
	case VK_PA1:				break;
	case VK_OEM_CLEAR:			break;
	}

	return KeyCodes::KEY_UNKNOWN;
}
//
//uint16_t Win32Window::ConvertScancode( size_t scanin )
//{
//	using namespace ENGINE::INPUT;
//
////	Specifies the scan code. The value depends on the OEM.
//	scanin = (scanin>>16)&511;
//
//	//printf("")
//
//	switch( scanin )
//	{
//	case 1: return KeyCodes::KEY_ESCAPE;      //esc
//	case 0: return KeyCodes::KEY_0;
//	case 41: return 97;     //`
//	case 14: return 8;      //backspace
//	case 87: return 292;    //F11
//	case 88: return 293;    //F12
//	case 12: return 45;     //-
//	case 13: return 61;     //=
//	case 43: return 92;     //backslash
//	case 15: return 9;      //tab
//	case 58: return 15;     //Caps lock
//	case 42: return 160;    //[lshift]
//	case 54: return 161;    //[rshift]
//
//	case 30: return 'a';
//	case 48: return 'b';
//	case 46: return 'c';
//	case 32: return 'd';
//	case 18: return 'e';
//	case 33: return 'f';
//	case 34: return 'g';
//	case 35: return 'h';
//	case 23: return 'i';
//	case 36: return 'j';
//	case 37: return 'k';	
//	case 38: return 'l';	
//	case 50: return 'm';	
//	case 49: return 'n';	
//	case 24: return 'o';	
//	case 25: return 'p';	
//	case 16: return 'q';	
//	case 19: return 'r';	
//	case 31: return 's';	
//	case 20: return 't';	
//	case 22: return 'u';	
//	case 47: return 'v';	
//	case 17: return 'w';	
//	case 45: return 'x';	
//	case 21: return 'y';	
//	case 44: return 'z';	
//
//	case 39: return 59;	//;
//	case 40: return 39;	//'
//	case 51: return 44;	//,
//	case 52: return 46;	//.
//	case 53: return 47;	// /
//
//	case 328: return 273;	//arrow keys: up
//	case 331: return 276;	//arrow keys: left
//	case 333: return 275;	//arrow keys: right
//	case 336: return 274;	//arrow keys: down
////STOPPED HERE
//	case 29: return 162;	//left ctrl
//	case 347: return 91;	//left super (aka win)
//	case 64: return KeyCodes::KEY_L_ALT;	//left alt
//	case 57: return 32;	//space bar
//	case 108: return KeyCodes::KEY_R_ALT;	//right alt
//	case 134: return 91;	//right super (aka win)
//	case 349: return 93;	//menu
//	case 285: return 268;	//right control
//	
//	case 107: return 316;	//Print Screen
//	//case 78: scroll lock
//	case 127: return 19;	//Pause
//	case 118: return 277;	//Insert
//	case 110: return 278;	//Home
//	case 112: return 280;	//Page Up
//	case 119: return 127;	//Delete
//	case 115: return 279;	//End
//	case 117: return 181;	//Page Down
//	
//	//case 77: Num Lock (not mapped)
//	case 106: return 267;	//Keypad /
//	case 63: return 268;	//Keypad *
//	case 82: return 269;	//Keypad -
//	case 79: return 263;	//Keypad 7
//	case 80: return 264;	//Keypad 8
//	case 81: return 265;	//Keypad 9
//	case 86: return 270;	//Keypad +
//	case 83: return 260;	//Keypad 4
//	case 84: return 261;	//Keypad 5
//	case 85: return 262;	//Keypad 6
////	case 87: return 257;	//Keypad 1
////	case 88: return 258;	//Keypad 2
//	case 89: return 259;	//Keypad 3
////	case 36:		//Enter
//	case 104: return 13;	//Keypad enter
//	case 90: return 260;	//Keypad 0
//	case 91: return 266;	//Keypad .
//	
//	default:
//		// numbers
//		if( scanin >= 10 && scanin <= 18 )
//			return (uint16_t)(scanin + ( (short)'1' - 10 ));
//		// f1 -- f10
//		if( scanin >= 67 && scanin <= 76 )
//			return (uint16_t)(scanin + ( 282 - 67 ));
//		return (uint16_t)scanin;
//	}
//}

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
