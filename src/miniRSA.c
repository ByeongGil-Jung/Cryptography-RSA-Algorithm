/*
 * @file    miniRSA.c
 * @date    18/11/18
 * @brief   mini RSA implementation code
 * @details 세부 설명
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "miniRSA.h"

uint p, q, e, d, n;

/*========================================================================================*/

uint GCD(uint a, uint b) {
    uint prev_a;

    while(b != 0) {
        prev_a = a;
        a = b;
        while(prev_a >= b) prev_a -= b;
        b = prev_a;
    }
    return a;
}

// a mod n (qAndr 은 [몫, 나머지])
void customMod(uint a, uint n, uint* qAndr) {
    *qAndr = 0;
    *(qAndr + 1) = 0;
    uint quotient = 0;

    while (TRUE) {
        if (a < n) {
            break;
        } else {
            a -= n;
            quotient++;
        }
    }

    *qAndr = quotient;
    *(qAndr + 1) = a;
}

/*========================================================================================*/

/*
 * @brief     모듈러 덧셈 연산을 하는 함수.
 * @param     uint a     : 피연산자1.
 * @param     uint b     : 피연산자2.
 * @param     byte op    : +, - 연산자.
 * @param     uint n      : 모듈러 값.
 * @return    uint result : 피연산자의 덧셈에 대한 모듈러 연산 값. (a op b) mod n
 */
uint ModAdd(uint a, uint b, byte op, uint n) {
    uint result = 0;
    uint tempA[] = {0, 0};
    uint tempB[] = {0, 0};
    uint qAndr[] = {0, 0};
    uint overflowCount = 0;

    // + 일 경우
    if (op == 43) {
        customMod(a, n, tempA);
        customMod(b, n, tempB);
        // 각 mod 끼리 더했는데 오버플로우 발생한다면
        if (tempA[1] >= 0xffffffff - tempB[1]) {
            overflowCount++;
        }

        customMod(tempA[1] + tempB[1], n, qAndr);
        result = qAndr[1];

        // 오버플로우 처리
        if (overflowCount > 0) {
            customMod(0xffffffff, n, qAndr);
            result += (qAndr[1] + 1);
        }
    // - 일 경우
    } else if (op == 45) {
        customMod(a, n, tempA);
        customMod(b, n, tempB);

        // 만약 음수가 나올 경우 (오버플로우 발생한 경우)
        if (tempA[1] < tempB[1]) {
            overflowCount++;
        }

        // 오버플로우를 고려한 최종 연산
        if (overflowCount > 0) {
            result = n - (0xffffffff- (tempA[1] - tempB[1]) + 1);
        } else {
            customMod(tempA[1] - tempB[1], n, qAndr);
            result = qAndr[1];
        }
    // '+', '-' 부호가 아닐 경우
    } else {
        printf("It's the incorrect operation. Please put '+' or '-'");
        result = 0;
    }

    return result;
}

/*
 * @brief      모듈러 곱셈 연산을 하는 함수.
 * @param      uint x       : 피연산자1.
 * @param      uint y       : 피연산자2.
 * @param      uint n       : 모듈러 값.
 * @return     uint result  : 피연산자의 곱셈에 대한 모듈러 연산 값. (a x b) mod n
 */
uint ModMul(uint x, uint y, uint n) {
    uint result = 0;
    uint temp = 0;
    uint count = 0;
    uint qAndr[] = {0, 0};
    uint overflowModValue = 0;
    uint tempIndex = 0;
    uint binaryArray[32] = 
        {0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0};

    customMod(0xffffffff, n, qAndr);
    overflowModValue = qAndr[1];

    // 해당하는 이진수 index 체크
    while (y != 0) {
        binaryArray[count] = y & 1;
        y >>= 1;
        count++;
    }

    // 각 이진수 인덱스별로 계산한 결과를 binaryArray에 insert
    for (uint i = 0; i < count; i++) {
        tempIndex = i;
        // 해당 이진수값이 존재 할 경우
        if (binaryArray[i] == 1) {
            temp = x;
            // 이진수 첫번째 index 일 경우
            if (tempIndex == 0) {
                binaryArray[i] = temp;
            }
            // 2제곱 연산
            while (tempIndex != 0) {
                // 오버플로우가 일어날 경우
                if (temp >> 31 == 1) {
                    temp <<= 1;
                    temp += (overflowModValue + 1);
                // 오버플로우가 안 일어날 경우
                } else {
                    temp <<= 1;
                }
                customMod(temp, n, qAndr);
                temp = qAndr[1];
                tempIndex--;

                if (tempIndex == 0) {
                    binaryArray[i] = qAndr[1];
                }
            }
        }
    }

    // 각 추출된 결과를 총합
    for (uint i = 0; i < count; i++) {
        if (binaryArray[i] > 0) {
            result = ModAdd(result, binaryArray[i], '+', n);
        }
    }

    return result;
}

/*
 * @brief      모듈러 거듭제곱 연산을 하는 함수.
 * @param      uint base   : 피연산자1.
 * @param      uint exp    : 피연산자2.
 * @param      uint n      : 모듈러 값.
 * @return     uint result : 피연산자의 연산에 대한 모듈러 연산 값. (base ^ exp) mod n
 */
uint ModPow(uint base, uint exp, uint n) {
    uint result = 1;
    uint count = 0;
    uint temp = 0;
    uint binaryArray[32] = 
        {0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0};
    
    // 이진수로 변환했을 때 인덱스 위치랑, 존재 여부 찾기
    while (exp != 0) {
        binaryArray[count] = exp & 1;
        exp >>= 1;
        count++;
    }

    // 각 존재하는 인덱스에서 ModMul 했을 때 binaryArray 에 누적값 기록
    for (uint i = 0; i < count; i++) {
        if (i == 0) {
            temp = ModMul(base, 1, n);
        } else {
            temp = ModMul(temp, temp, n);
        }
        binaryArray[i] *= temp;
    }
    
    // 존재하는 누적된 value 들만 순차적으로 ModMul 하기
    for (uint i = 0; i < count; i++) {
        if (binaryArray[i] > 0) {
            result = ModMul(result, binaryArray[i], n);
        }
    }
    
    return result;
}

/*
 * @brief      입력된 수가 소수인지 입력된 횟수만큼 반복하여 검증하는 함수.
 * @param      uint testNum   : 임의 생성된 홀수.
 * @param      uint repeat    : 판단함수의 반복횟수.
 * @return     uint result    : 판단 결과에 따른 TRUE, FALSE 값.
 */
bool IsPrime(uint testNum, uint repeat) {
    bool mayBePrime = TRUE;
    uint qAndr[] = {0, 0};
    uint d = testNum - 1;
    uint s = 0;
    uint a = 0;
    uint d2j = 0;
    uint temp = 0;

    if (testNum == 0) {
        return FALSE;
    }

    // Check if 'n' is the odd number (첫 비트가 1인 것을 거름)
    if ((testNum & 1) == 0) {
        if (testNum != 2) {
            return FALSE;
        } else {
            return TRUE;
        }
    }

    do {
        customMod(d, 2, qAndr);
        if (qAndr[1] == 1) {
            break;
        }
        d = qAndr[0];
        s++;
    } while (TRUE);

    // Miller-Rabin 시작
    for (uint i = 0; i < repeat; i++) {
        // Create 'a'
        do {
            a = (uint) (WELLRNG512a() * testNum);
        } while (a <= 1 || a == testNum);

        // Check using GCD
        if (GCD(a, testNum) != 1) {
            mayBePrime = FALSE;
            break;
        }

        d2j = ModPow(a, d, testNum);
        // s == 1 인데 d2j 가 1 이나 -1 이 아니면 무조건 소수가 아님
        if (s == 1) {
            if (d2j != 1 && d2j != (testNum - 1)) {
                mayBePrime = FALSE;
                break;
            }
        }
        // 만약 이것이 성립하면 소수일 수 있음
        if (d2j == 1 || d2j == (testNum - 1)) {
            continue;
        }

        for (uint j = 1; j < s; j++) {
            d2j = ModMul(d2j, d2j, testNum);

            if (d2j == (testNum - 1)) {
                break;
            }
            if (j == (s - 1)) {
                mayBePrime = FALSE;
            }
        }
        
        // 만약 소수가 아니면 바로 종료
        if (mayBePrime == FALSE) {
            break;
        }
    }

    return mayBePrime;
}

/*
 * @brief       모듈러 역 값을 계산하는 함수.
 * @param       uint a      : 피연산자1.
 * @param       uint m      : 모듈러 값.
 * @return      uint result : 피연산자의 모듈러 역수 값.
 */
uint ModInv(uint a, uint m) {
    uint x[] = {1, 0};
    uint xTemp = 0;
    uint r[] = {a, m};
    uint q = 0;
    uint temp = 0;

    while (TRUE) {
        if (*(r + 1) == 0) {
            break;
        }

        temp = *(r + 1);
        while (*r >= temp) {
            if (temp == 0)
                break;
            *r -= temp;
            q += 1;
        }

        temp = *r;
        *r = *(r + 1);
        *(r + 1) = temp;

        xTemp = *x;
        *x = *(x + 1);
        *(x + 1) = ModAdd(xTemp, ModMul(*x, q, m), '-', m);

        // init params
        temp = 0;
        xTemp = 0;
        q = 0;
    }
    
    return *x;
}

/*
 * @brief     RSA 키를 생성하는 함수.
 * @param     uint *p   : 소수 p.
 * @param     uint *q   : 소수 q.
 * @param     uint *e   : 공개키 값.
 * @param     uint *d   : 개인키 값.
 * @param     uint *n   : 모듈러 n 값.
 * @return    void
 */
void miniRSAKeygen(uint *p, uint *q, uint *e, uint *d, uint *n) {
    uint repeat = 100;
    uint qAndr[] = {0, 0};
    uint phi = 0;
    uint minPrimeNum = 33000; // 33000 보다 작으면 다른 소수를 찾는데 너무 오래걸림
    uint maxPrimeNum = 100000;

    // 소수 만들기
    while (!(*p > minPrimeNum && IsPrime(*p, repeat) == 1)) {
        *p = 0;
        *p = (uint) (WELLRNG512a() * maxPrimeNum);
    }

    customMod(0xffffffff, *p * 2, qAndr);
    minPrimeNum = qAndr[0];
    customMod(0xffffffff, *p, qAndr);
    maxPrimeNum = qAndr[0];

    while ((0xffffffff - (*p * *q)) >= 0x80000000) {
        *q = 0;
        while (!(*q > minPrimeNum && IsPrime(*q, repeat) == 1)) {
            *q = 0;
            *q = (uint) (WELLRNG512a() * maxPrimeNum);
        }
    }

    // n, e, d 만들기
    *n = *p * *q;
    phi = (*p - 1) * (*q - 1);

    do {
        *e = 0;
        *e = (uint) (WELLRNG512a() * phi);
    } while (!(GCD(*e, phi) == 1));

    *d = ModInv(*e, phi);

    // 검산
    printf("\n [ Verification ]");
    printf("\np * q = %u", (*p * *q));
    printf("\nphi = %u // It's the same value with (p - 1) * (q - 1)", phi);
    printf("\ne * d (mod phi) = %u", ModMul(*e, *d, phi));
    printf("\n\n");
}

/*
 * @brief     RSA 암복호화를 진행하는 함수.
 * @param     uint data   : 키 값.
 * @param     uint key    : 키 값.
 * @param     uint n      : 모듈러 n 값.
 * @return    uint result : 암복호화에 결과값
 */
uint miniRSA(uint data, uint key, uint n) {
    return ModPow(data, key, n);
}

int main(int argc, char* argv[]) {
    byte plain_text[4] = {0x12, 0x34, 0x56, 0x78};
    uint plain_data, encrpyted_data, decrpyted_data;
    uint seed = time(NULL);

    memcpy(&plain_data, plain_text, 4);

    // 난수 생성기 시드값 설정
    seed = time(NULL);
    InitWELLRNG512a(&seed);

    // RSA 키 생성
    miniRSAKeygen(&p, &q, &e, &d, &n);
    printf("0. Key generation is Success!\n ");
    printf("p : %u\n q : %u\n e : %u\n d : %u\n N : %u\n\n", p, q, e, d, n);

    // RSA 암호화 테스트
    encrpyted_data = miniRSA(plain_data, e, n);
    printf("1. plain text : %u\n", plain_data);    
    printf("2. encrypted plain text : %u\n\n", encrpyted_data);

    // RSA 복호화 테스트
    decrpyted_data = miniRSA(encrpyted_data, d, n);
    printf("3. cipher text : %u\n", encrpyted_data);
    printf("4. Decrypted plain text : %u\n\n", decrpyted_data);

    // 결과 출력
    printf("RSA Decryption: %s\n", (decrpyted_data == plain_data) ? "SUCCESS!" : "FAILURE!");

    return 0;
}
