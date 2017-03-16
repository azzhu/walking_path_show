// walking_path_show.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

vector<int> split(string& str, const char* c);
void redraw_line(Mat* img);
void redraw_trajectory(Mat* img, vector<Point> ps);

//输入
string txtpath = "C:\\Users\\Administrator\\Desktop\\floortech\\a.txt";
vector<vector<int>> res;

int _tmain(int argc, _TCHAR* argv[])
{
	//获取txt中内容，存在res中
	ifstream txtin(txtpath);
	if (!txtin.is_open())
	{
		cout << "dakaishibai" << endl;
	}
	else
	{
		string temp;
		//ofstream txtout("C:\\Users\\Administrator\\Desktop\\floortech\\a_.txt");
		char c[] = " ";
		while (getline(txtin, temp))
		{
			vector<int> restemp = split(temp, c);
			//sort(restemp.begin(), restemp.end());
			//txtout << restemp[0] << " " << restemp[1] << " " << restemp[2] << " " << restemp[3] << " " << restemp[4]
			//	<< "    " << restemp[127] << " " << restemp[126] << " " << restemp[125] << " "
			//	<< restemp[124] << " " << restemp[123] << endl;
			res.push_back(restemp);
		}
		//txtout.close();
		txtin.close();
	}

	//生成图像
	Mat floor_(800, 800, CV_8UC3);
	floor_.setTo(cv::Vec3b(0, 255, 255));
	Vec3b c1 = Vec3b(96, 207, 160);
	Vec3b c2 = Vec3b(96, 151, 56);
	Vec3b c3 = Vec3b(96, 128, 00);
	Vec3b c4 = Vec3b(255, 255, 255);
	vector<pair<Point, int>> paw;	//有效点和权重的集合（每一帧）
	vector<Point> trajectory_ps;		//轨迹点集合
	int s = res.size();
	//ma、mi：触发的阈值范围，在此范围内判断颜色的轻重
	int ma = -150;
	int mi = -600;
	int th = ma;
	for (int v = 0; v < s; v++)
	{
		cout << v << endl;
		int ispeoplein = 0;	//判断这帧里面是否有人（是否有<-150的，累加计数）
		vector<int> te = res[v];
		int ss = te.size();
		paw.clear();
		for (int i = 0; i < ss; i++)
		{
			int x = (i / 2) % 8;
			int y = i / 16;
			if ((x % 2) == (y % 2))		//偶偶或奇奇
			{
				int x_ = 100 * x;
				int y_ = 100 * y;
				for (int m = y_; m < y_ + 100; m++)
				{
					for (int n = x_; n < x_ + 100; n++)
					{
						if (i % 2 == 0)	//第一个小三角
						{
							int val = te[i];
							if (m - 100 * y > n - 100 * x&&val < th)
							{
								ispeoplein++;
								if (val >= -300)
									floor_.at<Vec3b>(Point(n, m)) = c1;
								else if (val < -300 && val >= -450)
									floor_.at<Vec3b>(Point(n, m)) = c2;
								else
									floor_.at<Vec3b>(Point(n, m)) = c3;

								int w = th - val;
								Point p = Point(x_ + 30, y_ + 70);
								paw.push_back(pair<Point, int>(p, w));
							}								
						}
						else
						{
							int val = te[i];
							if (m - 100 * y < n - 100 * x&&val < th)
							{
								ispeoplein++;
								if (val >= -300)
									floor_.at<Vec3b>(Point(n, m)) = c1;
								else if (val < -300 && val >= -450)
									floor_.at<Vec3b>(Point(n, m)) = c2;
								else
									floor_.at<Vec3b>(Point(n, m)) = c3;

								int w = th - val;
								Point p = Point(x_ + 70, y_ + 30);
								paw.push_back(pair<Point, int>(p, w));
							}
						}
					}
				}
			}
			else
			{
				int x_ = 100 * x;
				int y_ = 100 * y;
				for (int m = y_; m < y_ + 100; m++)
				{
					for (int n = x_; n < x_ + 100; n++)
					{
						if (i % 2 == 0)	//第一个小三角
						{
							int val = te[i];
							if ((m + n) < 100 * (x + y + 1) && val < th)
							{
								ispeoplein++;
								if (val >= -300)
									floor_.at<Vec3b>(Point(n, m)) = c1;
								else if (val < -300 && val >= -450)
									floor_.at<Vec3b>(Point(n, m)) = c2;
								else
									floor_.at<Vec3b>(Point(n, m)) = c3;

								int w = th - val;
								Point p = Point(x_ + 30, y_ + 30);
								paw.push_back(pair<Point, int>(p, w));
							}
						}
						else
						{
							int val = te[i];
							if ((m + n) > 100 * (x + y + 1) && val < th)
							{
								ispeoplein++;
								if (val >= -300)
									floor_.at<Vec3b>(Point(n, m)) = c1;
								else if (val < -300 && val >= -450)
									floor_.at<Vec3b>(Point(n, m)) = c2;
								else
									floor_.at<Vec3b>(Point(n, m)) = c3;

								int w = th - val;
								Point p = Point(x_ + 70, y_ + 70);
								paw.push_back(pair<Point, int>(p, w));
							}
						}
					}
				}
			}
		}
		if (ispeoplein == 0)	//人出去后，轨迹重新画
			trajectory_ps.clear();
		else
		{
			//计算轨迹
			vector<pair<Point, int>>::iterator it, ite;
			it = paw.begin(), ite = paw.end();
			int total_w = 0;
			float x = 0, y = 0;
			for (; it != ite; it++)
			{
				total_w += -150 - it->second;
			}
			it = paw.begin();
			for (; it != ite; it++)
			{
				float x_ = it->first.x;
				float y_ = it->first.y;
				float rad = (float)(-150 - it->second) / (float)total_w;
				x += (x_*rad);
				y += (y_*rad);
			}
			trajectory_ps.push_back(Point((int)x, (int)y));
		}

		redraw_trajectory(&floor_, trajectory_ps);
		redraw_line(&floor_);
		imshow(" ", floor_);
		floor_.setTo(cv::Vec3b(0, 255, 255));
		waitKey(200);
	}
	
	waitKey();
	return 0;
}

//字符串分割
vector<int> split(string& str, const char* c)
{
	char *cstr, *p;
	vector<int> res;
	cstr = new char[str.size() + 1];
	strcpy(cstr, str.c_str());
	p = strtok(cstr, c);
	while (p != NULL)
	{
		res.push_back(atoi(p));
		p = strtok(NULL, c);
	}
	return res;
}

//重绘底板格子
void redraw_line(Mat* img)
{
	int thickness = 1;
	for (int i = 1; i <= 7; i++)
	{
		line(*img, Point(100 * i, 0), Point(100 * i, 800), Scalar(0, 0, 0), thickness);
		line(*img, Point(0, 100 * i), Point(800, 100 * i), Scalar(0, 0, 0), thickness);
		int x, y, x1, y1;
		if (i <= 4)
		{
			x = 200 * i;
			y = 0;
		}
		else
		{
			x = 800;
			y = 200 * (i - 4);
		}
		line(*img, Point(x, y), Point(y, x), Scalar(0, 0, 0), thickness);
		if (i <= 4)
		{
			x = 800;
			y = 200 * i;
			x1 = 800 - 200 * i;
			y1 = 0;
		}
		else
		{
			x = 200 * (i - 4);
			y = 800;
			x1 = 0;
			y1 = 200 * (8 - i);
		}
		line(*img, Point(x, y), Point(x1, y1), Scalar(0, 0, 0), thickness);
	}
}

//画轨迹
void redraw_trajectory(Mat* img, vector<Point> ps)
{
	int s = ps.size();
	for (int i = 1; i < s; i++)
	{
		line(*img, ps[i - 1], ps[i], Scalar(255, 0, 0), 3);
	}
}