#include "analyTable.h"

istream &operator>>(istream &is, analyTable &obj) {
    cout << "请输入上下文无关文法G所在的路径（直接输入回车表示该文件夹）:";
    getline(is, obj.folderPath);
    if (obj.folderPath == "") {
        obj.folderPath = "";
        obj.inputPath = "input.txt";
        obj.outputPath = "output.txt";
        obj.tempPath = "temp.txt";
    } else {
        obj.inputPath = obj.folderPath + "/input.txt";
        obj.outputPath = obj.folderPath + "/output.txt";
        obj.tempPath = obj.folderPath + "/temp.txt";
    }

    return is;
}

analyTable::analyTable() { firstNonSymbol = ""; }

void analyTable::_fileInput() {
    fstream fileInput(inputPath, ios::in);
    if (!fileInput.is_open()) {
        throw runtime_error("输入文件路径错误！");
        return;
    }

    string temp;
    while (getline(fileInput, temp)) {
        proInput.push_back(temp);
    }
    fileInput.close();
}

void analyTable::_generateTempFile() {
    fstream tempOutput(tempPath, ios::out);
    if (!tempOutput.is_open()) {
        throw runtime_error("临时文件打开失败！");
        return;
    }
    for (auto str : proInput) {
        stringstream ss(str);
        string nonSymbol;
        string production;
        vector<string> productions;
        set<vector<string>> productionSet;

        ss >> nonSymbol;
        _removeSpace(nonSymbol);
        nonSymbolList.insert(nonSymbol);
        if (firstNonSymbol == "") {
            firstNonSymbol = nonSymbol;
        }

        auto it = this->production.find(nonSymbol);
        if (it != this->production.end()) {
            productionSet = it->second;
            this->production.erase(it);
        }

        getline(ss, production);
        production = production.erase(0, 3);
        productions = _split(production, "|");

        for (auto str : productions) {
            _removeSpace(str);
            if (str != "") {
                productionSet.insert(_split(str, " "));
                tempOutput << nonSymbol << " -> " << str << endl;
            }
        }

        this->production[nonSymbol] = productionSet;
    }
    tempOutput.close();

    // 增加起始产生式
    set<vector<string>> productionSet;
    vector<string> tempProduction = {"$", firstNonSymbol, "$"};
    string tempNonProduction = "__" + firstNonSymbol;
    nonSymbolList.insert(tempNonProduction);
    productionSet.insert(tempProduction);
    this->production[tempNonProduction] = productionSet;
}

void analyTable::_findSymbol() {
    for (auto i : production) {
        for (auto j : i.second) {
            for (auto temp : j) {
                if (nonSymbolList.find(temp) == nonSymbolList.end()) {
                    symbolList.insert(temp);
                }
            }
        }
    }

    for (auto i : symbolList) {
        map<string, string> tempMap;
        for (auto j : symbolList) {
            tempMap[j] = "";
        }
        priorityTable[i] = tempMap;
    }
}

void analyTable::_findLeftSet() {
    for (auto i : nonSymbolList) {
        set<string> left;
        set<string> isVisit;
        isVisit.insert(i);
        _findLeftSet(i, isVisit, left);
        leftSet[i] = left;
    }
}

void analyTable::_findLeftSet(string non, set<string> &isVisit,
                              set<string> &left) {
    set<string> tempLeft;

    for (auto i : production[non]) {
        if (symbolList.find(i[0]) != symbolList.end()) {
            tempLeft.insert(i[0]);
        } else {
            if (isVisit.find(i[0]) == isVisit.end()) {
                isVisit.insert(i[0]);
                _findLeftSet(i[0], isVisit, tempLeft);
            }
            if (i.size() >= 2 && symbolList.find(i[1]) != symbolList.end()) {
                tempLeft.insert(i[1]);
            }
        }
    }

    for (auto i : tempLeft) {
        left.insert(i);
    }
}

void analyTable::_findRightSet() {
    for (auto i : nonSymbolList) {
        set<string> right;
        set<string> isVisit;
        isVisit.insert(i);
        _findRightSet(i, isVisit, right);
        rightSet[i] = right;
    }
}

void analyTable::_findRightSet(string non, set<string> &isVisit,
                               set<string> &right) {
    set<string> tempRight;

    for (auto i : production[non]) {
        if (symbolList.find(i[i.size() - 1]) != symbolList.end()) {
            tempRight.insert(i[i.size() - 1]);
        } else {
            if (isVisit.find(i[i.size() - 1]) == isVisit.end()) {
                isVisit.insert(i[i.size() - 1]);
                _findRightSet(i[i.size() - 1], isVisit, tempRight);
            }
            if (i.size() >= 2 &&
                symbolList.find(i[i.size() - 2]) != symbolList.end()) {
                tempRight.insert(i[i.size() - 2]);
            }
        }
    }

    for (auto i : tempRight) {
        right.insert(i);
    }
}

void analyTable::_calPriTable() {
    for (auto i : production) {
        for (auto j : i.second) {
            for (auto k = j.begin(); k != j.end() - 1; k++) {
                if (symbolList.find(*k) != symbolList.end() &&
                    symbolList.find(*(k + 1)) != symbolList.end()) {
                    _addRelation((*k), *(k + 1), "=");
                } else if (j.size() >= 2 && k <= j.end() - 2 &&
                           symbolList.find(*k) != symbolList.end() &&
                           nonSymbolList.find(*(k + 1)) !=
                               nonSymbolList.end() &&
                           symbolList.find(*(k + 2)) != symbolList.end()) {
                    _addRelation((*k), *(k + 2), "=");
                }

                if (symbolList.find(*k) != symbolList.end() &&
                    nonSymbolList.find(*(k + 1)) != nonSymbolList.end()) {
                    for (auto i : leftSet[*(k + 1)]) {
                        _addRelation(*k, i, "<");
                    }
                }

                if (nonSymbolList.find(*k) != nonSymbolList.end() &&
                    symbolList.find(*(k + 1)) != symbolList.end()) {
                    for (auto i : rightSet[*(k)]) {
                        _addRelation(i, *(k + 1), ">");
                    }
                }
            }
        }
    }
}

void analyTable::_addRelation(const string &a, const string &b,
                              const string &relation) {
    if (priorityTable[a][b].find(relation) == string::npos) {
        if (priorityTable[a][b] == ">") {
            priorityTable[a][b] = "<>";
        } else {
            priorityTable[a][b] += relation;
        }
    }
}

void analyTable::_writeAnsToFile() {
    fstream fileOutput(outputPath, ios::out);
    if (!fileOutput.is_open()) {
        throw runtime_error("输出文件路径错误！");
        return;
    }

    for (auto i : symbolList) {
        fileOutput << "\t" << i;
    }
    fileOutput << endl;
    for (auto i : symbolList) {
        fileOutput << i;
        for (auto j : symbolList) {
            fileOutput << '\t' << priorityTable[i][j];
        }
        fileOutput << '\n';
    }

    return;
}

void analyTable::build() {
    try {
        cout << "第一步：从input.txt读入上下文无关文法 ... ";
        _fileInput();
        cout << "完成！" << endl;

        cout << "第二步：对读入的产生式分离，找出非终结符，并生成中间文件 ... ";
        _generateTempFile();
        cout << "完成！" << endl;

        cout << "第三步：找出所有的终结符 ... ";
        _findSymbol();
        cout << "完成！" << endl;

        cout << "第四步：找出所有非终结符的左集合 ... ";
        _findLeftSet();
        cout << "完成！" << endl;

        cout << "第五步：找出所有非终结符的右集合 ... ";
        _findRightSet();
        cout << "完成！" << endl;

        cout << "第六步：计算优先关系表 ... ";
        _calPriTable();
        cout << "完成！" << endl;

        cout << "第七步：将优先关系表写到output.txt中 ... ";
        _writeAnsToFile();
        cout << "完成！" << endl;

        cout << endl << endl << "下面是输出的优先关系表:" << endl;
        for (auto i : symbolList) {
            cout << "\t" << i;
        }
        cout << endl;
        for (auto i : symbolList) {
            cout << i;
            for (auto j : symbolList) {
                cout << '\t' << priorityTable[i][j];
            }
            cout << '\n';
        }
    } catch (exception &ex) {
        cerr << ex.what() << endl;
    }

    //    for(auto i:production) {
    //        string temp=i.first;
    //        for(auto j:i.second) {
    //            cout<<temp<<" -> ";
    //            for(auto k:j) {
    //                cout<<k<<' ';
    //            }
    //            cout<<endl;
    //        }
    //    }
    //
    //    for(auto i:nonSymbolList) {
    //        cout<<i<<'\t';
    //    }
    //    cout<<'\n';
    //    for(auto i:symbolList) {
    //        cout<<i<<'\t';
    //    }
    //    cout<<'\n';
    //
    //    for(auto i:leftSet) {
    //        cout<<i.first<<' '<<"left ";
    //        for(auto j:i.second) {
    //            cout<<j<<' ';
    //        }
    //        cout<<endl;
    //    }
    //
    //    for(auto i:rightSet) {
    //        cout<<i.first<<' '<<"right ";
    //        for(auto j:i.second) {
    //            cout<<j<<' ';
    //        }
    //        cout<<endl;
    //    }
}

vector<string> analyTable::_split(const string &str, const string &delim) {
    vector<string> ans;
    if (str == "") {
        return ans;
    }

    char *strs = new char[str.length() + 1];
    strcpy(strs, str.c_str());

    char *d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char *p = strtok(strs, d);
    while (p) {
        string s = p;
        ans.push_back(s);
        p = strtok(NULL, d);
    }

    return ans;
}

string &analyTable::_removeSpace(string &s) {
    if (s.empty()) {
        return s;
    }
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}
