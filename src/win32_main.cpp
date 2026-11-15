#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include "explorer.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <exception>
#include <vector>
#include <random>
namespace {
mv::Explorer state;
bool dragging=false, moved=false;
mv::Point down={},previous={};
std::chrono::steady_clock::time_point last;
struct Layout{int x,y,w,h;double scale;};
Layout layout(HWND hwnd){RECT r;GetClientRect(hwnd,&r);double s=std::max(0.01,std::min(r.right/1200.0,r.bottom/810.0));int w=int(1200*s),h=int(810*s);return {(r.right-w)/2,(r.bottom-h)/2,w,h,s};}
mv::Point logical(HWND hwnd,int x,int y){auto r=layout(hwnd);return {(x-r.x)/r.scale,(y-r.y)/r.scale};}
void advance(){auto now=std::chrono::steady_clock::now();state.tick(std::chrono::duration<double>(now-last).count());last=now;}
// Own the entire client-sized back buffer. Never clear the visible window
// between animation frames: present the completed image with one BitBlt.
struct BackBuffer {
 HDC dc=nullptr; HBITMAP bitmap=nullptr; HGDIOBJ original=nullptr;
 int width=0,height=0;
 ~BackBuffer(){clear();}
 void clear(){if(dc){if(original)SelectObject(dc,original);if(bitmap)DeleteObject(bitmap);DeleteDC(dc);}dc=nullptr;bitmap=nullptr;original=nullptr;width=height=0;}
 bool resize(HDC target,int w,int h){
  if(dc&&w==width&&h==height)return true;
  clear();dc=CreateCompatibleDC(target);if(!dc)return false;
  bitmap=CreateCompatibleBitmap(target,w,h);if(!bitmap){clear();return false;}
  original=SelectObject(dc,bitmap);width=w;height=h;return true;
 }
} backBuffer;
void paint(HWND hwnd){
 PAINTSTRUCT ps;HDC dc=BeginPaint(hwnd,&ps);
 try {
 RECT client;GetClientRect(hwnd,&client);int w=client.right,h=client.bottom;
 if(w>0&&h>0&&backBuffer.resize(dc,w,h)){
 mv::Canvas canvas(1200,810,{9,23,39});mv::renderExplorer(canvas,state);
 static std::vector<unsigned char> pixels(1200*810*4);
 for(int y=0;y<810;++y)for(int x=0;x<1200;++x){auto c=canvas.get(x,y);std::size_t i=(y*1200+x)*4;pixels[i]=c.b;pixels[i+1]=c.g;pixels[i+2]=c.r;}
 BITMAPINFO info={};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=1200;info.bmiHeader.biHeight=-810;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
 FillRect(backBuffer.dc,&client,static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
 auto r=layout(hwnd);SetStretchBltMode(backBuffer.dc,COLORONCOLOR);
 StretchDIBits(backBuffer.dc,r.x,r.y,r.w,r.h,0,0,1200,810,pixels.data(),&info,DIB_RGB_COLORS,SRCCOPY);
 BitBlt(dc,0,0,w,h,backBuffer.dc,0,0,SRCCOPY);
 }
 }catch(const std::exception&){TextOutA(dc,20,20,"Unable to render MediView.",25);}
 EndPaint(hwnd,&ps);
}
LRESULT CALLBACK procedure(HWND hwnd,UINT message,WPARAM w,LPARAM l){
 switch(message){
 case WM_CREATE:last=std::chrono::steady_clock::now();if(!SetTimer(hwnd,1,33,nullptr))return -1;return 0;
 case WM_ERASEBKGND:return 1;
 case WM_PAINT:paint(hwnd);return 0;
 case WM_SIZE:InvalidateRect(hwnd,nullptr,FALSE);return 0;
 case WM_GETMINMAXINFO:{auto m=reinterpret_cast<MINMAXINFO*>(l);m->ptMinTrackSize={900,650};return 0;}
 case WM_TIMER:{bool wasPlaying=state.playing;advance();if((wasPlaying||state.playing)&&!IsIconic(hwnd))InvalidateRect(hwnd,nullptr,FALSE);return 0;}
 case WM_LBUTTONDOWN:{SetFocus(hwnd);advance();auto p=logical(hwnd,GET_X_LPARAM(l),GET_Y_LPARAM(l));
 if(mv::activate(state,p)){InvalidateRect(hwnd,nullptr,FALSE);return 0;}
 if(mv::inViewport(p)){down=previous=p;dragging=true;moved=false;SetCapture(hwnd);}return 0;}
 case WM_MOUSEMOVE:if(dragging){auto p=logical(hwnd,GET_X_LPARAM(l),GET_Y_LPARAM(l));
 if(!moved&&std::hypot(p.x-down.x,p.y-down.y)>4)moved=true;
 if(moved){state.panX+=p.x-previous.x;state.panY+=p.y-previous.y;InvalidateRect(hwnd,nullptr,FALSE);}previous=p;}return 0;
 case WM_LBUTTONUP:if(dragging){auto p=logical(hwnd,GET_X_LPARAM(l),GET_Y_LPARAM(l));dragging=false;ReleaseCapture();if(!moved)state.selectOrgan(mv::hitTest(state,p));InvalidateRect(hwnd,nullptr,FALSE);}return 0;
 case WM_CAPTURECHANGED:dragging=false;return 0;
 case WM_MOUSEWHEEL:{POINT p={GET_X_LPARAM(l),GET_Y_LPARAM(l)};ScreenToClient(hwnd,&p);auto q=logical(hwnd,p.x,p.y);if(mv::inViewport(q)){state.zoomAt(q,std::pow(1.15,GET_WHEEL_DELTA_WPARAM(w)/120.0));InvalidateRect(hwnd,nullptr,FALSE);}return 0;}
 case WM_KEYDOWN:{advance();bool repeat=(l&(1L<<30))!=0;
 if(state.quiz&&w>='1'&&w<='4'){if(!repeat)state.answerQuiz(int(w-'1'));}
 else switch(w){
 case VK_SPACE:if(!repeat)state.toggleAnimation();break;
 case VK_RETURN:if(!repeat)state.nextQuestion();break;
 case VK_TAB:state.cycleSelection(GetKeyState(VK_SHIFT)<0?-1:1);break;
 case VK_PRIOR:if(!state.quiz||state.answer>=0)state.turnPage(-1);break;
 case VK_NEXT:if(!state.quiz||state.answer>=0)state.turnPage(1);break;
 case 'Q':if(!repeat){if(state.quiz)state.exitQuiz();else state.startQuiz(std::random_device{}());}break;
 case 'C':if(!repeat&&!state.quiz){if(state.comparing)state.nextSecondary();else state.toggleCompare();}break;
 case 'X':if(!repeat)state.swapSystems();break;
 case 'F':if(!repeat)state.toggleVariant();break;
 case 'L':if(!repeat&&!state.quiz)state.labels=!state.labels;break;
 case 'R':state.resetView();break;
 case 'B':if(!state.quiz&&(state.system==2||state.system==3))state.resetAnimation();break;
 case VK_ESCAPE:if(state.quiz)state.exitQuiz();else state.selectOrgan(-1);break;
 case VK_ADD:case VK_OEM_PLUS:state.zoomAt({518,406},1.15);break;
 case VK_SUBTRACT:case VK_OEM_MINUS:state.zoomAt({518,406},1/1.15);break;
 default:if(w>='1'&&w<='8')state.selectSystem(int(w-'1'));else return DefWindowProcA(hwnd,message,w,l);}
 InvalidateRect(hwnd,nullptr,FALSE);return 0;}
 case WM_DESTROY:backBuffer.clear();KillTimer(hwnd,1);PostQuitMessage(0);return 0;
 default:return DefWindowProcA(hwnd,message,w,l);
 }
}
}
int WINAPI WinMain(HINSTANCE instance,HINSTANCE,LPSTR,int show){
 SetProcessDPIAware();
 WNDCLASSA wc={};wc.lpfnWndProc=procedure;wc.hInstance=instance;wc.lpszClassName="MediViewNative";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);
 if(!RegisterClassA(&wc)){MessageBoxA(nullptr,"Window registration failed.","MediView",MB_ICONERROR);return 1;}
 RECT work={};SystemParametersInfoA(SPI_GETWORKAREA,0,&work,0);
 double initialScale=std::min(1.0,std::min((work.right-work.left-40)/1200.0,(work.bottom-work.top-90)/810.0));
 initialScale=std::max(0.6,initialScale);
 RECT size={0,0,LONG(1200*initialScale),LONG(810*initialScale)};AdjustWindowRect(&size,WS_OVERLAPPEDWINDOW,FALSE);
 HWND window=CreateWindowExA(0,wc.lpszClassName,"MediView - Whole-Body Anatomy Explorer",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,size.right-size.left,size.bottom-size.top,nullptr,nullptr,instance,nullptr);
 if(!window){MessageBoxA(nullptr,"Window creation failed.","MediView",MB_ICONERROR);return 1;}
 ShowWindow(window,show);UpdateWindow(window);MSG msg;int result;
 while((result=GetMessageA(&msg,nullptr,0,0))>0){TranslateMessage(&msg);DispatchMessageA(&msg);}return result<0?1:int(msg.wParam);
}
