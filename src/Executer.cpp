//
// Created by abdallah-selim on 3/11/25.
//
#include "../include/Executer.h"
#include <iostream>

using namespace std;

void executeInsert(const string &tableName, const vector<string> &values) {
    writeRecord(tableName, values);
    cout << "✅ Record inserted into " << tableName << endl;
}

void executeSelect(const string &tableName, const vector<string> &columns, const vector<Condition> &conditions) {
    displayQueryResults(tableName, columns, conditions);
}

void executeDelete(const string &tableName, int id) {
    if (deleteRecord(tableName, id)) {
        cout << "✅ Record with ID " << id << " deleted from " << tableName << endl;
    } else {
        cerr << "Error: Could not delete record" << endl;
    }
}

void executeCreateTable(const string &tableName, const string &columnsInfo) {
    createTable(tableName, columnsInfo);
    cout << "✅ Table '" << tableName << "' created with schema: " << columnsInfo << endl;
}
