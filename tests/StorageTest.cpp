#include <gtest/gtest.h>
#include "../src/Storage.h"
#include <fstream>
#include <vector>
using namespace std;

const string testDataPath = "./test_data/"; // Ensure this directory exists
const string testFile = "test_table";

// Helper function to remove test files after each run
void cleanupTestFiles() {
    remove((testDataPath + testFile + dataFileType).c_str());
    remove((testDataPath + testFile + schemaFileType).c_str());
}

class StorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        cleanupTestFiles();
    }
    void TearDown() override {
        cleanupTestFiles();
    }
};

TEST_F(StorageTest, WriteAndReadHeader) {
    DBHeader header ;
    memcpy(header.magic, "DBH", 4);  // Set the magic identifier
    header.numRecords = 0;
    header.freeOffset = 128;
    writeHeader(testFile, header);
    DBHeader readHeaderData = readHeader(testFile);
    EXPECT_TRUE(memcmp(readHeaderData.magic, "DBH", 3) == 0);
    EXPECT_EQ(readHeaderData.numRecords, 0);
    EXPECT_EQ(readHeaderData.freeOffset, 128);
}

TEST_F(StorageTest, ParseSchemaLine) {
    ColumnInfo col1 = parseSchemaLine("Name:string(20)");
    EXPECT_EQ(col1.name, "Name");
    EXPECT_EQ(col1.type, "string");
    EXPECT_EQ(col1.size, 20);

    ColumnInfo col2 = parseSchemaLine("Age:int");
    EXPECT_EQ(col2.name, "Age");
    EXPECT_EQ(col2.type, "int");
    EXPECT_EQ(col2.size, 4);
}

TEST_F(StorageTest, WriteAndReadSchema) {
    ofstream schemaFile(testDataPath + testFile + schemaFileType);
    schemaFile << "Name:string(20)\nAge:int\n";
    schemaFile.close();

    vector<ColumnInfo> schema = readSchema(testFile);
    ASSERT_EQ(schema.size(), 2);
    EXPECT_EQ(schema[0].name, "Name");
    EXPECT_EQ(schema[0].type, "string");
    EXPECT_EQ(schema[0].size, 20);
    EXPECT_EQ(schema[1].name, "Age");
    EXPECT_EQ(schema[1].type, "int");
    EXPECT_EQ(schema[1].size, 4);
}

TEST_F(StorageTest, WriteAndReadRecord) {
    DBHeader header ;
    memcpy(header.magic, "DBH", 4);  // Set the magic identifier
    header.numRecords = 0;
    header.freeOffset = 128;
    writeHeader(testFile, header);

    ofstream schemaFile(testDataPath + testFile + schemaFileType);
    schemaFile << "Name:string(10)\nAge:int\n";
    schemaFile.close();

    ofstream dataFile(testDataPath + testFile + dataFileType, ios::binary);
    string name = "Alice";
    name.resize(10, '\0');
    int age = 25;
    dataFile.write(name.c_str(), 10);
    dataFile.write(reinterpret_cast<char*>(&age), sizeof(int));
    dataFile.close();

    readRecord(testFile); // Should output "Alice" and 25
}
