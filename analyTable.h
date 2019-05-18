#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class analyTable {
    friend istream &operator>>(istream &, analyTable &);

   private:
    // 所有的非终结符
    set<string> nonSymbolList;
    // 所有的终结符
    set<string> symbolList;
    // 直接从文件中输入的产生式
    vector<string> proInput;
    // 非终结符的所有产生式
    map<string, set<vector<string>>> production;
    // 存储所有非终结符的左集合
    map<string, set<string>> leftSet;
    // 存储所有非终结符的右集合
    map<string, set<string>> rightSet;
    // 文件夹路径
    string folderPath;
    // 输入文件路径
    string inputPath;
    // 生成的中间文件路径
    string tempPath;
    // 结果文件路径
    string outputPath;
    // 记录第一个非终结符
    string firstNonSymbol;
    // 记录优先关系表
    map<string, map<string, string>> priorityTable;

    // 从输入文件中读入上下文无关文法
    void _fileInput();
    // 对读入的产生式进行处理，并生成中间文件
    void _generateTempFile();
    // 对字符串进行分割
    vector<string> _split(const string &, const string &);
    // 找出终结符
    void _findSymbol();
    // 在 priorityTable 里加入两个非终结符的关系
    void _addRelation(const string &, const string &, const string &);
    // 寻找非终结符的左集合
    void _findLeftSet();
    // 寻找左集合的递归函数
    void _findLeftSet(string, set<string> &, set<string> &);
    // 寻找非终结符的右集合
    void _findRightSet();
    // 寻找右集合的递归函数
    void _findRightSet(string, set<string> &, set<string> &);
    // 计算优先关系表
    void _calPriTable();
    // 将优先关系表写入到文件
    void _writeAnsToFile();
    // 去除字符串前后空格
    string &_removeSpace(string &);

   public:
    // 生成结果
    analyTable();
    void build();
};