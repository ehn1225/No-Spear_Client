#pragma once
typedef struct {
	char** pazResult;    /* Results of the query */
	int pnRow;           /* Number of result rows written here */
	int pnColumn;        /* Number of result columns written here */
	char* pzErrmsg;       /* Error msg written here */
}sqlite3_select, * sSelect;
class sqlite3;
class SQLITE{
	sqlite3* dbref;
	char* errmsg;
public:
	SQLITE();
	~SQLITE();
	int DatabaseOpen(CString dbname);
	int ExecuteSqlite(CString query);
	sqlite3_select SelectSqlite(CString query);
	static CString Utf8ToCString(char *);
	CString GetErrMessage();
};

