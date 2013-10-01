#include "Utils.h"
#include <cmath>


#include "Reflection.h"

#include "Tserial.h"

#define SCALE_X 0.94
#define SCALE_Y 1.17

#define SPEED 2
#define RADIUS 4

#define SLEEP_TIME 20

//#include <windows.h>


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

		pair[numOfPairs][0][0] = 0;
		pair[numOfPairs][0][1] = 0;
		pair[numOfPairs][1][0] = 128 / SCALE_X;
		pair[numOfPairs++][1][1] = 0;

		
		pair[numOfPairs][0][0] = 0;
		pair[numOfPairs][0][1] = 0;
		pair[numOfPairs][1][0] = 0;
		pair[numOfPairs++][1][1] = 160 / SCALE_Y;

		
		pair[numOfPairs][0][0] = 128 / SCALE_X;
		pair[numOfPairs][0][1] = 160 / SCALE_Y;
		pair[numOfPairs][1][0] = 128 / SCALE_X;
		pair[numOfPairs++][1][1] = 0;

		
		pair[numOfPairs][0][0] = 128 / SCALE_X;
		pair[numOfPairs][0][1] = 160 / SCALE_Y;
		pair[numOfPairs][1][0] = 0;
		pair[numOfPairs++][1][1] = 160 / SCALE_Y;

		ball_x = x;
		ball_y = y;
		break;
	case 3:		//	it is drawing the starting point of the direction line.
		direction[0][0] = x;
		direction[0][1] = y;
		break;
	case 4:		//	it is drawing the endpoint of the direction line.
		direction[1][0] = x;
		direction[1][1] = y;
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


void Output() {
	com->sendChar('l');		//	lines
	for(int i = 0;i < numOfPairs - 4;i++) {
		WacomTrace("(%d, %d), (%d, %d)\n", pair[i][0][0], pair[i][0][1], pair[i][1][0], pair[i][1][1]);
		com->sendChar(pair[i][0][0] * SCALE_X);
		com->sendChar(pair[i][0][1] * SCALE_Y);
		com->sendChar(pair[i][1][0] * SCALE_X);
		com->sendChar(pair[i][1][1] * SCALE_Y);
		Sleep(50);
	}
	com->sendChar('b');		//	ball
	WacomTrace("(%lf, %lf)", ball_x, ball_y);
	com->sendChar((char)ball_x * SCALE_X);
	com->sendChar((char)(ball_y * SCALE_Y));

	com->sendChar('d');		//	direction
	WacomTrace("(%d, %d), (%d, %d)\n", direction[0][0], direction[0][1], direction[1][0], direction[1][1]);
	com->sendChar(direction[0][0] * SCALE_X);
	com->sendChar(direction[0][1] * SCALE_Y);
	com->sendChar(direction[1][0] * SCALE_X);
	com->sendChar(direction[1][1] * SCALE_Y);

	
	com->sendChar('a');		//	animation
	Sleep(6000);
	
	WacomTrace("direction: (%d %d), (%d %d)\n", direction[0][0], direction[0][1], direction[1][0], direction[1][1]);
	getDir();
	WacomTrace("normDir: %lf %lf\n", normalizedDir[0], normalizedDir[1]);
	while(1) {		
		Sleep(SLEEP_TIME);
		int colli = collision();
		if(colli != -1) {
			WacomTrace("Collision with line %d.\n", colli);
			preCollidedLine = colli;
			ballOnTheLine(colli, 1);		//	Get the reflected direction
			com->sendChar(250);			//	To change the ball color
		}
		move();
		//WacomTrace("x: %lf, ", normalizedDir[0]);
		//WacomTrace("y: %lf\n", normalizedDir[1]);
		//WacomTrace("%lf\n", getPointLineDis(ball_x, ball_y, 0));
		com->sendChar(ball_x * SCALE_X);
		com->sendChar(ball_y * SCALE_Y);
	}
}



void getDir() {
    normalizedDir[0] = direction[1][0] -  direction[0][0];
    normalizedDir[1] = direction[1][1] -  direction[0][1];
	double tmp = sqrt(pow(normalizedDir[0], 2) + pow(normalizedDir[1], 2));
	normalizedDir[0] /= tmp;
	normalizedDir[1] /= tmp;
}

int collision() {
    int lineCollided = -1;
	double min = 999999;
	for(int i = 0;i < numOfPairs;i++) {
		if(i == preCollidedLine) continue;
		double dis = getPointLineDis(ball_x, ball_y, i);
		if(getPointLineDis(ball_x, ball_y, i) < RADIUS && ballOnTheLine(i, 0)) {
			if(dis < min) {
				min = dis;
				lineCollided = i;
			}
		}
	}
    return lineCollided;
}

void normalize(double &x, double &y) {
	double tmp = sqrt(x * x + y * y);
	x /= tmp;
	y /= tmp;
}

bool ballOnTheLine(int i, int flag) {
	double dis = getPointLineDis(ball_x, ball_y, i);
	double verticalToLine[2][2];		//	Two possibilities
	double vertical[2];				//	Correct one
	double intersect[2];
	double normLineDir[2];
	normLineDir[0] = pair[i][1][0] - pair[i][0][0];
	normLineDir[1] = pair[i][1][1] - pair[i][0][1];
	normalize(normLineDir[0], normLineDir[1]);
	verticalToLine[0][0] = - normLineDir[1];
	verticalToLine[0][1] =   normLineDir[0];
	verticalToLine[1][0] =   normLineDir[1];
	verticalToLine[1][1] = - normLineDir[0];

	
	double a1 = pair[i][1][1] - pair[i][0][1];		//	y2 - y1
	double a2 = pair[i][0][0] - pair[i][1][0];		//	x1 - x2
	double b1 = -a2;								//	x2 - x1
	double b2 = a1;									//	y2 - y1
	double c1 = ball_y * a1 + ball_x * b1;
	double c2 = pair[i][0][0] * pair[i][1][1] - pair[i][1][0] * pair[i][0][1];		//	x1y2 - x2y1

	intersect[0] = (c1 * a2 - c2 * a1) / (a2 * b1 - a1 * b2);
	intersect[1] = (c1 * b2 - c2 * b1) / (a1 * b2 - a2 * b1);
	
	if(getPointLineDis(ball_x + dis * verticalToLine[0][0], ball_y + dis * verticalToLine[0][1], i) 
	 < getPointLineDis(ball_x + dis * verticalToLine[1][0], ball_y + dis * verticalToLine[1][1], i)) {
		//intersect[0] = ball_x + dis * verticalToLine[0][0];
		//intersect[1] = ball_y + dis * verticalToLine[0][1];
		vertical[0] = verticalToLine[1][0];
		vertical[1] = verticalToLine[1][1];
	} else {
		//intersect[0] = ball_x + dis * verticalToLine[1][0];
		//intersect[1] = ball_y + dis * verticalToLine[1][1];
		vertical[0] = verticalToLine[0][0];
		vertical[1] = verticalToLine[0][1];
	}
	
	WacomTrace("normLineDir: %lf, %lf\n", normLineDir[0], normLineDir[1]);
	WacomTrace("vertical: %lf, %lf\n", vertical[0], vertical[1]);

	
	if(((pair[i][0][0] - intersect[0]) * (pair[i][1][0] - intersect[0]) <= 0 && 
		(pair[i][0][1] - intersect[1]) * (pair[i][1][1] - intersect[1]) <= 0) && vertical[0] * normalizedDir[0] + vertical[1] * normalizedDir[1] < 0 ) {
		
		if(flag == 1) {
				/* TO DO: .....  */
			double vec[2];
			double dotProd;

			dotProd = (-normalizedDir[0] * vertical[0]) + (-normalizedDir[1] * vertical[1]);
			vec[0] = dotProd * vertical[0];
			vec[1] = dotProd * vertical[1];
		
			WacomTrace("vec: %lf, %lf\n", vec[0], vec[1]);
			WacomTrace("pre: %lf, %lf\n", normalizedDir[0], normalizedDir[1]);
			normalizedDir[0] = 2 * vec[0] - (-normalizedDir[0]);
			normalizedDir[1] = 2 * vec[1] - (-normalizedDir[1]);

			WacomTrace("post: %lf, %lf\n", normalizedDir[0], normalizedDir[1]);
		}

		return true;
	} else {
		return false;
	}
	
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

void move() {
    ball_x = ball_x + SPEED * normalizedDir[0];
    ball_y = ball_y + SPEED * normalizedDir[1];
}

double ABS(double x) {
	return x > 0 ? x : -x;
}