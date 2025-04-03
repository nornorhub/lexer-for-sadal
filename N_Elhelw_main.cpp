#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include "lex.h"

using std::cout;
using std::endl;
using std::ifstream;

bool lexemeInVector(vector<LexItem> v, LexItem li);
vector<string> lexemesVector(vector<LexItem> v);
bool shouldConvertToFloat(string s);
vector<double> doubleVector(vector<string> v);
bool hasEforInt(string s);

int main(int argc, char* argv[]) {

    int filec = 0;
    vector<string> files;

    bool all = false;
    bool num = false;
    bool str = false;
    bool idb = false;
    bool kw = false;

    int lines = 1;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-') {
            filec++;
            files.push_back(argv[i]);
        }
    }

    if (filec > 1) {
        cout << "Only one file name is allowed." << endl;
        exit(0);
    } else if (filec == 0) {
        cout << "No specified input file." << endl;
        exit(0);
    }

    ifstream fin("PA1 Test Cases/" + files[0]);

    if (!fin.is_open()) {
        cout << "CANNOT OPEN THE FILE " + string(files[0]) << endl;
        exit(0);
    }


    if (fin.peek() == std::ifstream::traits_type::eof()) {
        cout << "Empty file." << endl;
        exit(0);
    }

    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-' &&
            string(argv[i]) != "-all" &&
            string(argv[i]) != "-num" &&
            string(argv[i]) != "-str" &&
            string(argv[i]) != "-id" &&
            string(argv[i]) != "-kw") {
            cout << "UNRECOGNIZED FLAG " << "{" << argv[i] << "}" << endl;
            exit(0);
        }
    }

    for (int i = 0; i < argc; i++) {
        if (string(argv[i]) == "-all") {
            all = true;
        } else if (string(argv[i]) == "-num") {
            num = true;
        } else if (string(argv[i]) == "-str") {
            str = true;
        } else if (string(argv[i]) == "-id") {
            idb = true;
        } else if (string(argv[i]) == "-kw") {
            kw = true;
        }
    }

    LexItem lexit = getNextToken(fin, lines);
    vector<LexItem> lexits;

    while (lexit.GetToken() != Token::DONE && lexit.GetToken() != Token::ERR) {

        lexits.push_back(lexit);

        lexit = getNextToken(fin, lines);
    }

    if (lexit.GetToken() == Token::ERR) {
        lexits.push_back(lexit);
    }
    
    bool flag = true;
    int texts = 0;
    int kws = 0;

    if (all) {
        for (auto i : lexits) {
            cout << i << endl;
            if (i.GetToken() == Token::ERR) {
                flag = false;
            }
        }
    }


    vector<LexItem> numeralsVector;
    vector<LexItem> idVector;
    vector<LexItem> kwVector;
    vector<LexItem> charsAndStrings;
    for (auto i : lexits)
    {
        if (i.GetToken() == ICONST || i.GetToken() == FCONST) {
            if (!lexemeInVector(numeralsVector, i)) {
                numeralsVector.push_back(i);
            }
        }
        if (i.GetToken() == SCONST || i.GetToken() == CCONST) {
            if (!lexemeInVector(charsAndStrings, i)) {
                charsAndStrings.push_back(i);
            }
        }
        if (i.GetToken() == IDENT) {
            if (!lexemeInVector(idVector, i)) {
                idVector.push_back(i);
            }
        }
        if (i.GetToken() == IF || i.GetToken() == ELSE || i.GetToken() == ELSIF || i.GetToken() == PUT || i.GetToken() == PUTLN || i.GetToken() == GET || 
        i.GetToken() == INT || i.GetToken() == FLOAT || i.GetToken() == CHAR || i.GetToken() == STRING || i.GetToken() == BOOL || i.GetToken() == PROCEDURE || 
        i.GetToken() == END || i.GetToken() == IS || i.GetToken() == BEGIN || i.GetToken() == THEN || i.GetToken() == CONST || i.GetToken() == AND || i.GetToken() == OR || i.GetToken() == NOT || i.GetToken() == MOD) {
            if (!lexemeInVector(kwVector, i)) {
                kwVector.push_back(i);
            }
        }
    }
    

    if (flag) {
        cout << endl;
        cout << "Lines: " << lines-1 << endl;
        cout << "Total Tokens: " << lexits.size() << endl;
        cout << "Numerals: " << numeralsVector.size() << endl;
        cout << "Characters and Strings: " << charsAndStrings.size() << endl;
        cout << "Identifiers: " << idVector.size() << endl;
        cout << "Keywords: " << kwVector.size() << endl;
    }

    vector<string> numeralsLexemes = lexemesVector(numeralsVector);
    vector<double> dv = doubleVector(numeralsLexemes);
    if (num && flag) {
        cout << "NUMERIC CONSTANTS:" << endl;
        for (int i = 0; i < dv.size(); i++)
        {
            if (i != dv.size()-1)
                cout << dv[i] << string(", ");
            else
                cout << dv[i];
        }
        
        cout << endl;
    }

    vector<string> charsAndStringsLexemes = lexemesVector(charsAndStrings);
    if (str && flag) {
        std::sort(charsAndStringsLexemes.begin(), charsAndStringsLexemes.end());
        cout << "CHARACTERS AND STRINGS:" << endl;
        for (int i = 0; i < charsAndStringsLexemes.size(); i++)
        {
            if (i != charsAndStringsLexemes.size()-1)
                cout << charsAndStringsLexemes[i] + string(", ");
            else
                cout << charsAndStringsLexemes[i];
        }
        
        cout << endl;
    }

    vector<string> idLexemes = lexemesVector(idVector);
    if (idb && flag) {
        std::sort(idLexemes.begin(), idLexemes.end());
        cout << "IDENTIFIERS:" << endl;
        for (int i = 0; i < idLexemes.size(); i++)
        {
            if (i != idLexemes.size()-1)
                cout << idLexemes[i] + string(", ");
            else
                cout << idLexemes[i];
        }
        
        cout << endl;
    }

    if (kw && flag) {
        std::sort(kwVector.begin(), kwVector.end(), [](const LexItem& a, const LexItem& b) {
            return a.GetToken() < b.GetToken();
        });
        cout << "KEYWORDS:" << endl;
        for (int i = 0; i < kwVector.size(); i++)
        {
            if (i != kwVector.size()-1)
                cout << kwVector[i].GetLexeme() + string(", ");
            else
                cout << kwVector[i].GetLexeme();
        }
        
        cout << endl;
    }
      

    return 0;
}

bool lexemeInVector(vector<LexItem> v, LexItem li) {
    for (auto i : v)
    {
        if (i.GetLexeme() == li.GetLexeme()) {
            return true;
        }
    }

    return false;
}

vector<string> lexemesVector(vector<LexItem> v) {
    vector<string> lexemesVector;
    for (auto i : v)
    {
        lexemesVector.push_back(i.GetLexeme());
    }

    return lexemesVector;
}

bool shouldConvertToFloat(string s) {
    for (int i = 0; i < s.size(); i++)
    {
        if (((s[i] == 'e' || s[i] == 'E') && s[i+1] == '-') || s[i] == '.') {
            return true;
        }
    }
    
    return false;
}

vector<double> doubleVector(vector<string> v) {
    vector<double> dv;

    vector<int> iv;
    double num;
    for (auto i : v)
    {
        if (!shouldConvertToFloat(i)) {
            if (hasEforInt(i)) {
                num = stod(i);
                int intNum = static_cast<int>(num);
                iv.push_back(intNum);
            } else {
                iv.push_back(stoi(i));
            }
        }
    }
    sort(iv.begin(), iv.end());

    vector<float> fv;
    for (auto i : v)
    {
        if (shouldConvertToFloat(i)) {
            fv.push_back(stof(i));
        }
    }
    sort(fv.begin(), fv.end());

    dv.reserve(iv.size() + fv.size());

    std::merge(iv.begin(), iv.end(), fv.begin(), fv.end(), std::back_inserter(dv));

    return dv;    
}

bool hasEforInt(string s) {
    for (auto i : s)
    {
        if (i == 'e' || i == 'E') {
            return true;
        }
    }

    return false;
}