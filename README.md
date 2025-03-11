# **SimDB - A Lightweight C++ Database System** 🚀  

SimDB is a **lightweight file-based database system** written in **C++**, supporting **basic SQL operations** like `CREATE TABLE`, `INSERT`, `SELECT`, and `DELETE`. It utilizes **binary file storage** and **indexing with B-Trees (future feature)** for efficient data retrieval.

---

## **📌 Features**  

✔️ **Basic SQL Operations** (`CREATE`, `INSERT`, `SELECT`, `DELETE`)  
✔️ **File-Based Storage** (Data stored in binary files)  
✔️ **Indexing with Hash Index** (Current Indexing)  
✔️ **Indexing with B-Trees** (Upcoming feature)  
✔️ **SQL Query Parser**  
✔️ **Docker Support** for easy deployment  

---

## **🛠️ Project Structure**

```
    SimpleDB/
    │── src/
    │   ├── main.cpp          # Entry point
    │   ├── storage.cpp       # Handles file-based storage
    │   ├── btree.cpp         # B-Tree indexing implementation
    │   ├── parser.cpp        # SQL query parsing
    │   ├── executor.cpp      # Executes parsed queries
    │── include/
    │   ├── storage.h
    │   ├── btree.h
    │   ├── parser.h
    │   ├── executor.h
    │── data/                 # Stores database files
    │── tests/                # Unit tests
    │── CMakeLists.txt        # Build configuration
    │── README.md             # Project documentation

```


## **🔧 Installation & Setup**  

### **📌 Prerequisites**  
- **C++ (GCC/Clang/MSVC)**  
- **CMake**  
- **Docker (Optional for containerized execution)**  

### **🔹 Build the Project**  
Run the following commands to build and compile:  
```sh
mkdir build && cd build
cmake ..
make
./SimDB
``` 

### 🚀 Running with Docker
🔹 Build the Docker Image

```sh
docker build -t simdb .
```
Run the Container

```sh
docker run -it simdb
```
🔹 Inside  the Container shell
```sh

cd build

./SimDB
```



## **🛠️ How It Works**  

### **1️⃣ Query Parsing (`parser.cpp`)**  
- Parses SQL queries into structured commands.  
- **Example**:  
  ```sql
  INSERT INTO employees VALUES (John, 25, 5000)
  ```
  **→** `{ Table: employees, Values: [John, 25, 5000] }`  

### **2️⃣ Query Execution (`executor.cpp`)**  
- Calls **Storage functions** based on parsed queries.  
- **Example**:  
  ```cpp
  parseInsert(query) → executeInsert(tableName, values)
  ```  

### **3️⃣ Storage Engine (`storage.cpp`)**  
- Stores **records in binary files**.  
- Uses **indexed offsets** for fast retrieval.  

### **4️⃣ Indexing (`btree.cpp` - Future Work)**  
- Uses **indexed offsets** for fast retrieval as ***HashMap Index***.  
- Will improve **search performance** using a **B-Tree index**.  

---

## **💡 Challenges Faced & Solutions**  

### **1️⃣ Handling Dynamic SQL Parsing**  
- **Problem**: Parsing SQL queries dynamically while supporting different formats.  
- **Solution**: Implemented a **string tokenizer & condition parser** to extract and process queries efficiently.  

### **2️⃣ Efficient Storage & Retrieval**  
- **Problem**: Storing data in a **binary file** while allowing **fast lookups**.  
- **Solution**: Used  an **index file** to store offsets for quick access.  

### **3️⃣ Data Consistency in Deletions**  
- **Problem**: Deleting records without breaking **indexing order**.  
- **Solution**: Implemented a **record swapping mechanism** to maintain **sequential integrity**.  
