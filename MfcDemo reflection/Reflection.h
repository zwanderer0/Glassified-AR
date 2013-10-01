void Output();
int Set(int x, int y, int flag);
void Flood(int x, int y);
void ConnectPoints();
void CollectPoints();
void Reset();
void SetSerial();

void getDir();
int collision();
double getPointLineDis(int x, int y, int lineIndex);
int getDisSquare(int x1, int y1, int x2, int y2);
void move();
double ABS(double x);
bool ballOnTheLine(int i, int flag);
void normalized(double &x, double &yverticalToLine);