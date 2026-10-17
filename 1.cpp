
#include <bits/stdc++.h>
using namespace std;

// 不要修改这个标准输入函数
void read_prog(string& prog) {
    char c;
    while (scanf("%c", &c) != EOF) {
        prog += c;
    }
}

struct Token {
    int type;
    string value;
};

unordered_set<string> keywords = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto",
    "bitand", "bitor", "bool", "break", "case", "catch",
    "char", "char8_t", "char16_t", "char32_t", "class",
    "compl", "concept", "const", "consteval", "constexpr",
    "constinit", "const_cast", "continue", "co_await",
    "co_return", "co_yield", "decltype", "default", "delete",
    "do", "double", "dynamic_cast", "else", "enum", "explicit",
    "export", "extern", "false", "float", "for", "friend",
    "goto", "if", "inline", "int", "long", "mutable",
    "namespace", "new", "noexcept", "not", "not_eq", "nullptr",
    "operator", "or", "or_eq", "private", "protected", "public",
    "register", "reinterpret_cast", "requires", "return",
    "short", "signed", "sizeof", "static", "static_assert",
    "static_cast", "struct", "switch", "template", "this",
    "thread_local", "throw", "true", "try", "typedef",
    "typeid", "typename", "union", "unsigned", "using",
    "virtual", "void", "volatile", "wchar_t", "while",
    "xor", "xor_eq"
};

bool isOperator(const string& s) {
    static const unordered_set<string> operators = {
        "+", "-", "*", "/", "%", "=",
        "==", "!=", "<", ">", "<=", ">=",
        "++", "--", "+=", "-=", "*=", "/=", "%=",
        "&&", "||", "!", "&", "|", "^", "~",
        "<<", ">>", "<<=", ">>=", "->",
        "&=", "|=", "^="
    };

    return operators.count(s) > 0;
}

vector<Token> Lexical(const string& code) {
    vector<Token> tokens;
    int n = (int)code.size();
    int i = 0;

    // 多字符运算符优先匹配
    static const vector<string> operators = {
        "<<=", ">>=", "++", "--", "==", "!=", "<=", ">=",
        "+=", "-=", "*=", "/=", "%=", "&&", "||",
        "<<", ">>", "->", "&=", "|=", "^="
    };

    while (i < n) {
        unsigned char c = (unsigned char)code[i];

        // # 为结束符
        if (code[i] == '#') {
            break;
        }

        // 忽略空白字符
        if (isspace(c)) {
            i++;
            continue;
        }

        // 忽略单行注释
        if (code[i] == '/' && i + 1 < n && code[i + 1] == '/') {
            tokens.push_back({5, "//"});
            i += 2;
            continue;
        }

        // 忽略多行注释
        if (code[i] == '/' && i + 1 < n && code[i + 1] == '*') {
            i += 2;
            while (i + 1 < n &&
                   !(code[i] == '*' && code[i + 1] == '/')) {
                if (code[i] == '#') {
                    break;
                }
                i++;
            }

            if (i + 1 < n &&
                code[i] == '*' && code[i + 1] == '/') {
                i += 2;
            }
            continue;
        }

        // 识别字符串
        if (code[i] == '"') {
            tokens.push_back({5, "\""});
            i++;

            while (i < n && code[i] != '"') {
                unsigned char ch = (unsigned char)code[i];

                // 遇到结束符，停止分析
                if (code[i] == '#') {
                    break;
                }

                // 处理转义字符
                if (code[i] == '\\' && i + 1 < n) {
                    i += 2;
                    continue;
                }

                // 非 ASCII 字节：按原题要求逐字节报错
                if (ch >= 0x80) {
                    tokens.push_back({0, ""});
                    i++;
                    continue;
                }

                // 忽略字符串内部的空白字符
                if (isspace(ch)) {
                    i++;
                    continue;
                }

                // 识别字符串内部的标识符
                if (isalpha(ch) || ch == '_') {
                    string word;
                    while (i < n) {
                        unsigned char x = (unsigned char)code[i];
                        if (!isalnum(x) && x != '_') {
                            break;
                        }
                        word += code[i++];
                    }
                    tokens.push_back({2, word});
                    continue;
                }

                // 识别字符串内部的数字
                if (isdigit(ch)) {
                    string number;
                    while (i < n &&
                           isdigit((unsigned char)code[i])) {
                        number += code[i++];
                    }
                    tokens.push_back({3, number});
                    continue;
                }

                // 识别字符串内部的运算符
                string op(1, code[i]);
                if (isOperator(op)) {
                    tokens.push_back({4, op});
                }

                i++;
            }

            // 字符串结束引号
            if (i < n && code[i] == '"') {
                tokens.push_back({5, "\""});
                i++;
            }

            continue;
        }

        // 识别标识符和关键字
        if (isalpha(c) || c == '_') {
            string word;

            while (i < n) {
                unsigned char x = (unsigned char)code[i];
                if (!isalnum(x) && x != '_') {
                    break;
                }
                word += code[i++];
            }

            if (keywords.count(word)) {
                tokens.push_back({1, word});
            } else {
                tokens.push_back({2, word});
            }

            continue;
        }

        // 识别整数
        if (isdigit(c)) {
            string number;

            while (i < n &&
                   isdigit((unsigned char)code[i])) {
                number += code[i++];
            }

            tokens.push_back({3, number});
            continue;
        }

        // 最长匹配运算符
        bool matched = false;

        for (const string& op : operators) {
            if (i + (int)op.size() <= n &&
                code.compare(i, op.size(), op) == 0) {
                tokens.push_back({4, op});
                i += (int)op.size();
                matched = true;
                break;
            }
        }

        if (matched) {
            continue;
        }

        // 单字符运算符或界符
        string one(1, code[i]);

        if (isOperator(one)) {
            tokens.push_back({4, one});
        } else {
            tokens.push_back({5, one});
        }

        i++;
    }

    return tokens;
}

void Analysis() {
    string prog;
    read_prog(prog);

    cout << "Please input string:" << endl;

    vector<Token> ans = Lexical(prog);

    for (const auto& t : ans) {
        if (t.type == 0) {
            cout << "Error in row 8!" << endl;
        } else {
            cout << "(" << t.type << "," << t.value << ")" << endl;
        }
    }
}

int main() {
    Analysis();
    return 0;
}