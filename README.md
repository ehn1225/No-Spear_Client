# client
Vaccine client

<h2>Manual Upload Usage</h2>
1. build and execute NoSpear_Client<br>
2. build sslsource<br>
3. execute /sslsource/tel.exe with cmd<br>
4. since client program hardcoded "127.0.0.1", "42524", execute "tel.exe *42524" where cmd prompt

<h2>서버 주소 변경 방법</h2>
1. client 실행 파일 경로로 이동<br>
2. config.dat 파일 생성<br>
3. config.dat 파일의 첫줄에는 서버 ip주소, 2번째 줄에는 서버 포트 입력 후 저장<br>
4. 프로그램 다시 실행<br>

<h2>커널 드라이버 실행 방법</h2>
1. Kernel.inf, Kernel.sys 복사<br>
2. Kernel.inf 마우스 우클릭 -> Install<br>
3. 관리자 권한으로 CMD 실행 후 "sc start scanner"<br>
4. 이후 scanner 클라이언트 프로그램 실행(scanuser.exe, cmd, 관리자권한으로)<br>
5. kernel.sys 종료는 "sc stop scanner"<br>
