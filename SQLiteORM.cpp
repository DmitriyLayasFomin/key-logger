#include "SQLiteORM.h"
#include <map>
#include <algorithm>
bool SQLiteORM::replace(std::string& str, string from, string to) 
{
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}
string SQLiteORM::trim(string str) {
    if (str.size() > 1) {
        str.erase(0, 1);
        str.erase(str.size() - 1);
    }
    return str;
}
string SQLiteORM::getFieldCreateTable(SQLiteField field)
{
    string str = this->fieldInsertTemplate;
    this->replace(str, this->fieldNameTemplate, field.name);
    this->replace(str, this->fieldTypeTemplate, field.type);
    if (field.primary) {
        this->replace(str, this->primaryTemplate, this->trim(this->primaryTemplate));
    }
    else {
        this->replace(str, this->primaryTemplate, "");
    }
    if (field.autoincrement) {
        this->replace(str, this->autoincrementTemplate, this->trim(this->autoincrementTemplate));
    }
    else {
        this->replace(str, this->autoincrementTemplate, "");
    }
    return str;
}
string SQLiteORM::getFieldNames(vector<SQLiteField> fields)
{
    string str;
    int i = 0;
    for (SQLiteField field : fields)
    {
        if (!field.autoincrement) {
            if (i > 0) {
                str += ",";
            }
            str += field.name;
            i++;
        }
    }
    return str;
}
string SQLiteORM::getFieldValues(vector<SQLiteField> fields)
{
    string str;
    int i = 0;
    for (SQLiteField field : fields)
    {
        if (!field.autoincrement) {
            if (i > 0) {
                str += ",";
            }
            str += field.value;
            i++;
        }
    }
    return str;
}
SQLiteORM* SQLiteORM::open()
{
    sqlite3_open(this->fileName, &this->db);
    return this;
}
SQLiteORM* SQLiteORM::close()
{
    sqlite3_close(this->db);
    return this;
}
SQLiteORM* SQLiteORM::setTable(string tableName)
{
    this->tableName = tableName;
    return this;
}
int SQLiteORM::callback(void *data, int argc, char** argv, char** azColName) {
    
    vector <SQLiteField> *fieldsVector = (vector <SQLiteField> *)data;
    for (int i = 0; i < argc; i++) {
        SQLiteField field = SQLiteField(string(azColName[i]), "");
        field.setValue(argv[i] ? argv[i] : "NULL");
        fieldsVector->push_back(field);
    }
    return 0;
}
SQLiteORM* SQLiteORM::createTable(vector<SQLiteField> fieldsInsert)
{
    string sql = this->createTableTemplate;
    string fieldsTemplate;
    int i = 0;
    for (SQLiteField field : fieldsInsert)
    {
        if (i > 0) {
            fieldsTemplate += ",";
        }
        fieldsTemplate += this->getFieldCreateTable(field);
        i++;
    }

    this->replace(sql, this->tableNameTemplate, this->tableName);
    this->replace(sql, this->fieldsTemplate, fieldsTemplate);
    sqlite3_exec(this->db, sql.c_str(), 0, 0, &this->err);
    return this;
}
SQLiteORM* SQLiteORM::insert(vector<SQLiteField> fieldsInsert)
{
    int rc = SQLITE_BUSY;
    string sql = this->insertTemplate;
    for (SQLiteField& field : fieldsInsert) {
        if (field.value == "") {
            field.value = "0";
        }
    }
    this->replace(sql, this->tableNameTemplate, this->tableName);
    this->replace(sql, this->fieldNamesTemplate, this->getFieldNames(fieldsInsert));
    this->replace(sql, this->fieldValuesTemplate, this->getFieldValues(fieldsInsert));
    do {
        rc = sqlite3_exec(this->db, sql.c_str(), 0, 0, &this->err);
    } while (rc == SQLITE_BUSY);
    
    return this;
}
void SQLiteORM::selectAll(void *result,string sortFieldName, string sortOrder = "ASC")
{
    string sql = this->selectTemplate;
    vector <SQLiteField> pvector;
    vector <map<string, string>>* vectorMap = (vector <map<string, string>> *)result;
    map<string, string> tmpMap;
    this->replace(sql, this->tableNameTemplate, this->tableName);
    this->replace(sql, this->fieldNameTemplate, sortFieldName);
    this->replace(sql, this->orderTemplate, sortOrder);
    sqlite3_exec(this->db, sql.c_str(), callback, &pvector, & this->err);
    for (auto &value : pvector) {
        if (tmpMap.size() > 0 && tmpMap.find(value.name) != tmpMap.end()) {
                vectorMap->push_back(tmpMap);
                tmpMap.clear();
        }
        tmpMap.insert({ value.name ,value.value });
    }
    sort(vectorMap->begin(), vectorMap->end(), [&sortFieldName](map<string, string> i1, map<string, string> i2){
        return (i1.find(sortFieldName)->second < i2.find(sortFieldName)->second);
    });
}

LPSTR SQLiteORM::getFileName()
{
    return this->fileName;
}
