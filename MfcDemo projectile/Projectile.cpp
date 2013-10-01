#include "Utils.h"
#include <cmath>


#include "Projectile.h"

#include "Tserial.h"

#define MAX_NUM_OF_LINES 100

#define SCALE_X 0.94
#define SCALE_Y 1.17

#define RADIUS 4

#define SLEEP_TIME 20

#define V_FACTOR 0.04
#define A_FACTOR 0.02

#define BOUNCING_FACTOR_BOUNDARY 0.85
#define BOUNCING_FACTOR_LINE 0.92
#define FRICTION_FACTOR 0.95

//#include <windows.h>


static double pair[MAX_NUM_OF_LINES][2][2];
static double ball_x, ball_y;
static double v_point[2][2];
static double g_point[2][2];
static double v[2];
static double a[2];

static bool serialFlag = false;


static int count_2 = 0;

static int last_x, last_y;

static int numOfPairs = 0;

Tserial *com;
char buffer[20];


//	The transparent display mode should be landscape

void Reset() {
	numOfPairs = 0;
	com->sendChar(0);
}

double GetDistance(int x1, int y1, int x2, int y2) {
	return sqrt((double)pow((double)x1 - x2, 2.0) + pow((double)y1 - y2, 2.0));
}

int Set(int x, int y, int flag) {
	
	switch(flag) {
	case 0:		
		g_point[count_2][0] = x;
		g_point[count_2][1] = y;
		count_2 = (count_2 + 1) % 2;
		break;
	case 1:		
		ball_x = x;
		ball_y = y;
		break;
	case 2:		
		v_point[count_2][0] = x;
		v_point[count_2][1] = y;
		count_2 = (count_2 + 1) % 2;
		break;
	case 3:										//	Set the starting point of a line
		pair[numOfPairs][count_2][0] = x;
		pair[numOfPairs][count_2][1] = y;
		count_2 = (count_2 + 1) % 2;
		break;
	case 4:										//	Set the endpoint of a line
		pair[numOfPairs][count_2][0] = last_x;
		pair[numOfPairs][count_2][1] = last_y;
		count_2 = (count_2 + 1) % 2;
		numOfPairs++;
	}

	return 0;
}

void Record(int x, int y) {
	last_x = x;
	last_y = y;
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


	com->sendChar(200);			//	Sign for sending lines
	for(int i = 0;i < numOfPairs;i++) {
		com->sendChar(pair[i][0][0] * SCALE_X);
		com->sendChar(pair[i][0][1] * SCALE_Y);
		com->sendChar(pair[i][1][0] * SCALE_X);
		com->sendChar(pair[i][1][1] * SCALE_Y);
	}

	com->sendChar(201);			//	Sign for animation
	getInitialVelocity();
	while(1) {
		if(ABS(v[0]) * V_FACTOR / SLEEP_TIME * 1000 < 0.8 && ABS(v[1]) * V_FACTOR / SLEEP_TIME * 1000 < 0.8) {
			//WacomTrace("velocity: %lf, %lf", v[0], v[1]);
			WacomTrace("Stop.\n");
			break;
		}
		Sleep(SLEEP_TIME);
		int colli = collision();
		if(colli != -1) {
			WacomTrace("Collision with line %d.\n", colli);
			ballOnTheLine(colli, 1);
		} else {
			if(hitTheGround() || hitTheCeiling()) {
				v[1] = -v[1] * BOUNCING_FACTOR_BOUNDARY;
				v[0] *= FRICTION_FACTOR;
			} 
			if(hitTheLeftWall() || hitTheRightWall()) {
				v[0] = -v[0] * BOUNCING_FACTOR_BOUNDARY;
			}
		}

		move();
		WacomTrace("%lf %lf\n", v[0], v[1]);
		WacomTrace("%lf %lf\n", a[0], a[1]);
		com->sendChar(ball_x * SCALE_X);
		com->sendChar(ball_y * SCALE_Y);
	}
}

int collision() {
    int lineCollided = -1;
	double min = 999999;
	for(int i = 0;i < numOfPairs;i++) {
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

	WacomTrace("ball: %lf, %lf\n", ball_x, ball_y);
	WacomTrace("p1: %lf, %lf  p2: %lf, %lf\n", pair[i][0][0], pair[i][0][1], pair[i][1][0], pair[i][1][1]);
	WacomTrace("intersect: %lf, %lf\n", intersect[0], intersect[1]);

	if(getPointLineDis(ball_x + dis * verticalToLine[0][0], ball_y + dis * verticalToLine[0][1], i) 
	 < getPointLineDis(ball_x + dis * verticalToLine[1][0], ball_y + dis * verticalToLine[1][1], i)) {
		//intersect[0] = ball_x + dis * verticalToLine[0][0];
		//intersect[1] = ball_y + dis * verticalToLine[0][1];
		vertical[0] = verticalToLine[1][0];
		vertical[1] = verticalToLine[1][1];
	} else {
		//intersect[0] = ball_x + dis * verticalToLine[1][0];
		//sintersect[1] = ball_y + dis * verticalToLine[1][1];
		vertical[0] = verticalToLine[0][0];
		vertical[1] = verticalToLine[0][1];
	}
	
	WacomTrace("normLineDir: %lf, %lf\n", normLineDir[0], normLineDir[1]);
	WacomTrace("vertical: %lf, %lf\n", vertical[0], vertical[1]);


	if(((pair[i][0][0] - intersect[0]) * (pair[i][1][0] - intersect[0]) <= 0 && 
		(pair[i][0][1] - intersect[1]) * (pair[i][1][1] - intersect[1]) <= 0) && vertical[0] * v[0] + vertical[1] * v[1] < 0 ) {
		
		if(flag == 1) {
			/* TO DO: .....  */
			double vec[2];
			double dotProd;
			double normalizedDir[2];		//	normalized velocity vector
			normalizedDir[0] = v[0];
			normalizedDir[1] = v[1];

			normalize(normalizedDir[0], normalizedDir[1]);

			dotProd = (-normalizedDir[0] * vertical[0]) + (-normalizedDir[1] * vertical[1]);
			vec[0] = dotProd * vertical[0];
			vec[1] = dotProd * vertical[1];
		
			WacomTrace("vec: %lf, %lf\n", vec[0], vec[1]);
			WacomTrace("pre: %lf, %lf\n", normalizedDir[0], normalizedDir[1]);
			normalizedDir[0] = 2 * vec[0] - (-normalizedDir[0]);
			normalizedDir[1] = 2 * vec[1] - (-normalizedDir[1]);

			double tmp = sqrt(v[0] * v[0] + v[1] * v[1]);
			v[0] = normalizedDir[0] * tmp * BOUNCING_FACTOR_LINE;
			v[1] = normalizedDir[1] * tmp * BOUNCING_FACTOR_LINE;

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

bool hitTheLeftWall() {
	if(ball_x * SCALE_X < RADIUS && v[0] < 0) {
		return true;
	}
	else return false;
}

bool hitTheRightWall() {
	if(ball_x * SCALE_X + RADIUS > 160 && v[0] > 0) {
		return true;
	}
	else return false;
}

bool hitTheCeiling() {
	if(ball_y * SCALE_Y < RADIUS && v[1] < 0) {
		return true;
	}
	else return false;
}

bool hitTheGround() {
	if(ball_y * SCALE_Y + RADIUS > 128 && v[1] > 0) {
		return true;
	}
	else return false;
}

void getInitialVelocity() {
	v[0] = v_point[1][0] - v_point[0][0];
	v[1] = v_point[1][1] - v_point[0][1];
}

void getAcceleration() {
    a[0] = 0; //g_point[1][0] -  g_point[0][0];
    a[1] = g_point[1][1] -  g_point[0][1];
}

void normalize(double &x, double &y) {
	double tmp = sqrt(x * x + y * y);
	x /= tmp;
	y /= tmp;
}


int getDisSquare(int x1, int y1, int x2, int y2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

void getVelocity() {
	getAcceleration();
	v[0] += a[0] * A_FACTOR;
	v[1] += a[1] * A_FACTOR;
}

void move() {
    ball_x = ball_x + v[0] * V_FACTOR;
    ball_y = ball_y + v[1] * V_FACTOR;
	getVelocity();
}

double ABS(double x) {
	return x > 0 ? x : -x;
}


/*
void recoverLines(int i) {
	double normLineDir[2];
	normLineDir[0] = pair[i][1][0] - pair[i][0][0];
	normLineDir[1] = pair[i][1][1] - pair[i][0][1];
	normalize(normLineDir[0], normLineDir[1]);

	double intLineDir[2][2];
	intLineDir[0][0] = (int)(pair[i][0][0] * SCALE_X);
	intLineDir[0][1] = (int)(pair[i][0][1] * SCALE_Y);
	intLineDir[1][0] = (int)(pair[i][1][0] * SCALE_X);
	intLineDir[1][1] = (int)(pair[i][1][1] * SCALE_Y);

	double extraLine1[2][2], extraLine2[2][2];

	WacomTrace("recove lines: %lf %lf\n", intLineDir[0][0], intLineDir[0][1]);


	
	com->sendChar(205);

	com->sendChar(i);

	if(ABS(normLineDir[0]) >= ABS(normLineDir[1])) {		//	x >= y
		com->sendChar(intLineDir[0][0]);
		com->sendChar(intLineDir[0][1] + 1);
		com->sendChar(intLineDir[1][0]);
		com->sendChar(intLineDir[1][1] + 1);
		
		com->sendChar(intLineDir[0][0]);
		com->sendChar(intLineDir[0][1] - 1);
		com->sendChar(intLineDir[1][0]);
		com->sendChar(intLineDir[1][1] - 1);
	} else {									//	x <= y
		com->sendChar(intLineDir[0][0] + 1);
		com->sendChar(intLineDir[0][1]);
		com->sendChar(intLineDir[1][0] + 1);
		com->sendChar(intLineDir[1][1]);
		
		com->sendChar(intLineDir[0][0] - 1);
		com->sendChar(intLineDir[0][1]);
		com->sendChar(intLineDir[1][0] - 1);
		com->sendChar(intLineDir[1][1]);
	}
}
*/