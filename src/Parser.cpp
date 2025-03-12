#include "../include/Parser.h"
#include "../include/Executer.h"
#include <algorithm>
#include <sstream>
#include <iostream>

using namespace std;

// Utility function to trim spaces and quotes
string trim(const string &s) {
    size_t first = s.find_first_not_of(" \t\"");
    size_t last = s.find_last_not_of(" \t\"");
    return (first == string::npos || last == string::npos) ? "" : s.substr(first, last - first + 1);
}

// **🔹 INSERT INTO table_name VALUES (value1, value2, ...)**
void parseInsert(const string &query) {
    stringstream ss(query);
    string command, intoPart, tableName, valuesPart;

    ss >> command >> intoPart >> tableName;
    if (intoPart != "INTO") {
        cerr << "Syntax Error: Expected INTO keyword" << endl;
        return;
    }

    string temp;
    ss >> temp;
    if (temp != "VALUES") {
        cerr << "Syntax Error: Expected VALUES keyword" << endl;
        return;
    }

    size_t start = query.find('(');
    size_t end = query.find_last_of(')');
    if (start == string::npos || end == string::npos || start >= end) {
        cerr << "Syntax Error: Invalid VALUES format" << endl;
        return;
    }

    valuesPart = query.substr(start + 1, end - start - 1);
    stringstream valuesStream(valuesPart);
    vector<string> values;
    string value;

    while (getline(valuesStream, value, ',')) {
        values.push_back(trim(value));
    }

    executeInsert(tableName, values);
}

// **🔹 SELECT columns FROM table_name WHERE column OP value**
void parseSelect(const string &query) {
    stringstream ss(query);
    string command, columnsPart, fromClause, tableName, whereKeyword;

    ss >> command >> columnsPart >> fromClause >> tableName;
    if (fromClause != "FROM") {
        cerr << "Syntax Error: Expected FROM keyword" << endl;
        return;
    }

    vector<string> columns;
    if (columnsPart == "*") {
        columns.push_back("*");
    } else {
        stringstream colStream(columnsPart);
        string column;
        while (getline(colStream, column, ',')) {
            columns.push_back(trim(column));
        }
    }

    vector<Condition> conditions;
    ss >> whereKeyword;
    transform(whereKeyword.begin(), whereKeyword.end(), whereKeyword.begin(), ::toupper);

    if (whereKeyword == "WHERE") {
        string columnName, op, value;
        ss >> columnName >> op >> value;
        if (!columnName.empty() && !op.empty() && !value.empty()) {
            if (isdigit(value[0])) {
                conditions.push_back({columnName, op, stoi(value)});
            } else {
                conditions.push_back({columnName, op, trim(value)});
            }
        }
    }

    executeSelect(tableName, columns, conditions);
}

// **🔹 DELETE FROM table_name WHERE ID = value**
void parseDelete(const string &query) {
    stringstream ss(query);
    string command, fromPart, tableName, whereClause, columnName, op;
    int id;

    ss >> command >> fromPart >> tableName;
    if (fromPart != "FROM") {
        cerr << "Syntax Error: Expected FROM keyword" << endl;
        return;
    }

    ss >> whereClause >> columnName >> op >> id;
    if (whereClause != "WHERE" || columnName != "ID" || op != "=") {
        cerr << "Syntax Error: DELETE must use 'WHERE ID = value'" << endl;
        return;
    }

    executeDelete(tableName, id);
}

// **🔹 CREATE TABLE table_name (column1 TYPE, column2 TYPE, ...)**
void parseCreateTable(const string &query) {
    stringstream ss(query);
    string command, tableWord, tableName;

    ss >> command >> tableWord >> tableName;
    if (tableWord != "TABLE") {
        cerr << "Syntax Error: Expected TABLE keyword" << endl;
        return;
    }

    size_t start = query.find('(');
    size_t end = query.find_last_of(')');
    if (start == string::npos || end == string::npos || start >= end) {
        cerr << "Syntax Error: Invalid CREATE TABLE format" << endl;
        return;
    }

    string columnsInfo = query.substr(start + 1, end - start - 1);
    executeCreateTable(tableName, columnsInfo);
}

// **🔹 Main Function: Determines which SQL command to parse**
void executeQuery(const string &query) {
    stringstream ss(query);
    string command;
    ss >> command;

    transform(command.begin(), command.end(), command.begin(), ::toupper);

    if (command == "INSERT") {
        parseInsert(query);
    } else if (command == "SELECT") {
        parseSelect(query);
    } else if (command == "DELETE") {
        parseDelete(query);
    } else if (command == "CREATE") {
        parseCreateTable(query);
    } else {
        cerr << "❌ Error: Unsupported SQL command" << endl;
    }
}
