//
// Created by abdallah-selim on 3/11/25.
//
#include <bits/stdc++.h>
using namespace std;
#include "Storage.h"
void executeInsert(const std::string &tableName, const std::vector<std::string> &values);
void executeSelect(const std::string &tableName, const std::vector<std::string> &columns, const std::vector<Condition> &conditions);
void executeDelete(const std::string &tableName, int id);
void executeCreateTable(const std::string &tableName, const std::string &columnsInfo);
