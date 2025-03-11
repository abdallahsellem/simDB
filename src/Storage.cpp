//
// Created by abdallah-selim on 3/7/25.
//

#include "Storage.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>

using namespace std;

// ==================== File Header Operations ====================

void writeHeader(const string &tableName, const DBHeader &header) {
    fstream file(dataPath + tableName + dataFileType, ios::in | ios::out | ios::binary);

    if (!file) {
        // File doesn't exist, create it
        ofstream newFile(dataPath + tableName + dataFileType, ios::binary);
        if (!newFile) {
            cerr << "Error creating file while writing a header!" << endl;
            return;
        }

        newFile.write(reinterpret_cast<const char *>(&header), sizeof(DBHeader));
        newFile.close();
        cout << "New table file created: " << tableName << endl;
        return;
    }

    // If file exists, modify only the header
    file.seekp(0);
    file.write(reinterpret_cast<const char *>(&header), sizeof(DBHeader));
    file.close();
}

DBHeader readHeader(const string &tableName) {
    ifstream file(dataPath + tableName + dataFileType, ios::binary);

    if (!file) {
        cerr << "Table file not found: " << tableName << endl;
        return DBHeader{}; // Return a default-initialized header
    }

    DBHeader header{};
    file.read(reinterpret_cast<char *>(&header), sizeof(DBHeader));

    if (!file) {
        cerr << "Error reading header!" << endl;
        return DBHeader{}; // Handle read failure
    }

    cout << "Table: " << tableName << endl;
    cout << "Magic: " << string(header.magic, 4) << endl;
    cout << "Number of Records: " << header.numRecords << endl;
    cout << "Free Offset: " << header.freeOffset << " bytes" << endl;

    file.close();
    return header;
}

// ==================== Schema Operations ====================

ColumnInfo parseSchemaLine(const string &line) {
    istringstream iss(line);
    string colName, colType;
    int colSize = 0, index = 0;

    // Extract column name
    while (index < line.size() && line[index] != ':') {
        colName += line[index++];
    }
    index++; // Skip ':'

    // Extract column type
    while (index < line.size() && line[index] != '(' && line[index] != ' ') {
        colType += line[index++];
    }

    // Assign size for int and float types
    if (colType == "int" || colType == "float") {
        colSize = 4;
    }
    // Extract column size for types like string(N)
    else if (index < line.size() && line[index] == '(') {
        index++; // Skip '('
        while (index < line.size() && line[index] != ')') {
            colSize = colSize * 10 + (line[index++] - '0');
        }
    }

    return {colName, colType, colSize};
}

vector<ColumnInfo> readSchema(const string &tableName) {
    ifstream file(dataPath + tableName + schemaFileType, ios::binary);
    if (!file) {
        cerr << "Schema file not found: " << tableName << endl;
        return {};
    }

    vector<ColumnInfo> columns;
    string line;

    while (getline(file, line)) {
        ColumnInfo column = parseSchemaLine(line);
        cout << "Column Parsed: " << column.name << " " << column.type
                << " " << (column.size ? to_string(column.size) : "") << endl;
        columns.push_back(column);
    }
    file.close();

    return columns;
}

int calculateRecordSize(const string &tableName) {
    vector<ColumnInfo> schemaInfo = readSchema(tableName);
    int recordSize = 0;

    for (const auto &column: schemaInfo) {
        recordSize += column.size;
    }

    return recordSize;
}

// ==================== Index Operations ====================

bool writeOffsetToFile(ostream &file, const int offset, const int position) {
    file.seekp(position, ios::beg);
    file.write(reinterpret_cast<const char *>(&offset), sizeof(int));

    if (!file) {
        cerr << "Error: Failed to write offset to index file" << endl;
        return false;
    }

    return true;
}

bool createAndWriteIndexFile(const string &indexPath, const string &tableName,
                             const int offset, const int writePosition) {
    ofstream newFile(indexPath, ios::binary);
    if (!newFile) {
        cerr << "Error: Failed to create index file for table: " << tableName << endl;
        return false;
    }

    bool success = writeOffsetToFile(newFile, offset, writePosition);
    newFile.close();

    if (success) {
        cout << "New index file created for table: " << tableName << endl;
    }

    return success;
}

bool updateIndex(const string &tableName, const int offset) {
    const string indexPath = dataPath + tableName + indexFileType;
    const DBHeader fileHeader = readHeader(tableName);

    // Calculate the position to write the new offset
    const int writePosition = fileHeader.numRecords * sizeof(int);

    // Try to open existing file first
    fstream file(indexPath, ios::in | ios::out | ios::binary);

    if (!file) {
        // File doesn't exist, create a new one
        return createAndWriteIndexFile(indexPath, tableName, offset, writePosition);
    }

    // Write to existing file
    bool success = writeOffsetToFile(file, offset, writePosition);
    file.close();
    return success;
}

int getIndex(const string &tableName, const int id) {
    const string indexPath = dataPath + tableName + indexFileType;
    ifstream file(indexPath, ios::binary);

    if (!file) {
        cerr << "Error: Failed to open index file for table: " << tableName << endl;
        return -1;
    }

    file.seekg(id * sizeof(int), ios::beg);
    int offset;
    file.read(reinterpret_cast<char *>(&offset), sizeof(int));

    if (!file) {
        cerr << "Error: Failed to read offset for ID " << id << endl;
        file.close();
        return -1;
    }

    file.close();
    return offset;
}

void displayIndexOffsets(const string &tableName) {
    const string indexPath = dataPath + tableName + indexFileType;
    ifstream file(indexPath, ios::binary);

    if (!file) {
        cerr << "Error: Failed to open index file for table: " << tableName << endl;
        return;
    }

    // Get the file size to determine how many offsets exist
    file.seekg(0, ios::end);
    const streampos fileSize = file.tellg();
    file.seekg(0, ios::beg);

    // Calculate the number of offsets (each offset is an integer)
    const int numOffsets = fileSize / sizeof(int);

    cout << "\nIndex file for table '" << tableName << "' contains " << numOffsets << " offsets:\n";
    cout << "-----------------------------------------\n";
    cout << "ID\t|\tOffset (bytes)\n";
    cout << "-----------------------------------------\n";

    // Read and display each offset
    for (int i = 0; i < numOffsets; i++) {
        int offset;
        file.read(reinterpret_cast<char *>(&offset), sizeof(int));

        if (!file) {
            cerr << "Error: Failed to read offset #" << i << endl;
            break;
        }

        cout << i << "\t|\t" << offset << endl;
    }

    cout << "-----------------------------------------\n";
    file.close();
}

// ==================== Table Operations ====================

void createTable(const string &tableName, const string &columns) {
    string schema, line;
    schema += "ID:int\n"; // Always add ID column first
    stringstream ss(columns);
    vector<string> result;
    while (ss.good()) {
        string substr;
        getline(ss, substr, ',');
        if (!substr.empty()) {
            schema += substr + "\n";
        }
    }
    string schemaPath = dataPath + tableName + schemaFileType;
    string dataFilePath = dataPath + tableName + dataFileType;

    cout << "Creating table: " << tableName << endl;
    cout << "Schema: " << schema << endl;

    // Write schema file
    ofstream schemaFile(schemaPath, ios::binary);
    if (!schemaFile) {
        cerr << "Error creating schema file: " << schemaPath << endl;
        return;
    }
    schemaFile.write(schema.c_str(), schema.size());
    schemaFile.close();

    // Create a default header and write it
    DBHeader newHeader{};
    strncpy(newHeader.magic, "DB01", 4);
    newHeader.numRecords = 0;
    newHeader.freeOffset = sizeof(DBHeader); // Data starts after the header

    writeHeader(tableName, newHeader);
}

// ==================== Record Operations ====================

void writeRecord(const string &tableName, vector<string> values) {
    string filePath = dataPath + tableName + dataFileType;
    fstream file(filePath, ios::in | ios::out | ios::binary);

    if (!file.is_open()) {
        throw runtime_error("Error opening file: " + filePath);
    }

    vector<ColumnInfo> schemaInfo = readSchema(tableName);
    DBHeader fileHeader = readHeader(tableName);

    // Seek to the free offset location for writing
    file.seekp(fileHeader.freeOffset, ios::beg);

    int recordSize = 0;
    int columnIndex = 0;
    for (auto &column: schemaInfo) {
        if (column.name == "ID") {
            int newID = fileHeader.numRecords; // Use the incremented value
            file.write(reinterpret_cast<const char *>(&newID), sizeof(int));
            recordSize += sizeof(int);
            cout << "Assigned ID: " << newID << endl;
            continue;
        }

        if (column.type == "int") {
            int value = stoi(values[columnIndex]);
            file.write(reinterpret_cast<const char *>(&value), sizeof(int));
            recordSize += sizeof(int);
        } else if (column.type == "float") {
            float value = stof(values[columnIndex]);
            file.write(reinterpret_cast<const char *>(&value), sizeof(float));
            recordSize += sizeof(float);
        } else if (column.type == "string") {
            string value = values[columnIndex];
            if (!value.empty() && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2); // Remove first and last character
            }
            if (value.size() > column.size) {
                throw runtime_error("Input exceeds maximum size for column: " + column.name);
            }
            value.resize(column.size, '\0'); // Ensure proper size
            file.write(value.c_str(), column.size);
            recordSize += column.size;
        }
        columnIndex++;
    }

    // Update header with new free offset and record count
    fileHeader.numRecords++;
    fileHeader.freeOffset += recordSize;

    // Update index file
    if (!updateIndex(tableName, fileHeader.freeOffset - recordSize)) {
        throw runtime_error("Failed to update index for table: " + tableName);
    }

    // Write updated header
    writeHeader(tableName, fileHeader);
    file.close();
    cout << "Record written successfully." << endl;
}

void readRecords(const string &tableName) {
    string filePath = dataPath + tableName + dataFileType;
    ifstream file(filePath, ios::binary);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return;
    }

    // Read file header
    DBHeader fileHeader = readHeader(tableName);

    // Read schema information
    vector<ColumnInfo> schemaInfo = readSchema(tableName);

    // Seek to the start of the records (after the header)
    file.seekg(headerSize, ios::beg);

    cout << "\nReading Records from " << tableName << "...\n";

    for (int recordIndex = 0; recordIndex < fileHeader.numRecords; ++recordIndex) {
        cout << "Record " << (recordIndex + 1) << ":\n";

        for (const auto &column: schemaInfo) {
            cout << column.name << ": ";

            if (column.type == "int") {
                int value;
                file.read(reinterpret_cast<char *>(&value), sizeof(int));
                cout << value;
            } else if (column.type == "float") {
                float value;
                file.read(reinterpret_cast<char *>(&value), sizeof(float));
                cout << value;
            } else if (column.type == "string") {
                vector<char> buffer(column.size);
                file.read(buffer.data(), column.size);
                string value(buffer.data(), strnlen(buffer.data(), column.size));
                cout << value;
            }

            cout << endl;
        }
        cout << "---------------------\n"; // Separator between records
    }

    file.close();
}

void readRecordWithIndex(const string &tableName, int id) {
    string filePath = dataPath + tableName + dataFileType;
    ifstream file(filePath, ios::binary);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return;
    }

    // Get record offset from index
    int offset = getIndex(tableName, id);
    if (offset == -1) {
        cerr << "Error: Could not find record with ID " << id << endl;
        file.close();
        return;
    }

    // Read schema information
    vector<ColumnInfo> schemaInfo = readSchema(tableName);

    // Seek to the record position
    file.seekg(offset, ios::beg);

    cout << "\nReading Record with ID: " << id << " from " << tableName << "...\n";

    for (const auto &column: schemaInfo) {
        cout << column.name << ": ";

        if (column.type == "int") {
            int value;
            file.read(reinterpret_cast<char *>(&value), sizeof(int));
            cout << value;
        } else if (column.type == "float") {
            float value;
            file.read(reinterpret_cast<char *>(&value), sizeof(float));
            cout << value;
        } else if (column.type == "string") {
            vector<char> buffer(column.size);
            file.read(buffer.data(), column.size);
            string value(buffer.data(), strnlen(buffer.data(), column.size));
            cout << value;
        }

        cout << endl;
    }

    cout << "---------------------\n";
    file.close();
}

bool deleteRecord(const string &tableName, int id) {
    const string dataFilePath = dataPath + tableName + dataFileType;
    const string indexFilePath = dataPath + tableName + indexFileType;

    // Display current index status
    displayIndexOffsets(tableName);

    fstream dataFile(dataFilePath, ios::in | ios::out | ios::binary);
    if (!dataFile.is_open()) {
        cerr << "Error opening file: " << dataFilePath << endl;
        return false;
    }

    fstream indexFile(indexFilePath, ios::in | ios::out | ios::binary);
    if (!indexFile.is_open()) {
        cerr << "Error opening index file: " << indexFilePath << endl;
        dataFile.close();
        return false;
    }

    cout << "Deleting record with ID: " << id << endl;

    // Read the header to get the number of records
    DBHeader fileHeader = readHeader(tableName);

    // Validate the ID
    if (id < 0 || id >= fileHeader.numRecords) {
        cerr << "Error: Invalid record ID " << id << endl;
        dataFile.close();
        indexFile.close();
        return false;
    }

    // Get the schema information and calculate record size
    vector<ColumnInfo> schemaInfo = readSchema(tableName);
    if (schemaInfo.empty()) {
        cerr << "Error: Failed to read schema for table: " << tableName << endl;
        dataFile.close();
        indexFile.close();
        return false;
    }

    // Calculate record size
    int recordSize = 0;
    for (const auto &column: schemaInfo) {
        if (column.type == "int" || column.type == "float") {
            recordSize += 4; // 4 bytes for int/float
        } else if (column.type == "string") {
            recordSize += column.size;
        }
    }

    // Get offset of the record to delete
    int deleteOffset = getIndex(tableName, id);
    if (deleteOffset == -1) {
        cerr << "Error: Could not find index for record with ID " << id << endl;
        dataFile.close();
        indexFile.close();
        return false;
    }

    cout << "Deleting record ID " << id << " at offset: " << deleteOffset << endl;

    // Get the last record's offset
    int lastRecordID = fileHeader.numRecords - 1;
    int lastRecordOffset = getIndex(tableName, lastRecordID);

    // Show the last record that will be moved
    readRecordWithIndex(tableName, lastRecordID);

    if (lastRecordOffset == -1) {
        cerr << "Error: Could not find the last record's offset" << endl;
        dataFile.close();
        indexFile.close();
        return false;
    }

    // If we're not deleting the last record, move the last record to the deleted position
    if (id != lastRecordID) {
        // Read the last record's data
        vector<char> lastRecordData(recordSize);
        dataFile.seekg(lastRecordOffset, ios::beg);
        dataFile.read(lastRecordData.data(), recordSize);

        if (!dataFile) {
            cerr << "Error: Failed to read the last record's data" << endl;
            dataFile.close();
            indexFile.close();
            return false;
        }

        // Write the last record's data to the deleted record's position
        dataFile.seekp(deleteOffset, ios::beg);
        dataFile.write(lastRecordData.data(), recordSize);

        // Update the index file by shifting entries
        indexFile.seekg((id + 1) * sizeof(int), ios::beg);
        vector<char> indexData((lastRecordID - id) * sizeof(int));
        indexFile.read(indexData.data(), (lastRecordID - id) * sizeof(int));

        indexFile.seekp(id * sizeof(int), ios::beg);
        indexFile.write(indexData.data(), (lastRecordID - id) * sizeof(int));
    }

    // Update the record count in the header
    fileHeader.numRecords--;
    writeHeader(tableName, fileHeader);

    dataFile.close();
    indexFile.close();
    cout << "Record deleted successfully." << endl;
    return true;
}


// Implementation for Storage.cpp

vector<vector<variant<int, float, string> > > getRecordsWithCondition(
    const string &tableName,
    const vector<string> &columnsToReturn,
    const vector<Condition> &conditions
) {
    // Result container - vector of rows, where each row is a vector of column values
    vector<vector<variant<int, float, string> > > results;

    string filePath = dataPath + tableName + dataFileType;
    ifstream dataFile(filePath, ios::binary);

    if (!dataFile.is_open()) {
        cerr << "Error opening data file: " << filePath << endl;
        return results;
    }

    // Read the header
    DBHeader header = readHeader(tableName);

    // Read the schema
    vector<ColumnInfo> schema = readSchema(tableName);

    // Calculate column offsets for faster access
    vector<int> columnOffsets(schema.size(), 0);
    int currentOffset = 0;
    for (size_t i = 0; i < schema.size(); i++) {
        columnOffsets[i] = currentOffset;
        currentOffset += schema[i].size;
    }

    // Determine which columns to return
    vector<int> columnIndices;
    bool returnAllColumns = (columnsToReturn.size() == 1 && columnsToReturn[0] == "*");

    if (!returnAllColumns) {
        for (const auto &columnName: columnsToReturn) {
            bool found = false;
            for (size_t i = 0; i < schema.size(); i++) {
                if (schema[i].name == columnName) {
                    columnIndices.push_back(i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                cerr << "Warning: Column '" << columnName << "' not found in schema" << endl;
            }
        }
    } else {
        // Return all columns
        for (size_t i = 0; i < schema.size(); i++) {
            columnIndices.push_back(i);
        }
    }

    // Calculate record size
    int recordSize = 0;
    for (const auto &col: schema) {
        recordSize += col.size;
    }

    // Buffer for reading a record
    vector<char> recordBuffer(recordSize);

    // Process each record
    for (int recordId = 0; recordId < header.numRecords; recordId++) {
        // Get the record offset from the index
        int offset = getIndex(tableName, recordId);

        if (offset == -1) {
            cerr << "Warning: Could not find offset for record ID " << recordId << endl;
            continue;
        }

        // Read the record
        dataFile.seekg(offset);
        dataFile.read(recordBuffer.data(), recordSize);

        if (!dataFile) {
            cerr << "Error reading record ID " << recordId << endl;
            continue;
        }

        // Check if record satisfies all conditions
        bool recordMatches = true;

        for (const auto &condition: conditions) {
            int colIndex = -1;

            // Find the column index for the condition
            for (size_t i = 0; i < schema.size(); i++) {
                if (schema[i].name == condition.columnName) {
                    colIndex = i;
                    break;
                }
            }

            if (colIndex == -1) {
                cerr << "Warning: Condition column '" << condition.columnName << "' not found" << endl;
                recordMatches = false;
                break;
            }

            // Get offset to the column in the record
            int colOffset = columnOffsets[colIndex];

            // Compare based on column type
            if (schema[colIndex].type == "int") {
                int recordValue = *reinterpret_cast<int *>(recordBuffer.data() + colOffset);
                int conditionValue = get<int>(condition.value);

                if (condition.operatorType == "=") {
                    if (!(recordValue == conditionValue)) recordMatches = false;
                } else if (condition.operatorType == "!=") {
                    if (!(recordValue != conditionValue)) recordMatches = false;
                } else if (condition.operatorType == "<") {
                    if (!(recordValue < conditionValue)) recordMatches = false;
                } else if (condition.operatorType == "<=") {
                    if (!(recordValue <= conditionValue)) recordMatches = false;
                } else if (condition.operatorType == ">") {
                    if (!(recordValue > conditionValue)) recordMatches = false;
                } else if (condition.operatorType == ">=") {
                    if (!(recordValue >= conditionValue)) recordMatches = false;
                }
            } else if (schema[colIndex].type == "float") {
                float recordValue = *reinterpret_cast<float *>(recordBuffer.data() + colOffset);
                float conditionValue = get<float>(condition.value);

                if (condition.operatorType == "=") {
                    if (!(recordValue == conditionValue)) recordMatches = false;
                } else if (condition.operatorType == "!=") {
                    if (!(recordValue != conditionValue)) recordMatches = false;
                } else if (condition.operatorType == "<") {
                    if (!(recordValue < conditionValue)) recordMatches = false;
                } else if (condition.operatorType == "<=") {
                    if (!(recordValue <= conditionValue)) recordMatches = false;
                } else if (condition.operatorType == ">") {
                    if (!(recordValue > conditionValue)) recordMatches = false;
                } else if (condition.operatorType == ">=") {
                    if (!(recordValue >= conditionValue)) recordMatches = false;
                }
            } else if (schema[colIndex].type == "string") {
                string recordValue(recordBuffer.data() + colOffset,
                                   strnlen(recordBuffer.data() + colOffset, schema[colIndex].size));
                string conditionValue = get<string>(condition.value);

                int cmpResult = recordValue.compare(conditionValue);

                if (condition.operatorType == "=") {
                    if (!(cmpResult == 0)) recordMatches = false;
                } else if (condition.operatorType == "!=") {
                    if (!(cmpResult != 0)) recordMatches = false;
                } else if (condition.operatorType == "<") {
                    if (!(cmpResult < 0)) recordMatches = false;
                } else if (condition.operatorType == "<=") {
                    if (!(cmpResult <= 0)) recordMatches = false;
                } else if (condition.operatorType == ">") {
                    if (!(cmpResult > 0)) recordMatches = false;
                } else if (condition.operatorType == ">=") {
                    if (!(cmpResult >= 0)) recordMatches = false;
                }
            }

            if (!recordMatches) break;
        }

        // If record matches all conditions, extract requested columns
        if (recordMatches) {
            vector<variant<int, float, string> > row;

            for (int colIdx: columnIndices) {
                int colOffset = columnOffsets[colIdx];

                if (schema[colIdx].type == "int") {
                    int value = *reinterpret_cast<int *>(recordBuffer.data() + colOffset);
                    row.push_back(value);
                } else if (schema[colIdx].type == "float") {
                    float value = *reinterpret_cast<float *>(recordBuffer.data() + colOffset);
                    row.push_back(value);
                } else if (schema[colIdx].type == "string") {
                    string value(recordBuffer.data() + colOffset,
                                 strnlen(recordBuffer.data() + colOffset, schema[colIdx].size));
                    row.push_back(value);
                }
            }

            results.push_back(row);
        }
    }

    dataFile.close();
    return results;
}

// Helper function to display the results
void displayResults(const string &tableName,
                    const vector<string> &columns,
                    const vector<vector<variant<int, float, string> > > &results) {
    // Get schema to determine column types
    vector<ColumnInfo> schema = readSchema(tableName);

    // Determine column indices and types
    vector<pair<int, string> > columnInfo;
    bool displayAllColumns = (columns.size() == 1 && columns[0] == "*");

    if (displayAllColumns) {
        for (size_t i = 0; i < schema.size(); i++) {
            columnInfo.push_back({i, schema[i].type});
        }
    } else {
        for (const auto &colName: columns) {
            for (size_t i = 0; i < schema.size(); i++) {
                if (schema[i].name == colName) {
                    columnInfo.push_back({i, schema[i].type});
                    break;
                }
            }
        }
    }

    // Display column headers
    cout << "\n-----------------------------------------\n";
    if (displayAllColumns) {
        for (const auto &col: schema) {
            cout << col.name << "\t";
        }
    } else {
        for (const auto &colName: columns) {
            cout << colName << "\t";
        }
    }
    cout << "\n-----------------------------------------\n";

    // Display results
    for (const auto &row: results) {
        for (size_t i = 0; i < row.size(); i++) {
            // Get the value based on its type
            if (holds_alternative<int>(row[i])) {
                cout << get<int>(row[i]);
            } else if (holds_alternative<float>(row[i])) {
                cout << get<float>(row[i]);
            } else if (holds_alternative<string>(row[i])) {
                cout << get<string>(row[i]);
            }

            cout << "\t";
        }
        cout << endl;
    }
    cout << "-----------------------------------------\n";
    cout << results.size() << " records found" << endl;
}

void displayQueryResults(const string &tableName,
                         const vector<string> &columns,
                         const vector<Condition> &conditions) {
    // Get the records that match the conditions
    vector<vector<variant<int, float, string> > > results =
            getRecordsWithCondition(tableName, columns, conditions);

    // Display query information
    cout << "\nQuery on table: " << tableName << endl;
    cout << "Columns: ";
    if (columns.size() == 1 && columns[0] == "*") {
        cout << "* (all columns)";
    } else {
        for (size_t i = 0; i < columns.size(); i++) {
            cout << columns[i];
            if (i < columns.size() - 1) cout << ", ";
        }
    }
    cout << endl;

    // Display conditions if any
    if (!conditions.empty()) {
        cout << "Conditions: ";
        for (size_t i = 0; i < conditions.size(); i++) {
            cout << conditions[i].columnName << " "
                    << conditions[i].operatorType << " ";

            // Display the condition value based on its type
            if (holds_alternative<int>(conditions[i].value)) {
                cout << get<int>(conditions[i].value);
            } else if (holds_alternative<float>(conditions[i].value)) {
                cout << get<float>(conditions[i].value);
            } else if (holds_alternative<string>(conditions[i].value)) {
                cout << "\"" << get<string>(conditions[i].value) << "\"";
            }

            if (i < conditions.size() - 1) cout << " AND ";
        }
        cout << endl;
    }

    // Use the existing displayResults function to show the data
    displayResults(tableName, columns, results);
}
