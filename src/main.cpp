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
    writeRecord("em") ;
    readRecords("em");
    displayQueryResults("em",{"Name","Age"},{Condition("ID",">",1)});

    return 0 ;

}