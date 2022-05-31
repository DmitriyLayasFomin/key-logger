#pragma once
#include <iostream>
#include <winsqlite/winsqlite3.h>
#include <string>
#include <vector>
using namespace std;
class SQLiteField {
public:
	string name;
	string type;
	string value;
	bool primary;
	bool autoincrement;
	SQLiteField(string name, string type, bool primary = false, bool autoincrement = false)
	{
		this->name = name;
		this->type = type;
		this->primary = primary;
		this->autoincrement = autoincrement;
	}
	void setValue(string value)
	{
		this->value = value;
	}
	string getValue()
	{
		return this->value;
	}
	
};

class SQLiteORM
{
	
public:
	SQLiteORM();
	SQLiteORM(string fileName);
	SQLiteORM* open();
	SQLiteORM* close();
	SQLiteORM* setTable(string tableName);
	SQLiteORM* createTable(vector<SQLiteField>fieldsInsert);
	SQLiteORM* insert(vector<SQLiteField> fieldsInsert);
	SQLiteORM* selectAll();
private:
	char* err = 0;
	sqlite3* db = 0;
	string fileName;
	string tableName;
	const string fieldNameTemplate = "#FIELD_NAME#";
	const string fieldNamesTemplate = "#FIELD_NAMES#";
	const string fieldValuesTemplate = "#VALUES#";
	const string fieldTypeTemplate = "#FIELD_TYPE#";
	const string primaryTemplate = "#PRIMARY_KEY#";
	const string autoincrementTemplate = "#AUTO_INCREMENT#";
	const string fieldsTemplate = "#FIELDS#";
	const string tableNameTemplate = "#TABLE_NAME#";
	const string fieldInsertTemplate = "#FIELD_NAME# #FIELD_TYPE# #PRIMARY_KEY# #AUTO_INCREMENT#";
	const string createTableTemplate = "CREATE TABLE #TABLE_NAME# (#FIELDS#);";
	const string insertTemplate = "INSERT INTO #TABLE_NAME# (#FIELD_NAMES#) VALUES (#VALUES#);";
	const string selectTemplate = "SELECT * FROM #TABLE_NAME#;";

	bool replace(std::string& str, string from, string to);
	string trim(string str);
	string getFieldCreateTable(SQLiteField field);
	string getFieldNames(vector<SQLiteField> fields);
	string getFieldValues(vector<SQLiteField> fields);
	
};
