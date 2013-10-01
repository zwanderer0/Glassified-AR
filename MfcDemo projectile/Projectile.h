void Output();
int Set(int x, int y, int flag);
void Record(int x, int y);
void Reset();
void SetSerial();
int collision();

bool ballOnTheLine(int i, int flag);
double getPointLineDis(int x, int y, int lineIndex);

bool hitTheGround();
bool hitTheCeiling();
bool hitTheLeftWall();
bool hitTheRightWall();
void getAcceleration();
void getVelocity();
void getInitialVelocity();
void recoverLines(int i);

int getDisSquare(int x1, int y1, int x2, int y2);
void move();
double ABS(double x);
void normalize(double &x, double &yverticalToLine);
