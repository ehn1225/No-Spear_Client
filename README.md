# client
<h1>Best of the Best 11th Team Project</h1>
No-Spear Vaccine Client Program<br>
문서형 악성코드 탐지 솔루션 클라이언트 프로그램<br>
※검사서버와 업데이트 서버가 중단된 관계로 프로그램 최초 실행이 오래걸립니다. (약 1분 소요)<br>
2023-02-20<br>

<h2>개발 환경</h2>
<ul>
	<li>Windows 10 x64</li>
	<li>Visual Studio Community 2022</li>
	<li>MFC(v143), C++</li>
	<li>Windows Driver Kit version 7.1.0</li>
	<li>Node.js v16.17.0(Update Server)</li>
</ul>

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
3. config.dat 파일의 첫줄에는 서버 ip주소, 2번째 줄에는 백신 서버, 3번째 줄에는 업데이트 서버 포트 입력 후 저장<br>
4. 프로그램 다시 실행<br>

<h2>커널 드라이버 실행 방법</h2>
1. Disable driver signature enforcement 모드로 윈도우 부팅<br>
2. github에서 scanner.inf, scanner.sys 다운로드<br>
3. scanner.inf 마우스 우클릭 -> 설치<br>
4. 드라이버 실행은 관리자 권한으로 CMD 실행 후 "sc start scanner"<br>
5. 이후 클라이언트 프로그램 실행 (관리자 권한으로)<br>
6. 드라이버 종료는 "sc stop scanner"<br>
7. 드라이버 삭제는 "sc delete scanner"<br>

<h2>커널 드라이버 빌드 방법</h2>
1. Windows Driver Kit version 7.1.0 설치<br>
2. C:\WinDDK\7600.16385.1\src\filesys\miniFilter\scanner\inc 경로로 scanuk.h 파일 덮어쓰기<br>
3. C:\WinDDK\7600.16385.1\src\filesys\miniFilter\scanner\filter 경로로 scanner.c, scanner.h 파일 덮어쓰기<br>
4. x64 Checked Build Environment 실행 후 "cd  C:\WinDDK\7600.16385.1\src\filesys\miniFilter\scanner\" 명령어 실행<br>
5. build -cZ 명령어로 빌드 수행<br>
6. C:\WinDDK\7600.16385.1\src\filesys\miniFilter\scanner\filter 경로에 빌드 결과 확인<br>
7. 빌드된 scanner.sys를 scanner.inf를 이용해 설치<br>

<h2>Version</h2>
<h4>1.0.0 : ProtoType 1st(File Upload)</h4>
<h4>2.0.0 : ProtoType 2nd(Mini Filter, File Viewer)</h4>
<h4>2.1.0 : ProtoType 3rd_1(Live Protect History, ADS Rules)</h4>
<h4>2.2.0 : ProtoType 3rd_2(LocalFileList DB, Tray Mode, Notification)</h4>
<h4>2.3.0 : ProtoType 3rd_3(Local File Viewer, Tab Change, UI Design)</h4>
<h4>2.4.0 : ProtoType 3rd_4(Live Protect, Diagnose Status, Auto Update, 화면 해상도 문제 수정)</h4>
<h4>2.5.0 : ProtoType 3rd_5(블랙리스트 패턴 업데이트 일부 구현, 검역소)</h4>
<h4>2.6.0 : ProtoType 4rd_1(블랙리스트 패턴 업데이트)</h4>
<h4>2.6.1 : ProtoType 4rd_2(파일 백업 및 복구)</h4>
<h4>2.6.2 : ProtoType 4rd_3(결과 보고서 확인 로직 수정)</h4>
<h4>2.6.3 : ProtoType 4rd_4(소스코드 정리 & 경로 오류 수정)</h4>
<h4>2.6.4 : ProtoType 4rd_5(ADS:NOSPEAR 내용 수정)</h4>

<h2>Alternate Data Stream 명령어(CMD)</h2>
ADS 존재 여부 확인 : dir /r <br>
특정 파일의 ADS 값 확인 : more < 파일명.파일확장자:ADS이름<br>
특정 파일의 ADS 값 수정 : notepad 파일명.파일확장자:ADS이름<br>
ex) more < hello.txt:Zone.Identifier<br>
ex) notepad hello.txt:Zone.Identifier<br>

<h2>참고자료</h2>
클라이언트 프로그램 업데이트 : https://www.codeproject.com/Articles/1205548/An-efficient-way-for-automatic-updating<br>
OpenSSL - SHA256 : https://cypsw.tistory.com/70<br>
소켓 파일 업로드 : https://codingwell.tistory.com/59<br>
SQLite : https://mangsby.com/blog/programming/c/c-c-sqlite-데이터베이스-사용하기-1-기본-번역/<br>
파일에 접근하는 Process의 PID : https://doexercise.github.io/post/WindowsDriver/<br>
미니필터 드라이버 예제 코드 이해 : https://geun-yeong.tistory.com/60<br>
IRP_MJ_CREATE(IFS) : https://learn.microsoft.com/ko-kr/windows-hardware/drivers/ifs/irp-mj-create?source=recommendations<br>
IO_STATUS_BLOCK : https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_io_status_block<br>
FLT_CALLBACK_DATA : https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/fltkernel/ns-fltkernel-_flt_callback_data<br>
[MFC]트레이아이콘(Tray)과 풍선(Balloon) 알림 사용 : https://master-hun.tistory.com/85<br>
[MFC] Tab Control(탭 컨트롤) : https://balabala.tistory.com/36<br>