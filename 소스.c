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
int oper(char buf[], int len)//����� ����Լ�
{
	int tmp[80] = { 0, };//�Է¹��� ���ڰ� �� �迭
	char op[80] = { 0, };//�����ڵ��� �� �迭
	int idx = 0, opidx = 0;
	int i = 0;
	int num = 0;
	for (i = 0; i < len; i++) {
		if ('0' <= buf[i] && buf[i] <= '9') {//���� �迭buf�˻��� ���ڰ����Ӵٸ�
			num *= 10;//���ڰ� �ǳ��Ӵٸ� ���� num�� 10��������(�ڸ������) but ó������ num�� 0�̱⶧���� �׳� ��
			num += buf[i] - '0';//���� num���� ���� ���ڸ� �����ش�.
		}
		else if (buf[i] == '-' || buf[i] == '+' || buf[i] == '=') {//�����ڶ��
			tmp[idx++] = num;//���ݱ����� ���� num�� tmp�� �ְ� 
			num = 0;//�ʱ�ȭ
			op[opidx++] = buf[i];//�����ڸ� �־��ش�
		}
	}//�ݺ��� ������
	int result = tmp[0];//ó�� ���ڸ� ������� �ְ�
	for (i = 0; i < opidx; i++) {//�����ڸ� �˻�
		if (op[i] == '-')//-�� ���Ӵٸ� 
			result = result - tmp[i + 1];//�������ڿ��� ���� ���ڸ� ���ش�
		else if (op[i] == '+')//+�� ���Ӵٸ�
			result = result + tmp[i + 1];//�����ش�
	}
	return result;//�ݺ��� ���� ���� ���� ����
}

int main(int argc, char** argv) {
	int button[13] = { LED0, LED1, LED2, LED3, LED4, LED5, LED6, LED7, LED8, LED9, LEDRs,LEDPlus,LEDMinus };//��ư���� �迭����
	char buf[80] = "\0";//�ǿ�����,�����ڸ� ������
	int len = 0;//buf �迭�� ����. for���� ����
	wiringPiSetup();
	initialize_textlcd();//�ʱ�ȭ
	num_check();//��ǲ����
	char cio[] = "Invalid operation"; //�����޼���
	int cion = strlen(cio);
	char cov[] = "over flow"; //�����޼���
	int covn = strlen(cov);
	int i = 0;//�ݺ��� ����
	int j = 0;//�ݺ��� ����
	int rsop = 0;//��� ����� �ٷ� ����ϱ� ���� ����
	int flag = 0;//�ݺ��� ���� ����
	int x = 1;//input���� �޾ƿ��� ������ digitalRead���� &�������� ����
	int size = 0;//������ ũ�� �Ҵ��� ����
	int  update = 0;// '=' ��ư�� ������ �ٽ� �ٸ��Է��� ������ ȭ�� �ʱ�ȭ ������ ���� ���� 
	int oc = 0; //overflow ����üũ�� ���� ����
	int ic = 0;//invalid operation ����üũ�� ���� ����
	while (1) {
		x = 1;
		if (size == 17) {//17�̳Ѿ��
			putCmd4(0xC0);//�����γ�������
		}
		if (size > 32) {//������ �Ѿ��
			initialize_textlcd();//�ʱ�ȭ��
			for (i = 0;i < covn;i++) {
				putChar(cov[i]);//�޼������

			}
			delay(2000);
			initialize_textlcd();
			memset(buf, '0', sizeof(buf));//���� 0���� �ʱ�ȭ
			update = 0; rsop = 0;//������ �ʱ�ȭ

			size = 0;//�׵��ȿ� �׾ƿ� �޾ƿ� ���� ũ�� �ʱ�ȭ
		}
		for (i = 0; i < 13; i++) {//
			if ((x &= digitalRead(button[i])) == LOW && !flag && i < 10) {//digitalRead���� 1�� &���� ������ 0�̸�(LOW)��(��ư�� ������) �׸��� �� �迭�� �ּҰ� 10���� �۴ٸ�
				if (update == 1 && rsop == 1) {   //��� ���� ������ 
					initialize_textlcd(); memset(buf, '0', sizeof(buf));//�ʱ�ȭ
					update = 0; rsop = 0;//������ �ʱ�ȭ
					size = 0;
				}
				ic = 0;//���ڰ����� �����ڿ����� �ȳ��ϱ� 0
				putChar(i + '0');  size++;//���� �迭�� �ε����� ȭ�鿡 ���
				buf[len++] = i + '0';
				flag = 1;//�ݺ�
			}
			else if ((x &= digitalRead(button[i])) == LOW && !flag && i == 10) {//'='�� �ԷµǸ�
				putChar('='); flag = 1;  size++; buf[len++] = '=';
				char rs[80];//������� ���� �迭
				int res = oper(buf, len);//��������� ����
				sprintf(rs, "%d", res);// (int)les�� rs�迭�� %d(��������int)�� �־��ش�
				int p = strlen(rs); len = 0; 
				memset(buf, '0', sizeof(buf));
				for (j = 0; j < p; j++) {//����� ���� �ݺ���
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
					putChar(rs[j]); size++; buf[len++] = rs[j]; //����� ���
				}
				update = 1;
				rsop = 1;
				ic = 0;
				oc = 0;
			}
			else if ((x &= digitalRead(button[i])) == LOW && !flag && i == 11) { //+�� ������
				if (ic == 1) {//������ �������ִ��� �����ڶ��
					initialize_textlcd();
					for (i = 0;i < cion;i++) {
						putChar(cio[i]);
					}
					memset(buf, '0', sizeof(buf));
					delay(2000);
					initialize_textlcd();//�����޼��� ����� �ʱ�ȭ
					update = 0; rsop = 0; ic = 0;
				}
				else {
					if (rsop == 1) {//������ ������� �ִٸ�
						putChar('+'); flag = 1;  size++;  buf[len++] = '+';
						rsop = 0;
						ic = 1;
					}
					else {//���ٸ�
						putChar('+'); flag = 1;  size++;  buf[len++] = '+';

						ic = 1;
					}
				}
			}
			else if ((x &= digitalRead(button[i])) == LOW && !flag && i == 12) {//-�� ���Ǵٸ�
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
