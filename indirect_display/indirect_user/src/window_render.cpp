////render to window
/// Fanxiushu 2022-05-17 ,
/// ��ʾ���չʾ��indirect_display����д�������ڴ��ͼ�����ݣ�������Ҫ���� capture_image ���ܣ������������������������ļ���
#include <Windows.h>
#include <stdio.h>
#include <conio.h>

///�������ݽṹ�������ӿ������ļ��л�ȡ
/////�ڴ湲��ʽ��ͼ
#define MONITOR_SHMEM_INFO   "FanxiushuIndirectDisplay_Info"
#define MONITOR_SHMEM_IMAGE  "FanxiushuIndirectDisplay_Image"

#define DIRTY_RECT_QUEUE_SIZE     50000

struct dirty_rect_queue_t
{
	unsigned int  next_index; /// ��һ����Ҫ���Ƶ�λ��, �� 0�� ( DIRTY_RECT_QUEUE_SIZE - 1 ),ѭ������ָʾ��
							  ///
	RECT          queue[DIRTY_RECT_QUEUE_SIZE]; ///
};

struct monitor_shminfo_t
{
	char                magic[16];
	int                 monitor_index;  ///
	unsigned int        cx;
	unsigned int        cy;
	////
	dirty_rect_queue_t  dirty_rects; /////
};


//�������ҵ��㷨���ٶȼ���������Բ������ͼ������
#include <math.h>
void DealWithImgData(BYTE *srcdata, BYTE *drcdata, int width, int height)//����һΪԭͼ�����������ָ�룬������Ϊ���Ϻ�ͼ�����������ָ�룬������Ϊͼ��Ŀ�������Ϊͼ��ĸ�
{
	int l_width = width * 4; // WIDTHBYTES(width * 24);//����λͼ��ʵ�ʿ�Ȳ�ȷ����Ϊ4byte�ı��� 
	double radius1 = (width+height) / 4;//��������뾶
	double radius2 = radius1*radius1;//��۵�ƽ��
	double x1, y1;//Ŀ����������ͼ�е�����λ��
	double x, y;//Ŀ����������ͼ�ж�Ӧԭͼ������λ��
	double middle2 = 2 * radius1 / 3.1416;//�������ʽ��
	double matan;//Ŀ����Բ��������x��ļн�
	int pixel_point;//����ͼ��ָ��
	int pixel_point_row;//����ͼ������ָ��
	double oa;//���Ӧ������

			  //˫���Բ�ֵ�㷨��ر���
	int i_original_img_hnum, i_original_img_wnum;//Ŀ�������
	double distance_to_a_y, distance_to_a_x;//��ԭͼ������a���ˮƽ����  
	int original_point_a, original_point_b, original_point_c, original_point_d;

	for (int hnum = 0; hnum < height; hnum++)
	{
		pixel_point_row = hnum*l_width;
		for (int wnum = 0; wnum < width; wnum++)
		{
			if ((hnum - height / 2)*(hnum - height / 2) + (wnum - width / 2)*(wnum - width / 2) < radius2)//�������ӳ��ڲŴ���
			{
				pixel_point = pixel_point_row + wnum * 4;//����λ��ƫ��������Ӧ��ͼ��ĸ����ص�RGB�����
														 /***********��������***********/
				x1 = wnum - width / 2;
				y1 = height / 2 - hnum;

				if (x1 != 0)
				{
					oa = middle2*asin(sqrt(y1*y1 + x1*x1) / radius1);//������ȷ��ͼ���С������¿����ò������ɣ���������������Ч��
					matan = atan2(y1, x1);
					x = cos(matan)*oa;
					y = sin(matan)*oa;
				}
				else
				{
					y = asin(y1 / radius1)*middle2;
					x = 0;
				}
				/***********��������***********/

				/***********˫���Բ�ֵ�㷨***********/
				i_original_img_hnum = (height / 2 - y);
				i_original_img_wnum = (x + width / 2);
				distance_to_a_y = (height / 2 - y) - i_original_img_hnum;
				distance_to_a_x = (x + width / 2) - i_original_img_wnum;//��ԭͼ������a��Ĵ�ֱ����  

				original_point_a = i_original_img_hnum*l_width + i_original_img_wnum * 4;//����λ��ƫ��������Ӧ��ͼ��ĸ����ص�RGB�����,�൱�ڵ�A    
				original_point_b = original_point_a + 4;//����λ��ƫ��������Ӧ��ͼ��ĸ����ص�RGB�����,�൱�ڵ�B  
				original_point_c = original_point_a + l_width;//����λ��ƫ��������Ӧ��ͼ��ĸ����ص�RGB�����,�൱�ڵ�C   
				original_point_d = original_point_c + 4;//����λ��ƫ��������Ӧ��ͼ��ĸ����ص�RGB�����,�൱�ڵ�D  

				if (hnum == height - 1)
				{
					original_point_c = original_point_a;
					original_point_d = original_point_b;
				}
				if (wnum == width - 1)
				{
					original_point_a = original_point_b;
					original_point_c = original_point_d;
				}

				drcdata[pixel_point + 0] =
					srcdata[original_point_a + 0] * (1 - distance_to_a_x)*(1 - distance_to_a_y) +
					srcdata[original_point_b + 0] * distance_to_a_x*(1 - distance_to_a_y) +
					srcdata[original_point_c + 0] * distance_to_a_y*(1 - distance_to_a_x) +
					srcdata[original_point_c + 0] * distance_to_a_y*distance_to_a_x;
				drcdata[pixel_point + 1] =
					srcdata[original_point_a + 1] * (1 - distance_to_a_x)*(1 - distance_to_a_y) +
					srcdata[original_point_b + 1] * distance_to_a_x*(1 - distance_to_a_y) +
					srcdata[original_point_c + 1] * distance_to_a_y*(1 - distance_to_a_x) +
					srcdata[original_point_c + 1] * distance_to_a_y*distance_to_a_x;
				drcdata[pixel_point + 2] =
					srcdata[original_point_a + 2] * (1 - distance_to_a_x)*(1 - distance_to_a_y) +
					srcdata[original_point_b + 2] * distance_to_a_x*(1 - distance_to_a_y) +
					srcdata[original_point_c + 2] * distance_to_a_y*(1 - distance_to_a_x) +
					srcdata[original_point_c + 2] * distance_to_a_y*distance_to_a_x;
				/***********˫���Բ�ֵ�㷨***********/
			}
		}
	}
}


struct render_window
{
	HBITMAP             hbmp;
	HDC                 memdc;
	////
	HANDLE              handle1;
	HANDLE              handle2;
	monitor_shminfo_t*  shinfo;
	byte*               image_data;

	HWND                hwnd;
};

static LRESULT CALLBACK _WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	render_window* render = (render_window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (render) {

	}
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		::SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
static DWORD CALLBACK update_thread(void* _p)
{
	render_window* render = (render_window*)_p;
	////
	unsigned int next_index = 0;
	/// test
	int w = render->shinfo->cx;
	int h = render->shinfo->cy;
//	byte* dst = (byte*)malloc(w*h * 4); 
	BITMAPINFOHEADER bi; memset(&bi, 0, sizeof(bi));
	bi.biSize = sizeof(bi);
	bi.biWidth = w;
	bi.biHeight = -h; // ���ϳ���ɨ��
	bi.biPlanes = 1;
	bi.biBitCount = 32; //BGRA
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	HDC hdc = GetDC(NULL); //��ĻDC
	HDC mdc = CreateCompatibleDC(hdc);
	PVOID data = NULL;
	HBITMAP bmp = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&data, NULL, 0);
	ReleaseDC(NULL, hdc);
	SelectObject(mdc, bmp); ///

	/////
	while (1) {
		///
		bool is_change = false;
		if (next_index != render->shinfo->dirty_rects.next_index) {
			is_change = true;
			next_index = render->shinfo->dirty_rects.next_index;
		}

		if (is_change) {
			///�ڴ˴����Զ�ͼ�����ݽ��ж��⴦��
			bool do_image_conv = false;
		//	DealWithImgData((BYTE*)render->image_data, (BYTE*)data, render->shinfo->cx, render->shinfo->cy);

			/////
			RECT rc; GetClientRect(render->hwnd, &rc);
			HDC hdc = GetDC(render->hwnd);
			SetStretchBltMode(hdc, COLORONCOLOR);

			StretchBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
				do_image_conv ? mdc : render->memdc, 
				0, 0, render->shinfo->cx, render->shinfo->cy, SRCCOPY);

			ReleaseDC(render->hwnd, hdc);
		}
		/////
		Sleep(16);
	}
	return 0;
}

render_window* create_render(int index)
{
	char name[256];
	sprintf(name, "Global\\%s_%d", MONITOR_SHMEM_INFO, index);
	HANDLE handle1 = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name);
	if (!handle1) {
		printf("** OpenFileMapping [%s] err=%d\n",name, GetLastError() );
		return NULL;
	}
	monitor_shminfo_t* info=(monitor_shminfo_t*)MapViewOfFile(handle1, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(monitor_shminfo_t));
	if (!info) {
		CloseHandle(handle1);
		return NULL;
	}
	sprintf(name, "Global\\%s_%d_%dX%d", MONITOR_SHMEM_IMAGE, index, info->cx, info->cy);
	HANDLE handle2 = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name);
	if (!handle2) {
		printf("** OpenFileMapping [%s] err=%d\n", name, GetLastError());
		UnmapViewOfFile(info);
		CloseHandle(handle1);
		return NULL;
	}
	
	int w = info->cx;
	int h = info->cy;
	////
	BITMAPINFOHEADER bi; memset(&bi, 0, sizeof(bi));
	bi.biSize = sizeof(bi);
	bi.biWidth = w;
	bi.biHeight = -h; // ���ϳ���ɨ��
	bi.biPlanes = 1;
	bi.biBitCount = 32; //BGRA
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	HDC hdc = GetDC(NULL); //��ĻDC
	HDC mdc = CreateCompatibleDC(hdc);
	PVOID data = NULL;
	HBITMAP bmp = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&data, handle2, 0);
	ReleaseDC(NULL, hdc);
	if (!bmp) {
		printf("CreateDIBSection err=%d\n", GetLastError());
		
		return NULL;
	}
	SelectObject(mdc, bmp); ///

	/////
	render_window* render = new render_window;

	render->handle1 = handle1; render->handle2 = handle2;
	render->hbmp = bmp;
	render->memdc = mdc;
	render->image_data = (byte*)data;
	render->shinfo = info;

	HMODULE hmod = GetModuleHandle(NULL);
	HWND hwnd;
	char* cls_name = "Fanxiushu.Test";
	WNDCLASS wndcls = { CS_HREDRAW | CS_VREDRAW, _WndProc, 0,0, hmod , NULL,
		LoadCursor(NULL,IDC_ARROW), NULL, NULL, cls_name };
	RegisterClass(&wndcls);
	hwnd = CreateWindow(cls_name, "", 
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	//	WS_POPUP, 0, 0, 2560, 1600, 
		NULL, NULL, hmod, NULL);
	if (!hwnd) {
		printf("*** Can not Create Message DxHook Window.\n");
		///
		return NULL;
	}
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)render);
	render->hwnd = hwnd;

	/////
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	DWORD tid;
	CreateThread(NULL, 0, update_thread, render, 0, &tid);

	return render;
}

#if 1

int main(int argc, char** argv)
{
	SetProcessDPIAware();
	printf("Please Input Indirect Display Index [Range: 0 - 15]: ");
	int index = 0; scanf("%d", &index);
	render_window* render = create_render(index);
	if (!render) {
		printf("[ESC] to exit.\n"); while (_getch() != 27);
		return -1;
	}
	////
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

#endif

