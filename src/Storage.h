//
// Created by abdallah-selim on 3/7/25.
//
#include <bits/stdc++.h>
using namespace std ;
const string dataPath = "../data/";
const string dataFileType = ".bin";
const string schemaFileType = ".schema";
const string indexFileType =".idx";
const string ID_COLUMN = "ID";

constexpr int headerSize=60 ;

struct DBHeader {
    char magic[4];       // File identifier (e.g., "SDB1")
    int numRecords; // Number of records in the file
    uint32_t freeOffset; // Next free space for writing new records
    char reserved[48];   // Reserved space for future use (padding)

    DBHeader() {
        memcpy(magic, "SDB1", 4);
        numRecords = 0;
        freeOffset = sizeof(DBHeader);
        memset(reserved, 0, sizeof(reserved));
    }
};
struct ColumnInfo {
    string name;
    string type;  // "int", "string(20)", "float", etc.
    int size;  // Size in bytes (calculated for strings)
};

// Define a simple condition structure
struct Condition {
    string columnName;
    string operatorType;  // "=", "<", ">", "<=", ">=", "!="
    variant<int, float, string> value;

    // Constructor for int values
    Condition(const string& col, const string& op, int val)
        : columnName(col), operatorType(op), value(val) {}

    // Constructor for float values
    Condition(const string& col, const string& op, float val)
        : columnName(col), operatorType(op), value(val) {}

    // Constructor for string values
    Condition(const string& col, const string& op, const string& val)
        : columnName(col), operatorType(op), value(val) {}
};

// Template function to compare values based on operator
template <typename T>
bool applyComparison(const T &recordValue, const T &conditionValue, const string &op) {
    if (op == "=") return recordValue == conditionValue;
    if (op == "!=") return recordValue != conditionValue;
    if (op == "<") return recordValue < conditionValue;
    if (op == "<=") return recordValue <= conditionValue;
    if (op == ">") return recordValue > conditionValue;
    if (op == ">=") return recordValue >= conditionValue;
    return false;
}

void writeHeader(const string &tableName,const DBHeader &header)  ;
struct DBHeader readHeader(const string &tableName) ;
ColumnInfo parseSchemaLine(const string &line) ;
vector<ColumnInfo> readSchema(const string &tableName);
int calculateRecordSize(const string &tableName) ;
void writeRecord(const string &tableName,vector<string>values) ;
void readRecords(const string &tableName) ;

void createTable(const string &tableName,const string &columnsInfoPartq) ;
bool updateIndex(const string &tableName, const int offset)  ;
void readRecordWithIndex(const string &tableName, int id) ;
bool updateRecord(const string &tableName, int id, const vector<string> &newValues) ;
bool deleteRecord(const string &tableName, int id) ;

vector<vector<variant<int, float, string>>> getRecordsWithCondition(
    const string& tableName,
    const vector<string>& columnsToReturn,
    const vector<Condition>& conditions
);
void displayQueryResults(const string& tableName,
                          const vector<string>& columns,
                          const vector<Condition>& conditions) ;