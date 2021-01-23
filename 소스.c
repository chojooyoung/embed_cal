#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <wiringPi.h>

#define LED1 12
#define LED2 13
#define LED3 14
#define LED4 21
#define LED5 22
#define LED6 23
#define LED7 24
#define LED8 10
#define LED9 11
#define LED0 26
#define LEDRs 27
#define LEDPlus 5
#define LEDMinus 6

#define LCD_D4 2
#define LCD_D5 3
#define LCD_D6 1
#define LCD_D7 4
#define LCD_RS 7
#define LCD_EN 0
void write4bits(unsigned char command)
{
	digitalWrite(LCD_D4, (command & 1));
	command >>= 1;
	digitalWrite(LCD_D5, (command & 1));
	command >>= 1;
	digitalWrite(LCD_D6, (command & 1));
	command >>= 1;
	digitalWrite(LCD_D7, (command & 1));
	digitalWrite(LCD_EN, 1);
	delayMicroseconds(10);
	digitalWrite(LCD_EN, 0);
	delayMicroseconds(10);
}
void sendDataCmd4(unsigned char data)
{
	write4bits(((data >> 4) & 0x0f));
	write4bits((data & 0x0f));
	delayMicroseconds(100);
}

void putCmd4(unsigned char cmd)
{
	digitalWrite(LCD_RS, 0);
	sendDataCmd4(cmd);
}

void putChar(char c) {
	digitalWrite(LCD_RS, 1);
	sendDataCmd4(c);
}
void initialize_textlcd() {
	pinMode(LCD_RS, OUTPUT); pinMode(LCD_EN, OUTPUT);
	pinMode(LCD_D4, OUTPUT); pinMode(LCD_D5, OUTPUT);
	pinMode(LCD_D6, OUTPUT); pinMode(LCD_D7, OUTPUT);
	digitalWrite(LCD_RS, 0); digitalWrite(LCD_EN, 0);
	digitalWrite(LCD_D4, 0); digitalWrite(LCD_D5, 0);
	digitalWrite(LCD_D6, 0);  digitalWrite(LCD_D7, 0);
	delay(35);

	putCmd4(0x28);
	putCmd4(0x28); putCmd4(0x28);
	putCmd4(0x0e);
	delay(2);
	putCmd4(0x01);
	delay(2);
}
void num_check()
{
	pinMode(LED1, INPUT);   pullUpDnControl(LED1, PUD_UP);
	pinMode(LED2, INPUT);   pullUpDnControl(LED2, PUD_UP);
	pinMode(LED3, INPUT);   pullUpDnControl(LED3, PUD_UP);
	pinMode(LED4, INPUT);   pullUpDnControl(LED4, PUD_UP);
	pinMode(LED5, INPUT);   pullUpDnControl(LED5, PUD_UP);
	pinMode(LED6, INPUT);   pullUpDnControl(LED6, PUD_UP);
	pinMode(LED7, INPUT);   pullUpDnControl(LED7, PUD_UP);
	pinMode(LED8, INPUT);   pullUpDnControl(LED8, PUD_UP);
	pinMode(LED9, INPUT);   pullUpDnControl(LED9, PUD_UP);
	pinMode(LED0, INPUT);   pullUpDnControl(LED0, PUD_UP);
	pinMode(LEDRs, INPUT);   pullUpDnControl(LEDRs, PUD_UP);
	pinMode(LEDPlus, INPUT);   pullUpDnControl(LEDPlus, PUD_UP);
	pinMode(LEDMinus, INPUT);   pullUpDnControl(LEDMinus, PUD_UP);
}
int oper(char buf[], int len)//결과값 출력함수
{
	int tmp[80] = { 0, };//입력받은 숫자가 들어갈 배열
	char op[80] = { 0, };//연산자들이 들어갈 배열
	int idx = 0, opidx = 0;
	int i = 0;
	int num = 0;
	for (i = 0; i < len; i++) {
		if ('0' <= buf[i] && buf[i] <= '9') {//받은 배열buf검사중 숫자가나왓다면
			num *= 10;//숫자가 또나왓다면 기존 num에 10을곱해줌(자릿수고려) but 처음에는 num이 0이기때문에 그냥 들어감
			num += buf[i] - '0';//곱한 num값에 나온 숫자를 더해준다.
		}
		else if (buf[i] == '-' || buf[i] == '+' || buf[i] == '=') {//연산자라면
			tmp[idx++] = num;//지금까지의 기존 num을 tmp에 넣고 
			num = 0;//초기화
			op[opidx++] = buf[i];//연산자를 넣어준다
		}
	}//반복이 끝나면
	int result = tmp[0];//처음 숫자를 결과값에 넣고
	for (i = 0; i < opidx; i++) {//연산자를 검사
		if (op[i] == '-')//-가 나왓다면 
			result = result - tmp[i + 1];//기존숫자에서 다음 숫자를 빼준다
		else if (op[i] == '+')//+가 나왓다면
			result = result + tmp[i + 1];//더해준다
	}
	return result;//반복이 끝난 최종 값을 리턴
}

int main(int argc, char** argv) {
	int button[13] = { LED0, LED1, LED2, LED3, LED4, LED5, LED6, LED7, LED8, LED9, LEDRs,LEDPlus,LEDMinus };//버튼들의 배열선언
	char buf[80] = "\0";//피연산자,연산자를 담을곳
	int len = 0;//buf 배열의 길이. for문을 위함
	wiringPiSetup();
	initialize_textlcd();//초기화
	num_check();//인풋선언
	char cio[] = "Invalid operation"; //오류메세지
	int cion = strlen(cio);
	char cov[] = "over flow"; //오류메세지
	int covn = strlen(cov);
	int i = 0;//반복문 변수
	int j = 0;//반복문 변수
	int rsop = 0;//결과 출력후 바로 계산하기 위한 변수
	int flag = 0;//반복을 위한 변수
	int x = 1;//input으로 받아오기 때문에 digitalRead값에 &연산을할 변수
	int size = 0;//버퍼의 크기 할당할 변수
	int  update = 0;// '=' 버튼이 눌리고 다시 다른입력을 받을때 화면 초기화 조건을 위한 변수 
	int oc = 0; //overflow 오류체크를 위한 변수
	int ic = 0;//invalid operation 오류체크를 위한 변수
	while (1) {
		x = 1;
		if (size == 17) {//17이넘어가면
			putCmd4(0xC0);//밑으로내려간다
		}
		if (size > 32) {//두줄이 넘어가면
			initialize_textlcd();//초기화후
			for (i = 0;i < covn;i++) {
				putChar(cov[i]);//메세지출력

			}
			delay(2000);
			initialize_textlcd();
			memset(buf, '0', sizeof(buf));//버퍼 0으로 초기화
			update = 0; rsop = 0;//변수들 초기화

			size = 0;//그동안에 쌓아온 받아온 버퍼 크기 초기화
		}
		for (i = 0; i < 13; i++) {//
			if ((x &= digitalRead(button[i])) == LOW && !flag && i < 10) {//digitalRead값이 1과 &연산 했을때 0이면(LOW)면(버튼이 눌리면) 그리고 그 배열의 주소가 10보다 작다면
				if (update == 1 && rsop == 1) {   //결과 값이 있으면 
					initialize_textlcd(); memset(buf, '0', sizeof(buf));//초기화
					update = 0; rsop = 0;//변수도 초기화
					size = 0;
				}
				ic = 0;//숫자가오면 연산자오류는 안나니까 0
				putChar(i + '0');  size++;//눌린 배열의 인덱스를 화면에 출력
				buf[len++] = i + '0';
				flag = 1;//반복
			}
			else if ((x &= digitalRead(button[i])) == LOW && !flag && i == 10) {//'='이 입력되면
				putChar('='); flag = 1;  size++; buf[len++] = '=';
				char rs[80];//결과값을 넣을 배열
				int res = oper(buf, len);//결과값넣을 변수
				sprintf(rs, "%d", res);// (int)les를 rs배열에 %d(서식지정int)을 넣어준다
				int p = strlen(rs); len = 0; 
				memset(buf, '0', sizeof(buf));
				for (j = 0; j < p; j++) {//출력을 위한 반복문
					if (size == 17) {
						putCmd4(0xC0);
					}
					if (size > 32) {
						initialize_textlcd();
						for (i = 0;i < covn;i++) {
							putChar(cov[i]);
						}
						initialize_textlcd();
						memset(buf, '0', sizeof(buf));
						update = 0; rsop = 0;
					}
					putChar(rs[j]); size++; buf[len++] = rs[j]; //결과값 출력
				}
				update = 1;
				rsop = 1;
				ic = 0;
				oc = 0;
			}
			else if ((x &= digitalRead(button[i])) == LOW && !flag && i == 11) { //+가 눌리면
				if (ic == 1) {//기존에 눌려져있던게 연산자라면
					initialize_textlcd();
					for (i = 0;i < cion;i++) {
						putChar(cio[i]);
					}
					memset(buf, '0', sizeof(buf));
					delay(2000);
					initialize_textlcd();//오류메세지 띄운후 초기화
					update = 0; rsop = 0; ic = 0;
				}
				else {
					if (rsop == 1) {//기존에 결과값이 있다면
						putChar('+'); flag = 1;  size++;  buf[len++] = '+';
						rsop = 0;
						ic = 1;
					}
					else {//없다면
						putChar('+'); flag = 1;  size++;  buf[len++] = '+';

						ic = 1;
					}
				}
			}
			else if ((x &= digitalRead(button[i])) == LOW && !flag && i == 12) {//-가 눌렷다면
				if (ic == 1) {
					initialize_textlcd();
					for (i = 0;i < cion;i++) {

						putChar(cio[i]);
					}
					memset(buf, '0', sizeof(buf));
					update = 0; rsop = 0;
					delay(2000);
					initialize_textlcd();
					ic = 0;
				}

				else {
					if (rsop == 1) {
						putChar('-'); flag = 1;  size++;  buf[len++] = '-';
						rsop = 0;
						ic = 1;
					}
					else {
						putChar('-'); flag = 1;  size++;  buf[len++] = '-';
						ic = 1;
					}
				}
			}
			else continue;
		}
		delay(10);
		if (x && flag)
			flag = 0;
	}
}
