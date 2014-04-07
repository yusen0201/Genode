#include <base/printf.h>
#include <sqlite3.h>

int main() 
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_open("/test.sqlite", &db);
	if( rc ){
		PERR("Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return(1);
	}
	rc = sqlite3_exec(db, "CREATE TABLE Testing(Id INTEGER);", 0, 0, &zErrMsg);
	if( rc!=SQLITE_OK ){
		PERR("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	sqlite3_close(db);
	return 0;
}
