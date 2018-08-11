//#include <windows.h>
#include <WinSock2.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#include "Resource.h"
#pragma comment(lib,"WSock32.Lib")
#pragma comment(lib,"WinMM.Lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <vector>

#define MINESIZE 16
using namespace std;
vector <SOCKET> socklist;
SOCKET G_sockClient;
BYTE *mineDisp=0;
BYTE *mineState=0;
BOOL START=FALSE;
int minenum=0;
int OKnum=0;
UINT TIME,S_TIME;
//UINT MINE;
enum gamestate
{
	MainMenu,
	ClientWait,
	ClientGame,
	ServerGame,
	ServerWait
} G_state;
//MainMenu
HWND hMM_Host,hMM_Join;
HWND hList_Player;
HWND hSW_Start,hQuit,hBack;
int hei,wid,Tnum;
HWND hMine,hNUM;
HWND hWnd;
HBITMAP hBitmap;
HBITMAP hBitnum;
HBITMAP hFace;
HDC hdcMem,hdcMem2,hdcMem3;
WNDPROC OldFunc;
//CRITICAL_SECTION cs;
HBRUSH BKbrush;
PAINTSTRUCT ps;
BOOL G_GAMING;
INT faceID=0;
BOOL (*PostIPMsg)(
    _In_ UINT Msg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam);
struct IP_MSG
{
	UINT msg;
	WPARAM wParam;
	LPARAM lParam;
	IP_MSG(UINT m=0,WPARAM w=0,LPARAM l=0)
		:msg(m),wParam(w),lParam(l){}
};

BOOL PostLocal(UINT Msg,WPARAM wParam,LPARAM lParam)
{
	return PostMessage(hWnd,Msg,wParam,lParam);
}
BOOL PostNet( UINT Msg,WPARAM wParam, LPARAM lParam)
	//not finish
{
	IP_MSG msg(Msg,wParam,lParam);
	send(G_sockClient,(char*)&msg,sizeof(IP_MSG),0);
	return TRUE;
}
void recvProc_server(SOCKET sockClient)
{
	while(G_state==ServerGame||G_state==ServerWait)
{
//Sleep(1);
IP_MSG msgRcv;
int state=recv(sockClient,(char*)&msgRcv,sizeof(msgRcv),0);
if(SOCKET_ERROR==state||state==0)
{
	int i;
	for(i=0;socklist[i]!=sockClient;i++);
closesocket(sockClient);
socklist.erase(socklist.begin()+i);
PostMessage(hList_Player,LVM_DELETEITEM,i,0);
return;
}
PostMessage(hWnd,msgRcv.msg,msgRcv.wParam,msgRcv.lParam);
}
closesocket(sockClient);
}
void recvProc_client(SOCKET sockClient)
{
	while(G_state==ClientGame||G_state==ClientWait)
{
//Sleep(1);
IP_MSG msgRcv;
int state=recv(sockClient,(char*)&msgRcv,sizeof(msgRcv),0);
if(SOCKET_ERROR==state||state==0)
{
closesocket(sockClient);
return;
}
PostMessage(hWnd,msgRcv.msg,msgRcv.wParam,msgRcv.lParam);
}
closesocket(sockClient);
}

void IP_PostMessage(IP_MSG msg)
{
	for(int i=0;i<socklist.size();i++)
		send(socklist[i],(char*)&msg,sizeof(IP_MSG),0);
}

void ServerListening()
{
SOCKET sockSrv=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
sockaddr_in addrSrv;
memset(&addrSrv,0,sizeof(addrSrv));
addrSrv.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
addrSrv.sin_family=AF_INET;
addrSrv.sin_port=htons(5000);
if(bind(sockSrv,(sockaddr*)&addrSrv,sizeof(sockaddr))==SOCKET_ERROR)return;
if(listen(sockSrv,5)==SOCKET_ERROR)return;
SOCKADDR_IN addrClient;
int len=sizeof(sockaddr);
while(G_state==ServerWait)
{
SOCKET sockConnect=accept(sockSrv,(sockaddr*)&addrClient,&len);
CreateThread(NULL,0,
			 (LPTHREAD_START_ROUTINE)recvProc_server,(void*)sockConnect,
0,NULL);
LV_ITEM lvi;
   lvi.mask=LVIF_TEXT;
   lvi.iItem=socklist.size();
   lvi.iSubItem=0;
   lvi.pszText="Player";
   PostMessage(hList_Player,LVM_INSERTITEM,0,(LPARAM)&lvi);
socklist.push_back(sockConnect);
}
closesocket(sockSrv);
}
INT_PTR CALLBACK NewFunc(HWND wnd,
  UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	//return CallWindowProc (OldFunc,wnd,uMsg,wParam,lParam);
	switch(uMsg)
	{
	case WM_PAINT:
		hdc = BeginPaint (wnd, &ps) ;
		BitBlt (hdc, 0, 0, 32, 32,
			hdcMem3, 0, 32*faceID, SRCCOPY) ;
		EndPaint(wnd,&ps);
		return 0;
	default:
		return CallWindowProc (OldFunc,wnd,uMsg,wParam,lParam);
	}
}
INT_PTR CALLBACK DialogFunc(HWND hDlg,
  UINT uMsg,WPARAM wParam,LPARAM lParam)
 {
    switch(uMsg){
		
     case WM_COMMAND:
        switch(LOWORD( wParam )) {
          case IDOK:
            {
				{HWND hEdit=GetDlgItem(hDlg,IDC_EDIT1);
					long ret=(long)SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) + 1;
					LPSTR szNum=new char[ret];
					SendMessage(hEdit, WM_GETTEXT, ret, long(szNum));
					wid=atoi(szNum);
					delete []szNum;
					
					hEdit=GetDlgItem(hDlg,IDC_EDIT2);
					ret=(long)SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) + 1;
					szNum=new char[ret];
					SendMessage(hEdit, WM_GETTEXT, ret, long(szNum));
					hei=atoi(szNum);
					delete [] szNum;
					hEdit=GetDlgItem(hDlg,IDC_EDIT3);
					ret=(long)SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) + 1;
					szNum=new char[ret];
					SendMessage(hEdit, WM_GETTEXT, ret, long(szNum));
					Tnum=atoi(szNum);
					delete [] szNum;
				} 
			PostMessage(hWnd,WM_GAMESTART,wid,hei);
			IP_PostMessage(IP_MSG(WM_GAMESTART,wid,hei));
                EndDialog( hDlg, 0 );
            }
            return 0;
		  case IDC_BUTTON1:
			  wid=hei=8;Tnum=10;
			PostMessage(hWnd,WM_GAMESTART,wid,hei);
			IP_PostMessage(IP_MSG(WM_GAMESTART,wid,hei));
			  EndDialog( hDlg, 0 );
            return 0;
		  case IDC_BUTTON2:
			  wid=hei=16;Tnum=40;
			PostMessage(hWnd,WM_GAMESTART,wid,hei);
			IP_PostMessage(IP_MSG(WM_GAMESTART,wid,hei));
			  EndDialog( hDlg, 0 );
            return 0;
			  case IDC_BUTTON3:
				  wid=30;hei=16;Tnum=99;
			PostMessage(hWnd,WM_GAMESTART,wid,hei);
			IP_PostMessage(IP_MSG(WM_GAMESTART,wid,hei));
			EndDialog( hDlg, 0 );
            return 0;
			case IDCANCEL:
                EndDialog( hDlg, 0 );
            return 0;
        };
        break;
      case WM_CLOSE:
        EndDialog( hDlg, 0 );
        return 0;
    };
    return DefWindowProc(hDlg, uMsg, wParam, lParam);

}
INT_PTR CALLBACK DialogFunc2(
  HWND hDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam
)
 {
    switch(uMsg){
	case WM_CREATE:
		{HDC hdc = BeginPaint (hWnd, &ps) ;
			RECT rect;
		GetClientRect(hWnd,&rect);
		InvalidateRect(hWnd,&rect,TRUE); 
		FillRect(hdc,&rect,BKbrush);
			EndPaint(hWnd,&ps);}
		return 0;
     case WM_COMMAND:
        switch(LOWORD( wParam )) {
          case IDOK:
            {
				HWND hEdit=GetDlgItem(hDlg,IDC_EDIT1);
					long ret=(long)SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) + 1;
					LPSTR szNum=new char[ret];
					SendMessage(hEdit, WM_GETTEXT, ret, long(szNum));
					wid=atoi(szNum);
					
					SOCKADDR_IN addrSrv;
addrSrv.sin_addr.S_un.S_addr=inet_addr(szNum);
delete []szNum;
addrSrv.sin_family=AF_INET;
addrSrv.sin_port=htons(5000);
G_sockClient=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
if (connect(G_sockClient,(sockaddr*)&addrSrv,sizeof(sockaddr)) == SOCKET_ERROR)
{
	closesocket(G_sockClient);
				//::MessageBox(hDlg,"connect to sever fail","Error",MB_OK);
                EndDialog( hDlg, 0 );
return 0;
}
			G_state=ClientWait;
CreateThread(NULL,0,
			 (LPTHREAD_START_ROUTINE)recvProc_client,(void*)G_sockClient,
0,NULL);
			ShowWindow(hMM_Host,SW_HIDE);
			ShowWindow(hMM_Join,SW_HIDE);
			ShowWindow(hList_Player,SW_SHOW);
			ShowWindow(hQuit,SW_SHOW);
HDC hdc = BeginPaint (hWnd, &ps) ;
			RECT rect;
		GetClientRect(hWnd,&rect);
		InvalidateRect(hWnd,&rect,TRUE); 
		FillRect(hdc,&rect,BKbrush);
			EndPaint(hWnd,&ps);
            }
			EndDialog( hDlg, 0 );
            return 0;
		  
			case IDCANCEL:
                EndDialog( hDlg, 0 );
            return 0;
        };
        break;
      case WM_CLOSE:
        EndDialog( hDlg, 0 );
        return 0;
    };
    return DefWindowProc(hDlg, uMsg, wParam, lParam);

}

LRESULT CALLBACK NumProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	
HDC hdc;
PAINTSTRUCT ps ;
switch(msg)
{
case WM_PAINT:
	hdc = BeginPaint (wnd, &ps) ;
	BitBlt (hdc, 0, 0, 16, 32,
			hdcMem2, 16*((TIME/100)%10), 0, SRCCOPY) ;
	BitBlt (hdc, 16, 0, 16, 32,
			hdcMem2, 16*((TIME/10)%10), 0, SRCCOPY) ;
	BitBlt (hdc, 32, 0, 16, 32,
			hdcMem2, 16*((TIME)%10), 0, SRCCOPY) ;
	if(minenum<0)
	{
		BitBlt (hdc, 80, 0, 16, 32,
			hdcMem2, 160, 0, SRCCOPY) ;
	BitBlt (hdc, 96, 0, 16, 32,
			hdcMem2, 16*((-minenum/10)%10), 0, SRCCOPY) ;
	BitBlt (hdc, 112, 0, 16, 32,
			hdcMem2, 16*((-minenum)%10), 0, SRCCOPY) ;
	}
	else
	{BitBlt (hdc, 80, 0, 16, 32,
			hdcMem2, 16*((minenum/100)%10), 0, SRCCOPY) ;
	BitBlt (hdc, 96, 0, 16, 32,
			hdcMem2, 16*((minenum/10)%10), 0, SRCCOPY) ;
	BitBlt (hdc, 112, 0, 16, 32,
			hdcMem2, 16*((minenum)%10), 0, SRCCOPY) ;
	}
	EndPaint (wnd, &ps) ;
	return 0;
default:
	return DefWindowProc(wnd,msg,wParam,lParam);
}
}

LRESULT CALLBACK MineProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
HDC hdc;
PAINTSTRUCT ps ;
static BOOL MOVED=TRUE;
static BOOL LRCLICK=FALSE;
static BOOL LCLICK=FALSE;
static int x,y;
if(G_GAMING)
{
	switch(msg)
	{
	case WM_PAINT:
		hdc = BeginPaint (wnd, &ps) ;
for(int i=0;i<hei;i++)
	for(int j=0;j<wid;j++)
	{
		BitBlt (hdc, j*MINESIZE, i*MINESIZE, MINESIZE, MINESIZE,
			hdcMem, mineDisp[i*wid+j]*MINESIZE, 0, SRCCOPY) ;
	}
if(MOVED)
	{if(LRCLICK)
{
	int pos=y*wid+x;
	if(x>0)
	{
		if(mineDisp[pos-1]==11)BitBlt (hdc, (x-1)*MINESIZE, y*MINESIZE, MINESIZE, MINESIZE,
			hdcMem, 14*MINESIZE, 0, SRCCOPY) ;
		if(y>0)if(mineDisp[pos-1-wid]==11)BitBlt (hdc, (x-1)*MINESIZE, (y-1)*MINESIZE, MINESIZE, MINESIZE,
			hdcMem, 14*MINESIZE, 0, SRCCOPY) ;
			if(y<hei-1)if(mineDisp[pos-1+wid]==11)BitBlt (hdc, (x-1)*MINESIZE, (y+1)*MINESIZE, MINESIZE, MINESIZE,
			hdcMem, 14*MINESIZE, 0, SRCCOPY) ;
	}
	if(y>0)if(mineDisp[pos-wid]==11)BitBlt (hdc, (x)*MINESIZE, (y-1)*MINESIZE, MINESIZE, MINESIZE,
			hdcMem, 14*MINESIZE, 0, SRCCOPY) ;
			if(y<hei-1)if(mineDisp[pos+wid]==11)BitBlt (hdc, (x)*MINESIZE, (y+1)*MINESIZE, MINESIZE, MINESIZE,
			hdcMem, 14*MINESIZE, 0, SRCCOPY) ;
	if(x<wid-1)
	
	{
		if(mineDisp[pos+1]==11)BitBlt (hdc, (x+1)*MINESIZE, y*MINESIZE, MINESIZE, MINESIZE,
			hdcMem, 14*MINESIZE, 0, SRCCOPY) ;
		if(y>0)if(mineDisp[pos+1-wid]==11)BitBlt (hdc, (x+1)*MINESIZE, (y-1)*MINESIZE, MINESIZE, MINESIZE,
			hdcMem, 14*MINESIZE, 0, SRCCOPY) ;
			if(y<hei-1)if(mineDisp[pos+1+wid]==11)BitBlt (hdc, (x+1)*MINESIZE, (y+1)*MINESIZE, MINESIZE, MINESIZE,
			hdcMem, 14*MINESIZE, 0, SRCCOPY) ;
	}
}
	else
	if(LCLICK)
	{
		if(mineDisp[y*wid+x]==11)BitBlt (hdc, x*MINESIZE, y*MINESIZE, MINESIZE, MINESIZE,
			hdcMem, 14*MINESIZE, 0, SRCCOPY) ;
	}}
EndPaint (wnd, &ps) ;
		return 0;
		case WM_LBUTTONDOWN:
		if(MOVED)
		{
			x = LOWORD(lParam)/MINESIZE;
			y = HIWORD(lParam)/MINESIZE;
			LCLICK=TRUE;
			if(MK_RBUTTON & wParam)		//左右键盘同时按下
			{
				LRCLICK=TRUE;
				return 0;
			}		
			
		}
		return 0;
	case WM_RBUTTONDOWN:
		if(MOVED)
		{
			x = LOWORD(lParam)/MINESIZE;
			y = HIWORD(lParam)/MINESIZE;
			if(MK_LBUTTON & wParam)		//左右键盘同时按下
			{
				LRCLICK=TRUE;
				return 0;
			}	
		}
		return 0;
	case WM_LBUTTONUP:
		if(MOVED)
		{
			if(LRCLICK)
			{PostIPMsg(MINE_RLCLICK,x,y);
			LRCLICK=FALSE;}
			else
				PostIPMsg(MINE_LCLICK,x,y);
			LCLICK=FALSE;
			MOVED=FALSE;
		}
		if(!(MK_RBUTTON & wParam))MOVED=TRUE;
		return 0;
	case WM_MOUSEMOVE:
		if(MK_LBUTTON & wParam||MK_RBUTTON & wParam)
		if(x != LOWORD(lParam)/MINESIZE||y != HIWORD(lParam)/MINESIZE)
			{MOVED=FALSE;
		LCLICK=FALSE;}
		return 0;
	case WM_RBUTTONUP:
		if(MOVED)
		{
			if(LRCLICK)
			{PostIPMsg(MINE_RLCLICK,x,y);
			LRCLICK=FALSE;}
			else
				PostIPMsg(MINE_RCLICK,x,y);
			MOVED=FALSE;
			LCLICK=FALSE;
		}
		if(!(MK_LBUTTON & wParam))MOVED=TRUE;
		return 0;
	default:
return DefWindowProc(wnd, msg, wParam, lParam);
	}}
else
{switch(msg)
	{
	case WM_PAINT:
		hdc = BeginPaint (wnd, &ps) ;
for(int i=0;i<hei;i++)
	for(int j=0;j<wid;j++)
	{
		BitBlt (hdc, j*MINESIZE, i*MINESIZE, MINESIZE, MINESIZE,
			hdcMem, mineDisp[i*wid+j]*MINESIZE, 0, SRCCOPY) ;
	}
EndPaint (wnd, &ps) ;
		return 0;
	default:
return DefWindowProc(wnd, msg, wParam, lParam);
	}}
}

LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int pos;
	switch(G_state)
	{
	case MainMenu:
    switch (msg) {
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_CREATE:
			ShowWindow(hMM_Host,SW_HIDE);
			ShowWindow(hMM_Join,SW_HIDE);
			ShowWindow(hList_Player,SW_SHOW);
			ShowWindow(hQuit,SW_SHOW);
			ShowWindow(hSW_Start,SW_SHOW);
			G_state=ServerWait;
			CreateThread(NULL,0,
			(LPTHREAD_START_ROUTINE)ServerListening,NULL,
			0,NULL);
			return 0;
		case ID_JOIN:
			DialogBox(0,MAKEINTRESOURCE(   IDD_DIALOG2), wnd,&DialogFunc2);
			return 0;
		}
    case WM_DESTROY:
        PostQuitMessage(0); 
        return 0;              
    default:
        return DefWindowProc(wnd, msg, wParam, lParam);
    }
    return 0;
	case ClientGame: 
	switch (msg) {
		case MM_WIN:
		faceID=1;
		G_GAMING=FALSE;
		return 0;
		case MM_SETTIME:
			TIME=wParam;
			return 0;
		case MM_MINENUM:
			minenum=lParam;
			return 0;
		case WM_GAMEBACK:
			G_state=ClientWait;
			ShowWindow(hNUM,SW_HIDE);
			ShowWindow(hList_Player,SW_SHOW);
			ShowWindow(hQuit,SW_SHOW);
			ShowWindow(hBack,SW_HIDE);
			ShowWindow(hMine,SW_HIDE);
			ShowWindow(hBack,SW_HIDE);
			SetWindowPos(hWnd,HWND_TOP,0,0,200,250,SWP_NOMOVE|SWP_NOZORDER);
			{HDC hdc = BeginPaint (wnd, &ps) ;
			RECT rect;
		GetClientRect(hWnd,&rect);
		InvalidateRect(hWnd,&rect,TRUE); 
		FillRect(hdc,&rect,BKbrush);
		EndPaint(wnd,&ps);}
			return 0;
	case MM_GAMEOVER:
		faceID=2;
		G_GAMING=FALSE;
		case MM_SET:  

		mineDisp[wParam]=lParam;
		PostMessage(hWnd,WM_PAINT,0,0);
		//ExcludeClipRect
		//UpdateWindow(hWnd);
		return 0;
	case WM_TIMER:
		switch(wParam)
		{case TIMER:
		RECT rect;
		GetClientRect(hWnd,&rect);
		InvalidateRect(hWnd,&rect,TRUE); 
		}
		return 0;
    case WM_DESTROY:
        PostQuitMessage(0); 
        return 0;              
    default:
        return DefWindowProc(wnd, msg, wParam, lParam);
    }
	case ClientWait: 
	switch (msg) {
		case WM_GAMESTART:
			{G_state=ClientGame;
			G_GAMING=TRUE;
			TIME=0;
			faceID=4;
			hei=lParam;
			wid=wParam;
			ShowWindow(hList_Player,SW_HIDE);
			ShowWindow(hQuit,SW_HIDE);
			ShowWindow(hNUM,SW_SHOW);
			ShowWindow(hBack,SW_SHOW);
			SetWindowPos(hWnd,HWND_TOP,0,0,wid* MINESIZE+16,hei* MINESIZE+100,SWP_NOMOVE|SWP_NOZORDER);
			
			int Tsize=wid*hei;
			if(mineDisp)delete []mineDisp;
			mineDisp=new BYTE[Tsize];
			memset(mineDisp,11,Tsize);
			MoveWindow(hMine,0,50,wid* MINESIZE,hei* MINESIZE,TRUE);
			ShowWindow(hMine,SW_SHOW);
			HDC hdc = BeginPaint (wnd, &ps) ;
			RECT rect;
		GetClientRect(hWnd,&rect);
		InvalidateRect(hWnd,&rect,TRUE); 
		FillRect(hdc,&rect,BKbrush);
			EndPaint(wnd,&ps);
			PostIPMsg=PostNet;
			return 0;}
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_QUIT:
			G_state=MainMenu;
			
			PostMessage(hList_Player,LVM_DELETEALLITEMS,0,0);
			ShowWindow(hMM_Host,SW_SHOW);
			ShowWindow(hMM_Join,SW_SHOW);
			ShowWindow(hList_Player,SW_HIDE);
			ShowWindow(hQuit,SW_HIDE);
			HDC hdc = BeginPaint (wnd, &ps) ;
			RECT rect;
		GetClientRect(hWnd,&rect);
		InvalidateRect(hWnd,&rect,TRUE); 
		FillRect(hdc,&rect,BKbrush);
			EndPaint(wnd,&ps);
			return 0;
		
		}
    case WM_DESTROY:
        PostQuitMessage(0); 
        return 0;              
    default:
        return DefWindowProc(wnd, msg, wParam, lParam);
    }
	case ServerWait: 
	switch (msg) {
	case WM_GAMESTART:
		{
			faceID=4;
			TIME=0;
			S_TIME=0;
			OKnum=0;
			G_GAMING=TRUE;
			G_state=ServerGame;
			START=FALSE;
			hei=lParam;
			wid=wParam;
			minenum=Tnum;
			IP_PostMessage(IP_MSG(ID_MINENUM,0,minenum));
			ShowWindow(hList_Player,SW_HIDE);
			ShowWindow(hQuit,SW_HIDE);
			ShowWindow(hSW_Start,SW_HIDE);
			ShowWindow(hNUM,SW_SHOW);
			ShowWindow(hBack,SW_SHOW);
			SetWindowPos(hWnd,HWND_TOP,0,0,wid* MINESIZE+16,hei* MINESIZE+100,SWP_NOMOVE|SWP_NOZORDER);
			int Tsize=wid*hei,Tpos,Tx,Ty;
			
			if(mineState)delete []mineState;
			mineState=new BYTE[Tsize];
			memset(mineState,0,Tsize);
			
			if(mineDisp)delete []mineDisp;
			mineDisp=new BYTE[Tsize];
			memset(mineDisp,11,Tsize);
			MoveWindow(hMine,0,50,wid* MINESIZE,hei* MINESIZE,TRUE);
			ShowWindow(hMine,SW_SHOW);
			//UpdateWindow(hMine);
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint (wnd, &ps) ;
			//SetBkColor(hdc,COLOR_BTNFACE+1);
			RECT rect;
		GetClientRect(hWnd,&rect);
		InvalidateRect(hWnd,&rect,TRUE); 
		FillRect(hdc,&rect,BKbrush);
			EndPaint(wnd,&ps);
			PostIPMsg=PostLocal;
		}
	
		return 0;
		case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_QUIT:
			G_state=MainMenu;
			PostMessage(hList_Player,LVM_DELETEALLITEMS,0,0);
			ShowWindow(hMM_Host,SW_SHOW);
			ShowWindow(hMM_Join,SW_SHOW);
			ShowWindow(hList_Player,SW_HIDE);
			ShowWindow(hQuit,SW_HIDE);
			ShowWindow(hSW_Start,SW_HIDE);
			{HDC hdc = BeginPaint (wnd, &ps) ;
			RECT rect;
		GetClientRect(hWnd,&rect);
		InvalidateRect(hWnd,&rect,TRUE); 
		FillRect(hdc,&rect,BKbrush);
		EndPaint(wnd,&ps);}
			return 0;
		case ID_START:
			
			mineState=new BYTE[wid*hei];
			memset(mineState,0,wid*hei);
			DialogBox(0,MAKEINTRESOURCE(   IDD_DIALOG1), wnd,&DialogFunc);
			return 0;
		}
    case WM_DESTROY:
        PostQuitMessage(0); 
        return 0;              
    default:
        return DefWindowProc(wnd, msg, wParam, lParam);
    }
	case ServerGame:
	switch (msg) {
	case MM_GAMEOVER:
		faceID=2;
		G_GAMING=FALSE;
		for(int i=0;i<wid*hei;i++)
			if(!(mineState[i]&0x40))
		{
			if((mineState[i]&0x80)&&(!(mineState[i]&0x20)))
			{PostMessage(hWnd,MM_SET,i,10);
			IP_PostMessage(IP_MSG(MM_SET,i,10));}
			else if((!(mineState[i]&0x80))&&(mineState[i]&0x20))
				{PostMessage(hWnd,MM_SET,i,13);
			IP_PostMessage(IP_MSG(MM_SET,i,13));}/*
			else
			{
				PostMessage(hWnd,MM_SET,i,mineState[i]&0xf);
			IP_PostMessage(IP_MSG(MM_SET,i,mineState[i]&0xf));
			}*/
		}
		return 0;
	case MINE_LCLICK:
		if(!START)
		{START=TRUE;
			int Tpos,Cpos=wParam+lParam*wid,Tsize=hei*wid,Tx,Ty;
			for(int i=0;i<Tnum;i++)
			{
				do
				{Tpos=rand()%Tsize;}
				while((mineState[Tpos]&0x80)||Tpos==Cpos);
				mineState[Tpos]|=0x80;
				Tx=Tpos%wid;
				Ty=Tpos/wid;
				if(Tx>0)
				{
					mineState[Tpos-1]++;
					if(Ty>0)mineState[Tpos-1-wid]++;
					if(Ty<hei-1)mineState[Tpos-1+wid]++;
				}
				if(Ty>0)mineState[Tpos-wid]++;
				if(Ty<hei-1)mineState[Tpos+wid]++;
				if(Tx<wid-1)
				{
					mineState[Tpos+1]++;
					if(Ty>0)mineState[Tpos+1-wid]++;
					if(Ty<hei-1)mineState[Tpos+1+wid]++;
				}
			}
			
		}
		
		if(wParam<wid&&lParam>-1&&lParam<hei)
		{
		pos=wParam+lParam*wid;
		if(!(mineState[pos]&0x60))
		{
			mineState[pos]|=0x40;
			if(mineState[pos]&0x80)
			{PostMessage(hWnd,MM_SET,pos,9);
			IP_PostMessage(IP_MSG(MM_SET,pos,9));
			PostMessage(hWnd,MM_GAMEOVER,0,0);
			IP_PostMessage(IP_MSG(MM_GAMEOVER,0,0));
			}
			else
			{
				OKnum++;
				PostMessage(hWnd,MM_SET,pos,mineState[pos]&0xf);
			IP_PostMessage(IP_MSG(MM_SET,pos,mineState[pos]&0xf));
				if((mineState[pos]&0xf)==0)
				{
					PostMessage(hWnd,MINE_LCLICK,wParam-1,lParam-1);
					PostMessage(hWnd,MINE_LCLICK,wParam+1,lParam-1);
					PostMessage(hWnd,MINE_LCLICK,wParam,lParam-1);
					PostMessage(hWnd,MINE_LCLICK,wParam-1,lParam);
					PostMessage(hWnd,MINE_LCLICK,wParam+1,lParam);
					PostMessage(hWnd,MINE_LCLICK,wParam-1,lParam+1);
					PostMessage(hWnd,MINE_LCLICK,wParam+1,lParam+1);
					PostMessage(hWnd,MINE_LCLICK,wParam,lParam+1);
				}
			}
		}
		}
		if(OKnum+Tnum==wid*hei)
			{faceID=1;
		IP_PostMessage(IP_MSG(MM_WIN,0,0));
		G_GAMING=FALSE;
		}
		return 0;
	case MINE_RCLICK:
		if(!START)
		{START=TRUE;
			int Tpos,Tsize=hei*wid,Tx,Ty;
			for(int i=0;i<Tnum;i++)
			{
				do
				{Tpos=rand()%Tsize;}
				while(mineState[Tpos]&0x80);
				mineState[Tpos]|=0x80;
				Tx=Tpos%wid;
				Ty=Tpos/wid;
				if(Tx>0)
				{
					mineState[Tpos-1]++;
					if(Ty>0)mineState[Tpos-1-wid]++;
					if(Ty<hei-1)mineState[Tpos-1+wid]++;
				}
				if(Ty>0)mineState[Tpos-wid]++;
				if(Ty<hei-1)mineState[Tpos+wid]++;
				if(Tx<wid-1)
				{
					mineState[Tpos+1]++;
					if(Ty>0)mineState[Tpos+1-wid]++;
					if(Ty<hei-1)mineState[Tpos+1+wid]++;
				}
			}
			
		}
		pos=wParam+lParam*wid;
		if(!(mineState[pos]&0x40))
		{
			if(mineState[pos]&0x20)
			{
				minenum++;
				PostMessage(hWnd,MM_SET,pos,11);
				IP_PostMessage(IP_MSG(MM_SET,pos,11));
			}
			else
			{
				minenum--;
				PostMessage(hWnd,MM_SET,pos,12);
				IP_PostMessage(IP_MSG(MM_SET,pos,12));
			}
			IP_PostMessage(IP_MSG(MM_MINENUM,0,minenum));
			mineState[pos]^=0x20;
		}

		return 0;
	case MINE_RLCLICK:
		if(!START)
		{START=TRUE;
			int Tpos,Tsize=hei*wid,Tx,Ty;
			for(int i=0;i<Tnum;i++)
			{
				do
				{Tpos=rand()%Tsize;}
				while(mineState[Tpos]&0x80);
				mineState[Tpos]|=0x80;
				Tx=Tpos%wid;
				Ty=Tpos/wid;
				if(Tx>0)
				{
					mineState[Tpos-1]++;
					if(Ty>0)mineState[Tpos-1-wid]++;
					if(Ty<hei-1)mineState[Tpos-1+wid]++;
				}
				if(Ty>0)mineState[Tpos-wid]++;
				if(Ty<hei-1)mineState[Tpos+wid]++;
				if(Tx<wid-1)
				{
					mineState[Tpos+1]++;
					if(Ty>0)mineState[Tpos+1-wid]++;
					if(Ty<hei-1)mineState[Tpos+1+wid]++;
				}
			}
			
		}
		pos=wParam+lParam*wid;
		if(mineState[pos]&0x40)
		{
			int num=0;
			if(wParam>0)
				{
					if(mineState[pos-1]&0x20)num++;
					if(lParam>0)if(mineState[pos-1-wid]&0x20)num++;
					if(lParam<hei-1)if(mineState[pos-1+wid]&0x20)num++;
				}
				if(lParam>0)if(mineState[pos-wid]&0x20)num++;
				if(lParam<hei-1)if(mineState[pos+wid]&0x20)num++;
				if(wParam<wid-1)
				{
					if(mineState[pos+1]&0x20)num++;
					if(lParam>0)if(mineState[pos+1-wid]&0x20)num++;
					if(lParam<hei-1)if(mineState[pos+1+wid]&0x20)num++;
				}
			if(num==(mineState[pos]&0xf))
			{PostMessage(hWnd,MINE_LCLICK,wParam-1,lParam-1);
					PostMessage(hWnd,MINE_LCLICK,wParam+1,lParam-1);
					PostMessage(hWnd,MINE_LCLICK,wParam,lParam-1);
					PostMessage(hWnd,MINE_LCLICK,wParam-1,lParam);
					PostMessage(hWnd,MINE_LCLICK,wParam+1,lParam);
					PostMessage(hWnd,MINE_LCLICK,wParam-1,lParam+1);
					PostMessage(hWnd,MINE_LCLICK,wParam+1,lParam+1);
					PostMessage(hWnd,MINE_LCLICK,wParam,lParam+1);
				}
		}
		return 0;
	case MM_SET:  

		mineDisp[wParam]=lParam;
		PostMessage(hWnd,WM_PAINT,0,0);
		return 0;
	case WM_TIMER:
		switch(wParam)
		{case TIMER:
		RECT rect;
		GetClientRect(hWnd,&rect);
		InvalidateRect(hWnd,&rect,TRUE); 
		if(START&&G_GAMING)S_TIME++;
		if(S_TIME%40==0)
		{
			TIME=S_TIME/40;
			IP_PostMessage(IP_MSG(MM_SETTIME,TIME,0));
		}
		}
		return 0;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_BACK:
			G_state=ServerWait;
			CreateThread(NULL,0,
			(LPTHREAD_START_ROUTINE)ServerListening,NULL,
			0,NULL);
			
			ShowWindow(hNUM,SW_HIDE);
			ShowWindow(hList_Player,SW_SHOW);
			ShowWindow(hQuit,SW_SHOW);
			ShowWindow(hSW_Start,SW_SHOW);
			ShowWindow(hMine,SW_HIDE);
			ShowWindow(hBack,SW_HIDE);
			SetWindowPos(hWnd,HWND_TOP,0,0,200,250,SWP_NOMOVE|SWP_NOZORDER);
			//MoveWindow(hWnd,50, 50,wid* MINESIZE+16,hei* MINESIZE+70,TRUE);
			HDC hdc = BeginPaint (wnd, &ps) ;
			RECT rect;
		GetClientRect(hWnd,&rect);
		InvalidateRect(hWnd,&rect,TRUE); 
		FillRect(hdc,&rect,BKbrush);
			EndPaint(wnd,&ps);
			IP_PostMessage(IP_MSG(WM_GAMEBACK,0,0));
			return 0;
		}
		return 0;
    case WM_DESTROY:
        PostQuitMessage(0); 
        return 0;   
    default:
        return DefWindowProc(wnd, msg, wParam, lParam);
    }
	default:
        return DefWindowProc(wnd, msg, wParam, lParam);
	}

}

int WINAPI WinMain(  
   HINSTANCE hInstance,         
    HINSTANCE hPrevInstance,   
    char * lpCmdLine,          
    int nCmdShow                
)
{
    WNDCLASSEX  wce = {0};
    wce.cbSize          = sizeof(wce);
    wce.style           = CS_VREDRAW | CS_HREDRAW| CS_DBLCLKS;
    wce.lpfnWndProc     = &WndProc;  //指明回调函数
    wce.hInstance       = GetModuleHandle(0);
    wce.hIcon           = LoadIcon(hInstance, MAKEINTRESOURCE(ID_ICON));
    wce.hCursor         = LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW));
    //wce.hbrBackground   = reinterpret_cast<HBRUSH>(COLOR_BTNFACE+1);
	BKbrush=reinterpret_cast<HBRUSH>(COLOR_BTNFACE+1);
	 wce.hbrBackground=NULL;
    wce.lpszClassName   = TEXT("minesweep"); //独一无二的类名
    //wce.hIconSm         = wce.hIcon;
    RegisterClassEx(&wce);
	wce.hIcon=NULL;
	hWnd = CreateWindowEx(0, "minesweep", "MineSweep", WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 200, 250,
        0, 0, GetModuleHandle(0), 0);
	hMM_Host=CreateWindow("Button", "Create",  
WS_VISIBLE | WS_CHILD ,  
45, 20, 100, 30, hWnd,(HMENU)ID_CREATE, hInstance, 0);
	hMM_Join=CreateWindow("Button", "Join",  
WS_VISIBLE | WS_CHILD ,  
45, 120, 100, 30, hWnd,(HMENU)ID_JOIN, hInstance, 0);
	hList_Player=CreateWindow(WC_LISTVIEW,NULL,WS_CHILD|WS_BORDER|LVS_REPORT,0,0,100,200,hWnd,(HMENU)ID_LIST, hInstance,NULL);
	LVCOLUMN m_Lc;
 // LVITEM m_Lt; 
  m_Lc.mask = LVCF_FMT| LVCF_TEXT|LVCF_WIDTH|LVCF_ORDER;
m_Lc.fmt = LVCFMT_LEFT ;
m_Lc.cx = 100;
m_Lc.cchTextMax = 30;
//
m_Lc.pszText = TEXT("Player");
//ListView_InsertColumn(hList_Player, 0, &m_Lc);

	//ShowWindow(hList_Player,SW_HIDE);
hSW_Start=CreateWindow("Button", "Start", WS_CHILD ,  
		100, 20, 80, 30, hWnd,(HMENU)ID_START, hInstance, 0);
	hQuit=CreateWindow("Button", "Quit", WS_CHILD ,  
100, 120, 80, 30, hWnd,(HMENU)ID_QUIT, hInstance, 0);
	//InitializeCriticalSection(&cs);
	wce.lpfnWndProc=MineProc;
	wce.lpszClassName=TEXT("Mine");
	RegisterClassEx(&wce);
	hMine = CreateWindow("Mine", "", WS_CHILD,
        00, 120, 80, 30, hWnd,(HMENU)ID_MINE, hInstance, 0);
	
	wce.lpfnWndProc=NumProc;
	wce.lpszClassName=TEXT("Num");
	RegisterClassEx(&wce);
	
	hBack=CreateWindow("Button", "", WS_CHILD ,  
48, 0, 32, 32, hWnd,(HMENU)ID_BACK, hInstance, 0);
	OldFunc=(WNDPROC)GetWindowLong(hBack,GWL_WNDPROC);
	SetWindowLong(hBack,GWL_WNDPROC,(LONG)NewFunc);
	hNUM = CreateWindow("Num", "", WS_CHILD,
        0, 0, 128, 32, hWnd,(HMENU)ID_MINENUM, hInstance, 0);
	
        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);
		G_state=MainMenu;
	MSG msg;
	
	WORD wVersionRequested;
WSADATA wsaData;
wVersionRequested = MAKEWORD( 1, 1 );
WSAStartup( wVersionRequested, &wsaData );
hBitmap = LoadBitmap (hInstance, MAKEINTRESOURCE(IDB_MM)) ;
hBitnum = LoadBitmap (hInstance, MAKEINTRESOURCE(IDB_NUM)) ;
hFace = LoadBitmap (hInstance, MAKEINTRESOURCE(IDB_FACE)) ;
		hdcMem= CreateCompatibleDC (NULL) ;
		hdcMem2= CreateCompatibleDC (NULL) ;
		hdcMem3= CreateCompatibleDC (NULL) ;
SelectObject (hdcMem, hBitmap) ;
SelectObject (hdcMem3, hFace) ;
SelectObject (hdcMem2, hBitnum) ;

SetTimer(hWnd, TIMER, 25, NULL);
srand(timeGetTime());
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
	//DeleteCriticalSection(&cs);
KillTimer(hWnd, TIMER);
DeleteDC (hdcMem);

			if(mineState)delete []mineState;
			if(mineDisp)delete []mineDisp;
WSACleanup();
	return msg.wParam;
}
