/*
 cl /LD gvimfullscreen.c user32.lib
 ------------------------------
 :call libcallnr("gvimfullscreen.dll", "EnableFullScreen", 1)
*/
#include <windows.h>

BOOL CALLBACK FindWindowProc(HWND hwnd, LPARAM lParam)
{
    HWND* pphWnd = (HWND*)lParam;

	if (GetParent(hwnd))
	{
   		*pphWnd = NULL;
		return TRUE;
	}
   	*pphWnd = hwnd;
   	return FALSE;
}

LONG _declspec(dllexport) ToggleFullScreen()
{
	HWND hTop = NULL;
    HWND hTextArea = NULL;

	EnumThreadWindows(GetCurrentThreadId(), FindWindowProc, (LPARAM)&hTop);
    hTextArea = FindWindowEx(hTop, NULL, "VimTextArea", "Vim text area");

	if (hTop != NULL && hTextArea != NULL)
	{
		/* Determine the current state of the window */
		if ( GetWindowLong(hTop, GWL_STYLE) & WS_CAPTION )
		{
			/* Has a caption, so isn't maximised */
            int g_x, g_y, g_dx, g_dy;

			MONITORINFO mi;
			RECT rc;
			HMONITOR hMonitor;

			GetWindowRect(hTop, &rc);
			hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);

			// 
			// get the work area or entire monitor rect, if have multiple display monitors, the rect is current work area.
			// 
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);

			g_x = mi.rcMonitor.left;
			g_y = mi.rcMonitor.top;
			g_dx = mi.rcMonitor.right - g_x;
			g_dy = mi.rcMonitor.bottom - g_y;
			//cx = GetSystemMetrics(SM_CXSCREEN);
			//cy = GetSystemMetrics(SM_CYSCREEN);

			/* Remove border, caption, and edges */
			SetWindowLong(hTop, GWL_STYLE, GetWindowLong(hTop, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW); 
			SetWindowLong(hTop, GWL_EXSTYLE, GetWindowLong(hTop, GWL_EXSTYLE) & ~WS_EX_WINDOWEDGE); 
            SetWindowPos(hTop, HWND_TOP, g_x, g_y, g_dx, g_dy, SWP_SHOWWINDOW);
            // modify textarea
            SetWindowLong(hTextArea, GWL_EXSTYLE, GetWindowLong(hTextArea, GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE); 

            // use DC_BRUSH to modify textarea background brush color to darkblue(my vim colorschema)
            {
                HDC dc = GetDC(hTextArea);
                if (dc != NULL)
                {
                    SetDCBrushColor(dc, RGB(0, 0, 64));
                    ReleaseDC(hTextArea, dc);
                }
            }
            // SetClassLong(hTextArea, GCL_HBRBACKGROUND, (LONG)CreateSolidBrush(RGB(0 ,0, 64)));
            SetClassLong(hTextArea, GCL_HBRBACKGROUND, (LONG)GetStockObject(DC_BRUSH));

            SetWindowPos(hTextArea, HWND_TOP, 0, 0, g_dx-g_x, g_dy-g_y, SWP_SHOWWINDOW);
        }
        else
        {
            /* Already full screen, so restore all the previous styles */
            SetWindowLong(hTop, GWL_STYLE, GetWindowLong(hTop, GWL_STYLE) | WS_OVERLAPPEDWINDOW); 
            SetWindowLong(hTop, GWL_EXSTYLE, GetWindowLong(hTop, GWL_EXSTYLE) | WS_EX_WINDOWEDGE); 
            // modify textarea
            SetWindowLong(hTextArea, GWL_EXSTYLE, GetWindowLong(hTextArea, GWL_EXSTYLE) | WS_EX_CLIENTEDGE); 

            SendMessage(hTop, WM_SYSCOMMAND, SC_RESTORE, 0);
            SendMessage(hTop, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        }
    }
    return GetLastError();
}
