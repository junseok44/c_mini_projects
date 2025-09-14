# JSON Parser

JSON 파싱과 쿼리 기능을 제공하는 C 라이브러리입니다.

## 기능

- **JSON 파싱**: JSON 문자열을 파싱하여 내부 데이터 구조로 변환
- **JSON 출력**: 파싱된 데이터를 다시 JSON 형태로 출력 (포맷팅/압축 모드)
- **경로 쿼리**: JSONPath 스타일의 경로 표현식으로 특정 값 추출

## 빌드

```bash
make
```

## 사용법

### 1. JSON 파일 파싱

```bash
./bin/json-parser parse data.json
```

JSON 파일을 파싱하고 포맷팅된 형태로 출력합니다.

### 2. 특정 값 추출

```bash
./bin/json-parser get data.json "user.name"
./bin/json-parser get data.json "items[0].price"
./bin/json-parser get data.json "config.settings.debug"
```

JSONPath 스타일의 경로 표현식을 사용하여 특정 값을 추출합니다.

#### 경로 표현식 예시

- `foo.bar`: 객체의 속성 접근
- `arr[0]`: 배열의 인덱스 접근
- `user.items[2].name`: 중첩된 구조 접근
- `config.settings`: 객체 전체 접근

## 프로젝트 구조

```
json-parser/
├── src/
│   ├── main.c           # 메인 실행 파일
│   ├── lexer_next.c     # JSON 토큰화
│   ├── parser.c         # JSON 파싱 로직
│   ├── value.c          # JSON 값 타입 구현
│   ├── dump.c           # JSON 출력 기능
│   ├── read_file.c      # 파일 읽기 유틸리티
│   └── resolve_path.c   # 경로 해석 로직
├── include/
│   ├── lexer_next.h     # 렉서 헤더
│   ├── parser.h         # 파서 헤더  
│   ├── value.h          # 값 타입 정의
│   ├── dump.h           # 출력 기능 헤더
│   └── read_file.h      # 파일 읽기 헤더
├── fixtures/            # 테스트용 JSON 파일들
└── bin/                 # 빌드된 실행 파일
```

## 지원하는 JSON 타입

- **Object**: `{"key": "value"}`
- **Array**: `[1, 2, 3]`
- **String**: `"hello world"`
- **Number**: `42`, `3.14`
- **Boolean**: `true`, `false`
- **Null**: `null`

## 예시

### 예시 JSON 파일 (test.json)
```json
{
  "name": "John Doe",
  "age": 30,
  "address": {
    "city": "Seoul",
    "zipcode": "12345"
  },
  "hobbies": ["reading", "coding", "music"]
}
```

### 명령어 예시
```bash
# 전체 JSON 파싱 및 출력
./bin/json-parser parse test.json

# 이름 추출
./bin/json-parser get test.json "name"
# 출력: "John Doe"

# 도시 추출  
./bin/json-parser get test.json "address.city"
# 출력: "Seoul"

# 첫 번째 취미 추출
./bin/json-parser get test.json "hobbies[0]"
# 출력: "reading"
```

## 오류 처리

- 잘못된 JSON 문법 시 파싱 에러와 위치 정보 제공
- 존재하지 않는 경로 접근 시 적절한 오류 메시지 출력
- 파일 읽기 실패 시 시스템 오류 메시지 제공