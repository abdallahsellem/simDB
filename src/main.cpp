#include <bits/stdc++.h>
#include "Storage.h"
using namespace std ;


int main()
{
    // writeHeader("sel.bin",readHeader("sel.bin"));
    createTable();
    writeRecord("em");
    writeRecord("em") ;
    writeRecord("em");
    writeRecord("em") ;
    readRecords("em");
    deleteRecord("em",2);
    readRecords("em");



    // readRecordWithIndex("em",1);
    //
    // updateRecord("em",1,{"seleeeem","123"});
    // readRecordWithIndex("em",1);

    return 0 ;

}