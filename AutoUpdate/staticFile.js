//No-Spear Client 프로그램 자동 업데이트 예제 서버
//npm i express	명령어로 express 모듈 설치
//node staticFile.js 명령어로 실행
//public 폴더에 클라이언트 프로그램 넣기(NoSpear_Client.2.6.0.0.exe)
// 모듈을 추출합니다.
const express = require('express');
// 서버를 생성합니다.
const app = express();
app.use(express.static('public'));
// request 이벤트 리스너를 설정합니다.
app.get('/', (request, response) => {
	//루트 경로 접속 시 최신 버젼 표시
    response.send('2.6.0.0');
});
// 서버를 실행합니다.
app.listen(80, () => {
    console.log('Server running at http://127.0.0.1:80');
});