///////////////////////////////////////////////////////////////////////////////
// サンプルプログラム
//	OpenCV 3.1.0を利用
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <conio.h>

#include <Windows.h>


// OpenCV
#include <opencv2/opencv.hpp>

// STLport
#include <vector>
#include <string>

using namespace std;


//トリミングクラス
class Cut {
	public:
		static int i;
		void cut(const string &a);
		cv::Mat cut_img;
		operator cv::Mat() {
			return cut_img;
		}
};

void Cut::cut(const string &a) {
	//画像をトリミング
	cv::Mat img1 = cv::imread(a);
	cv::Mat a1(img1, cv::Rect(500, 400, 300, 300));
	cut_img = a1;
	string name = "cut";
	name += to_string(i) + ".jpg";
	cv::imwrite(name, cut_img);
	i++;
}

//HoughCirclesクラス
class Circles {
public:
	cv::Mat circle_img;
	void Hough(const cv::Mat &a);
	void Hough1(const cv::Mat &a);
	int x;
	int y;
};

void Circles::Hough(const cv::Mat &a) {
	//HoughCircles
	cv::Mat Hou = a;
	cv::Mat gray1;
	cvtColor(a, gray1, CV_BGR2GRAY);
	cv::GaussianBlur(gray1, gray1, cv::Size(9, 9), 2, 2);
	vector<cv::Vec3f> circles;
	cv::HoughCircles(gray1, circles, CV_HOUGH_GRADIENT, 2, gray1.rows / 4, 200, 100);
	for (size_t i = 0; i < circles.size(); i++)
	{
		cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		// 円の中心を描画します．
		circle(Hou, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
		// 円を描画します．
		circle(Hou, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
	}
	cv::namedWindow("circles", 1);
	cv::imshow("circles", Hou);
}

void Circles::Hough1(const cv::Mat &a) {
	cv::Mat a1(a, cv::Rect(300, 200, 700, 650));

	//HoughCircles
	cv::Mat Hou1 = a1.clone();
	cv::Mat Hou2 = a1.clone();

	cv::Mat gray1;
	cvtColor(a1, gray1, CV_BGR2GRAY);
	cv::GaussianBlur(gray1, gray1, cv::Size(9, 9), 2, 2);
	vector<cv::Vec3f> circles1;
	cv::HoughCircles(gray1, circles1, CV_HOUGH_GRADIENT, 1, gray1.rows / 3, 200, 30, 50);

	cv::Point center(cvRound(circles1[0][0]), cvRound(circles1[0][1]));
	int radius = cvRound(circles1[0][2]);
	// 円の中心を描画します．
	circle(Hou1, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
	// 円を描画します．
	circle(Hou1, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);

	x = cvRound(circles1[0][0]);
	y = cvRound(circles1[0][1]);

	//cv::imshow("circles2", Hou1);
	cv::Mat a2(Hou2, cv::Rect(x-150, y-150, 300, 300));
	circle_img = a2;
	//cv::imshow("切り抜き", circle_img);
}


//差分で人物を検出するクラス
class Diff {
	public:
		int ans = 0;
		double com = 0;
		void diff(const cv::Mat &a, const cv::Mat &b);
		cv::Mat diff_img;
		void label(const cv::Mat &a);
		cv::Mat label_img;
		operator cv::Mat() {
			return label_img;
		}
};

void Diff::diff(const cv::Mat &a, const cv::Mat &b) {
	cv::Mat gray1;
	cv::Mat gray2;

	//color-->grayscale
	cvtColor(a, gray1, CV_BGR2GRAY);
	cvtColor(b, gray2, CV_BGR2GRAY);

	//diff = | gray2 - gray1 |
	absdiff(gray1, gray2, diff_img);

	threshold(diff_img, diff_img, 60, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

	cv::imwrite("diff1.jpg", diff_img);

	//opening処理
	cv::erode(diff_img, diff_img, cv::Mat(), cv::Point(-1, -1), 1);

	//cv::imwrite("diff2.jpg", diff_img);
	cv::imshow("diff", diff_img);
	label(diff_img);

	if (ans > 1500) {
		cv::line(b, cv::Point(50, 50), cv::Point(250, 250), cv::Scalar(0, 0, 255), 5, 8);
		cv::line(b, cv::Point(250, 50), cv::Point(50, 250), cv::Scalar(0, 0, 255), 5, 8);
		cv::imshow("result", b);
		std::cout << "×" << std::endl;
	}
	else {
		cv::circle(b, cv::Point(150, 150), 100, cv::Scalar(0, 0, 255), 8, 8);
		cv::imshow("result", b);
		std::cout << "〇" << std::endl;
	}
}

void Diff::label(const cv::Mat &a) {
	//ラべリング処理
	cv::Mat LabelImg;
	cv::Mat stats;
	cv::Mat centroids;
	int nLab = cv::connectedComponentsWithStats(a, LabelImg, stats, centroids);
	int count = 1;

	// ラベリング結果の描画色を決定
	std::vector<cv::Vec3b> colors(nLab);
	colors[0] = cv::Vec3b(0, 0, 0);
	for (int i = 1; i < nLab; ++i) {
		colors[i] = cv::Vec3b((rand() & 255), (rand() & 255), (rand() & 255));
	}

	// ラベリング結果の描画
	cv::Mat Dst(a.size(), CV_8UC3);
	for (int i = 0; i < Dst.rows; ++i) {
		int *lb = LabelImg.ptr<int>(i);
		cv::Vec3b *pix = Dst.ptr<cv::Vec3b>(i);
		for (int j = 0; j < Dst.cols; ++j) {
			pix[j] = colors[lb[j]];
		}
	}

	//ROIの設定
	for (int i = 1; i < nLab; ++i) {
		int *param = stats.ptr<int>(i);

		int x = param[cv::ConnectedComponentsTypes::CC_STAT_LEFT];
		int y = param[cv::ConnectedComponentsTypes::CC_STAT_TOP];
		double height = param[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT];
		double width = param[cv::ConnectedComponentsTypes::CC_STAT_WIDTH];
		int area = param[cv::ConnectedComponentsTypes::CC_STAT_AREA];

		if (height > width) com = height / width;
		else com = width / height;
		std::cout << "com = " << com << std::endl;

		if (area >= 500 && com < 2) {
			cv::rectangle(Dst, cv::Rect(x, y, width, height), cv::Scalar(0, 255, 0), 2);
			ans += area;
		}
	}

	//面積値の出力
	for (int i = 1; i < nLab; ++i) {
		int *param = stats.ptr<int>(i);
		int area = param[cv::ConnectedComponentsTypes::CC_STAT_AREA];
		int height = param[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT];
		int width = param[cv::ConnectedComponentsTypes::CC_STAT_WIDTH];
		if (height > width) com = height / width;
		else com = width / height;

		if (area >= 500 && com < 2) {
			std::cout << "area " << count << " = " << param[cv::ConnectedComponentsTypes::CC_STAT_AREA] << std::endl;

			//ROIの左上に番号を書き込む
			int x = param[cv::ConnectedComponentsTypes::CC_STAT_LEFT];
			int y = param[cv::ConnectedComponentsTypes::CC_STAT_TOP];
			std::stringstream num;
			num << count;
			cv::putText(Dst, num.str(), cv::Point(x + 5, y + 20), cv::FONT_HERSHEY_COMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
			count++;
		}
	}
	label_img = Dst;
	cv::imshow("label", label_img);
	cv::imwrite("label.jpg", label_img);
}

int Cut::i = 0;

// メイン関数
int main(int argc, char *argv[])
{
	string fileName1 = "..\\MILAiS\\base.jpg";
	string fileName2;

#if 0
	fileName2 = argv[1];
#else
	fileName2 = "..\\MILAiS\\09.jpg";
#endif

	cv::Mat img1 = cv::imread(fileName1);
	cv::Mat img2 = cv::imread(fileName2);


	Circles Circle1;
	Circles Circle2;
	Circle1.Hough1(img1);
	Circle2.Hough1(img2);

	cv::imshow("base1", Circle1.circle_img);
	//cv::imshow("base2", Circle2.circle_img);
	Diff diff0;
	diff0.diff(Circle1.circle_img, Circle2.circle_img);


//	Daen Daen1;
	//Daen1.daen(img2);

	cv::waitKey();
	return 0;
}
