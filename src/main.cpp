#include <bits/stdc++.h>
#include "Storage.h"
using namespace std ;


int main()
{
    // writeHeader("sel.bin",readHeader("sel.bin"));
    createTable();
    writeRecord("em");
    readRecordWithIndex("em",0) ;
    writeRecord("em") ;
    writeRecord("em");
    writeRecord("em") ;
    writeRecord("em") ;
    readRecords("em");
    readRecords("em");
    deleteRecord("em",2);
    readRecords("em");
    readRecordWithIndex("em",2);
    deleteRecord("em",2);
    readRecords("em");

    return 0 ;

}