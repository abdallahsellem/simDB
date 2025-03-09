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

void writeHeader(const string &tableName,const DBHeader &header)  ;
struct DBHeader readHeader(const string &tableName) ;
ColumnInfo parseSchemaLine(const string &line) ;
vector<ColumnInfo> readSchema(const string &tableName);
int calculateRecordSize(const string &tableName) ;
void writeRecord(const string &tableName) ;
void readRecords(const string &tableName) ;

void createTable() ;
bool updateIndex(const string &tableName, const int offset)  ;
void readRecordWithIndex(const string &tableName, int id) ;
bool updateRecord(const string &tableName, int id, const vector<string> &newValues) ;
bool deleteRecord(const string &tableName, int id) ;