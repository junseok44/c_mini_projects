# Mini Bash-like Shell

간단한 bash 스타일의 셸 구현체입니다. 기본적인 명령어 실행, 파이프, 리디렉션, 내장 명령어를 지원합니다.

## 기능

### 내장 명령어
- `exit [code]`: 셸 종료 (선택적 종료 코드)
- `pwd`: 현재 작업 디렉토리 출력
- `cd [directory]`: 디렉토리 변경 (인수 없으면 홈 디렉토리로)
- `env`: 환경 변수 출력

### 파이프라인
- `command1 | command2`: command1의 출력을 command2의 입력으로 연결

### 리디렉션
- `command > file`: 출력을 파일로 리디렉션
- `command < file`: 파일에서 입력 읽기

### 기타 기능
- **외부 명령어 실행**: PATH에서 프로그램 탐색 및 실행
- **신호 처리**: 부모 셸은 Ctrl+C 무시, 자식 프로세스는 정상 처리
- **상태 코드**: 마지막 명령어의 종료 상태를 프롬프트에 표시
- **토큰화**: 공백, 따옴표, 이스케이프 문자 처리

## 빌드

```bash
make
```

## 사용법

```bash
./bin/minishell
```

### 프롬프트

```
[user@hostname pwd] (last_exit_code)$
```

- `pwd`: 현재 작업 디렉토리
- `last_exit_code`: 마지막 명령어의 종료 상태 (0이면 표시 안함)

### 명령어 예시

#### 기본 명령어
```bash
$ ls -la
$ pwd
$ cd /tmp
$ env | grep PATH
```

#### 파이프
```bash
$ ls | grep txt
$ cat file.txt | sort | uniq
$ ps aux | grep bash
```

#### 리디렉션
```bash
$ echo "hello" > output.txt
$ cat < input.txt
$ ls -la > directory_list.txt
```

#### 내장 명령어
```bash
$ pwd
/Users/username/current/directory

$ cd ..
$ pwd
/Users/username/current

$ env
PATH=/usr/bin:/bin:/usr/local/bin
HOME=/Users/username
...

$ exit 0
```

## 프로젝트 구조

```
mini-bash-like-shell/
├── src/
│   ├── main.c          # 메인 실행 로직, 명령어 처리
│   ├── parse.c         # 토큰화 및 파싱
│   └── sig.c           # 신호 처리
├── include/
│   ├── common.h        # 공통 헤더 및 유틸리티
│   ├── parse.h         # 파싱 관련 구조체 및 함수
│   └── sig.h           # 신호 처리 함수
└── bin/
    └── minishell       # 빌드된 실행 파일
```

## 구현 특징

### 토큰화
- **공백 처리**: 공백/탭/개행으로 토큰 분리
- **따옴표 지원**: 작은따옴표(`'`)와 큰따옴표(`"`) 지원
- **이스케이프**: 백슬래시(`\`)로 특수 문자 이스케이프
- **연산자 분리**: 파이프(`|`), 리디렉션(`<`, `>`)을 별도 토큰으로 처리

### 명령어 실행
- **프로세스 관리**: fork/exec 모델로 자식 프로세스에서 명령어 실행
- **대기**: waitpid로 자식 프로세스 완료 대기
- **오류 처리**: 실행 실패 시 적절한 오류 코드 반환

### 파이프 구현
- **파이프 생성**: pipe() 시스템 콜로 파이프 생성
- **프로세스 연결**: 두 개의 자식 프로세스 생성하여 연결
- **파일 디스크립터**: dup2()로 stdin/stdout 재지정

### 리디렉션 구현
- **파일 열기**: open() 시스템 콜로 파일 열기
- **디스크립터 재지정**: dup2()로 stdin/stdout 재지정
- **권한 설정**: 출력 파일 생성 시 적절한 권한(0644) 설정

## 제한사항

- 현재 단일 파이프(`|`)만 지원 (복수 파이프 미지원)
- 고급 리디렉션(`>>`, `2>`) 미지원
- 환경 변수 확장 미지원
- 작업 제어(background jobs) 미지원
- 명령어 히스토리 미지원

## 학습 내용

이 프로젝트를 통해 다음을 학습할 수 있습니다:

- **시스템 프로그래밍**: fork, exec, pipe, dup2, waitpid
- **파일 디스크립터**: stdin/stdout/stderr 조작
- **신호 처리**: SIGINT 처리
- **문자열 파싱**: 토큰화 및 구문 분석
- **프로세스 간 통신**: 파이프를 통한 IPC
