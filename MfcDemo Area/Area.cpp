#include "Utils.h"
#include <cmath>


#include "Area.h"

#include "Tserial.h"

#define SCALE_X 0.94
#define SCALE_Y 1.17

#define SPEED 1
#define RADIUS 3

#define SLEEP_TIME 20

//#include <windows.h>

#define REAL_SIZE_X 3.7		//	cm
#define REAL_SIZE_Y 4.5

#define MAX_X 1200
#define MAX_Y 600
#define MAX_NUM_OF_POINTS 100


static int pointSet[MAX_NUM_OF_POINTS][2];		//	Each points corresponds to an index
static int pair[MAX_NUM_OF_POINTS + 5][2][2];			//	{point1, point2}


static int numOfPairs = 0;

static int first_x, first_y;
static int pre_x, pre_y;
static double ball_x, ball_y;
static int direction[2][2];
static int serialFlag = false;

int preCollidedLine = -1;

double normalizedDir[2];		//	normalized velocity vector

Tserial *com;
char buffer[20];



void Reset() {
	numOfPairs = 0;
	com->sendChar(0);
}

double GetDistance(int x1, int y1, int x2, int y2) {
	return sqrt((double)pow((double)x1 - x2, 2.0) + pow((double)y1 - y2, 2.0));
}

int Set(int x, int y, int flag) {
	if(x >= MAX_X || y >= MAX_Y) return -1;
	
	switch(flag) {
	case 0:		//	it is drawing the very first point
		first_x = x;
		first_y = y;
		pre_x = x;
		pre_y = y;
		break;
	case 1:		//	it is an endpoint(not the very first one).
		pair[numOfPairs][0][0] = pre_x;
		pair[numOfPairs][0][1] = pre_y;
		pair[numOfPairs][1][0] = x;
		pair[numOfPairs][1][1] = y;
		pre_x = x;
		pre_y = y;
		numOfPairs++;
		break;
	case 2:		//	it is choosing the  position of the ball			Meanwhile, should change the last point to the first point	
		pair[numOfPairs - 1][1][0] = first_x;
		pair[numOfPairs - 1][1][1] = first_y;
	}

	return 0;
}



void SetSerial() {

	if(serialFlag == true) return;

    com = new Tserial();
    if (com!=0)
    {
		if(com->connect("\\\\.\\COM11", 115200, spNONE) == 0) {
			WacomTrace("Successful!\n");
		} else {
			WacomTrace("Errors with Serial communication.\n");
		}
	}
	serialFlag = true;
}

double getTriangleArea(int i, double x, double y) {
	double h = getPointLineDis(x, y, i);
	double a = sqrt((double)getDisSquare(pair[i][0][0], pair[i][0][1], pair[i][1][0], pair[i][1][1]));
	return a * h / 2;
}

void Output() {
	
	for(int i = 0;i < numOfPairs;i++) {
		WacomTrace("(%d, %d), (%d, %d)\n", pair[i][0][0], pair[i][0][1], pair[i][1][0], pair[i][1][1]);
		com->sendChar(pair[i][0][0] * SCALE_X);
		com->sendChar(pair[i][0][1] * SCALE_Y);
		com->sendChar(pair[i][1][0] * SCALE_X);
		com->sendChar(pair[i][1][1] * SCALE_Y);
		Sleep(50);
	}

	com->sendChar(250);		//	Read to send area
	Sleep(1000);

	double area = 0;
	WacomTrace("numOfPairs: %d\n", numOfPairs);
	for(int i = 0;i < numOfPairs;i++) {
		area += getTriangleArea(i, 128 / SCALE_X, 160 / SCALE_Y);		//	Choose mid point as the anchor point.
	}
	
	WacomTrace("area: %lf\n", area);
	area /= (128 * 160 / (REAL_SIZE_X * REAL_SIZE_Y));
	WacomTrace("area: %lf\n", area);

	char ret_string[100];
	sprintf(ret_string,"%f", (float)area);

	int numOfBytesToSend;
	for(int i = 0;i < 100;i++) {
		if(ret_string[i] == '.') {
			numOfBytesToSend = i + 2;
		}
	}

	for(int i = 0;i < numOfBytesToSend;i++) {
		com->sendChar(ret_string[i]);
	}
	com->sendChar('c');
	com->sendChar('m');
	com->sendChar('2');

}


double getPointLineDis(int x, int y, int lineIndex) {
    if(lineIndex >= numOfPairs) {
        return -1;
	}
    double m_sq, n_sq, d_sq;
    m_sq = getDisSquare(x, y, pair[lineIndex][0][0], pair[lineIndex][0][1]);
    n_sq = getDisSquare(x, y, pair[lineIndex][1][0], pair[lineIndex][1][1]);
    d_sq = getDisSquare(pair[lineIndex][0][0], pair[lineIndex][0][1], pair[lineIndex][1][0], pair[lineIndex][1][1]);

    return sqrt( m_sq - (m_sq - n_sq + d_sq) * (m_sq - n_sq + d_sq) / (4 * d_sq) );

}

int getDisSquare(int x1, int y1, int x2, int y2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

double ABS(double x) {
	return x > 0 ? x : -x;
}