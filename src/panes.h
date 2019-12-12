#include <windows.h>
#include <windowsx.h>
#include <vector>

using namespace std;

#ifndef RECTD_H
#define RECTD_H
struct RECTD{
	double left;
	double right;
	double top;
	double bottom;
};
#endif /*RECTD*/

#ifndef POINTD_H
#define POINTD_H
struct POINTD{
	double x;
	double y;
};
#endif /*POINTD_H*/

#ifndef PANE_H
#define PANE_H
struct PANE{
	HWND hWnd;
	struct RECTD rect;
	double x_ratio;
	double y_ratio;
	double width_ratio;
	double height_ratio;
};
#endif /*PANE_H*/


void AttachWindow(HWND hWnd, int paneIndex);
void DettachWindow(int paneIndex);
void PanesToString();
void MoveAllPanes(long x, long y);
void TransformWindows(RECT client_rect);
void SplitPane(int paneIndex, bool Vertical);
void DeletePane(int paneIndex);
bool WindowMoved(int paneIndex);
bool WindowResized(int paneIndex);
void ResizePane(int paneIndex, RECTD change);
void CalculatePaneSizeRatios();
void NormalResizeAllPanes(RECT client_rect);
void InitPanes();
void UpdatePanes(RECT window_rect, RECT client_rect);
void ActivatePaneWindow(int paneIndex, RECT window_rect, RECT client_rect);
int GetAdjacentPane(int paneIndex, int direction, RECT window_rect);
int GetPaneFromWindow(HWND hWnd);
int GetPaneFromPoint(POINT point, RECT client_rect);
void BringPanesToFront(HWND hWndTopWindow);
