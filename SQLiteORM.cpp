#include "SQLiteORM.h"
SQLiteORM::SQLiteORM()
{
}
SQLiteORM::SQLiteORM(string fileName)
{
    this->fileName = fileName;
}
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
    sqlite3_open(this->fileName.c_str(), &this->db);
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
    string sql = this->insertTemplate;
    for (SQLiteField& field : fieldsInsert) {
        if (field.value == "") {
            field.value = "0";
        }
    }
    this->replace(sql, this->tableNameTemplate, this->tableName);
    this->replace(sql, this->fieldNamesTemplate, this->getFieldNames(fieldsInsert));
    this->replace(sql, this->fieldValuesTemplate, this->getFieldValues(fieldsInsert));
    sqlite3_exec(this->db, sql.c_str(), 0, 0, &this->err);
    return this;
}
SQLiteORM* SQLiteORM::selectAll()
{
    return this;
}