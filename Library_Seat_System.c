#include <stdio.h>
#include <string.h> // strcpy, strlen
#include <time.h> // 현재 시각

/* 열람실 좌석관리시스템
* 일반 사용자
* 이름 입력 -> 좌석 선택 -> 소요시간 부여
* 이미 좌석이 있는 사람의 경우 좌석정보와 연장가능여부 확인 -> 연장/퇴실/취소 선택
* 종료시각까지만 좌석 부여 후 자동 좌석 퇴실 처리
*
*
* 문제점(버그)
* 폐장시간을 앞당기는 경우, 이용시간이 초기화되지 않음.
* 연장시각이 익일인 경우, 일부 경우에서 익일이라는 표현이 출력되지 않는 오류가 발생함.
*
* 
* 작성자 : YHC03
* 작성일 : 2024/4/25-2024/4/28
*/

// 좌석 수 설정
#define SEATS 10

// 사용자 데이터를 저장하는 구조체 생성
struct userData
{
    char seatsName[20]; // 좌석 사용자명 기록, ""(strlen=0)이면 빈 좌석
    long long int endTime; //사용 종료시각 기록(Unix 시간) - 초 단위
};

// 전역 변수로 사용자 데이터 저장소 생성
struct userData user[SEATS];

// 좌석 초기화
void resetSeats()
{
    for (int i = 0; i < SEATS; i++)
    {
        strncpy((user + i)->seatsName, "", 20);
        (user + i)->endTime = 0;
    }
    return;
}

// 종료 시각 출력
void printEndTime(int location)
{
    time_t Time;
    struct tm* pTime;
    Time = time(NULL);
    pTime = localtime(&Time);
    if (!strlen((user + location)->seatsName))
    {
        return;
    }

    // 남은 시간
    int remainSecondOrig = (user + location)->endTime - ((long long int)Time);
    int remainHour = remainSecondOrig / 3600;
    int remainMinute = (remainSecondOrig % 3600) / 60;
    int remainSecond = remainSecondOrig % 60;

    // 현재 시각
    pTime = localtime(&Time);
    int currHour = pTime->tm_hour;
    int currMin = pTime->tm_min;
    int currSecond = pTime->tm_sec;

    // 종료 시각 계산
    int computeSecond = remainSecond + currSecond;
    int computeMinute = remainMinute + currMin;
    int computeHour = remainHour + currHour;
    if (computeSecond >= 60)
    {
        computeMinute += computeSecond / 60;
        computeSecond %= 60;
    }
    if (computeMinute >= 60)
    {
        computeHour += computeMinute / 60;
        computeMinute %= 60;
    }
    printf("종료 시각 : ");
    if (computeHour >= 24)
    {
        computeHour %= 24;
        printf("익일 ");
    }
    printf("%d시 %d분 %d초\n", computeHour, computeMinute, computeSecond);
}

//좌석 정보를 모두 출력
void printSeatInfo(int isMaster)
{
    for (int i = 0; i < SEATS; i++)
    {
        if (isMaster) // 관리자 여부 확인
        {
            printf("%d번 좌석 : %s\n", i + 1, strlen((user + i)->seatsName) ? (user + i)->seatsName : "Empty");
        }
        else {
            printf("%d번 좌석 : %s\n", i + 1, strlen((user + i)->seatsName) ? "Used" : "Empty");
        }
    }
}

// 관리자 모드 함수
void adminMode(int* MaxTime, int* MaxRenewable, int* OpenTime, int* CloseTime)
{
    int menu_sel = -1, tmp = 0, oldData = 0;
    while (1)
    {
        printf("1 : 좌석 초기화, 2 : 최대 이용 가능 시간 수정, 3: 연장 가능 시간 수정, 4 : 개장시각 수정, 5: 폐장시각 수정, 6: 모든 좌석 정보 보기, 0: 나가기 : ");
        scanf("%d", &menu_sel);
        switch (menu_sel)
        {
        case 1:
            resetSeats();
            break;
        case 2:
            oldData = *MaxTime;
            do {
                *MaxTime = oldData;
                printf("현재 최대 이용 가능 시간 : %d 시간 %d 분\n", *MaxTime / 60, *MaxTime % 60);
                printf("최대 이용 가능 시간 수정. 시간 : ");
                scanf("%d", &tmp);
                *MaxTime = tmp * 60;
                printf("분 : ");
                scanf("%d", &tmp);
                *MaxTime += tmp;

                if (*MaxTime <= 0 || *MaxTime > 24 * 60)
                {
                    printf("최대 이용 가능 시간은 1분 이상 24시간 이하여야 합니다.\n");
                }
            } while (*MaxTime <= 0 || *MaxTime > 24 * 60);
            if (*MaxTime < *MaxRenewable)
            {
                *MaxRenewable = *MaxTime;
                printf("최대 이용 가능시간 변경으로 인해 연장 가능 시간이 끝나기 %d 시간 %d 분 전으로 변경되었습니다.\n", *MaxRenewable / 60, *MaxRenewable % 60);
            }
            break;
        case 3:
            oldData = *MaxRenewable;
            do {
                *MaxRenewable = oldData;
                printf("현재 최대 이용 가능 시간 : %d 시간 %d 분\n", *MaxTime / 60, *MaxTime % 60);
                printf("현재 연장 가능 시간 : 끝나기 %d 시간 %d 분 전\n", *MaxRenewable / 60, *MaxRenewable % 60);
                printf("연장 가능 시간 수정. 시간 : ");
                scanf("%d", &tmp);
                *MaxRenewable = tmp * 60;
                printf("분 : ");
                scanf("%d", &tmp);
                *MaxRenewable += tmp;

                if ((*MaxRenewable < 0 || *MaxRenewable > 24 * 60) || (*MaxTime < *MaxRenewable))
                {
                    printf("연장 가능 시간은 0분 이상 24시간 이하여야 하며, 최대 이용 가능 시간을 초과할 수 없습니다.\n");
                }
            } while ((*MaxRenewable < 0 || *MaxRenewable > 24 * 60) || (*MaxTime < *MaxRenewable));
            break;
        case 4:
            oldData = *OpenTime;
            do {
                *OpenTime = oldData;
                printf("현재 개장 시각 : %d시 %d분, 현재 폐장 시각 : %d시 %d분\n", *OpenTime / 60, *OpenTime % 60, *CloseTime / 60, *CloseTime % 60);
                printf("개장 시각 수정(폐장 시각과 동일하면 24시간) 시간 : ");
                scanf("%d", &tmp);
                *OpenTime = tmp * 60;
                printf("분 : ");
                scanf("%d", &tmp);
                *OpenTime += tmp;

                if (*OpenTime < 0 || *OpenTime >= 24 * 60)
                {
                    printf("개장 시각은 0시 0분부터 23시 59분까지로 설정 가능합니다.\n");
                }
            } while ((*OpenTime < 0 || *OpenTime >= 24 * 60));
            break;
        case 5:
            oldData = *CloseTime;
            do {
                *CloseTime = oldData;
                printf("현재 개장 시각 : %d시 %d분, 현재 폐장 시각 : %d시 %d분\n", *OpenTime / 60, *OpenTime % 60, *CloseTime / 60, *CloseTime % 60);
                printf("폐장 시각 수정(개장 시각과 동일하면 24시간) 시간 : ");
                scanf("%d", &tmp);
                *CloseTime = tmp * 60;
                printf("분 : ");
                scanf("%d", &tmp);
                *CloseTime += tmp;

                if (*CloseTime < 0 || *CloseTime >= 24 * 60)
                {
                    printf("폐장 시각은 0시 0분부터 23시 59분까지로 설정 가능합니다.\n");
                }
            } while ((*CloseTime < 0 || *CloseTime >= 24 * 60));
            break;
        case 6:
            printSeatInfo(1);
            break;
        case 0:
            return;
        default:
            printf("잘못된 값을 입력하였습니다.\n");
        }
    }
}

// 메뉴 입력 함수
void menuSelect(char* tmp)
{
    printf("사용자명을 입력하세요.(관리자 모드: 0) : ");
    scanf("%s", tmp);
    return;
}

// 초기화 함수
void init()
{
    resetSeats();
    return;
}

// 연장가능 시각 출력
// 폐장시간 직전에 오류 발생 가능
void printRenewTime(int location, int* renewableTime, int* startTime, int* lclEndTime)
{
    time_t Time;
    struct tm* pTime;
    Time = time(NULL);
    pTime = localtime(&Time);

    int isTomorrow = 0; // 익일 여부 판단

    // 빈자리 처리
    if (!strlen((user + location)->seatsName))
    {
        return;
    }

    // 남은 시간
    long long int remainSecondOrig = (user + location)->endTime - ((long long int)Time);
    int remainHour = remainSecondOrig / 3600;
    int remainMinute = (remainSecondOrig % 3600) / 60;
    int remainSecond = remainSecondOrig % 60;

    // 현재 시각
    pTime = localtime(&Time);
    int currHour = pTime->tm_hour;
    int currMin = pTime->tm_min;
    int currSecond = pTime->tm_sec;

    // 연장 시각 계산
    int computeSecond = remainSecond + currSecond;
    int computeMinute = remainMinute + currMin - (*renewableTime % 60);
    int computeHour = remainHour + currHour - (*renewableTime / 60);

    if (*startTime < *lclEndTime) // 개장시각이 폐장시각보다 앞에 있는 경우
    {
        // 종료 시각(초) - 현재 시각(초) >= 폐장 시각(초) - 현재 시각(초) = 남은 시각(초)
        if (((user + location)->endTime - ((long long int)Time)) >= ((*lclEndTime * 60) - currHour * 60 * 60 - currMin * 60 - currSecond)) // 종료시간이 임박하여 연장불가한 경우
        {
            printf("연장 불가\n");
            return;
        }
    }else if (*startTime > *lclEndTime){
        if (((*startTime * 60) - currHour * 60 * 60 - currMin * 60 - currSecond) >= 0) // 개장시각이 폐장시각보다 뒤에 있으며, 00시를 지난 경우 (개장 시각 이후)
        {
            // 종료 시각(초) - 현재 시각(초) >= 폐장 시각(초) - 현재 시각(초) = 남은 시각(초)
            if (((user + location)->endTime - ((long long int)Time)) >= ((*lclEndTime * 60) - currHour * 60 * 60 - currMin * 60 - currSecond)) // 종료시간이 임박하여 연장불가한 경우
            {
                printf("연장 불가\n");
                return;
            }
        }else{ // 개장시각이 폐장시각보다 뒤에 있으며, 00시를 지나지 않은 경우 (개장 시각 이전)
            // 종료 시각(초) - 현재 시각(초) >= 폐장 시각(초) + 86400 - 현재 시각(초) = 남은 시각(초)
            if (((user + location)->endTime - ((long long int)Time)) >= ((*lclEndTime * 60) + 24 * 60 * 60 - currHour * 60 * 60 - currMin * 60 - currSecond)) // 종료시간이 임박하여 연장불가한 경우
            {
                printf("연장 불가\n");
                return;
            }
        }
        
    } // 24시간제의 경우 여기서 처리되지 않음
    
    if (computeSecond >= 60)
    {
        computeMinute += computeSecond / 60;
        computeSecond %= 60;
    }
    if (computeMinute >= 60)
    {
        computeHour += computeMinute / 60;
        computeMinute %= 60;
    }
    if (computeHour >= 24)
    {
        computeHour %= 24;
        isTomorrow = 1;
    }

    while (computeMinute < 0 || computeHour < 0)
    {
        if (computeMinute < 0)
        {
            computeHour--;
            computeMinute += 60;
        }
        if (computeHour < 0)
        {
            computeHour += 24;
            isTomorrow = 0;
        }
    }

    printf("연장 가능 시각 : ");
    if (isTomorrow)
    {
        printf("익일 ");
    }
    printf("%d시 %d분 %d초\n", computeHour, computeMinute, computeSecond);

    return;
}

// 이용자명을 이용해 좌석번호를 찾는 함수
int findUser(char* tmpName)
{
    for (int i = 0; i < SEATS; i++)
    {
        if (!strcmp((user+i)->seatsName, tmpName))
        {
            return i;
        }
    }
    return -1;
}

// 만석인 경우를 찾아서 출력. 만석이면 1 출력.
int isFull()
{
    for (int i = 0; i < SEATS; i++)
    {
        if (!strlen((user + i)->seatsName)) // 빈 좌석 발견 시
        {
            return 0;
        }
    }
    return 1;
}

// 폐장시간까지 남은 초를 계산
int leftSeconds(int* startTime, int* lclEndTime)
{
    if (*lclEndTime == *startTime) // 24시간 운영인 경우
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
    int leftSeconds = 0;

    if (*lclEndTime < *startTime) // 개장시각이 폐장시각보다 늦는 경우
    {
        if (hour * 60 * 60 + minute * 60 + second <= (*startTime * 60)) // 다음날이 된 경우(다음날의 개장시간 이전)
        {
            leftSeconds = *lclEndTime * 60 - (hour * 60 * 60 + minute * 60 + second);
        }else{ // 아직 날짜 바뀌지 않은 경우
            leftSeconds = *lclEndTime * 60 - (hour * 60 * 60 + minute * 60 + second) + 60 * 60 * 24;
        }
    }else{ //일반적인 경우
        leftSeconds = *lclEndTime * 60 - (hour * 60 * 60 + minute * 60 + second);
    }
    return leftSeconds;
}

// 좌석 배정
void setSeat(char* tmpName, int location, int* maxTime, int* startTime, int* lclEndTime)
{
    time_t Time;
    Time = time(NULL);
    strncpy((user + location)->seatsName, tmpName, 20);
    int leftTime = leftSeconds(startTime, lclEndTime);
    if ((*maxTime * 60) > leftTime) // 폐장시간까지 얼마 안남은경우
    {
        (user + location)->endTime = (long long int)(Time) + leftTime; // 분단위인 시각을 초단위로 변경
    }else{
        (user + location)->endTime = (long long int)(Time) + *maxTime * 60; // 분단위인 시각을 초단위로 변경
    }
    return;
}

//좌석이 연장 가능한지 확인하는 함수(연장 가능시 1 출력)
//폐장시간 직전에 오류 발생 가능
int isRenewable(int location, int* maxRenewTime, int* startTime, int* lclEndTime)
{
    time_t Time;
    struct tm* pTime;
    Time = time(NULL);
    pTime = localtime(&Time);

    int answer = 1;

    long long int tmpTime = (user + location)->endTime - (long long int)(Time);

    if (*startTime < *lclEndTime) // 개장시각이 폐장시각보다 앞에 있는 경우
    {
        // 개인별 종료 시각(Unix 초) - 현재 시각(Unix 초) >= 폐장 시각(초) - 현재 시각(초) = 남은 시각(초)
        if (((user + location)->endTime - ((long long int)Time)) >= ((*lclEndTime * 60) - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec))) // 종료시간이 임박하여 연장불가한 경우
        {
            answer = 0;
        }
    }else if (*startTime > *lclEndTime){
        if (((*startTime * 60) - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec)) >= 0) // 개장시각이 폐장시각보다 뒤에 있으며, 00시를 지난 경우 (개장 시각 이후)
        {
            // 개인별 종료 시각(Unix 초) - 현재 시각(Unix 초) >= 폐장 시각(초) - 현재 시각(초) = 남은 시각(초)
            if (((user + location)->endTime - ((long long int)Time)) >= ((*lclEndTime * 60) - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec))) // 종료시간이 임박하여 연장불가한 경우
            {
                answer = 0;
            }
        }else{ // 개장시각이 폐장시각보다 뒤에 있으며, 00시를 지나지 않은 경우 (개장 시각 이전)
            // 개인별 종료 시각(Unix 초) - 현재 시각(Unix 초) >= 폐장 시각(초) + 86400 - 현재 시각(초) = 남은 시각(초)
            if (((user + location)->endTime - ((long long int)Time)) >= ((*lclEndTime * 60) + 24 * 60 * 60 - (pTime->tm_hour) * 60 * 60 - (pTime->tm_min) * 60 - (pTime->tm_sec))) // 종료시간이 임박하여 연장불가한 경우
            {
                answer = 0;
            }
        }

    } // 24시간제의 경우 여기서 처리되지 않음

    return (tmpTime <= *maxRenewTime * 60) && answer;
}

// 좌석 연장
void renewSeat(int location, int* maxTime, int* startTime, int* lclEndTime)
{
    time_t Time;
    Time = time(NULL);

    int leftTime = leftSeconds(startTime, lclEndTime); // 현재 시각 기준 폐장까지 남은 시간 출력

    // 개인별 종료 시각(Unix 초) - 현재 시각(Unix 초) + 연장 시간(초) > 남은 시간(초)
    if (((user + location)->endTime - ((long long int)Time) + (*maxTime * 60)) > leftTime) // 폐장시간까지 얼마 안남은경우
    {
        (user + location)->endTime = ((long long int)(Time)) + leftTime; // 분단위인 시각을 초단위로 변경
    }else{
        //기존 이용시간에 기본 이용시간 추가, (user + i)->endTime에는 Unix 시간을 저장하므로 날짜가 바뀌어서 생기는 문제는 없음.
        (user + location)->endTime += *maxTime * 60; // 분단위인 시각을 초단위로 변경
    }
    return;
}

// 퇴실
void checkOut(int location)
{
    strncpy((user + location)->seatsName, "", 20);
    (user + location)->endTime = 0;
    return;
}

// 좌석 배정 시스템
void seatSelector(char* tmpName, int* maxTime, int* maxRenew, int* startTime, int* lclEndTime)
{
    int location = findUser(tmpName);
    int tmpSeatNo = -1, isRenewableRes = 0, tmpMenu = 0;
    if (location == -1) //새로운 사람
    {
        if (isFull()) // 빈 좌석 없는지 확인
        {
            printf("만석입니다.\n");
            return;
        }

        // 좌석 있으면
        printSeatInfo(0); //User용 좌석 목록 출력
        // 좌석 선택
        while (1)
        {
            do {
                printf("좌석 번호 선택 : ");
                scanf("%d", &tmpSeatNo);
                tmpSeatNo--;
                if (tmpSeatNo < 0 || tmpSeatNo >= SEATS)
                {
                    printf("잘못된 값을 입력하셨습니다.\n");
                }
            } while (tmpSeatNo < 0 || tmpSeatNo >= SEATS);
            // 좌석 찼는지 확인하고
            if (strlen((user + tmpSeatNo)->seatsName))
            {
                printf("이미 사용중인 좌석입니다.\n다른 좌석을 선택해주세요.\n");
            }else{
                break;
            }
        }
        setSeat(tmpName, tmpSeatNo, maxTime, startTime, lclEndTime); // 좌석 배정

        // 연장가능시각, 이용종료시각 출력
        printRenewTime(tmpSeatNo, maxRenew, startTime, lclEndTime);
        printEndTime(tmpSeatNo);

    }else{ //좌석 사용중인 사람
        // 연장 가능여부 확인
        isRenewableRes = isRenewable(findUser(tmpName), maxRenew, startTime, lclEndTime);

        // 좌석정보, 연장가능시각, 이용종료시각 출력
        printf("%d번 좌석\n", location + 1);
        printRenewTime(location, maxRenew, startTime, lclEndTime);
        printEndTime(location);

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
        tmpSeatNo = findUser(tmpName);
        switch (tmpMenu)
        {
            // 연장
        case 1:
            renewSeat(tmpSeatNo, maxTime, startTime, lclEndTime);

            // 좌석 정보 출력
            printRenewTime(tmpSeatNo, maxRenew, startTime, lclEndTime);
            printEndTime(tmpSeatNo);
            break;

            // 반납
        case 2:
            checkOut(tmpSeatNo);
            break;

            // 취소
        case 3:
            return;
        }
    }
    return;
}

//좌석이 만료된 경우, 좌석 지정을 해제함
void seatInvalidCheck()
{
    time_t Time;
    Time = time(NULL);

    for (int i = 0; i < SEATS; i++)
    {
        // Unix 시간 기준이므로, 다음날 구분은 자동으로 가능함.
        if (((user + i)->endTime < (int)Time) && (user + i)->endTime) //(user + i)->endTime=0(미배정)은 예외
        {
            strncpy((user + i)->seatsName, "", 20);
            (user + i)->endTime = 0;
        }
    }
    return;
}

int main()
{
    int MAX_TIME = 240; //최대 점유 시간(분)
    int MAX_RENEWABLE_TIME = 30; //좌석 연장 가능 시간(분) (횟수제한 없음. 반납후 재발급받으면 그만이라.)
    int OPEN_TIME = 24 * 60 - 1; //문 여는 시간(시, 분)-분으로 환산
    int CLOSE_TIME = 24 * 60 - 1; //문 닫는 시간(시, 분)-분으로 환산
    // 24시간 운영시 OPEN_TIME == CLOSE TIME. 좌석 초기화 없음.

    char tmpName[20];
    int tmp_time = 0;
    time_t Time;
    struct tm* pTime;
    Time = time(NULL);
    init(); //초기화
    while (1)
    {
        menuSelect(tmpName);
        seatInvalidCheck(); //시간 만료되면 자동 퇴실

        Time = time(NULL); // 시간 최신화
        if (tmpName[0] == '0' && strlen(tmpName) == 1)
        {
            adminMode(&MAX_TIME, &MAX_RENEWABLE_TIME, &OPEN_TIME, &CLOSE_TIME);
        }else{
            pTime = localtime(&Time);
            tmp_time = (pTime->tm_hour * 60 * 60) + (pTime->tm_min * 60) + (pTime->tm_sec);
            if ((OPEN_TIME < CLOSE_TIME) && (tmp_time >= CLOSE_TIME * 60 || tmp_time < OPEN_TIME * 60)) //개장 전이거나 폐장 이후이며 개장시간이 폐장시간보다 빠른 경우.
            {
                printf("운영시간이 아닙니다.\n");
            }else if ((OPEN_TIME > CLOSE_TIME) && (tmp_time >= CLOSE_TIME * 60 && tmp_time < OPEN_TIME * 60)){ //개장 전이거나 폐장 이후이며 폐장시간이 개장시간보다 빠른 경우.
                printf("운영시간이 아닙니다.\n");
            }else{
                //운영시간 내라면(24시간제 포함)
                seatSelector(tmpName, &MAX_TIME, &MAX_RENEWABLE_TIME, &OPEN_TIME, &CLOSE_TIME);
            }
        }

        // 운영 종료시 자동 퇴실 처리
        if (OPEN_TIME != CLOSE_TIME) //24시간 여부 확인
        {
            //운영시간 확인(지났으면 모두 퇴실(init))
            //자동퇴실에만 맡길수 없는 이유 : 관리자가 운영시간 바꾼 경우
            pTime = localtime(&Time);
            tmp_time = (pTime->tm_hour * 60 * 60) + (pTime->tm_min * 60) + (pTime->tm_sec);
            if ((tmp_time >= CLOSE_TIME * 60 || tmp_time < OPEN_TIME * 60) && (OPEN_TIME < CLOSE_TIME)) //개장 전이거나 폐장 이후이며 개장시간이 폐장시간보다 빠른 경우.
            {
                init(); //좌석 초기화
            }else if ((tmp_time >= CLOSE_TIME * 60 && tmp_time < OPEN_TIME * 60) && (OPEN_TIME > CLOSE_TIME)){ //개장 전이거나 폐장 이후이며 폐장시간이 개장시간보다 빠른 경우.
                init(); //좌석 초기화
            }
        }
    }
    return 0;
}