# client
Vaccine client

<h2>Manual Upload Usage</h2>
1. 서버 주소 확인(기본 설정 ip : 127.0.0.1, port : 42524)<br>
	만약 서버 주소가 다를 경우 아래 "서버 주소 변경 방법"에 따라 서버 주소를 바꿔주면 됩니다.<br>
2. 클라이언트 프로그램 실행<br>
3. 클라이언트 프로그램에서 "파일선택" 버튼을 눌러 문서 파일 선택 후 "검사" 버튼 클릭<br>
4. 서버에 연결할 수 없을 경우 "서버에 연결할 수 없습니다" 메시지박스 출력됨<br>
5. 서버에 성공적으로 파일 전송이 완료되면, 검사 결과가 메시지박스로 출력됨<br>

<h2>서버 주소 변경 방법</h2>
1. client 프로그램이 존재하는 폴더로 이동<br>
2. config.dat 파일 생성<br>
3. config.dat 파일의 첫줄에는 서버 ip주소, 2번째 줄에는 서버 포트 입력 후 저장<br>
4. 프로그램 다시 실행<br>

<h2>커널 드라이버 실행 방법</h2>
1. Disable driver signature enforcement 모드로 윈도우 부팅<br>
2. github에서 scanner.inf, scanner.sys 다운로드<br>
3. scanner.inf 마우스 우클릭 -> 설치<br>
4. 드라이버 실행은 관리자 권한으로 CMD 실행 후 "sc start scanner"<br>
5. 이후 클라이언트 프로그램 실행 (관리자 권한으로)<br>
6. 드라이버 종료는 "sc stop scanner"<br>
7. 드라이버 삭제는 "sc delete scanner"<br>
