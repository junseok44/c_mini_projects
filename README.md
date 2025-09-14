# C 언어 학습 실습 프로젝트

C 언어와 시스템 프로그래밍을 공부하면서 만든 실습 프로젝트들입니다. 


## 📁 만들어본 프로젝트들

| 프로젝트 | 배운것 | 난이도 | 만든것 |
|---------|----------|---------|------------|
| [**mycp**](mycp/) | 파일 다루기, 재귀 | ⭐⭐ | cp 명령어 흉내내기 |
| [**stack-calculator**](stack-calculator/) | 스택, 알고리즘 | ⭐⭐⭐ | 계산기 (괄호도 되는) |
| [**json-parser**](json-parser/) | 파싱, 메모리 관리 | ⭐⭐⭐⭐ | JSON 읽고 원하는 값 뽑기 |
| [**mini-bash-like-shell**](mini-bash-like-shell/) | 프로세스, 파이프 | ⭐⭐⭐⭐ | 간단한 셸 만들기 |
| [**socket-server**](socket-server/) | 네트워크 | ⭐⭐⭐⭐⭐ | 채팅 서버 (여러명 접속 가능) |

## 🤔 뭘 배웠나

### 시스템 프로그래밍
- **파일 다루기**: open, read, write 같은 저수준 함수들
- **프로세스**: fork해서 자식 만들고, exec로 다른 프로그램 실행
- **파이프**: 프로그램끼리 연결해서 데이터 주고받기
- **네트워크**: 소켓으로 서버 만들고 클라이언트들 관리하기

### 자료구조
- **스택**: 나중에 들어간게 먼저 나오는 구조 (계산기에서 썼음)
- **문자열 파싱**: 텍스트를 의미있는 조각들로 나누기
- **트리**: JSON 데이터를 트리 형태로 저장

### C언어
- **포인터**: 이중포인터, 함수포인터 등등
- **메모리 관리**: malloc/free 제대로 쓰기
- **구조체**: 복잡한 데이터 구조 만들기
- **모듈화**: .h 파일과 .c 파일 나누어서 정리

## 🛠️ 어떻게 실행하나

### 필요한 것들
- 리눅스나 맥 (또는 WSL)
- gcc 컴파일러
- make

### 빌드하기
```bash
# 각 폴더에서 make 하면 됨
cd json-parser && make && cd ..
cd mini-bash-like-shell && make && cd ..
cd mycp && make && cd ..
cd socket-server && make && cd ..
cd stack-calculator && make && cd ..
```

### 간단히 테스트해보기
```bash
# 1. 파일 복사 도구 테스트
echo "Hello World" > test.txt
./mycp/bin/mycp test.txt backup.txt

# 2. 계산기 테스트  
echo "2 + 3 * 4" | ./stack-calculator/bin/calc

# 3. JSON 파서 테스트
echo '{"name": "test", "value": 42}' > test.json
./json-parser/bin/json-parser parse test.json

# 4. 셸 테스트
./mini-bash-like-shell/bin/minishell
# 셸에서: echo "hello" | cat

# 5. 소켓 서버 테스트
./socket-server/bin/socket-server 8080 &
telnet localhost 8080
```

## 📖 각 프로젝트 설명

### 1. [mycp](mycp/) - cp 명령어 만들어보기
```bash
./bin/mycp source.txt dest.txt      # 파일 복사
./bin/mycp source_dir/ dest_dir/    # 폴더 복사
```
**배운 것**: 파일 읽기/쓰기, 재귀로 폴더 탐색

### 2. [stack-calculator](stack-calculator/) - 계산기 만들기
```bash
./bin/calc
enter the expression: ( 2 + 3 ) * 4
result: 20
```
**배운 것**: 스택 자료구조, 수식 계산 알고리즘

### 3. [json-parser](json-parser/) - JSON 파서 만들기
```bash
./bin/json-parser parse data.json           # JSON 파싱
./bin/json-parser get data.json "user.name" # 특정 값 찾기
```
**배운 것**: 문자열 파싱, 트리 구조, 메모리 관리

### 4. [mini-bash-like-shell](mini-bash-like-shell/) - 셸 만들기
```bash
./bin/minishell
$ ls | grep txt > results.txt
$ cd /tmp && pwd
$ exit
```
**배운 것**: 프로세스 만들기, 파이프, 리디렉션

### 5. [socket-server](socket-server/) - 채팅 서버 만들기
```bash
./bin/socket-server 8080
# 클라이언트가 접속하면:
/nick Alice
Hello everyone!
```
**배운 것**: 네트워크 프로그래밍, 여러 클라이언트 동시 처리

## 📁 폴더 구조

다 비슷하게 만들어놨어요:

```
project/
├── src/           # .c 파일들 
├── include/       # .h 파일들
├── build/         # 컴파일된 .o 파일들
├── bin/           # 최종 실행 파일
├── Makefile       # 빌드 설정
└── README.md      # 설명
```

### Make 명령어들
```bash
make           # 컴파일
make clean     # 빌드 파일 삭제
```


## 💡 도움되는 책들

공부하면서 참고한 책들:
- **APUE** (Advanced Programming in the UNIX Environment) - 시스템 프로그래밍
- **UNP** (Unix Network Programming) - 네트워크
- **CLRS** - 알고리즘

## 📝 기록

- C언어 기초부터 시작해서 조금씩 어려운 걸 해봤음
- 메모리 관리가 제일 어려웠고, 포인터는 여전히 헷갈림
- 네트워크 프로그래밍은 생각보다 재미있었음
