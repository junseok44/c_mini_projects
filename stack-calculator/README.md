# Stack Calculator - 중위 표기법 계산기

스택 자료구조를 사용하여 중위 표기법(infix notation) 수식을 계산하는 계산기입니다. Shunting Yard 알고리즘을 구현하여 중위 표기법을 후위 표기법으로 변환한 후 계산합니다.

## 기능

- **중위 표기법 지원**: 일반적인 수학 표기법 입력 (`2 + 3 * 4`)
- **연산자 우선순위**: 곱셈/나눗셈이 덧셈/뺄셈보다 우선 처리
- **괄호 처리**: 소괄호를 사용한 연산 우선순위 변경 지원
- **음수 지원**: 음수 입력 가능
- **스택 기반**: 두 개의 스택을 사용한 효율적인 계산

## 빌드

```bash
make
```

## 사용법

```bash
./bin/calc
```

프로그램 실행 후 수식을 입력하면 결과를 출력합니다:

```bash
$ ./bin/calc
enter the expression: 2 + 3 * 4
토큰 0 : 2
토큰 1 : +
토큰 2 : 3
토큰 3 : *
토큰 4 : 4
result: 14
```

## 지원하는 연산

### 산술 연산자
- `+`: 덧셈
- `-`: 뺄셈  
- `*`: 곱셈
- `/`: 나눗셈

### 우선순위
1. **높음**: `*`, `/` (우선순위 2)
2. **낮음**: `+`, `-` (우선순위 1)
3. **최고**: `()` 괄호 내부 연산

### 괄호
- `(` `): 연산 우선순위 변경

## 사용 예시

### 기본 연산
```bash
# 단순 덧셈
enter the expression: 5 + 3
result: 8

# 연산자 우선순위
enter the expression: 2 + 3 * 4
result: 14  # (2 + (3 * 4))

# 괄호 사용
enter the expression: ( 2 + 3 ) * 4
result: 20  # ((2 + 3) * 4)
```

### 복잡한 수식
```bash
# 혼합 연산
enter the expression: 10 - 2 * 3 + 8 / 4
result: 6   # ((10 - (2 * 3)) + (8 / 4))

# 중첩 괄호
enter the expression: ( 1 + 2 ) * ( 3 + 4 )
result: 21  # ((1 + 2) * (3 + 4))

# 음수 처리
enter the expression: -5 + 3
result: -2
```

## 구현 알고리즘

### 1. Shunting Yard 알고리즘
중위 표기법을 후위 표기법으로 변환:

```
입력: 2 + 3 * 4
후위: 2 3 4 * +
```

#### 변환 과정:
1. **숫자**: 즉시 출력 큐에 추가
2. **연산자**: 우선순위에 따라 스택 처리
3. **여는 괄호**: 스택에 푸시
4. **닫는 괄호**: 여는 괄호까지 스택 팝

### 2. 후위 표기법 계산
스택을 사용하여 후위 표기법 계산:

```
후위: 2 3 4 * +
과정: [2] → [2,3] → [2,3,4] → [2,12] → [14]
결과: 14
```

## 프로젝트 구조

```
stack-calculator/
├── src/
│   ├── main.c              # 메인 실행 파일
│   ├── calc.c              # 계산 로직 및 변환 알고리즘
│   ├── number_stack.c      # 숫자 스택 구현
│   └── token_stack.c       # 토큰(연산자) 스택 구현
├── include/
│   ├── calc.h              # 계산 함수 선언
│   ├── number_stack.h      # 숫자 스택 헤더
│   └── token_stack.h       # 토큰 스택 헤더
└── bin/
    └── calc                # 빌드된 실행 파일
```

## 핵심 함수

### calc.c
- `calc()`: 메인 계산 함수
- `infix_to_postfix()`: 중위→후위 변환
- `calculate_postfix_tokens()`: 후위 표기법 계산
- `precedence()`: 연산자 우선순위 반환
- `is_number()`: 숫자 토큰 판별

### number_stack.c
```c
void init2(NumberStack* stack);        // 스택 초기화
void push2(NumberStack* stack, int n); // 숫자 푸시
int pop2(NumberStack* stack);          // 숫자 팝
```

### token_stack.c
```c
void init(TokenStack* stack);          // 스택 초기화
void push(TokenStack* stack, char* op); // 토큰 푸시
char* pop(TokenStack* stack);          // 토큰 팝
char* peek(TokenStack* stack);         // 스택 톱 확인
int is_empty(TokenStack* stack);       // 스택 공백 확인
```

## 구현 특징

### 토큰화
```c
char *token = strtok(expr, " ");  // 공백으로 토큰 분리
```
- 공백을 구분자로 사용
- 각 토큰을 동적 할당하여 저장

### 우선순위 처리
```c
int precedence(const char* op) {
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 1;
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0) return 2;
    return 0;
}
```

### 메모리 관리
- 동적 할당된 토큰들의 적절한 해제
- 스택 구조의 안전한 초기화 및 정리

## 입력 형식

수식은 **공백으로 구분된 토큰**으로 입력해야 합니다:

✅ **올바른 형식:**
```
2 + 3 * 4
( 1 + 2 ) * 3
-5 + 10
```

❌ **잘못된 형식:**
```
2+3*4          # 공백 없음
(1+2)*3        # 괄호와 숫자 사이 공백 없음
```

## 제한사항

- **정수만 지원**: 소수점 연산 미지원
- **공백 필수**: 모든 토큰 사이에 공백 필요
- **최대 토큰**: 100개 토큰으로 제한 (STACK_MAX)
- **오류 처리**: 잘못된 수식에 대한 오류 검증 부족

## 학습 내용

이 프로젝트를 통해 학습할 수 있는 개념:

- **스택 자료구조**: LIFO 원리와 구현
- **Shunting Yard 알고리즘**: 중위→후위 변환
- **연산자 우선순위**: 수학적 우선순위 처리
- **재귀적 구조**: 괄호 처리 메커니즘
- **토큰화**: 문자열 파싱과 토큰 분리
- **메모리 관리**: 동적 할당과 해제
- **알고리즘 설계**: 두 단계 계산 과정

## 확장 가능성

- 실수 연산 지원
- 더 많은 연산자 (제곱, 나머지 등)
- 함수 지원 (sin, cos, log 등)
- 오류 검증 강화
- 변수 지원
