#include <stdio.h>
#include <string.h>
#include <time.h>

/* 열람실 좌석관리 시스템
*
* 이용 방법
* 이용자명 입력 -> 좌석 선택 -> 이용시간 부여
* 이미 좌석이 있는 이용자의 경우 좌석정보와 연장가능여부 확인 -> 연장/퇴실/취소 선택
* 종료시각까지만 좌석 부여 후 자동 좌석 퇴실 처리
*
* 작성자 : YHC03
* 작성일 : 2024/4/25-2024/5/1
*/


#define SEATS 10 // 좌석 수 설정
#define MAX_NAME_LENGTH 20 // 이용자명의 최대 길이 설정

// 좌석 정보를 저장하는 구조체 생성
typedef struct seatsData
{
    char seatsName[MAX_NAME_LENGTH]; // 좌석 이용자명 기록, ""(strlen=0)이면 빈 좌석
    long long int endTime; // 이용 종료시각 기록(Unix 시간) - 초 단위, 해당 값이 0인 경우 빈 좌석, -1인 경우 이용불가 좌석
} SeatsData;

// 열람실 운영정보를 저장하는 구조체 생성
typedef struct libraryData
{
    int MAX_TIME; // 최대 이용 시간(분)
    int MAX_RENEWABLE_TIME; // 좌석 연장 가능 시간(분) (횟수제한 없음. 반납 직후 재발급받으면 그만이라.)
    int OPEN_TIME; // 열람실 개장 시간(시, 분)-분으로 환산
    int CLOSE_TIME; // 열람실 폐장 시간(시, 분)-분으로 환산
} LibraryData;



// 함수 목록

// 초기화 함수
void init(SeatsData* libSeats);

// 관리자 모드
void adminMode(SeatsData* libSeats, LibraryData* libData);

// 메뉴 선택 함수
void menuSelect(char* tmp);

// 좌석 배정 시스템 함수
void seatSelector(char* tmpName, SeatsData* libSeats, LibraryData* libData);

// 좌석 배정, 연장 및 퇴실 함수
void setSeat(char* tmpName, int location, SeatsData* libSeats, LibraryData* libData); // 좌석 배정
void renewSeat(int location, SeatsData* libSeats, LibraryData* libData); // 좌석 연장
void checkOut(int location, SeatsData* libSeats); // 퇴실

// 좌석 정보 출력 함수
void printSeatInfo(SeatsData* libSeats, int isMaster); // 좌석 정보 출력
void printRenewTime(int location, SeatsData* libSeats, LibraryData* libData); // 연장가능시각 출력
void printEndTime(int location, SeatsData* libSeats); // 이용종료시각 출력

// 관리 함수
void seatInvalidCheck(SeatsData* libSeats); // 이용종료시간이 지난 좌석 자동 회수
void resetSeats(SeatsData* libSeats); // 모든좌석 초기화
void renewSeatEndTime(SeatsData* libSeats, LibraryData* libData); // 폐장시각 변경 시 이용종료시각 조정

// 보조 함수
int findUser(char* tmpName, SeatsData* libSeats); // 이용자가 이용중인 좌석번호 찾기
int isFull(SeatsData* libSeats); // 열람실이 가득찼는지 확인
int isRenewable(int location, SeatsData* libSeats, LibraryData* libData); // 연장가능시각이 지났는지 확인
int leftSeconds(LibraryData* libData); // 이용 종료까지 남은 시간(초) 확인

// 함수 목록 끝



/*
* resetSeats 함수
* 기능 : 모든 좌석을 초기화함. 최초 실행시에는 모든 좌석을 초기화하며, 이후에는 이용불가 좌석을 제외한 모든 좌석을 초기화함.
* 입력값 : 좌석 정보 구조체 포인터 *libSeats, isFirst(최초 실행 여부를 나타내는 변수이며, 1인 경우 최초 실행, 0인 경우 최초 실행이 아님)
* 반환값 없음.
* 설명 최종 수정 일자 : 2024/04/30
*/
void resetSeats(SeatsData* libSeats, int isFirst)
{
    // 모든 좌석에 대해 실행
    for (int i = 0; i < SEATS; i++)
    {
        // 좌석 이용자명 초기화
        strncpy((libSeats + i)->seatsName, "", MAX_NAME_LENGTH);

        if (isFirst)
        {
            // 첫 실행인 경우에는 이용종료시각을 모두 0, 즉 이용가능상태로 초기화
            (libSeats + i)->endTime = 0;

        }else if ((libSeats + i)->endTime > 0){ // 함수의 최초 실행이 아니며, 이용불가 상태(endTime = -1)의 좌석이 아닌 경우

            // 최초 실행이 아닌 경우에는, 이용불가 좌석을 제외한 좌석의 이용종료시각을 모두 0으로 초기화
            (libSeats + i)->endTime = 0;
        }
    }

    return;
}


/*
* printEndTime 함수
* 기능 : 주어진 좌석번호의 종료시각을 출력함.
* 입력값 : location(0번부터 시작하는 좌석번호), 좌석 정보 구조체 포인터 *libSeats
* 반환값 없음
* 설명 최종 수정 일자 : 2024/04/30
*/
void printEndTime(int location, SeatsData* libSeats)
{
    // 현재 시각 관련 변수 선언
    time_t Time;
    struct tm* pTime;
    Time = time(NULL);
    pTime = localtime(&Time);

    // 해당 좌석의 이용자가 없는 경우, 즉 빈좌석인 경우 내용을 출력하지 않음.
    if (!strlen((libSeats + location)->seatsName)) // 빈 좌석의 이용자 이름의 길이는 0이다.
    {
        return; // 함수 종료
    }

    // 남은 시간 계산
    // 종료 시각(Unix 초) - 현재 시각(Unix 초)의 방법으로 계산하며, 결괏값은 초 단위이다.
    int remainSecondOrig = (libSeats + location)->endTime - ((long long int)Time);
    int remainHour = remainSecondOrig / 3600;
    int remainMinute = (remainSecondOrig % 3600) / 60;
    int remainSecond = remainSecondOrig % 60;

    // 현재 시각 가져오기
    pTime = localtime(&Time);
    int currHour = pTime->tm_hour;
    int currMin = pTime->tm_min;
    int currSecond = pTime->tm_sec;

    // 종료 시각 계산
    // 현재 시각에 남은 시간을 더한다.
    int computeSecond = remainSecond + currSecond;
    int computeMinute = remainMinute + currMin;
    int computeHour = remainHour + currHour;


    // 종료 시각의 초단위 값이 60 이상인 경우, 이를 조절한다.
    if (computeSecond >= 60)
    {
        computeMinute += computeSecond / 60; // 초단위 값 초과분 만큼 분단위 값을 증가시킨다.
        computeSecond %= 60; // 초단위 값은 60 미만의 값만을 처리한다.
    }

    // 종료 시각의 분단위 값이 60 이상인 경우, 이를 조절한다.
    if (computeMinute >= 60)
    {
        computeHour += computeMinute / 60; // 분단위 값 초과분 만큼 시단위 값을 증가시킨다.
        computeMinute %= 60; // 분단위 값은 60 미만의 값만을 처리한다.
    }

    // 종료 시각 문자 출력
    printf("종료 시각 : ");

    // 종료 시각의 시단위 값이 24 이상인 경우, 이를 조절한다.
    if (computeHour >= 24)
    {
        computeHour %= 24; // 시단위 값은 24 미만의 값만을 처리한다.

        // 이 경우 다음날까지 이용하는 것이므로, 다음날까지 이용하는 것이라고 출력한다.
        printf("익일 "); // 최대 좌석 이용 시간은 24시간이므로, 2일 이후까지 이용하는 경우는 없다.
    }

    // 남은 종료 시각 정보 출력
    printf("%d시 %d분 %d초\n", computeHour, computeMinute, computeSecond);

    return;
}


/*
* printSeatInfo 함수
* 기능 : 모든 좌석의 이용정보를 출력한다. 관리자의 경우, 이용중인 좌석에는 이용자명을 출력한다.
* 입력값 : 좌석 정보 구조체 포인터 *libSeats, isMaster(관리자 모드 여부를 나타내는 변수이며, 1인 경우 관리자 모드, 0인 경우 이용자 모드이다)
* 반환값 없음
* 설명 최종 수정 일자 : 2024/04/30
*/
void printSeatInfo(SeatsData* libSeats, int isMaster)
{
    // 출력할 이용 정보를 저장하는 변수
    char info[30] = "";
    for (int i = 0; i < SEATS; i++)
    {
        // 좌석의 이용가능여부 판단
        if ((libSeats + i)->endTime == -1) // 이용불가 좌석의 경우
        {
            // 이용불가 출력
            strcpy(info, "Unavailable");

        }else{ // 이용가능 좌석의 경우

            // 빈 좌석 여부 판단
            if ((libSeats + i)->endTime) // 빈 좌석(endtime == 0)이 아닌 경우
            {
                // 관리자 여부 확인
                if (isMaster) // 관리자인 경우
                {
                    // 이용자의 이름 출력
                    strcpy(info, "User ");
                    strcat(info, (libSeats + i)->seatsName);
                }else{
                    // 이용중인 좌석임을 출력
                    strcpy(info, "Used");
                }
            }else{ // 빈 좌석(endtime == 0)인 경우
                // 빈 좌석임을 출력
                strcpy(info, "Empty");
            }
        }
        
        // 좌석 정보 출력
        printf("%d번 좌석: %s\n", i + 1, info); // 이용자에게 표시되는 좌석정보는 1부터 시작하는 좌석번호이므로, 0부터 시작하는 좌석번호에 1을 더한다.
    }

    return;
}


/*
* renewSeatEndTime 함수
* 기능 : 폐장시각이 바뀌어 이용종료시각이 폐장시각 이후가 된 경우 이를 폐장시각으로 조정한다.
* 입력값 : 좌석 정보 구조체 포인터 *libSeats, 시설 정보 구조체 포인터 *libData
* 반환값 없음
* 설명 최종 수정 일자 : 2024/04/30
*/
void renewSeatEndTime(SeatsData* libSeats, LibraryData* libData)
{
    // 현재 시각 관련 변수 선언
    time_t Time;
    struct tm* pTime;
    Time = time(NULL);
    pTime = localtime(&Time);

    // 임시로 시간 정보를 저장하는 변수 선언. 이 변수의 최댓값은 86400(1일의 초 단위)임.
    int tmpTime = 0;

    // 24시간제의 경우 해당 함수가 필요 없으므로 함수 종료.
    if (libData->OPEN_TIME == libData->CLOSE_TIME) { return; }

    // 모든 좌석에 대해 반복
    for (int i = 0; i < SEATS; i++)
    {
        // 좌석 이용불가 상태 건너뛰기
        if (((libSeats + i)->endTime) <= 0) // endTime <= 0, 즉 좌석 미배정 혹은 이용불가 상태인 경우
        {
            i++; // 다음 좌석으로 이동
            if (i >= SEATS) { return; } // 모든 좌석을 확인한 경우, 함수 종료.
        }

        // 좌석의 남은 시간을 임시로 저장함.
        tmpTime = (libSeats + i)->endTime - (long long int)(Time);

        // 개장시각과 폐장시각에 따라 구분
        // 24시간제의 경우, 폐장시각으로 인해 연장 불가능한 경우가 없으으로 해당 조건문에서 구분하지 않음.
        if (libData->OPEN_TIME < libData->CLOSE_TIME) // 개장시각이 폐장시각보다 앞에 있는 경우
        {
            // 개인별 종료 시각(Unix 초) - 현재 시각(Unix 초) = 남은 시간(초) > 폐장 시각(초) - 현재 시각(초) = 남은 시각(초) 인 경우, 이용종료시각 조정이 필요함

            // 폐장시각이 변경되어 퇴실시각이 폐장시각 이후가 된 경우
            if (((libSeats + i)->endTime - ((long long int)Time)) >= ((libData->CLOSE_TIME * 60) - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec)))
            {
                // 개인별 종료 시각(Unix 초)을 현재 시각(Unix 초) + 폐장시각까지 남은 시각(초) 으로 변경
                (libSeats + i)->endTime = (long long int)Time + (libData->CLOSE_TIME * 60) - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec);
            }

        }else if (libData->OPEN_TIME > libData->CLOSE_TIME){ // 개장시각이 폐장시각보다 뒤에 있는 경우

            if (((libData->OPEN_TIME * 60) - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec)) >= 0) // 개장시각이 폐장시각보다 뒤에 있으며, 00시를 지난 경우 (개장 시각 이후)
            {
                // 개인별 종료 시각(Unix 초) - 현재 시각(Unix 초) = 남은 시간(초) > 폐장 시각(초) - 현재 시각(초) = 남은 시각(초) 인 경우, 이용종료시각 조정이 필요함

                // 폐장시각이 변경되어 퇴실시각이 폐장시각 이후가 된 경우
                if (((libSeats + i)->endTime - ((long long int)Time)) > ((libData->CLOSE_TIME * 60) - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec)))
                {
                    // 개인별 종료 시각(Unix 초)을 현재 시각(Unix 초) + 폐장시각까지 남은 시각(초) 으로 변경
                    (libSeats + i)->endTime = (long long int)Time + (libData->CLOSE_TIME * 60) - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec);
                }

            }else{ // 개장시각이 폐장시각보다 뒤에 있으며, 00시를 지나지 않은 경우 (개장 시각 이전)

                // 개인별 종료 시각(Unix 초) - 현재 시각(Unix 초) = 남은 시간(초) > 폐장 시각(초) + 1일(86400초) - 현재 시각(초) = 남은 시각(초) 인 경우, 이용종료시각 조정이 필요함

                // 폐장시각이 변경되어 퇴실시각이 폐장시각 이후가 된 경우
                if (((libSeats + i)->endTime - ((long long int)Time)) > ((libData->CLOSE_TIME * 60) + 24 * 60 * 60 - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec)))
                {
                    // 개인별 종료 시각(Unix 초)을 현재 시각(Unix 초) + 폐장시각까지 남은 시각(초) 으로 변경
                    (libSeats + i)->endTime = (long long int)Time + (libData->CLOSE_TIME * 60) - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec);
                }
            }

        }
    }

    return;
}


/*
* adminMode 함수
* 기능 : 관리자 모드 실행
* 입력값 : 좌석 정보 구조체 포인터 *libSeats, 시설 정보 구조체 포인터 *libData
* 반환값 없음
* 설명 최종 수정 일자 : 2024/04/30
*/
void adminMode(SeatsData* libSeats, LibraryData *libData)
{
    /*
    * 관리자 모드 관련 변수 선언
    * 
    * menu_sel : 입력한 메뉴의 번호
    * tmpTime : 입력한 시간값을 임시로 저장하는 변수
    * oldData : 잘못 입력할 것을 대비해, 기존 시간값을 임시로 저장하는 변수
    * tmpSeatNo : 좌석 이용불가 설정에서, 좌석 이용불가 설정을 바꿀 좌석번호를 임시로 저장하는 변수
    */
    int menu_sel = -1, tmpTime = 0, oldData = 0, tmpSeatNo = 0;

    // 무한 반복, 관리자 모드 종료(0 입력)시 return으로 함수 종료
    while (1)
    {
        /*
        * 관리자 모드의 메뉴
        * 
        * 1 : 좌석 초기화
        * 2 : 최대 이용 가능 시간 수정
        * 3 : 연장 가능 시간 수정
        * 4 : 개장시각 수정
        * 5 : 폐장시각 수정
        * 6 : 모든 좌석 정보 보기
        * 7 : 좌석 이용불가 설정
        * 0 : 관리자 모드 나가기
        */

        // 관리자 모드의 메뉴 출력 및 입력값 입력
        printf("1 : 좌석 초기화, 2 : 최대 이용 가능 시간 수정, 3: 연장 가능 시간 수정, 4 : 개장시각 수정, 5: 폐장시각 수정, 6: 모든 좌석 정보 보기, 7: 좌석 이용불가 설정, 0: 나가기 : ");
        scanf("%d", &menu_sel);

        // 선택한 관리자 모드의 메뉴 실행
        switch (menu_sel)
        {
        case 1: // 좌석 초기화

            // 이용불가좌석을 제외한 모든 좌석을 초기화함.
            resetSeats(libSeats, 0);

            break;

        case 2: // 최대 이용 가능 시간 수정

            // 기존 최대이용시간 저장
            oldData = libData->MAX_TIME;

            do {
                // 기존 최대 이용 가능 시간 복원. 이는 잘못된 값을 입력받았을 때 복원하는 용도로 이용된다.
                libData->MAX_TIME = oldData;

                // 기존값 출력 및 신규값 입력
                printf("현재 최대 이용 가능 시간 : %d 시간 %d 분\n", libData->MAX_TIME / 60, libData->MAX_TIME % 60);
                printf("최대 이용 가능 시간 수정. 시간 : ");
                scanf("%d", &tmpTime); // 시간 단위 입력
                libData->MAX_TIME = tmpTime * 60; // 시간 단위를 분단위로 환산 후 저장
                printf("분 : ");
                scanf("%d", &tmpTime); // 분단위 입력
                libData->MAX_TIME += tmpTime; // 저장된 시간 단위 입력값에 분단위 입력값 추가

                // 잘못된 값인지 확인
                if (libData->MAX_TIME <= 0 || libData->MAX_TIME > 24 * 60)
                {
                    printf("최대 이용 가능 시간은 1분 이상 24시간 이하여야 합니다.\n");
                }
            } while (libData->MAX_TIME <= 0 || libData->MAX_TIME > 24 * 60); // 옳은 입력값이 입력될때까지 반복

            // 새 최대 이용가능 시간이 최대 연장가능 시간을 초과하는 경우
            if (libData->MAX_TIME < libData->MAX_RENEWABLE_TIME)
            {
                // 최대 연장가능 시간을 새 최대 이용가능 시간으로 수정
                libData->MAX_RENEWABLE_TIME = libData->MAX_TIME;

                // 최대 연장가능 시간을 새 최대 이용가능 시간으로 수정하였음을 알림
                printf("최대 이용 가능시간 변경으로 인해 연장 가능 시간이 끝나기 %d 시간 %d 분 전으로 변경되었습니다.\n", libData->MAX_RENEWABLE_TIME / 60, libData->MAX_RENEWABLE_TIME % 60);
            }

            break;

        case 3: // 연장 가능 시간 수정

            // 기존 연장 가능 시간 저장
            oldData = libData->MAX_RENEWABLE_TIME;

            do {
                // 기존 연장 가능 시간 복원. 이는 잘못된 값을 입력받았을 때 복원하는 용도로 이용된다.
                libData->MAX_RENEWABLE_TIME = oldData;

                // 기존값 출력 및 신규값 입력
                printf("현재 최대 이용 가능 시간 : %d 시간 %d 분\n", libData->MAX_TIME / 60, libData->MAX_TIME % 60);
                printf("현재 연장 가능 시간 : 끝나기 %d 시간 %d 분 전\n", libData->MAX_RENEWABLE_TIME / 60, libData->MAX_RENEWABLE_TIME % 60);
                printf("연장 가능 시간 수정. 시간 : ");
                scanf("%d", &tmpTime); // 시간 단위 입력
                libData->MAX_RENEWABLE_TIME = tmpTime * 60; // 시간 단위를 분단위로 환산 후 저장
                printf("분 : ");
                scanf("%d", &tmpTime); // 분단위 입력
                libData->MAX_RENEWABLE_TIME += tmpTime; // 저장된 시간 단위 입력값에 분단위 입력값 추가

                // 잘못된 값인지 확인. 이는 0분 이상 24시간 이하여야 하며, 최대 이용가능 시간을 초과할 수 없음.
                if ((libData->MAX_RENEWABLE_TIME < 0 || libData->MAX_RENEWABLE_TIME > 24 * 60) || (libData->MAX_TIME < libData->MAX_RENEWABLE_TIME))
                {
                    printf("연장 가능 시간은 0분 이상 24시간 이하여야 하며, 최대 이용 가능 시간을 초과할 수 없습니다.\n");
                }

            } while ((libData->MAX_RENEWABLE_TIME < 0 || libData->MAX_RENEWABLE_TIME > 24 * 60) || (libData->MAX_TIME < libData->MAX_RENEWABLE_TIME)); // 옳은 입력값이 입력될때까지 반복

            break;

        case 4: // 개장시각 수정

            // 기존 개장시각 저장
            oldData = libData->OPEN_TIME;

            if (libData->OPEN_TIME == libData->CLOSE_TIME) // 기존 운영방식이 24시간제인 경우
            {
                // 경고문 출력
                printf("경고 : 개장 시각과 폐장 시각이 달라지는 경우, 폐장 시각 이후로 지정된 모든 이용자의 퇴실 시각은 폐장 시각으로 일괄 변경됩니다.\n");
            }

            do {
                // 기존 개장시각 복원. 이는 잘못된 값을 입력받았을 때 복원하는 용도로 이용된다.
                libData->OPEN_TIME = oldData;

                // 기존값 출력 및 신규값 입력
                printf("현재 개장 시각 : %d시 %d분, 현재 폐장 시각 : %d시 %d분\n", libData->OPEN_TIME / 60, libData->OPEN_TIME % 60, libData->CLOSE_TIME / 60, libData->CLOSE_TIME % 60);
                printf("개장 시각 수정(폐장 시각과 동일하면 24시간) 시간 : ");
                scanf("%d", &tmpTime); // 시간 단위 입력
                libData->OPEN_TIME = tmpTime * 60; // 시간 단위를 분단위로 환산 후 저장
                printf("분 : ");
                scanf("%d", &tmpTime); // 분단위 입력
                libData->OPEN_TIME += tmpTime; // 저장된 시간 단위 입력값에 분단위 입력값 추가

                // 잘못된 값인지 확인
                if (libData->OPEN_TIME < 0 || libData->OPEN_TIME >= 24 * 60)
                {
                    printf("개장 시각은 0시 0분부터 23시 59분까지로 설정 가능합니다.\n");
                }
            } while ((libData->OPEN_TIME < 0 || libData->OPEN_TIME >= 24 * 60)); // 옳은 입력값이 입력될때까지 반복

            // 폐장 시각을 초과하는 퇴실 시각을 조정함. 24시간제인 경우, 해당 함수가 작동하지 않음.
            renewSeatEndTime(libSeats, libData);
            break;

        case 5: // 폐장시각 수정

            // 경고문 출력
            printf("경고 : 폐장 시각 단축 시, 폐장 시각 이후로 지정된 모든 이용자의 퇴실 시각은 폐장 시각으로 일괄 변경됩니다.\n");

            // 기존 폐장시각 저장
            oldData = libData->CLOSE_TIME;

            do {
                // 기존 폐장시각 복원. 이는 잘못된 값을 입력받았을 때 복원하는 용도로 이용된다.
                libData->CLOSE_TIME = oldData;

                // 기존값 출력 및 신규값 입력
                printf("현재 개장 시각 : %d시 %d분, 현재 폐장 시각 : %d시 %d분\n", libData->OPEN_TIME / 60, libData->OPEN_TIME % 60, libData->CLOSE_TIME / 60, libData->CLOSE_TIME % 60);
                printf("폐장 시각 수정(개장 시각과 동일하면 24시간) 시간 : ");
                scanf("%d", &tmpTime); // 시간 단위 입력
                libData->CLOSE_TIME = tmpTime * 60; // 시간 단위를 분단위로 환산 후 저장
                printf("분 : ");
                scanf("%d", &tmpTime); // 분단위 입력
                libData->CLOSE_TIME += tmpTime; // 저장된 시간 단위 입력값에 분단위 입력값 추가

                // 잘못된 값인지 확인.
                if (libData->CLOSE_TIME < 0 || libData->CLOSE_TIME >= 24 * 60)
                {
                    printf("폐장 시각은 0시 0분부터 23시 59분까지로 설정 가능합니다.\n");
                }
            } while ((libData->CLOSE_TIME < 0 || libData->CLOSE_TIME >= 24 * 60)); // 옳은 입력값이 입력될때까지 반복

            // 폐장 시각을 초과하는 퇴실 시각을 조정함.
            renewSeatEndTime(libSeats, libData);
            break;

        case 6: // 모든 좌석 정보 보기

            // 모든 좌석의 정보를 관리자 모드로 출력함.
            printSeatInfo(libSeats, 1);
            break;

        case 7: // 좌석 이용불가 설정

            // 무한 반복, 좌석 이용불가 설정 종료(0 입력)시 break으로 설정 종료
            while (1)
            {
                // 모든 좌석의 정보를 관리자 모드로 출력함.
                printSeatInfo(libSeats, 1);

                // 이용불가 설정을 수정할 좌석 번호를 입력받을 변수를 -2로 초기화
                tmpSeatNo = -2;

                do {
                    // 이용불가 설정을 수정할 좌석 번호를 입력받음.
                    printf("이용불가 설정을 바꿀 좌석을 선택하세요.(종료 : 0) : ");
                    scanf("%d", &tmpSeatNo);

                    // 입력받은 1부터 시작하는 좌석 번호를 0으로 시작하는 좌석 번호로 변환 
                    tmpSeatNo--;

                    // 잘못된 좌석 번호인 경우(1부터 시작하는 좌석번호 입력값 기준 0(종료)부터 SEATS(마지막 좌석번호)까지의 범위를 벗어난 경우)
                    if (tmpSeatNo < -1 || tmpSeatNo >= SEATS)
                    {
                        printf("잘못된 값을 입력하였습니다.\n");
                    }
                } while (tmpSeatNo < -1 || tmpSeatNo >= SEATS); // 옳은 입력값이 입력될때까지 반복

                // 0을 입력받은 경우 좌석 이용불가 설정을 종료 (0 - 1 = -1)
                if (tmpSeatNo == -1)
                {
                   break;
                }

                // 이미 이용중인 좌석의 경우 좌석 초기화를 진행함
                if (((libSeats + tmpSeatNo)->endTime) > 0) // 이미 이용중인 좌석(종료시각이 0 이상인 경우)
                {
                    // 좌석 이용자명 정보를 초기화함
                    strncpy(((libSeats + tmpSeatNo)->seatsName), "", MAX_NAME_LENGTH);

                    // 좌석 이용종료시각 정보를 초기화함
                    ((libSeats + tmpSeatNo)->endTime) = 0;
                }

                // endTime이 0(이용가능)인 경우 -1(이용불가)로, -1(이용불가)인 경우 0(이용가능)으로 변경
                ((libSeats + tmpSeatNo)->endTime) = -1 - ((libSeats + tmpSeatNo)->endTime);
            }

            break;

        case 0: // 관리자 모드 나가기
            return;

        default: // 그 외의 값이 입력된 경우
            printf("잘못된 값을 입력하였습니다.\n");
        }
    }

    return;
}


/*
* menuSelect 함수
* 기능 : 메인 메뉴에서 이용자명 입력값을 받이 반환함
* 입력값 : 이용자명을 반환할 문자열 포인터 *tmp
* 반환값 : *tmp으로 입력받은 문자열 반환
* 설명 최종 수정 일자 : 2024/05/01
*/
void menuSelect(char* tmp)
{
    // 이용자 정보를 입력받음
    printf("이용자 정보를 입력하세요.(관리자 모드: 0) : ");
    scanf("%s", tmp);

    return;
}


/*
* init 함수
* 기능 : 프로그램 최초 실행 시 실행되어 초기화 등을 진행함
* 입력값 : 좌석 정보 구조체 포인터 *libSeats
* 반환값 없음
* 설명 최종 수정 일자 : 2024/05/01
*/
void init(SeatsData* libSeats)
{
    // 모든 좌석을 빈좌석으로 초기화함.
    resetSeats(libSeats, 1);

    return;
}


/*
* printRenewTime 함수
* 기능 : 주어진 좌석번호의 연장가능시각을 출력함.
* 입력값 : location(0번부터 시작하는 좌석번호), 좌석 정보 구조체 포인터 *libSeats, 시설 정보 구조체 포인터 *libData
* 반환값 없음
* 설명 최종 수정 일자 : 2024/05/01
*/
void printRenewTime(int location, SeatsData* libSeats, LibraryData *libData)
{
    // 현재 시간 관련 변수 선언
    time_t Time;
    struct tm* pTime;
    Time = time(NULL);
    pTime = localtime(&Time);

    // 익일 여부를 판단하기 위한 변수 선언
    int isTomorrow = 0; // 1인 경우 익일이며, 0인 경우 익일이 아님.

    // 빈자리의 경우 함수를 종료함.
    if (!strlen((libSeats + location)->seatsName))
    {
        return;
    }

    // 남은 시간 계산
    // 종료 시각(Unix 초) - 현재 시각(Unix 초)의 방법으로 계산하며, 결괏값은 초 단위이다.
    long long int remainSecondOrig = (libSeats + location)->endTime - ((long long int)Time);
    int remainHour = remainSecondOrig / 3600;
    int remainMinute = (remainSecondOrig % 3600) / 60;
    int remainSecond = remainSecondOrig % 60;

    // 현재 시각 가져오기
    pTime = localtime(&Time);
    int currHour = pTime->tm_hour;
    int currMin = pTime->tm_min;
    int currSecond = pTime->tm_sec;

    // 연장 시각 계산
    // 현재 시각에 남은 시간을 더한 후, 연장가능시각을 뺀다.
    // 종료시각에서부터 현재시각의 차이가 연장가능시각보다 작을 때 연장이 가능하다.
    int computeSecond = remainSecond + currSecond;
    int computeMinute = remainMinute + currMin - (libData->MAX_RENEWABLE_TIME % 60);
    int computeHour = remainHour + currHour - (libData->MAX_RENEWABLE_TIME / 60);

    // 개장시각과 폐장시각에 따라 구분
    // 24시간제의 경우, 폐장시각으로 인해 연장 불가능한 경우가 없으으로 해당 조건문에서 구분하지 않음.
    if (libData->OPEN_TIME < libData->CLOSE_TIME) // 개장시각이 폐장시각보다 앞에 있는 경우
    {
        // 종료 시각(초) - 현재 시각(초) = 남은 시각(초) >= 폐장 시각(초) - 현재 시각(초) = 남은 시각(초) 인 경우, 좌석 연장이 불가능함

        // 종료시간이 임박하여 연장불가한 경우, 연장 불가능하다는 내용을 출력한 후, 함수를 종료함.
        if (((libSeats + location)->endTime - ((long long int)Time)) >= ((libData->CLOSE_TIME * 60) - currHour * 60 * 60 - currMin * 60 - currSecond))
        {
            printf("연장 불가\n");
            return;
        }

    }else if (libData->OPEN_TIME > libData->CLOSE_TIME){ // 개장시각이 폐장시각보다 뒤에 있는 경우

        if (((libData->OPEN_TIME * 60) - currHour * 60 * 60 - currMin * 60 - currSecond) >= 0) // 개장시각이 폐장시각보다 뒤에 있으며, 00시를 지난 경우 (개장 시각 이후)
        {
            // 종료 시각(초) - 현재 시각(초) = 남은 시각(초) >= 폐장 시각(초) - 현재 시각(초) = 남은 시각(초) 인 경우, 좌석 연장이 불가능함
            
            // 종료시간이 임박하여 연장불가한 경우, 연장 불가능하다는 내용을 출력한 후, 함수를 종료함.
            if (((libSeats + location)->endTime - ((long long int)Time)) >= ((libData->CLOSE_TIME * 60) - currHour * 60 * 60 - currMin * 60 - currSecond))
            {
                printf("연장 불가\n");
                return;
            }

        }else{ // 개장시각이 폐장시각보다 뒤에 있으며, 00시를 지나지 않은 경우 (개장 시각 이전)

            // 종료 시각(초) - 현재 시각(초) = 남은 시각(초) >= 폐장 시각(초) + 86400 - 현재 시각(초) = 남은 시각(초) 인 경우, 좌석 연장이 불가능함

            // 종료시간이 임박하여 연장불가한 경우, 연장 불가능하다는 내용을 출력한 후, 함수를 종료함.
            if (((libSeats + location)->endTime - ((long long int)Time)) >= ((libData->CLOSE_TIME * 60) + 24 * 60 * 60 - currHour * 60 * 60 - currMin * 60 - currSecond))
            {
                printf("연장 불가\n");
                return;
            }

        }
        
    }
    

    // 종료 시각의 초단위 값이 60 이상인 경우, 이를 조절한다.
    if (computeSecond >= 60)
    {
        computeMinute += computeSecond / 60; // 종료 시각의 시단위 값이 24 이상인 경우, 이를 조절한다.
        computeSecond %= 60; // 초단위 값은 60 미만의 값만을 처리한다.
    }

    // 종료 시각의 분단위 값이 60 이상인 경우, 이를 조절한다.
    if (computeMinute >= 60)
    {
        computeHour += computeMinute / 60; // 분단위 값 초과분 만큼 시단위 값을 증가시킨다.
        computeMinute %= 60; // 분단위 값은 60 미만의 값만을 처리한다.
    }

    // 종료 시각의 시단위 값이 24 이상인 경우, 이를 조절한다.
    if (computeHour >= 24)
    {
        computeHour %= 24; // 분단위 값은 60 미만의 값만을 처리한다.
        isTomorrow = 1; // 이 경우 익일이므로, 익일임을 표시하는 변수에 익일임을 표시한다.
    }

    // 연장가능시각의 시, 분단위 값이 0 미만인 경우, 이를 조절한다. 초단위의 경우, 연장가능시각이 분단위이므로, 음수가 되지 않는다.

    // 이를 분단위값과 시간 단위 값이 모두 0 이상일때까지 반복한다.
    while (computeMinute < 0 || computeHour < 0)
    {
        // 연장가능시각의 분단위 값이 0 미만인 경우, 이를 조절한다.
        if (computeMinute < 0)
        {
            computeHour--; // 분단위 값이 0 미만이므로 시간 단위 값을 1 감소시킨다.
            computeMinute += 60; // 시간 단위 값을 1 감소시켰으므로, 분단위 값을 60 증가시킨다.
        }

        // 연장가능시각의 분단위 값이 0 미만인 경우, 이를 조절한다.
        if (computeHour < 0)
        {
            computeHour += 24; // 시간 단위 값을 24 증가시킨다.
            isTomorrow = 0; // 시간 단위 값을 24 증가시켰으므로, 더이상 익일이 아니다. 따라서, 익일임을 표시하는 변수에 익일이 아님을 표시한다.
        }
    }

    // 연장가능시각 문자를 출력한다.
    printf("연장 가능 시각 : ");

    // 익일인 경우, 익일임을 출력한다.
    if (isTomorrow)
    {
        printf("익일 ");
    }

    // 남은 연장가능시각의 정보를 출력한다.
    printf("%d시 %d분 %d초\n", computeHour, computeMinute, computeSecond);

    return;
}


/*
* findUser 함수
* 기능 : 주어진 이름의 이용자가 이용하는 좌석번호(0부터 시작)을 출력함.
* 입력값 : *tmpName(찾을 이름이 저장된 문자열의 주소), 좌석 정보 구조체 포인터 *libSeats
* 반환값 : 해당 이름을 가진 이용자의 좌석번호(0부터 시작). 해당 이용자가 좌석을 배정받지 않은 경우 -1을 반환함.
* 설명 최종 수정 일자 : 2024/05/01
*/
int findUser(char* tmpName, SeatsData* libSeats)
{
    // 모든 좌석에 대해서 반복
    for (int i = 0; i < SEATS; i++)
    {
        // 주어진 이름의 이용자명이 발견된 경우
        if (!strcmp((libSeats+i)->seatsName, tmpName))
        {
            // 해당 좌석번호(0부터 시작) 반환
            return i;
        }
    }

    // 해당 이용자가 없으면 -1 반환
    return -1;
}


// 만석인 경우를 찾아서 출력. 만석이면 1 출력.
int isFull(SeatsData* libSeats)
{
    for (int i = 0; i < SEATS; i++)
    {
        if (((libSeats + i)->endTime) == 0) // 빈 좌석 발견 시
        {
            return 0;
        }
    }
    return 1;
}

// 폐장시간까지 남은 초를 계산
int leftSeconds(LibraryData* libData)
{
    if (libData->CLOSE_TIME == libData->OPEN_TIME) // 24시간 운영인 경우
    {
        return 86401; // 1일은 86400초
    }
    time_t Time;
    struct tm* pTime;
    Time = time(NULL);
    pTime = localtime(&Time);
    int hour = pTime->tm_hour;
    int minute = pTime->tm_min;
    int second = pTime->tm_sec;
    int leftSec = 0;

    if (libData->CLOSE_TIME < libData->OPEN_TIME) // 개장시각이 폐장시각보다 늦는 경우
    {
        if (hour * 60 * 60 + minute * 60 + second <= (libData->OPEN_TIME * 60)) // 다음날이 된 경우(다음날의 개장시간 이전)
        {
            leftSec = libData->CLOSE_TIME * 60 - (hour * 60 * 60 + minute * 60 + second);
        }else{ // 아직 날짜 바뀌지 않은 경우
            leftSec = libData->CLOSE_TIME * 60 - (hour * 60 * 60 + minute * 60 + second) + 60 * 60 * 24;
        }
    }else{ //일반적인 경우
        leftSec = libData->CLOSE_TIME * 60 - (hour * 60 * 60 + minute * 60 + second);
    }
    return leftSec;
}

// 좌석 배정
void setSeat(char* tmpName, int location, SeatsData* libSeats, LibraryData* libData)
{
    time_t Time;
    Time = time(NULL);
    strncpy((libSeats + location)->seatsName, tmpName, MAX_NAME_LENGTH);
    int leftTime = leftSeconds(libData);
    if ((libData->MAX_TIME * 60) > leftTime) // 폐장시간까지 얼마 안남은경우
    {
        (libSeats + location)->endTime = (long long int)(Time) + leftTime; // 분단위인 시각을 초단위로 변경
    }else{
        (libSeats + location)->endTime = (long long int)(Time) + libData->MAX_TIME * 60; // 분단위인 시각을 초단위로 변경
    }
    return;
}

//좌석이 연장 가능한지 확인하는 함수. 연장 가능시 1 출력.
int isRenewable(int location, SeatsData* libSeats, LibraryData* libData)
{
    time_t Time;
    struct tm* pTime;
    Time = time(NULL);
    pTime = localtime(&Time);

    int answer = 1;

    long long int tmpTime = (libSeats + location)->endTime - (long long int)(Time);

    // 개장시각과 폐장시각에 따라 구분
    // 24시간제의 경우, 폐장시각으로 인해 연장 불가능한 경우가 없으으로 해당 조건문에서 구분하지 않음.
    if (libData->OPEN_TIME < libData->CLOSE_TIME) // 개장시각이 폐장시각보다 앞에 있는 경우
    {
        // 개인별 종료 시각(Unix 초) - 현재 시각(Unix 초) >= 폐장 시각(초) - 현재 시각(초) = 남은 시각(초)
        if (((libSeats + location)->endTime - ((long long int)Time)) >= ((libData->CLOSE_TIME * 60) - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec))) // 종료시간이 임박하여 연장불가한 경우
        {
            answer = 0;
        }
    }else if (libData->OPEN_TIME > libData->CLOSE_TIME){
        if (((libData->OPEN_TIME * 60) - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec)) >= 0) // 개장시각이 폐장시각보다 뒤에 있으며, 00시를 지난 경우 (개장 시각 이후)
        {
            // 개인별 종료 시각(Unix 초) - 현재 시각(Unix 초) >= 폐장 시각(초) - 현재 시각(초) = 남은 시각(초)
            if (((libSeats + location)->endTime - ((long long int)Time)) >= ((libData->CLOSE_TIME * 60) - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec))) // 종료시간이 임박하여 연장불가한 경우
            {
                answer = 0;
            }
        }else{ // 개장시각이 폐장시각보다 뒤에 있으며, 00시를 지나지 않은 경우 (개장 시각 이전)
            // 개인별 종료 시각(Unix 초) - 현재 시각(Unix 초) >= 폐장 시각(초) + 86400 - 현재 시각(초) = 남은 시각(초)
            if (((libSeats + location)->endTime - ((long long int)Time)) >= ((libData->CLOSE_TIME * 60) + 24 * 60 * 60 - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec))) // 종료시간이 임박하여 연장불가한 경우
            {
                answer = 0;
            }
        }

    }

    return (tmpTime <= libData->MAX_RENEWABLE_TIME * 60) && answer;
}

// 좌석 연장을 처리하는 함수
void renewSeat(int location, SeatsData* libSeats, LibraryData* libData)
{
    time_t Time;
    Time = time(NULL);

    int leftTime = leftSeconds(libData); // 현재 시각 기준 폐장까지 남은 시간 출력

    // 개인별 종료 시각(Unix 초) - 현재 시각(Unix 초) + 연장 시간(초) > 남은 시간(초)
    if (((libSeats + location)->endTime - ((long long int)Time) + (libData->MAX_TIME * 60)) > leftTime) // 폐장시간까지 얼마 안남은경우
    {
        (libSeats + location)->endTime = ((long long int)(Time)) + leftTime; // 분단위인 시각을 초단위로 변경
    }else{
        //기존 이용시간에 기본 이용시간 추가, (libSeats + i)->endTime에는 Unix 시간을 저장하므로 날짜가 바뀌어서 생기는 문제는 없음.
        (libSeats + location)->endTime += libData->MAX_TIME * 60; // 분단위인 시각을 초단위로 변경
    }
    return;
}

// 퇴실
void checkOut(int location, SeatsData* libSeats)
{
    strncpy((libSeats + location)->seatsName, "", MAX_NAME_LENGTH);
    (libSeats + location)->endTime = 0;
    return;
}

// 좌석 배정 시스템
void seatSelector(char* tmpName, SeatsData* libSeats, LibraryData* libData)
{
    int location = findUser(tmpName, libSeats);
    int tmpSeatNo = -1, isRenewableRes = 0, tmpMenu = 0;
    if (location == -1) //새로운 사람
    {
        if (isFull(libSeats)) // 빈 좌석 없는지 확인
        {
            printf("만석입니다.\n");
            return;
        }

        // 좌석 있으면
        printSeatInfo(libSeats, 0); //User용 좌석 목록 출력
        // 좌석 선택
        while (1)
        {
            do {
                printf("좌석 번호 선택(취소 : 0) : ");
                scanf("%d", &tmpSeatNo);
                tmpSeatNo--;
                if (tmpSeatNo < -1 || tmpSeatNo >= SEATS)
                {
                    printf("잘못된 값을 입력하였습니다.\n");
                }
            } while (tmpSeatNo < -1 || tmpSeatNo >= SEATS);

            if (tmpSeatNo == -1) // 0 입력시 취소 (0 - 1 = -1)
            {
                printf("좌석 선택을 취소하였습니다.\n");
                return;
            }
            // 좌석 찼는지 확인하고
            if (strlen((libSeats + tmpSeatNo)->seatsName))
            {
                printf("이미 이용중인 좌석입니다.\n다른 좌석을 선택해주세요.\n");
            }else if ((libSeats + tmpSeatNo)->endTime == -1){
                printf("이용불가 좌석입니다.\n다른 좌석을 선택하세요.\n");
            }else{
                break;
            }
        }
        setSeat(tmpName, tmpSeatNo, libSeats, libData); // 좌석 배정

        // 연장가능시각, 이용종료시각 출력
        printRenewTime(tmpSeatNo, libSeats, libData);
        printEndTime(tmpSeatNo, libSeats);

    }else{ //좌석 이용중인 사람
        // 연장 가능여부 확인
        isRenewableRes = isRenewable(findUser(tmpName, libSeats), libSeats, libData);

        // 좌석정보, 연장가능시각, 이용종료시각 출력
        printf("%d번 좌석\n", location + 1);
        printRenewTime(location, libSeats, libData);
        printEndTime(location, libSeats);

        // 연장, 퇴실여부 확인
        while (1)
        {
            if (isRenewableRes) // 연장 가능하면
            {
                printf("연장 : 1, ");
            }
            printf("퇴실 : 2, 취소 : 3. : ");
            scanf("%d", &tmpMenu);
            if ((tmpMenu == 1 && isRenewableRes) || tmpMenu == 2 || tmpMenu == 3)
            {
                break;
            }else{
                printf("다시 입력해 주세요.\n");
            }
        }

        // 이용자의 좌석정보를 불러옴
        tmpSeatNo = findUser(tmpName, libSeats);
        switch (tmpMenu)
        {
            // 연장
        case 1:
            renewSeat(tmpSeatNo, libSeats, libData);

            // 좌석 정보 출력
            printRenewTime(tmpSeatNo, libSeats, libData);
            printEndTime(tmpSeatNo, libSeats);
            break;

            // 반납
        case 2:
            checkOut(tmpSeatNo, libSeats);
            break;

            // 취소
        case 3:
            return;
        }
    }
    return;
}

//좌석이 만료된 경우, 좌석 지정을 해제함
void seatInvalidCheck(SeatsData* libSeats)
{
    time_t Time;
    Time = time(NULL);

    for (int i = 0; i < SEATS; i++)
    {
        // Unix 시간 기준이므로, 다음날 구분은 자동으로 가능함.
        if (((libSeats + i)->endTime < (int)Time) && (libSeats + i)->endTime > 0) //(libSeats + i)->endTime=0(미배정)과 -1(이용불가)은 예외
        {
            strncpy((libSeats + i)->seatsName, "", MAX_NAME_LENGTH);
            (libSeats + i)->endTime = 0;
        }
    }
    return;
}

int main()
{
    LibraryData libData = { 240, 30, 24 * 60 - 1, 24 * 60 - 1 };
    // 24시간 운영시 OPEN_TIME == CLOSE TIME. 좌석 초기화 없음.

    //이용자 데이터 저장소 생성
    SeatsData libSeats[SEATS];

    char tmpName[MAX_NAME_LENGTH];
    int tmp_time = 0;
    time_t Time;
    struct tm* pTime;
    Time = time(NULL);
    init(libSeats); //초기화
    while (1)
    {
        menuSelect(tmpName);
        seatInvalidCheck(libSeats); //시간 만료되면 자동 퇴실

        Time = time(NULL); // 시간 최신화
        if (tmpName[0] == '0' && strlen(tmpName) == 1)
        {
            adminMode(libSeats , &libData);
        }else{
            pTime = localtime(&Time);
            tmp_time = (pTime->tm_hour * 60 * 60) + (pTime->tm_min * 60) + (pTime->tm_sec);
            if ((libData.OPEN_TIME < libData.CLOSE_TIME) && (tmp_time >= libData.CLOSE_TIME * 60 || tmp_time < libData.OPEN_TIME * 60)) //개장 전이거나 폐장 이후이며 개장시간이 폐장시간보다 빠른 경우.
            {
                printf("운영시간이 아닙니다.\n");
            }else if ((libData.OPEN_TIME > libData.CLOSE_TIME) && (tmp_time >= libData.CLOSE_TIME * 60 && tmp_time < libData.OPEN_TIME * 60)){ //개장 전이거나 폐장 이후이며 폐장시간이 개장시간보다 빠른 경우.
                printf("운영시간이 아닙니다.\n");
            }else{
                //운영시간 내라면(24시간제 포함)
                seatSelector(tmpName, libSeats, &libData);
            }
        }

        // 운영 종료시 자동 퇴실 처리
        if (libData.OPEN_TIME != libData.CLOSE_TIME) //24시간 여부 확인
        {
            //운영시간 확인(지났으면 모두 퇴실(init))
            //자동퇴실에만 맡길수 없는 이유 : 관리자가 운영시간 바꾼 경우
            pTime = localtime(&Time);
            tmp_time = (pTime->tm_hour * 60 * 60) + (pTime->tm_min * 60) + (pTime->tm_sec);
            if ((tmp_time >= libData.CLOSE_TIME * 60 || tmp_time < libData.OPEN_TIME * 60) && (libData.OPEN_TIME < libData.CLOSE_TIME)) //개장 전이거나 폐장 이후이며 개장시간이 폐장시간보다 빠른 경우.
            {
                init(libSeats); //좌석 초기화
            }else if ((tmp_time >= libData.CLOSE_TIME * 60 && tmp_time < libData.OPEN_TIME * 60) && (libData.OPEN_TIME > libData.CLOSE_TIME)){ //개장 전이거나 폐장 이후이며 폐장시간이 개장시간보다 빠른 경우.
                init(libSeats); //좌석 초기화
            }
        }
    }
    return 0;
}