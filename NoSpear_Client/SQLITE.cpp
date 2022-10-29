#include "pch.h"
#include "sqlite3.h"
#include "SQLITE.h"
#pragma comment(lib,"sqlite3.lib")

SQLITE::SQLITE(){
	dbref = NULL;
	errmsg = NULL;
}

SQLITE::~SQLITE(){
	if (dbref != NULL)	sqlite3_close(dbref);
	if (errmsg != NULL) errmsg = NULL;
}

// Create Database
int SQLITE::DatabaseOpen(CString dbname){
	int rc;
	CT2CA pszConvertedAnsiString(dbname);
	std::string strdbname(pszConvertedAnsiString);
	rc = sqlite3_open(strdbname.c_str(), &dbref);
	if (rc) {
		sqlite3_close(dbref);
	}

	return rc;
}

//  Exec Query(create, insert, update, delete)
int SQLITE::ExecuteSqlite(CString query){
	int rc = 0;
	//CString to UTF-8
	std::string strQuery = CW2A(query, CP_UTF8);;
	rc = sqlite3_exec(dbref, strQuery.c_str(), NULL, NULL, &errmsg);

	return rc;
}

// Select Query
sqlite3_select SQLITE::SelectSqlite(CString query){
	sqlite3_select selectResult;
	CT2CA pszConvertedAnsiString(query);
	char** results = NULL;
	int rows;
	int columns;
	//const char *sqlSelect = "SELECT * FROM TB_SETTING;";

	int rc = sqlite3_get_table(dbref, pszConvertedAnsiString, &results, &rows, &columns, &errmsg);
	//sqlite3_get_table(pDbRef, sqlSelect, &results, &rows, &columns, &m_ErrMsg);

	selectResult.pazResult = results;
	selectResult.pnRow = rows;
	selectResult.pnColumn = columns;
	selectResult.pzErrmsg = errmsg;

	return selectResult;
}
