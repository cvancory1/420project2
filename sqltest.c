#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void insert(char *sql, sqlite3 *db) {
	char *err_msg = 0;
	int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);
	}
}

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	NotUsed = 0;

	for (int i = 0; i < argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}

	printf("\n");
	return 0;
}

int main() {
	//printf("%s\n", sqlite3_libversion());
	/* Program 1
	sqlite3 *db;
	sqlite3_stmt *res;

	int rc = sqlite3_open(":memory:", &db);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);

		return 1;
	}
	
	rc = sqlite3_prepare_v2(db, "SELECT SQLITE_VERSION()", -1, &res, 0);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);

		return 1;
	}
	
	rc = sqlite3_step(res);

	if (rc == SQLITE_ROW) {
		printf("%s\n", sqlite3_column_text(res, 0));
	}

	sqlite3_finalize(res);
	sqlite3_close(db);
	*/

	/* Program 2
	sqlite3 *db;
	char *err_msg = 0;

	int rc = sqlite3_open("test.db", &db);	

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);

		return 1;
	}

	char *sql = "DROP TABLE IF EXISTS Cars;"
				"CREATE TABLE Cars(Id INT, Name TEXT, Price INT);"
				"INSERT INTO Cars VALUES(1, 'Audi', 52642);"
				"INSERT INTO Cars VALUES(2, 'Mercedes', 57127);"
				"INSERT INTO Cars VALUES(3, 'Skoda', 9000);"
				"INSERT INTO Cars VALUES(4, 'Volvo', 29000);"
				"INSERT INTO Cars VALUES(5, 'Bentley', 35000);"
				"INSERT INTO Cars VALUES(6, 'Citroen', 21000);"
				"INSERT INTO Cars VALUES(7, 'Hummer', 41400);"
				"INSERT INTO Cars VALUES(8, 'Volkswagen', 21600);";


	rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);

		return 1;
	}

	sqlite3_close(db);
	*/
	sqlite3 *db;
	char *err_msg = 0;

	int rc = sqlite3_open("meta.db", &db);	

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);

		return 1;
	}
	char *sql = "DROP TABLE IF EXISTS Meta;"
				"CREATE TABLE Meta(ind INT, id TEXT, title TEXT, author TEXT, abstract TEXT);"; 

	rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);

		return 1;
	}
	

	const char *fname = "testMetadata.txt";

	FILE *meta;
	meta = fopen(fname, "r");
	int count = 0;
	int index = 0;
	char buf[255];
	char *id = malloc(255*sizeof(char));
	char *title = malloc(255*sizeof(char));
	char *author = malloc(255*sizeof(char));
	char *abstract = malloc(255*sizeof(char));
	char ins[512];
	while (fgets(buf, 255, meta)) {
		buf[strlen(buf)-1] = '\0';
		//printf("%s\n", buf);

		if (count % 5 == 0) {
			sprintf(id, "%s", buf);
		}
		else if (count % 5 == 1) {
			sprintf(title, "%s", buf);
		}
		else if (count % 5 == 2) {
			sprintf(author, "%s", buf);
		}
		else if (count % 5 == 3) {
			sprintf(abstract, "%s", buf);
		}
		else {
			sprintf(ins, "INSERT INTO Meta VALUES(%d, \'%s\', \'%s\', \'%s\', \'%s\');", index, id, title, author, abstract);
			//char *sql = "INSERT INTO Meta VALUES(%s, %s, %s, %s
			//rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
			//printf("%s\n", ins);
			insert(ins, db);
			index++;
		}
		count++;
	}
	//sql = "select ind from Meta where id=\'0704.001\';";
	//char *test = "select * from Meta;";
	char *test = "select ind from Meta where id=\'0704.0001\';";
	rc = sqlite3_exec(db, test, callback, 0, &err_msg);

	free(id);
	free(title);
	free(author);
	free(abstract);
	sqlite3_close(db);
	return 0;
}
