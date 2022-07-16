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
    string str = fieldInsertTemplate;
    replace(str, fieldNameTemplate, field.name);
    replace(str, fieldTypeTemplate, field.type);
    if (field.primary) {
        replace(str, primaryTemplate, trim(primaryTemplate));
    }
    else {
        replace(str, primaryTemplate, "");
    }
    if (field.autoincrement) {
        replace(str, autoincrementTemplate, trim(autoincrementTemplate));
    }
    else {
        replace(str, autoincrementTemplate, "");
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
    sqlite3_open(fileName, &db);
    return this;
}
SQLiteORM* SQLiteORM::close()
{
    sqlite3_close(db);
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
    string sql = createTableTemplate;
    string fields;
    int i = 0;
    for (SQLiteField field : fieldsInsert)
    {
        if (i > 0) {
            fields += ",";
        }
        fields += getFieldCreateTable(field);
        i++;
    }

    replace(sql, tableNameTemplate, tableName);
    replace(sql, fieldsTemplate, fields);
    sqlite3_exec(db, sql.c_str(), 0, 0, &err);
    return this;
}
SQLiteORM* SQLiteORM::insert(vector<SQLiteField> fieldsInsert)
{
    int rc = SQLITE_BUSY;
    string sql = insertTemplate;
    for (SQLiteField& field : fieldsInsert) {
        if (field.value == "") {
            field.value = "0";
        }
    }
    replace(sql, tableNameTemplate, tableName);
    replace(sql, fieldNamesTemplate, getFieldNames(fieldsInsert));
    replace(sql, fieldValuesTemplate, getFieldValues(fieldsInsert));
    do {
        rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
    } while (rc == SQLITE_BUSY);
    
    return this;
}
void SQLiteORM::selectAll(void *result,string sortFieldName, string sortOrder = "ASC")
{
    string sql = selectTemplate;
    vector <SQLiteField> pvector;
    vector <map<string, string>>* vectorMap = (vector <map<string, string>> *)result;
    map<string, string> tmpMap;
    replace(sql, tableNameTemplate, tableName);
    replace(sql, fieldNameTemplate, sortFieldName);
    replace(sql, orderTemplate, sortOrder);
    sqlite3_exec(db, sql.c_str(), callback, &pvector, & err);
    for (auto& value : pvector) {
        if (tmpMap.size() > 0 && tmpMap.find(value.name) == tmpMap.begin()) {
                vectorMap->push_back(tmpMap);
                tmpMap.clear();
        }
        tmpMap.insert({ value.name ,value.value });
        if (tmpMap.size() > 0 && pvector.back().name == value.name) {
            vectorMap->push_back(tmpMap);
            tmpMap.clear();
        }
    }
    sort(vectorMap->begin(), vectorMap->end(), [&sortFieldName](map<string, string> i1, map<string, string> i2){
        return (atoi(i1.find(sortFieldName)->second.c_str()) < atoi(i2.find(sortFieldName)->second.c_str()));
    });
}

LPSTR SQLiteORM::getFileName()
{
    return fileName;
}
