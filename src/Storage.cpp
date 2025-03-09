//
// Created by abdallah-selim on 3/7/25.
//

#include "Storage.h"

#include <string>
using namespace std ;

// Function to write a header (creates a new file if necessary)
void writeHeader(const string &tableName, const DBHeader &header) {
    std::fstream file(dataPath + tableName + dataFileType, ios::in | ios::out | ios::binary);

    if (!file) {
        // File doesn't exist, so create it
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

// Function to read a header (if file exists)
struct DBHeader readHeader(const string &tableName) {
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

// Function to parse a single schema line (e.g., "Name:string(20)")
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

// Function to read schema from a file and parse column information
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

void writeRecord(const string &tableName) {
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

    for (auto &column: schemaInfo) {
        if (column.name == "ID") {
            int newID = fileHeader.numRecords; // Use the incremented value
            file.write(reinterpret_cast<const char *>(&newID), sizeof(int));
            recordSize += sizeof(int);
            cout << "Assigned ID: " << newID << endl;
            continue;
        }

        cout << "Please enter value for " << column.name << ": ";
        if (column.type == "int") {
            int value;
            cin >> value;
            file.write(reinterpret_cast<const char *>(&value), sizeof(int));
            recordSize += sizeof(int);
        } else if (column.type == "float") {
            float value;
            cin >> value;
            file.write(reinterpret_cast<const char *>(&value), sizeof(float));
            recordSize += sizeof(float);
        } else if (column.type == "string") {
            string value;
            cin >> value;
            if (value.size() > column.size) {
                throw runtime_error("Input exceeds maximum size for column: " + column.name);
            }
            value.resize(column.size, '\0'); // Ensure proper size
            file.write(value.c_str(), column.size);
            recordSize += column.size;
        }
    }

    // Update header with new free offset and record count
    fileHeader.numRecords++;
    fileHeader.freeOffset += recordSize;

    // Write updated header immediately after writing the record
    writeHeader(tableName, fileHeader);

    // Update index file
    if (!updateIndex(tableName, fileHeader.freeOffset - recordSize)) {
        throw runtime_error("Failed to update index for table: " + tableName);
    }

    file.close();
    cout << "Record written successfully." << endl;
}
void readRecords(const string &tableName) {
    string filePath = dataPath + tableName + dataFileType;
    fstream file(filePath, ios::in | ios::binary);

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
                std::vector<char> buffer(column.size);
                file.read(buffer.data(), column.size); // Read bytes

                // Convert only the valid part of the buffer into a string (excluding nulls)
                std::string value(buffer.data(), strnlen(buffer.data(), column.size));

                std::cout << value;
            }

            cout << endl;
        }
        cout << "---------------------\n"; // Separator between records
    }

    file.close();
}

void createTable() {
    string tableName;
    cout << "Please enter table name: ";
    cin >> tableName;
    cin.ignore(); // Clear the newline character left in the buffer

    cout << "Please enter schema (press Enter on a blank line to finish):" << endl;
    string schema, line;
    schema += "ID:int\n";
    while (true) {
        getline(cin, line);
        if (line.empty()) {
            // Stop when the user presses Enter on a blank line
            break;
        }
        schema += line + "\n";
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

bool writeOffsetToFile(ostream &file, const int offset, const streampos position) {
    file.seekp(position, ios::beg);
    file.write(reinterpret_cast<const char *>(&offset), sizeof(int));

    if (!file) {
        cerr << "Error: Failed to write offset to index file" << endl;
        return false;
    }

    return true;
}

bool createAndWriteIndexFile(const string &indexPath, const string &tableName,
                             const int offset, const streampos writePosition) {
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
    const streampos writePosition = fileHeader.numRecords * sizeof(int);

    // Try to open existing file first
    fstream file(indexPath, ios::in | ios::out | ios::binary);

    if (!file) {
        // File doesn't exist, create a new one
        return createAndWriteIndexFile(indexPath, tableName, offset, writePosition);
    }

    // Write to existing file
    return writeOffsetToFile(file, offset, writePosition);
}

int getIndex(const string &tableName, const int id) {
    const string indexPath = dataPath + tableName + indexFileType;
    fstream file(indexPath, ios::in | ios::binary);
    if (!file) {
        cerr << "Error: Failed to open index file for table: " << tableName << endl;
        return -1;
    }
    file.seekg(id * sizeof(int), ios::beg);
    int offset;
    file.read(reinterpret_cast<char *>(&offset), sizeof(int));
    file.close();
    return offset;
}

void readRecordWithIndex(const string &tableName, int id) {
    string filePath = dataPath + tableName + dataFileType;
    fstream file(filePath, ios::in | ios::binary);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return;
    }

    // Read file header
    DBHeader fileHeader = readHeader(tableName);

    // Read schema information
    vector<ColumnInfo> schemaInfo = readSchema(tableName);
    int offset = getIndex(tableName, id);
    // Seek to the start of the records (after the header)
    file.seekg(offset, ios::beg);

    cout << "\nReading Record with id : "<<id<<" from " << tableName << "...\n";

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
            std::vector<char> buffer(column.size);
            file.read(buffer.data(), column.size); // Read bytes
            // Convert only the valid part of the buffer into a string (excluding nulls)
            std::string value(buffer.data(), strnlen(buffer.data(), column.size));

            std::cout << value;
        }
        cout << endl;
    }


    file.close();
}

// void updateRecord(const string &tableName,) {}
