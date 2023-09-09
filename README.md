# BEST OF THE BEST 11th Team Project
### 문서형 악성코드 탐지 솔루션 클라이언트 프로그램
- 문서형 악성코드를 정적분석 방식으로 검사하고, 사용자의 문서 관리를 도와주는 프로그램입니다.
- 프로젝트 기간 : 2022.09 ~ 2022.12

## 기능
- 드래그 & 드롭, 또는 직접 파일 선택을 통한 문서 파일 검사
	- 지원하는 확장자 : hwp, pdf, docx, pptx, xlsx
- 미니필터 드라이버를 이용한 실시간 감시 및 파일 연산 제어
	- 새로운 문서 파일이 유입되거나, 실행될 경우 문서 파일 열람 차단
 	- 악성 여부 검사 후 정상적인 문서 열람 가능
  	- 파일 생성, 수정, 삭제에 대한 히스토리 기록
  	- 카카오톡, 잔디 등 외부에서 유입된 문서 파일이 어떤 경로로 유입되었는지 추적 및 기록
  		- ```ADS를 이용한 파일정보 관리장치 및 방법, 특허출원 제10-2022-0177598```
-  문서 파일 뷰어
	- 컴퓨터 내에 있는 문서 파일 표시
	- hwp, pdf, ppt, word, excel 확장자 필터 기능
 	- 문서 파일을 선택하여 단독 검사 및 복수 검사 가능
  	- 랜섬웨어 대비 백업 및 복구 가능
   	- 문서 열람 권한을 직접 수정하거나, 문서를 검역소로 이동할 수 있음
   	- 이미 검사한 문서의 경우 검사 보고서를 확인할 수 있음
- 검역소
	- 악성으로 판명되거나 의심되는 경우, 문서를 별도의 경로로 격리
 	- 문서를 열람하지 못하도록 파일의 내용을 인코딩하고, 파일명 변경
  	- 검역소에서 파일을 삭제 및 복수할 수 있고, 검사 보고서를 확인할 수 있음
- 클라이언트 프로그램 업데이트 및 백신 패턴 업데이트

## 개발 환경 및 사용한 기술
- Windows 10 x64
- Visual Studio Community 2022
- MFC(v143), C++
- Windows Driver Kit version 7.1.0
- Node.js v16.17.0 (Update Server)

## 실행 화면 및 기능
- 메인 화면
	- 수동 검사 및 실시간 감시 활성화 가능
 	- <img src="https://github.com/ehn1225/ehn1225/assets/5174517/324435be-668d-49b3-88f3-f470ada68b7b"  width="700"/>
- 문서 관리 기능
	- 문서 파일 확장자 유형별 필터 기능
 	- 컴퓨터 내의 모든 문서 파일을 표시하고, 각 파일을 관리할 수 있음.
   	- <img src="https://github.com/ehn1225/ehn1225/assets/5174517/75ce7154-e500-44c9-b51f-a57561f15a05"  width="700"/>
- 검역소
	- 문서 파일이 악성으로 의심되거나 악성으로 판단된 경우, 검역소로 격리하여 별도로 관리
  	- <img src="https://github.com/ehn1225/ehn1225/assets/5174517/86064866-7cfb-4174-a660-63eed7763b93"  width="700"/>

- 프로그램 업데이트 및 백신 패턴 업데이트
	- <img src="https://github.com/ehn1225/ehn1225/assets/5174517/df53b90c-0e93-411d-9397-4beb45012c8a"  width="700"/>

## 검사 방법
1. 서버 주소 확인(기본 설정 ip : 127.0.0.1, port : 42524)
   - 만약 서버 주소가 다를 경우 아래 "서버 주소 변경 방법"에 따라 서버 주소를 바꿔주면 됩니다.
3. 클라이언트 프로그램 실행
4. 클라이언트 프로그램에서 "파일선택" 버튼을 눌러 문서 파일 선택 후 "검사" 버튼 클릭
5. 서버에 연결할 수 없을 경우 "서버에 연결할 수 없습니다" 메시지박스 출력됨
6. 서버에 성공적으로 파일 전송이 완료되면 검사 결과가 화면에 출력되고, 검사 보고서를 웹으로 확인할 수 있음
- 실시간 검사의 경우, 커널 드라이버 설치 및 실행 후 실시간 감시를 활성화

## 서버 주소 변경 방법
1. 클라이언트 프로그램이 존재하는 폴더로 이동
2. ```config.dat``` 파일 생성
3. ```config.dat``` 파일의 첫번째 줄에는 ```서버 ip주소```, 2번째 줄에는 ```백신 서버 포트```, 3번째 줄에는 ```업데이트 서버 포트``` 입력 후 저장
4. 프로그램 다시 실행

## 커널 드라이버 실행 방법
1. Disable driver signature enforcement 모드로 윈도우 부팅
2. ```scanner.inf```, ```scanner.sys``` 다운로드
3. ```scanner.inf``` 마우스 우클릭 -> 설치
4. 드라이버 실행은 관리자 권한으로 CMD 실행 후 ```sc start scanner```
5. 이후 클라이언트 프로그램 실행 (관리자 권한으로)
6. 드라이버 종료는 ```sc stop scanner```
7. 드라이버 삭제는 ```sc delete scanner```

## 커널 드라이버 빌드 방법
1. Windows Driver Kit version 7.1.0 설치
2. C:\WinDDK\7600.16385.1\src\filesys\miniFilter\scanner\inc 경로로 ```scanuk.h``` 파일 덮어쓰기
3. C:\WinDDK\7600.16385.1\src\filesys\miniFilter\scanner\filter 경로로 ```scanner.c```, ```scanner.h``` 파일 덮어쓰기
4. x64 Checked Build Environment 실행 후 ```cd  C:\WinDDK\7600.16385.1\src\filesys\miniFilter\scanner\``` 명령어 실행
5. ```build -cZ``` 명령어로 빌드 수행
6. C:\WinDDK\7600.16385.1\src\filesys\miniFilter\scanner\filter 경로에 빌드 결과 확인
7. 빌드된 scanner.sys를 scanner.inf를 이용해 설치

## Version history
- 1.0.0 : ProtoType 1st (Socket을 이용한 파일 전송 구현)
- 2.0.0 : ProtoType 2nd (Mini Filter Driver, File Viewer 구현)
- 2.1.0 : ProtoType 3rd_1 (파일 연산 미니필터 드라이버 연동, 파일 변경 이력 기능 추가)
- 2.2.0 : ProtoType 3rd_2 (LocalFileList DB, Tray Mode, Notification 기능 추가)
- 2.3.0 : ProtoType 3rd_3 (Local File Viewer, Tab Change, UI Design)
- 2.4.0 : ProtoType 3rd_4 (실시간 문서 열람 제어 수정, 검사 결과에 따른 열람 제어, 자동 업데이트, 화면 해상도 문제 수정)
- 2.5.0 : ProtoType 3rd_5 (블랙리스트 패턴 업데이트 일부 구현, 검역소 기능 추가)
- 2.6.0 : ProtoType 4th_1 (소켓을 이용한 블랙리스트 패턴 업데이트 구현 및 DB 저장 기능 구현)
- 2.6.1 : ProtoType 4th_2 (랜섬웨어 대비 문서 백업 및 복구 기능 구현)
- 2.6.2 : ProtoType 4th_3 (결과 보고서 확인 로직 수정)
- 2.6.3 : ProtoType 4th_4 (소스코드 정리 & 경로 오류 수정)
- 2.6.4 : ProtoType 4th_5 (ADS:NOSPEAR 내용 수정)

## Alternate Data Stream 명령어(CMD)
- ADS 존재 여부 확인 : ```dir /r```
- 특정 파일의 ADS 값 확인 : more < 파일명.파일확장자:ADS이름
	- ex) ```more < hello.txt:Zone.Identifier```
- 특정 파일의 ADS 값 수정 : notepad 파일명.파일확장자:ADS이름
	- ex) ```notepad hello.txt:Zone.Identifier```

## 참고자료
- 클라이언트 프로그램 업데이트 : https://www.codeproject.com/Articles/1205548/An-efficient-way-for-automatic-updating
- OpenSSL - SHA256 : https://cypsw.tistory.com/70
- 소켓 파일 업로드 : https://codingwell.tistory.com/59
- SQLite : https://mangsby.com/blog/programming/c/c-c-sqlite-데이터베이스-사용하기-1-기본-번역/
- 파일에 접근하는 Process의 PID : https://doexercise.github.io/post/WindowsDriver/
- 미니필터 드라이버 예제 코드 이해 : https://geun-yeong.tistory.com/60
- IRP_MJ_CREATE(IFS) : https://learn.microsoft.com/ko-kr/windows-hardware/drivers/ifs/irp-mj-create?source=recommendations
- IO_STATUS_BLOCK : https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_io_status_block
- FLT_CALLBACK_DATA : https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/fltkernel/ns-fltkernel-_flt_callback_data
- [MFC]트레이아이콘(Tray)과 풍선(Balloon) 알림 사용 : https://master-hun.tistory.com/85
- [MFC] Tab Control(탭 컨트롤) : https://balabala.tistory.com/36
