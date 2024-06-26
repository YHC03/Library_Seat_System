# 도서관 열람실 좌석배정 시스템

#### 사용 언어
C

###### 사용 라이브러리(헤더 파일)
stdio, string, time

---
## 작동 설명

#### 1. 메인 페이지
0을 입력하면 관리 페이지로 진입함.  
그 외의 20자 이내의 문자(이하 이용자명)를 입력하면 좌석배정 모드에 진입함.  

---
#### 2. 좌석배정 페이지
###### 진입 요건
좌석을 이용중인 이용자명이 아닌, 새로운 이용자명이 입력된 경우  

###### 작동 방법
모든 좌석의 사용여부가 나타남.  
이용중인 좌석을 선택 시, 이미 이용중인 좌석으로 표기됨.  
이용불가 좌석을 선택 시, 이용불가 좌석으로 표기됨.  
빈 좌석 선택 시, 해당 좌석으로 배정됨.  
배정 시, 연장가능시각과 이용종료시각이 나타남. 연장가능시각과 이용종료시각은 관리자 설정을 기준으로 하며, 폐장시각을 넘지 않음.  

###### 좌석이 만석인 경우

'만석입니다' 문구가 나오며 좌석 배정 과정이 취소됨.  

---
#### 3. 연장 및 퇴실 페이지
###### 진입 요건
좌석을 이용중인 이용자명이 입력된 경우  

###### 작동 방법
연장가능시각과 이용종료시각이 나타남.  
만일, 연장이 가능하다면 연장 기능이 활성화됨.  
이용자는 연장, 퇴실, 취소 기능을 이용할 수 있음.  

---
#### 4. 관리 페이지
###### 진입 요건
이용자명에 0이 입력된 경우  

###### 기능
1. 모든 좌석 초기화  
2. 이용가능시간 설정(기본: 4시간)  
3. 연장가능시간 설정(기본: 끝나기 30분 전)  
4. 개장시각 설정(기본: 23시 59분)  
5. 폐장시각 설정(기본: 23시 59분)  
6. 모든 좌석의 이용자명 보기  
7. 좌석 이용불가 설정(기본: 모든 좌석 이용 가능)  

---
#### 5. 자동 설정
1. 퇴실시각이 지난 이용자의 경우 자동으로 퇴실되며, 이는 빈 자리가 됨.  
2. 폐장시각이 지난 경우, 자동으로 퇴실되며, 이는 빈 자리가 됨.  
3. 운영시간이 아닌 경우, 운영시간이 아니라는 메시지와 함께 좌석배정을 거부함.  
4. 관리자가 폐장시각을 변경한 경우, 이용자의 퇴실시각이 새로운 폐장시각보다 늦어지게 되면 해당 이용자의 퇴실시각을 폐장시각으로 일괄 자동 조정함.  
5. 이용불가 설정 시, 해당 좌석을 이용중인 이용자는 자동 퇴실 처리됨.  

---
## 파일 내 주요 상수 소개
SEATS 상수는 열람실 내 좌석의 수(기본값 10) 입니다.  
MAX_NAME_LENGTH 상수는 이용자명 문자열의 최대 길이입니다.  

---
작성자 : YHC03  
최종 작성일 : 2024/04/30  