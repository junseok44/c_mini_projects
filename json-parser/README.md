# read_file_all

파일 전체 읽기, 널 종료 보장(buf[len] = '\0')
실패 시: false 반환 + 에러 메시지 문자열 제공(예: "open failed: <errno>")
큰 파일(수 MB)도 처리(리사이즈 루프 or stat 기반 단번에 할당 정책 중 하나 선택)

존재 파일: fixtures/nested.json → len > 0, 마지막 바이트 '\0'
미존재: fixtures/nope.json → 실패 메시지 확인
큰 파일: dd if=/dev/zero of=fixtures/big.json bs=1k count=512 후 읽기(널 종료 확인)

# 토큰 지원 조건.

1단계 지원 토큰: { } [ ] : , + EOF + ERROR
(리터럴/문자열/숫자는 내일)

공백 스킵: space, tab, CR, LF. line/col 카운팅 규칙:
\n 만나면 line++, col = 1
그 외 문자는 col++
토큰 구조 출력 포맷(디버그용):
예) L1:C1 TK_LBRACE, L1:C2 TK_RBRACE …

# 정책

정상: 0
잘못된 사용법/알 수 없는 커맨드: 2
파일 읽기 실패: 2
파싱 실패: 1
get에서 경로 미존재/타입 불일치: 3

## 에러 메시지 형식

파싱 실패: line <L>, col <C>: <message>

파일 실패: file error: <path>: <reason>
