#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <iomanip>
using namespace std;
string V[100][2]; //存储拆分后的文法
int vi = 0; //存储拆分后有多少行
int t = 0;
int s = 0;
int l;
int r;
string FIRSTVT[20][2]; //存储firstvt集
string LASTVT[20][2]; //存储lastvt集
string str; //存储输入文法
string str_a = "#"; // 下堆栈
string str_b; // 剩余输入串
string analysis_table[100][5]; // 文法分析过程
char table[100][100]; // 算符优先关系表
void init_string(string &a) // 删除字符串的第一个元素
{
    for (int i = 1; i <= a.length(); ++i)
    {
        a[i - 1] = a[i];
    }
}
bool is_CHAR(char c) // 判断是否为大写字母
{
    if (c >= 'A' && c <= 'Z')
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool is_in(int i, string x) // 判断从字符串x从最好一个开始算起连续的i个字符是否含有非大写字母
{
    bool flag = false;
    for (int j = 0; j < i; j++)
    {
        if (!is_CHAR(x[x.length() - j - 1]))
        {
            flag = true;
        }
    }
    return flag;
}
void split(string a) // 拆分文法，使其不含有｜
{
    for (int i = 3; i < a.length(); ++i)
    {
        V[vi][0] = a[0];
        while (a[i] != '|' && i < a.length())
        {
            V[vi][1] += a[i];
            i++;
        }
        vi++;
    }
}
void read_file(string file_path) //按行读取文件
{
    fstream f(file_path);
    vector<string> words;
    string line;
    while (getline(f, line))
    {
        words.push_back(line);
    }
    cout << "输入文法:" << endl;
    for (int i = 0; i < words.size(); i++)
    {
        cout << words[i] << endl;
        split(words[i]);
    }
}
int find_index(char a) //寻找字符a在firstvt或者lastvt中的位置
{
    for (int i = 0; i < t; ++i)
    {
        if (FIRSTVT[i][0][0] == a)
        {
            return i;
        }
    }
    return -1;
}
int find_table_index(char a) //寻找字符a在算符优先关系表中的位置
{
    for (int i = 0; i <= s; ++i)
    {
        if (table[i][0] == a)
        {
            return i;
        }
    }
    return -1;
}
void get_start() //获取非终结符
{
    for (int i = 0; i < vi; ++i)
    {
        bool flag = true;
        for (int j = 0; j < t; ++j)
        {
            if (FIRSTVT[j][0] == V[i][0])
            {
                flag = false;
            }
        }
        if (flag)
        {
            FIRSTVT[t][0] = V[i][0];
            LASTVT[t][0] = V[i][0];
            t++;
        }
    }
}
/********Beign********/
/*构造firstvt,lastvt*/
void add_firstvt(string b, int a) //判断字符串b是否在序号为a的firstvt中，没有则加入
{
    if (a < 0 || a >= 20)
    {
        return;
    }
    for (int i = 0; i < b.length(); ++i)
    {
        if (FIRSTVT[a][1].find(b[i]) == string::npos)
        {
            FIRSTVT[a][1] += b[i];
        }
    }
}
void add_firstvt(char c, int a) //判断字符c是否在序号为a的firstvt中，没有则加入
{
    if (a < 0 || a >= 20)
    {
        return;
    }
    if (FIRSTVT[a][1].find(c) == string::npos)
    {
        FIRSTVT[a][1] += c;
    }
}
void add_lastvt(string b, int a) //判断字符串b是否在序号为a的lastvt中，没有则加入
{
    if (a < 0 || a >= 20)
    {
        return;
    }
    for (int i = 0; i < b.length(); ++i)
    {
        if (LASTVT[a][1].find(b[i]) == string::npos)
        {
            LASTVT[a][1] += b[i];
        }
    }
}
void add_lastvt(char c, int a) //判断字符串c是否在序号为a的lastvt中，没有则加入
{
    if (a < 0 || a >= 20)
    {
        return;
    }
    if (LASTVT[a][1].find(c) == string::npos)
    {
        LASTVT[a][1] += c;
    }
}
string get_cur_firstvt(char c, int a) //获取当前字符的firstvt，并放入序号为a的firstvt中
{
    static bool visiting[128] = { false };
    if (a < 0 || a >= 20)
    {
        return "";
    }
    if (visiting[(int)c]) //正在求解中，直接返回，防止死递归
    {
        return FIRSTVT[a][1];
    }
    visiting[(int)c] = true;
    for (int i = 0; i < vi; ++i)
    {
        if (V[i][0].length() == 0 || V[i][0][0] != c)
        {
            continue;
        }
        string b = V[i][1];
        int len = b.length();
        if (len == 0)
        {
            continue;
        }
        if (!is_CHAR(b[0])) //P->a...，终结符a直接进FIRSTVT
        {
            add_firstvt(b[0], a);
        }
        else
        {
            if (len > 1 && !is_CHAR(b[1])) //P->Qa...，终结符a也进FIRSTVT
            {
                add_firstvt(b[1], a);
            }
            if (b[0] != c) //P->Q...，把FIRSTVT(Q)并入FIRSTVT(P)
            {
                add_firstvt(get_cur_firstvt(b[0], find_index(b[0])), a);
            }
        }
    }
    visiting[(int)c] = false;
    return FIRSTVT[a][1];
}
string get_cur_lastvt(char c, int a) //获取当前字符的lastvt，并放入序号为a的lastvt中
{
    static bool visiting[128] = { false };
    if (a < 0 || a >= 20)
    {
        return "";
    }
    if (visiting[(int)c])
    {
        return LASTVT[a][1];
    }
    visiting[(int)c] = true;
    for (int i = 0; i < vi; ++i)
    {
        if (V[i][0].length() == 0 || V[i][0][0] != c)
        {
            continue;
        }
        string b = V[i][1];
        int len = b.length();
        if (len == 0)
        {
            continue;
        }
        if (!is_CHAR(b[len - 1])) //P->...a，终结符a直接进LASTVT
        {
            add_lastvt(b[len - 1], a);
        }
        else
        {
            if (b[len - 1] != c) //P->...Q，把LASTVT(Q)并入LASTVT(P)
            {
                add_lastvt(get_cur_lastvt(b[len - 1], find_index(b[len - 1])), a);
            }
            if (len > 1 && !is_CHAR(b[len - 2])) //P->...aQ，终结符a也进LASTVT
            {
                add_lastvt(b[len - 2], a);
            }
        }
    }
    visiting[(int)c] = false;
    return LASTVT[a][1];
}
/*********End*********/
void get_firstvt() //获取所有文法的firstvt
{
    for (int i = 0; i < t; i++)
    {
        get_cur_firstvt(FIRSTVT[i][0][0], i);
    }
}
void get_lastvt() //获取所有文法的lastvt
{
    for (int i = 0; i < t; i++)
    {
        get_cur_lastvt(LASTVT[i][0][0], i);
    }
}
void print_firstvt(string t, string a) //打印非终极符为t的firstvt
{
    cout << "FIRSTVT(" << t << ") = {";
    for (int i = 0; i < a.length(); ++i)
    {
        if (i == a.length() - 1)
        {
            cout << "\"" << a[i] << "\"";
        }
        else
        {
            cout << "\"" << a[i] << "\"" << ", ";
        }
    }
    cout << "}" << endl;
}
void print_lastvt(string t, string a) //打印非终结符为t的lastvt
{
    cout << "LASTVT(" << t << ") = {";
    for (int i = 0; i < a.length(); ++i)
    {
        if (i == a.length() - 1)
        {
            cout << "\"" << a[i] << "\"";
        }
        else
        {
            cout << "\"" << a[i] << "\"" << ", ";
        }
    }
    cout << "}" << endl;
}
/********Beign********/
/*构造算符优先表*/

void init_table() //初始化算符优先关系表
{
    for (int i = 0; i < 100; ++i)
    {
        for (int j = 0; j < 100; ++j)
        {
            table[i][j] = ' ';
        }
    }
    s = 0;
    table[0][0] = '\\'; //表头左上角
    //扫描全部产生式，把终结符按出现顺序填入表格的第一行和第一列
    for (int i = 0; i < vi; ++i)
    {
        for (int j = 0; j < V[i][1].length(); ++j)
        {
            char c = V[i][1][j];
            if (is_CHAR(c))
            {
                continue;
            }
            if (find_table_index(c) != -1)
            {
                continue;
            }
            s++;
            table[0][s] = c;
            table[s][0] = c;
        }
    }
}

void set_table_relation(char a, char b, char relationship) //把终结符a，b的优先关系写入表中
{
    int row = find_table_index(a);
    int column = find_table_index(b);
    if (row == -1 || column == -1)
    {
        return;
    }
    table[row][column] = relationship;
}
void set_firstvt_relation(char a, char B, char relationship) //a < FIRSTVT(B)中的每个终结符
{
    int index = find_index(B);
    if (index == -1)
    {
        return;
    }
    for (int i = 0; i < FIRSTVT[index][1].length(); ++i)
    {
        set_table_relation(a, FIRSTVT[index][1][i], relationship);
    }
}
void set_lastvt_relation(char A, char b, char relationship) //LASTVT(A)中的每个终结符 > b
{
    int index = find_index(A);
    if (index == -1)
    {
        return;
    }
    for (int i = 0; i < LASTVT[index][1].length(); ++i)
    {
        set_table_relation(LASTVT[index][1][i], b, relationship);
    }
}

void get_table() //生成算符优先关系表
{
    for (int k = 0; k < vi; ++k)
    {
        string b = V[k][1];
        int len = b.length();
        for (int i = 0; i + 1 < len; ++i)
        {
            //两个终结符相邻：X[i] = X[i+1]
            if (!is_CHAR(b[i]) && !is_CHAR(b[i + 1]))
            {
                set_table_relation(b[i], b[i + 1], '=');
            }
            //终结符、非终结符、终结符：X[i] = X[i+2]
            if (i + 2 < len && !is_CHAR(b[i]) && is_CHAR(b[i + 1]) && !is_CHAR(b[i + 2]))
            {
                set_table_relation(b[i], b[i + 2], '=');
            }
            //终结符后跟非终结符：X[i] < FIRSTVT(X[i+1])中的每个终结符
            if (!is_CHAR(b[i]) && is_CHAR(b[i + 1]))
            {
                set_firstvt_relation(b[i], b[i + 1], '<');
            }
            //非终结符后跟终结符：LASTVT(X[i])中的每个终结符 > X[i+1]
            if (is_CHAR(b[i]) && !is_CHAR(b[i + 1]))
            {
                set_lastvt_relation(b[i], b[i + 1], '>');
            }
        }
    }
}
/*********End*********/
void print_table() //打印算符优先关系表
{
    for (int i = 0; i < s + 1; ++i)
    {
        for (int j = 0; j < s + 1; ++j)
        {
            cout << table[i][j] << " ";
        }
        cout << endl;
    }
}
char get_relationship(char a, char b) //获取终结符a，b的优先关系
{
    return table[find_table_index(a)][find_table_index(b)];
}
bool is_reduce() //判断是否可以规约
{
    for (int i = 0; i < vi; ++i)
    {
        int count = 0;
        int f = str_a.length() - 1;
        for (int j = V[i][1].length() - 1; j >= 0 && f >= 0; j--, f--)
        {
            if (is_in(V[i][1].length(), str_a))
            {
                if (is_CHAR(str_a[f]) && is_CHAR(V[i][1][j]))
                {
                    count++;
                }
                else if (str_a[f] == V[i][1][j])
                {
                    count++;
                }
            }
            else
            {
                continue;
            }
        }
        if (count == V[i][1].length())
        {
            r = i;
            return true;
        }
    }
    return false;
}
void analyze_input_string() // 生成算符优先文法的分析过程
{
    analysis_table[0][0] = "步骤";
    analysis_table[0][1] = "下堆栈";
    analysis_table[0][2] = "优先关系";
    analysis_table[0][3] = "剩余输入串";
    analysis_table[0][4] = "移进或规约";
    str_b = str;
    char relationship;
    l = 1;
    int x;
    stringstream ss;
    while (true)
    {
        ss << l;
        int index = str_a.length() - 1;
        analysis_table[l][0] = ss.str();
        analysis_table[l][3] = str_b;
        analysis_table[l][1] = str_a;
        ss.clear();
        ss.str("");
        if (is_CHAR(str_a[index]))
        {
            for (int i = str_a.length() - 1; i >= 0; i--)
            {
                if (!is_CHAR(str_a[i]))
                {
                    index = i;
                    break;
                }
            }
        }
        relationship = get_relationship(str_a[index], str_b[0]);
        analysis_table[l][2] = relationship;
        if (relationship == '=')
        {
            if (str_a[index] == '#' && str_b[0] == '#')
            {
                analysis_table[l][4] = "完成";
                break;
            }
            else
            {
                analysis_table[l][4] = "移进";
                str_a += str_b[0];
                analysis_table[l + 1][1] = str_a;
                init_string(str_b);
            }
        }
        else if (relationship == '<')
        {
            analysis_table[l][4] = "移进";
            str_a += str_b[0];
            analysis_table[l + 1][1] = str_a;
            init_string(str_b);
        }
        else if (relationship == '>')
        {
            if (is_reduce())
            {
                analysis_table[l][4] = "规约";
                str_a[str_a.length() - V[r][1].length()] = V[r][0][0];
                str_a.erase(str_a.length() - V[r][1].length() + 1, V[r][1].length() - 1);
            }
            else
            {
                cout << "输入串非法" << endl;
                exit(-1);
            }
        }
        l++;
    }
}
void print_analyze_process() //打印算符优先文法的分析过程
{
    cout << "算符优先分析过程" << endl;
    cout << setw(12) << analysis_table[0][0] << setw(16) << analysis_table[0][1] << setw(16) << analysis_table[0][2]
         << setw(24)
         << analysis_table[0][3] << setw(20)
         << analysis_table[0][4] << endl;
    for (int i = 1; i <= l; ++i)
    {
        cout.width(10);
        cout << analysis_table[i][0];
        cout.width(12);
        cout << analysis_table[i][1];
        cout.width(10);
        cout << analysis_table[i][2];
        cout.width(20);
        cout << analysis_table[i][3];
        cout << analysis_table[i][4];
        cout << endl;
    }
}
int main(int argv, char *arg[])
{
    cout.setf(std::ios::left);
    read_file("/data/workspace/myshixun/in.txt");
    cout << "拆分后文法:" << endl;
    for (int i = 0; i < vi; ++i)
    {
        cout << V[i][0] << "->" << V[i][1] << endl;
    }
    cout << "非终结符:" << endl;
    get_start();
    for (int j = 0; j < t; ++j)
    {
        cout << FIRSTVT[j][0] << endl;
    }
    cout << "FIRSTVT:" << endl;
    get_firstvt();
    for (int k = 0; k < t; ++k)
    {
        print_firstvt(FIRSTVT[k][0], FIRSTVT[k][1]);
    }
    cout << "LASTVT:" << endl;
    get_lastvt();
    for (int k = 0; k < t; ++k)
    {
        print_lastvt(LASTVT[k][0], LASTVT[k][1]);
    }
    cout << "算符优先关系表" << endl;
    init_table();
    get_table();
    print_table();
    cout << "请输入文法并以#结束:" << endl;
    cin >> str;
    analyze_input_string();
    print_analyze_process();
    return 0;
}

// made in Lyon
// Trying to do better!