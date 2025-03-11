#include <bits/stdc++.h>
#include "Parser.h"
#include "Storage.h"
using namespace std ;


int main()
{

    while (true) {
        string query;
        cout << "Enter SQL Query: ";
        getline(cin, query); // Read full line input

        executeQuery(query);
    }

    return 0 ;

}