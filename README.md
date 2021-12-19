# 설명
odroid 보드, uart, clcd를 이용한 감가산기입니다.


## 기능


### 입력키
a.숫자버튼: 숫자를 입력하고 CLCD에 표시
b.+ : 합
c.- : 차. 최초에 입력되면 minus 기호.
d.=: 연산 결과의 출력


### 조건:
1. WiringPi의 lcd library를 사용하지 말 것.
2. 입력 혹은 출력 중  넘어가면 화면이 clear 된 후 “Overflow”라는 문구를 2초간 출력 후 다시 화면이 clear되도록 할 것
3.올바르지 않은 수식 (예를 들어 1-+3와 같은 연속된 operation) 일 경우 “Invalid operation”라는 문구를 2초간 출력 후 다시 화면이 clear되도록 할 것





## 기타

사용한 하드웨어는 Odroid-C1과  clcd 입니다. 이를 통하여구현하였습니다


