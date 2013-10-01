#include "testApp.h"
#include <cstdio>
#include <string>

//--------------------------------------------------------------

#define FACE 0
#define NUM_OF_OBJECTS 10

#define ADD -10000
#define SUB -10001
#define MUL -10002
#define DIV -10003
	



FILE *myFile;

wchar_t equation[1000];
//char equation[1000];
ofSerial mySerial;
bool serialSetup = false;
bool answerReadyToSend = false;

clock_t clock_start[NUM_OF_OBJECTS], clock_current[NUM_OF_OBJECTS];

testApp::testApp(){

}

void testApp::Main() {
	for(int i = 0;i < NUM_OF_OBJECTS;i++) clock_start[i] = clock();
	
	int tmp;

	myFile = fopen("math.txt", "r");
	if(myFile == NULL) {
	  fprintf(stderr, "Can't open input file math.txt !\n");
	} else {
		fgetws(equation, 1000, myFile);
		fclose(myFile);
	}


	int length = 0;
	while(equation[length] != 0) {
		length++;
	}

	wprintf(L"%s\n", equation);
	cout << "String length: " << length << endl;

	for(int i = 0;i < length; i++) {
		wcout << i << ":" << (int)(equation[i]) << endl;
	}

	float *eqt = (float *)malloc(sizeof(float) * length);
	
	int eqtLength = 0;

	for(int i = 0;i < length;i++) {
		if((int)(equation[i]) == 43) {			//		+
			eqt[eqtLength++] = -10000;
		} else if((int)(equation[i]) == 63 || (int)(equation[i]) == 45){	//		-
			eqt[eqtLength++] = -10001;
		} else if((int)(equation[i]) == 226) {								//		-
			eqt[eqtLength++] = -10001;
			i += 2;
		} else if((int)(equation[i]) == 195 && (int)(equation[i + 1]) == 151){	//		*
			eqt[eqtLength++] = -10002;
			i++;
		} else if((int)(equation[i]) == 195 && (int)(equation[i + 1]) == 183){	//		/
			eqt[eqtLength++] = -10003;
			cout << "haha" << endl;
			i++;
		} else {
			eqt[eqtLength++] = equation[i] - '0';
		}
	}

	
	cout << endl;

	float result = cal(eqt, eqtLength);
	
	cout << "result: " << result << endl;
	
	if(mySerial.available()) {
		printf("Available.\n");
		vector<ofSerialDeviceInfo> deviceList = mySerial.getDeviceList();
		//mySerial.enumerateDevices();
		//mySerial.listDevices();
		for(int i = 0;i < deviceList.size();i++) {
			cout << deviceList[i].getDeviceName() << endl;
		}
		//mySerial.setup(deviceList[0].getDeviceName(), 9600);
		//cout << mySerial.setup("\\\\.\\"+deviceList[0].getDeviceName(), 9600);     // if success, 1, else 0.
		if(mySerial.setup("\\\\.\\"+deviceList[0].getDeviceName(), 115200)) {
			serialSetup = true;
		}
	}
	

	
	answerReadyToSend = true;
	if(answerReadyToSend) {
		char ret_string[100];
		sprintf(ret_string,"%f",result);
		
		int numOfBytesToSend;
		for(int i = 0;i < 100;i++) {
			if(ret_string[i] == '.') {
				numOfBytesToSend = i + 3;
			}
		}

		int bytesWasWritten = mySerial.writeBytes((unsigned char*)ret_string, numOfBytesToSend);
		cout << bytesWasWritten << endl;
		if ( bytesWasWritten == 0 ) {
			printf("byte was not written to serial port");
		}
		 mySerial.writeByte(0);		//	end of message
	}
	//cin >> tmp;
}

// Assume the equation is correct.
float testApp::cal(float *e, int length) {

	float a, b;
	int intLength;

	for(int i = 0;i < length;i++) {
		intLength = parseInt(e + i, a, length - i);
		i += intLength;

		if(i == length) return a;

		else {
			float op = e[i];
			i++;
			
			if(op == MUL || op == DIV) {
				intLength = parseInt(e + i, b, length - i);
				i += intLength;

				if(op == MUL) {
					e[i - 1] = a * b;
				} else {	//	DIV
					e[i - 1] = a / b;
				}
				return cal(e + i - 1, length - i + 1);

			} else if(op == ADD) {
				return a + cal(e + i, length - i);
			} else {	//	'-'
				intLength = parseInt(e + i, b, length - i);
				i += intLength;

				op = e[i];
				i++;
					
				float c;
				while(op == MUL || op == DIV) {
					intLength = parseInt(e + i, c, length - i);
					i += intLength;

					if(op == MUL) {
						b = b * c;
					} else {
						b = b / c;
					}

					op = e[i];
					i++;
				}

				e[i - 2] = a - b;
				return cal(e + i - 2, length - i + 2);
			} 
		}
	}
}

int testApp::parseInt(float *e, float &a, int length) {
	int i = 0;
	a = 0;
	while(e[i] != ADD && e[i] != SUB && e[i] != MUL && e[i] != DIV && i < length) {
		a = a * 10 + e[i];
		i++;
	}
	return i;
}
