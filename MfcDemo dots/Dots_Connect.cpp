#include "Utils.h"
#include <cmath>


#include "Dots_Connect.h"

#include "Tserial.h"
#include <conio.h>


//#include <windows.h>


#define MAX_X 1200
#define MAX_Y 600
#define MAX_NUM_OF_POINTS 100

#define SCALE_X 0.94
#define SCALE_Y 1.17

static bool paint[MAX_X][MAX_Y] = {0};
static bool visit[MAX_X][MAX_Y] = {0};


//int pointConnected[MAX_NUM_OF_POINTS] = {0};

static int pointSet[MAX_NUM_OF_POINTS][2];		//	Each points corresponds to an index
static int pair[MAX_NUM_OF_POINTS + 5][2];			//	{point1, point2}



static int numOfPointsDrawn = 0;
static int numOfPairs = 0;
static int pointSetOffset = 0;

//static int numOfConnectedPoints = 0;

static int serialFlag = false;


Tserial *com;
char buffer[20];


/*
void Flood(int x, int y) {
	if(x >= MAX_X || y >= MAX_Y) return;
	else if(paint[x][y] == 0 || visit[x][y] == 1) {
		return;
	} else {
		visit[x][y] = 1;
		Flood(x + 1, y + 1);
		Flood(x + 1, y);
		Flood(x + 1, y - 1);
		Flood(x, y + 1);
		Flood(x, y - 1);
		Flood(x - 1, y + 1);
		Flood(x - 1, y);
		Flood(x - 1, y - 1);
	}
}
*/

void Reset() {
	int i, j;
	for(i = 0;i < MAX_X;i++) {
		for(j = 0;j < MAX_Y;j++) {
			paint[i][j] = 0;
			visit[i][j] = 0;
		}
	}
	numOfPairs = 0;
	numOfPointsDrawn = 0;
	pointSetOffset = 0;
}

double GetDistance(int x1, int y1, int x2, int y2) {
	return sqrt((double)pow((double)x1 - x2, 2.0) + pow((double)y1 - y2, 2.0));
}

int Set(int x, int y) {
	if(x >= MAX_X || y >= MAX_Y) return -1;
	if(visit[x][y] == 1) return -1;

	visit[x][y] = 0;

	pointSet[numOfPointsDrawn][0] = x;
	pointSet[numOfPointsDrawn][1] = y;
	numOfPointsDrawn++;
	return 0;
}


/*
void ConnectPoints() {
	
	int i, j;
	double avgDis;
	double totalDis = 0;
	char degree[MAX_NUM_OF_POINTS] = {0};
	numOfPairs = 0;
	for(i = 0;i < numOfPoints - 1;i++) {
		int minDis = 9999999;
		for(j = 0;j < numOfPoints;j++) {
			if(i == j) continue;
			if(degree[j] == 2) continue;
			double tmp = GetDistance(pointSet[i][0], pointSet[i][1], pointSet[j][0], pointSet[j][1]);
			if(tmp <= minDis) {
				minDis = tmp;
				pair[numOfPairs][0] = i;
				pair[numOfPairs][1] = j;
			}
		}
		if(minDis != 9999999) {
			degree[pair[numOfPairs][0]]++;
			degree[pair[numOfPairs][1]]++;
			totalDis += minDis;
			numOfPairs++;
		}
	}
	
	

	avgDis = totalDis / numOfPairs;
	
	int p_index[2], count = 0;
	for(int i = 0;i < numOfPoints;i++) {
		if(count == 2) break;
		if(degree[i] == 1) {
			p_index[count++] = i;
		}
	}
	
	if(GetDistance(pointSet[p_index[0]][0], pointSet[p_index[0]][1], pointSet[p_index[1]][0], pointSet[p_index[1]][1]) < 1.5 * avgDis) {
		pair[numOfPairs][0] = p_index[0];
		pair[numOfPairs][1] = p_index[1];
		numOfPairs++;
	}
	
}
*/

/*
void CollectPoints() {
	int i, j;
	numOfPoints = 0;
	for(i = 0;i < MAX_X;i++) {
		for(j = 0;j < MAX_Y;j++) {
			if(paint[i][j] == 0 || visit[i][j] == 1) continue;
			else {
				// Flood(i, j);
				pointSet[numOfPoints][0] = i;
				pointSet[numOfPoints][1] = j;
				numOfPoints++;
			}
		}
	}
}
*/

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

void ConnectPoints() {
	
	WacomTrace("Connecting points\n");

	double totalDis = 0, avgDis;

	for(int i = pointSetOffset;i < numOfPointsDrawn - 1;i++) {
		pair[numOfPairs][0] = i;
		pair[numOfPairs][1] = i + 1;
		numOfPairs++;
		totalDis += GetDistance(pointSet[i][0], pointSet[i][1], pointSet[i + 1][0], pointSet[i + 1][1]);
	}

	avgDis = totalDis / (numOfPointsDrawn - pointSetOffset - 1);

	double last_first_dis = GetDistance(pointSet[pointSetOffset][0], pointSet[pointSetOffset][1], pointSet[numOfPointsDrawn - 1][0], pointSet[numOfPointsDrawn - 1][1]);
	if(last_first_dis < 1.5 * avgDis) {
		WacomTrace("Circle\n");
		pair[numOfPairs][0] = pointSetOffset;
		pair[numOfPairs][1] = numOfPointsDrawn - 1;
		numOfPairs++;
		
	}
	pointSetOffset = numOfPointsDrawn;



}

int Output() {
   
	//CollectPoints();
	WacomTrace("numOfPointsDrawn = %d\n", numOfPointsDrawn);
	for(int i = 0;i < numOfPointsDrawn;i++) {
		WacomTrace("(%d, %d)\n", pointSet[i][0], pointSet[i][1]);
	}
	//ConnectPoints();

	for(int i = 0;i < numOfPairs;i++) {
		WacomTrace("(%d, %d), (%d, %d)\n", pointSet[pair[i][0]][0], pointSet[pair[i][0]][1], pointSet[pair[i][1]][0], pointSet[pair[i][1]][1]);
		com->sendChar(pointSet[pair[i][0]][0] * SCALE_X);
		com->sendChar(pointSet[pair[i][0]][1] * SCALE_Y);
		com->sendChar(pointSet[pair[i][1]][0] * SCALE_X);
		com->sendChar(pointSet[pair[i][1]][1] * SCALE_Y);
		Sleep(350);
	}

	//com->sendChar(0);		//	End

	return 0;
}
