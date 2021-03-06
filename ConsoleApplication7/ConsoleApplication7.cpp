/*
filter.c
*/
#include "stdafx.h"
#include<stdio.h>//标准输入输出头文件
#include<stdlib.h>//标准库头文件，包含size_t等在内的宏定义和通用函数
#include <string.h>
#include "windows.h"
#include "conio.h"//控制台输入输出
//右击工程项目—配置属性—C/C++—代码生成—基本运行时检查，
//把他改为默认值，再把下面的运行库改为多线程 (/MT)，再Release一下，
//发现现在的exe文件大小变大了，原因就是改为静态编译，
typedef unsigned char BYTE;
//颜色表定义
//typedef struct tagRGBQUAD {
//	BYTE rgbBlue;// 蓝色的亮度（值范围为0-255)
//	BYTE rgbGreen; // 绿色的亮度（值范围为0-255)
//	BYTE rgbRed; // 红色的亮度（值范围为0-255)
//	BYTE rgbReserved;// 保留，必须为0
//} RGBQUAD;//windows 库函数与上面定义二选一！
/*
定义头文件型
*/
#pragma pack(1)//将字节对齐方式设置为1字节对齐
/*备注：
加31的作用是：如果原长度是4字节的整数倍，增加的31就会被后续的整数除法舍
去，不会改变长度值；如果不是4字节的整数倍，后续的整数除法中，增加的31就
会保证把长度增加到4字节的整数倍；
除以32（注意这里是整数除法）是为了求得补足4字节整数倍后，数据中有多少组4字节；
最后乘以4，就得出每行占用的字节数目了。*/
typedef struct
{
	unsigned char id1;//位图文件的类型，必须为BM(占用0-1字节) 
	unsigned char id2;
	unsigned int filesize;//位图文件的大小，以字节为单位（2-5字节）
	unsigned int reserved;// 位图文件保留字，必须为0(6-9字节)
	unsigned int bitmapdataoffset;//位图数据的起始位置，以相对于位图(10-13字节)
	unsigned int bitmapheadersize;//BMP头的大小，固定为40（14-17字节）
	unsigned int width;//图片宽度；以像素为单位（18-21字节）
	unsigned int height;//图片高度；以像素为单位（22-25字节）
	unsigned short planes;//图片位面数，必须为1(26-27字节）
	unsigned short bitperpixel;//每个像素所需的位数，每个像素所需的位数（28-29字节）
							   //只能是以下几个数：1（双色），4(16色），8(256色）或24（真彩色）  灰度级
	unsigned int compression;//是否压缩（30-33字节）
							 //只能是以下几个数：0（不压缩），1(BI_RLE8压缩类型），2(BI_RLE4压缩类型）
	unsigned int bitmapdatasize;//位图的大小，以字节为单位（34-37字节）
	unsigned int hresolution;//位图水平分辨率，每米像素数（38-41字节）
	unsigned int vresolution;//位图垂直分辨率，每米像素数（42-45字节)
	unsigned int colors;//位图实际使用的颜色表中的颜色数（46-49字节）
	unsigned int importantcolors;//位图显示过程中重要的颜色数（50-53字节）
								 //unsigned int  bmiColors[1];//调色板；(54 - 57字节)
	unsigned char palette[256][4];//调色板 占256*4=1024字节
}BMPheader;//总大小40+14+1024=1078字节
typedef struct
{
	BMPheader* bmpheader;
	unsigned char* bitmapdata;//图片数据；
}BMPheaderfile;
/*
求文件长度的函数
*/
long getfilesize(FILE *f)
{
	long pos, len;
	pos = ftell(f);//ftell函数用于得到文件指针当前位置相对于文件首的偏移字节数
	fseek(f, 0, SEEK_END);//fseek函数用于移动文件指针相对于SEEK_END的偏移量为0
	len = ftell(f);//len就是文件的长度
	fseek(f, pos, SEEK_SET);//将文件指针移动到原来的地方
	return len;
}
void press()
{
	printf("按任意键继续...");
	_getch();
}
/*
主函数
*/
int main()
{
	DWORD start, end;
	BMPheaderfile *output = (BMPheaderfile*)malloc(sizeof(BMPheaderfile));//定义一个输出指针
	unsigned char *data = NULL;
	FILE *fpr, *fpw;//fpr:file open read;fpw:file open write
	/*****************
	打开文件
	*****************/
	errno_t err;
	//fopen函数的返回值是一个指向文件的指针，而fopen_s函数的返回值则不同，正确返回0，不正确返回非0。
	if ((err=fopen_s(&fpr,"start.bmp", "rb")) != 0)//rb代表文件必须存在，且只能读取
	{
		printf("cannot open this file");
		exit(0);
	}
	if ((err=fopen_s(&fpw,"end1.bmp", "wb")) != 0)//wb代表新建一个二进制文件，已存在的文件将被删除，只允许写
	{
		printf("cannot wirte this file");
		exit(0);
	}
	long length = getfilesize(fpr);
	printf("文件的长度为%ld\n", length);
	printf("文件的头长度为%d\n", sizeof(BMPheader));
	data = (unsigned char*)malloc(length * sizeof(char));//分配空间
	if (0 == fread(data, 1, length, fpr))//读文件，从fpr指向的文件中读出length到data所指的内存空间去
	{
		printf("read failed\n");
		exit(0);
	}
	fclose(fpr);//释放指针
	output->bmpheader = (BMPheader*)malloc(sizeof(BMPheader));//memory allocation:动态内存分配；->指向结构体运算符，用处是使用一个指向结构体或对象的指针访问其内成员。
	memcpy(output->bmpheader, data, sizeof(BMPheader));//从data中拷贝sizeof(BMPheader)大小到output->bmpheader
	/**************************
	打印出图像中头文件的信息
	**************************/
	int height = output->bmpheader->height;
	int width = output->bmpheader->width;
	printf("filesize is %d\n", output->bmpheader->filesize);
	printf("该图像每个像素所需要的位数：%d\n", output->bmpheader->bitperpixel);
	printf("height is %d\n", output->bmpheader->height);
	printf("width is %d\n", output->bmpheader->width);
	data = data + sizeof(BMPheader);
	output->bitmapdata = data;
	/*******************************
	中值滤波算法(选择3×3的滑动窗口)
	*******************************/
	unsigned char pixel[9] = { 0 };//滑动窗口的像素值，初始为0
	unsigned char mid;//中值
	unsigned char temp;//中间变量
	int flag;
	int m, i, j, x, y;
	start = GetTickCount();
	for (j = 1; j<height - 1; j++)
	{
		for (i = 1; i<width - 1; i++)
		{
			//将3×3滑动窗口中的所有像素值放入pixel[m]
			m = 0;
			for (y = j - 1; y <= j + 1; y++)
				for (x = i - 1; x <= i + 1; x++)
				{
					pixel[m] = data[y*width + x];
					m = m + 1;
				}
			//让一位数组pixel[9]进行降序排列
			do
			{
				flag = 0;//循环结束的标志
				for (m = 0; m<8; m++)//attention ! m can not equal to 9 or the stack will overflow!
				{
					if (pixel[m]<pixel[m + 1])//overflow!
					{
						temp = pixel[m];
						pixel[m] = pixel[m + 1];
						pixel[m + 1] = temp;
						flag = 1;
					}//if
				}//for
			} while (flag == 1);
			mid = pixel[4];
			output->bitmapdata[width*j + i] = mid;
		}
	}
	end = GetTickCount();
	/**********************
	保存图像文件
	***********************/
	fseek(fpw, 0, 0);  //fseek(fpw,0,SEEK_SET)
	fwrite(output->bmpheader, 1, sizeof(BMPheader), fpw);//写入图像的头文件
	fwrite(output->bitmapdata, 1, length - sizeof(BMPheader), fpw);//写入图像的数据信息
	fclose(fpw);//释放指针
	printf("时间=%d毫秒\n",end-start);
	press();
	return 0;
}