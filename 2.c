#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef success
#define success 1
#endif

#define MAX_AMOUNT 20

//定义非终结符
typedef struct NOterminal
{
    char character;
    int  first_number;  //FIRST集的判定，初始为0
    int  follow_number; //FOLLOW集的判定，初始为0
    char* FIRST;       //FIRST 集合
    char* FOLLOW;      //FOLLOW集合
    struct NOterminal* next;
} noterminal;

//定义终结符
typedef struct Terminal
{
    char  character;        //当前的字符
    struct Terminal* next;
} terminal;

//定义产生式
typedef struct PRODUCTION
{
    char source;                //产生的开始
    char* result;              //产生的结果
    struct PRODUCTION* next;    //指向下一条
} production;

int amount = 0;
char TEST_STACK[20];
char* TEST_LIST[10];
char terminal_all[10] = { 0 };
terminal   TERMINAL_HEAD, * current_terminal = &TERMINAL_HEAD;
noterminal NOTERMINAL_HEAD, * current_noterminal = &NOTERMINAL_HEAD;
production PRODUCTION_HEAD, * current_production = &PRODUCTION_HEAD;

//函数申明
size_t read(void);
size_t test(void);
size_t write(void);
void Test_read(void);
size_t STACK_FULL();
void STACK_POP(void);
size_t STACK_EMPTY();
void init_stack(void);
void prediction(void);
void test_follow(void);
void emergency(int model);
void prediction_table(void);
void STACK_PUSH(char source);
void insert_to_terminal(char get);
void insert_to_noterminal(char get);
void eliminate_left_recursion(void);
void combine(char* destinction, char* source);
size_t is_appeared(char tobejudged, char* source);
void insert_to_production(char source, char* result);
size_t find_first(noterminal* this_noterminal, production* this_production);
size_t find_follow(noterminal* this_noterminal, production* this_production);



int main(void)
{
    TERMINAL_HEAD.next = NULL;
    NOTERMINAL_HEAD.next = NULL;
    PRODUCTION_HEAD.next = NULL;
    read();
    Test_read();
    printf("\n消除左递归\n\n");
    eliminate_left_recursion();

    Test_read();

    printf("\n求FIRST集\n\n");
    prediction();
    printf("\n求FOLLOW集\n\n");
    test_follow();
    prediction_table();
    emergency(0);
    return 0;
}

//读文件
size_t read(void)
{
    int line = 0, model = 0, old_line = 0;
    int number = 0;
    char current_get = 0, read_temp[10] = { 0 };
    FILE* read_stream = fopen("test.txt", "r");
    if (!read_stream)
    {
        printf("error in open file ,can`t open file\n");
        exit(EXIT_FAILURE);
    }
    insert_to_terminal('#');    //插入栈底元素，以#表示栈底
    insert_to_terminal('^');    //插入空串元素，以^表示栈底
    while (!feof(read_stream))
    {
        current_get = fgetc(read_stream);
        while (current_get == ' ')
            current_get = fgetc(read_stream); //忽略空格
        while (current_get == '\n')
        {
            current_get = fgetc(read_stream);
            line++;     //跳过换行
        }

        switch (line)
        {
        case 0:
            insert_to_noterminal(current_get);
            break;
        case 1:
            insert_to_terminal(current_get);
            break;
        case 3:
            ungetc(current_get, read_stream);
            old_line = 1;
            break;
        default:
            perror("error in format of program \n");
            emergency(0);
            break;
        }
        if (old_line)
            break;
    }
    while (!feof(read_stream))
    {
        memset(read_temp, 0, sizeof(read_temp));
        old_line = line;
        current_get = fgetc(read_stream);
        while (current_get == ' ')
            current_get = fgetc(read_stream);     //忽略空格
        while (current_get == '\n')
        {
            current_get = fgetc(read_stream);
            line++;     //跳过换行
        }
        model = ((line - old_line) > model) ? (line - old_line) : model;
        switch (model)
        {
        case 0:
        case 1:
            fscanf(read_stream, "%s", read_temp);
            insert_to_production(current_get, read_temp);
            break;
        case 2:
            ungetc(current_get, read_stream);
            TEST_LIST[number] = (char*)malloc(20);
            memset(TEST_LIST[number], 0, 20);
            fscanf(read_stream, "%s", TEST_LIST[number++]);
            break;
        default:
            perror("error in format of program \n");
            emergency(1);
            break;
        }
    }

    fclose(read_stream);
    return success;
}
//测试
size_t test(void)
{
    noterminal* TEMP_NOTERMINAL = NOTERMINAL_HEAD.next;

    for (; TEMP_NOTERMINAL != NULL; TEMP_NOTERMINAL = TEMP_NOTERMINAL->next)
    {
        printf("%c\tfirst number=%d\tfirst=%s\n",
            TEMP_NOTERMINAL->character,
            TEMP_NOTERMINAL->first_number,
            TEMP_NOTERMINAL->FIRST);
    }
    printf("\n");
    return success;
}

/******Begin******/
//在非终结符表中查找字符对应的结点，不是非终结符则返回NULL
static noterminal* get_noterminal(char get)
{
    noterminal* TEMP_NOTERMINAL = NOTERMINAL_HEAD.next;
    while (TEMP_NOTERMINAL != NULL)
    {
        if (TEMP_NOTERMINAL->character == get)
            return TEMP_NOTERMINAL;
        TEMP_NOTERMINAL = TEMP_NOTERMINAL->next;
    }
    return NULL;
}

//为集合申一块空间并置空
static char* new_set(void)
{
    char* temp = (char*)malloc(MAX_AMOUNT);
    if (!temp)
    {
        perror("can`t malloc for this program\n");
        emergency(0);
    }
    memset(temp, 0, MAX_AMOUNT);
    return temp;
}

//把字符get加入集合destinction，返回是否使集合发生了变化
static size_t add_to_set(char* destinction, char get)
{
    char temp[2] = { get, 0 };
    if (destinction == NULL || get == 0)
        return !success;
    if (is_appeared(get, destinction))
        return !success;
    strcat(destinction, temp);
    return success;
}

//求串beta的FIRST集合（不含空串^），返回值表示beta能否推出空串
static size_t first_of_string(char* beta, char* result)
{
    noterminal* TEMP_NOTERMINAL = NULL;
    char* temp = NULL;
    while (*beta != 0)
    {
        if (*beta == '^')               //右部为空串
            return success;
        TEMP_NOTERMINAL = get_noterminal(*beta);
        if (TEMP_NOTERMINAL == NULL)    //终结符，FIRST到此为止
        {
            add_to_set(result, *beta);
            return !success;
        }
        if (TEMP_NOTERMINAL->FIRST == NULL)
            return !success;
        for (temp = TEMP_NOTERMINAL->FIRST; *temp != 0; temp++)
            if (*temp != '^')
                add_to_set(result, *temp);
        if (!is_appeared('^', TEMP_NOTERMINAL->FIRST))  //该非终结符不能推出空串
            return !success;
        beta++;
    }
    return success;
}

//求FIRST集合
size_t find_first(noterminal* this_noterminal, production* this_production)
{
    noterminal* TEMP_NOTERMINAL = NULL;
    production* TEMP_PRODUCTION = NULL;
    char right_first[MAX_AMOUNT] = { 0 };
    char* temp = NULL;
    size_t change = !success;

    if (this_noterminal == NULL)
        return !success;

    //为所有非终结符的FIRST集合申空间
    for (TEMP_NOTERMINAL = NOTERMINAL_HEAD.next; TEMP_NOTERMINAL != NULL; TEMP_NOTERMINAL = TEMP_NOTERMINAL->next)
        if (TEMP_NOTERMINAL->FIRST == NULL)
            TEMP_NOTERMINAL->FIRST = new_set();

    //反复扫描全部产生式，直到所有FIRST集合都不再增大
    do
    {
        change = !success;
        for (TEMP_PRODUCTION = this_production; TEMP_PRODUCTION != NULL; TEMP_PRODUCTION = TEMP_PRODUCTION->next)
        {
            TEMP_NOTERMINAL = get_noterminal(TEMP_PRODUCTION->source);
            if (TEMP_NOTERMINAL == NULL)
                continue;
            memset(right_first, 0, sizeof(right_first));
            if (first_of_string(TEMP_PRODUCTION->result, right_first))  //右部可推出空串
                add_to_set(right_first, '^');
            for (temp = right_first; *temp != 0; temp++)                //FIRST(右部)并入FIRST(左部)
                if (add_to_set(TEMP_NOTERMINAL->FIRST, *temp))
                    change = success;
        }
    } while (change);

    for (TEMP_NOTERMINAL = NOTERMINAL_HEAD.next; TEMP_NOTERMINAL != NULL; TEMP_NOTERMINAL = TEMP_NOTERMINAL->next)
        TEMP_NOTERMINAL->first_number = (int)strlen(TEMP_NOTERMINAL->FIRST);

    return success;
}
/******End******/
/******Begin******/
//求FOLLOW集合
size_t find_follow(noterminal* this_noterminal, production* this_production)
{
    noterminal* TEMP_NOTERMINAL = NULL;
    noterminal* SOURCE_NOTERMINAL = NULL;
    production* TEMP_PRODUCTION = NULL;
    char beta_first[MAX_AMOUNT] = { 0 };
    char* temp = NULL;
    char* scan = NULL;
    size_t change = !success;
    size_t check_empty = !success;

    if (this_noterminal == NULL)
        return !success;

    //为所有非终结符的FOLLOW集合申空间
    for (TEMP_NOTERMINAL = NOTERMINAL_HEAD.next; TEMP_NOTERMINAL != NULL; TEMP_NOTERMINAL = TEMP_NOTERMINAL->next)
        if (TEMP_NOTERMINAL->FOLLOW == NULL)
            TEMP_NOTERMINAL->FOLLOW = new_set();

    //开始符号放入#
    if (NOTERMINAL_HEAD.next != NULL)
        add_to_set(NOTERMINAL_HEAD.next->FOLLOW, '#');

    //反复扫描全部产生式，直到所有FOLLOW集合都不再增大
    do
    {
        change = !success;
        for (TEMP_PRODUCTION = this_production; TEMP_PRODUCTION != NULL; TEMP_PRODUCTION = TEMP_PRODUCTION->next)
        {
            for (scan = TEMP_PRODUCTION->result; *scan != 0; scan++)
            {
                TEMP_NOTERMINAL = get_noterminal(*scan);
                if (TEMP_NOTERMINAL == NULL)        //终结符跳过
                    continue;
                //规则2：A -> αBβ，把FIRST(β)\{ε}加入FOLLOW(B)
                memset(beta_first, 0, sizeof(beta_first));
                check_empty = first_of_string(scan + 1, beta_first);
                for (temp = beta_first; *temp != 0; temp++)
                    if (add_to_set(TEMP_NOTERMINAL->FOLLOW, *temp))
                        change = success;
                //规则3：A -> αB 或 A -> αBβ 且 ε∈FIRST(β)，把FOLLOW(A)加入FOLLOW(B)
                if (check_empty)
                {
                    SOURCE_NOTERMINAL = get_noterminal(TEMP_PRODUCTION->source);
                    if (SOURCE_NOTERMINAL != NULL && SOURCE_NOTERMINAL != TEMP_NOTERMINAL)
                        for (temp = SOURCE_NOTERMINAL->FOLLOW; temp != NULL && *temp != 0; temp++)
                            if (add_to_set(TEMP_NOTERMINAL->FOLLOW, *temp))
                                change = success;
                }
            }
        }
    } while (change);

    for (TEMP_NOTERMINAL = NOTERMINAL_HEAD.next; TEMP_NOTERMINAL != NULL; TEMP_NOTERMINAL = TEMP_NOTERMINAL->next)
        TEMP_NOTERMINAL->follow_number = (int)strlen(TEMP_NOTERMINAL->FOLLOW);

    return success;
}
/******End******/


//紧急退出
void emergency(int model)
{
    current_noterminal = NOTERMINAL_HEAD.next;
    current_terminal = TERMINAL_HEAD.next;
    current_production = PRODUCTION_HEAD.next;
    while (current_noterminal)
    {
        NOTERMINAL_HEAD.next = current_noterminal->next;
        free(current_noterminal->FIRST);
        free(current_noterminal->FOLLOW);
        free(current_noterminal);
        current_noterminal = NOTERMINAL_HEAD.next;
    }
    while (current_terminal)
    {
        TERMINAL_HEAD.next = current_terminal->next;
        free(current_terminal);
        current_terminal = TERMINAL_HEAD.next;
    }
    while (current_production)
    {
        PRODUCTION_HEAD.next = current_production->next;
        free(current_production->result);
        free(current_production);
        current_production = PRODUCTION_HEAD.next;
    }
    printf("退出成功\n");
    exit(0);
}

//插入到终结符
void insert_to_terminal(char get)
{
    terminal* Temp_terminal = (terminal*)malloc(sizeof(terminal));
    if (!Temp_terminal)
    {
        perror("can`t malloc for this program\n");
        emergency(0);
    }
    Temp_terminal->character = get;
    Temp_terminal->next = NULL;
    current_terminal->next = Temp_terminal;
    current_terminal = Temp_terminal; //移向下一个节点
    return;
}

//插入到非终结符
void insert_to_noterminal(char get)
{
    noterminal* Temp_noterminal = (noterminal*)malloc(sizeof(noterminal));
    if (!Temp_noterminal)
    {
        perror("can`t malloc for this program\n");
        emergency(0);
    }
    Temp_noterminal->character = get;
    Temp_noterminal->next = NULL;
    Temp_noterminal->FIRST = NULL;
    Temp_noterminal->FOLLOW = NULL;
    Temp_noterminal->first_number = 0;
    Temp_noterminal->follow_number = 0;
    current_noterminal->next = Temp_noterminal;
    current_noterminal = Temp_noterminal;     //移向下一个节点
    return;
}

//插入到产生式
void insert_to_production(char source, char* result)
{

    char TEMP[20] = { 0 };
    int COUNT = 0, number = 0, length = 0, exit_condition = strlen(result);
    production* Temp_production;
    for (COUNT = 0; COUNT != exit_condition + 1; COUNT++)
    {
        if (*result == '-' && *(result + 1) == '>')
        {
            result += 2;
            COUNT += 2;
        }
        if ((*result != '|') && (*result))
            TEMP[number++] = *result;
        else
        {
            Temp_production = (production*)malloc(sizeof(production));
            length = strlen(TEMP) + 1;
            Temp_production->result = (char*)malloc(length);
            memset(Temp_production->result, 0, length);
            strncpy(Temp_production->result, TEMP, length - 1);
            memset(TEMP, 0, sizeof(TEMP));
            Temp_production->source = source;
            Temp_production->next = NULL;
            current_production->next = Temp_production;
            current_production = Temp_production;
            number = 0;
        }
        result++;
    }
    return;
}



//消除左递归
void eliminate_left_recursion(void)
{
    int number = 0;
    char new_char[3] = { 0 }, TEMP_RESULT[20], temp_empty[3] = { '^',0,0 };
    production* Temp_production = PRODUCTION_HEAD.next;
    production* Temp_FREE;
    terminal* temp = TERMINAL_HEAD.next;
    while (Temp_production)
    {
        if (Temp_production->source == Temp_production->result[0])
        {
            memset(TEMP_RESULT, 0, sizeof(TEMP_RESULT));
            new_char[0] = Temp_production->source - 'A' + 'a';
            //复制到新的产生式
            strcat(TEMP_RESULT, Temp_production->result + 1);
            strcat(TEMP_RESULT, new_char);
            insert_to_noterminal(new_char[0]);
            insert_to_production(new_char[0], TEMP_RESULT);
            insert_to_production(new_char[0], temp_empty);

            //修改当前的产生式
            memset(TEMP_RESULT, 0, sizeof(TEMP_RESULT));
            strcat(TEMP_RESULT, Temp_production->next->result);
            strcat(TEMP_RESULT, new_char);
            memset(Temp_production->result, 0, strlen(Temp_production->result));
            strncpy(Temp_production->result, TEMP_RESULT, strlen(TEMP_RESULT));

            Temp_FREE = Temp_production->next;
            Temp_production->next = Temp_production->next->next;
            free(Temp_FREE);
            continue;
        }
        Temp_production = Temp_production->next;
    }
    while (temp)
    {
        terminal_all[number++] = temp->character;
        temp = temp->next;
    }
    return;
}


void Test_read(void)
{
    int number = 1;
    production* TEMP_PRODUCTION = PRODUCTION_HEAD.next;
    terminal* TEMP_TERMINAL = TERMINAL_HEAD.next;
    noterminal* TEMP_NOTERMINAL = NOTERMINAL_HEAD.next;

    printf("\n产生式\n");
    for (number = 1; TEMP_PRODUCTION != NULL; TEMP_PRODUCTION = TEMP_PRODUCTION->next, number++)
    {
        printf("%d\t%c\t%s\n", number, TEMP_PRODUCTION->source, TEMP_PRODUCTION->result);
    }
    printf("\n终结符\n");
    for (; TEMP_TERMINAL != NULL; TEMP_TERMINAL = TEMP_TERMINAL->next)
    {
        printf("%c\t", TEMP_TERMINAL->character);
    }
    printf("\n");
    printf("\n非终结符\n");
    for (; TEMP_NOTERMINAL != NULL; TEMP_NOTERMINAL = TEMP_NOTERMINAL->next)
    {
        printf("%c\t", TEMP_NOTERMINAL->character);
    }
    printf("\n读取测试\n%s\n%s\n", TEST_LIST[0], TEST_LIST[1]);
    printf("\n");
    return;
}

size_t is_appeared(char tobejudged, char* source)
{
    size_t length = strlen(source), counts = 0;
    while ((counts != length) && (*source != tobejudged))
    {
        counts++;
        source++;
    }
    return counts == length ? !success : success;
}

void combine(char* destinction, char* source)
{
    char temp[2] = { 0 };
    while (*source)
    {
        if (!is_appeared(*source, destinction))
        {
            temp[0] = *source;
            strcat(destinction, temp);
        }
        source++;
    }
    return;
}


void prediction(void)
{
    noterminal* TEMP_NOTERMINAL = NOTERMINAL_HEAD.next;
    while (TEMP_NOTERMINAL != NULL)
    {
        find_first(TEMP_NOTERMINAL, PRODUCTION_HEAD.next);
        TEMP_NOTERMINAL = TEMP_NOTERMINAL->next;
    }
    test();

    TEMP_NOTERMINAL = NOTERMINAL_HEAD.next;
    while (TEMP_NOTERMINAL != NULL)
    {
        find_follow(TEMP_NOTERMINAL, PRODUCTION_HEAD.next);
        TEMP_NOTERMINAL = TEMP_NOTERMINAL->next;
    }
    return;
}

void test_follow(void)
{
    noterminal* TEMP_NOTERMINAL = NOTERMINAL_HEAD.next;

    for (; TEMP_NOTERMINAL != NULL; TEMP_NOTERMINAL = TEMP_NOTERMINAL->next)
    {
        printf("%c\tfollow number=%d\tfollow=%s\n",
            TEMP_NOTERMINAL->character,
            TEMP_NOTERMINAL->follow_number,
            TEMP_NOTERMINAL->FOLLOW);
    }
    printf("\n");
    return;
}

/******Begin******/
//查某终结符在分析表中所处的列，找不到返回-1
static int terminal_column(char get)
{
    int number = 0;
    while (terminal_all[number] != 0)
    {
        if (terminal_all[number] == get)
            return number;
        number++;
    }
    return -1;
}

//生成预测分析表
void prediction_table(void)
{
    int count = 0, rows = 0, i = 0, j = 0, k = 0;
    int number = 0, column = 0, pointer = 0, row = -1;
    int table[20][20];
    char row_char[MAX_AMOUNT] = { 0 };
    char right_first[MAX_AMOUNT] = { 0 };
    char stack_string[MAX_AMOUNT + 4] = { 0 };
    char top = 0, input_char = 0;
    char* temp = NULL;
    char* result = NULL;
    noterminal* TEMP_NOTERMINAL = NULL;
    production* TEMP_PRODUCTION = NULL;

    for (i = 0; i < 20; i++)
        for (j = 0; j < 20; j++)
            table[i][j] = 0;

    //分析表的行对应非终结符，列对应终结符
    for (TEMP_NOTERMINAL = NOTERMINAL_HEAD.next; TEMP_NOTERMINAL != NULL; TEMP_NOTERMINAL = TEMP_NOTERMINAL->next)
        row_char[rows++] = TEMP_NOTERMINAL->character;
    count = (int)strlen(terminal_all);

    //扫描全部产生式填表
    for (i = 0; i < rows; i++)
    {
        number = 0;
        for (TEMP_PRODUCTION = PRODUCTION_HEAD.next; TEMP_PRODUCTION != NULL; TEMP_PRODUCTION = TEMP_PRODUCTION->next)
        {
            number++;
            if (TEMP_PRODUCTION->source != row_char[i])
                continue;
            memset(right_first, 0, sizeof(right_first));
            if (first_of_string(TEMP_PRODUCTION->result, right_first))   //右部可推出空串
                add_to_set(right_first, '^');
            for (temp = right_first; *temp != 0; temp++)
            {
                if (*temp == '^')
                    continue;
                column = terminal_column(*temp);
                if (column >= 0 && table[i][column] == 0)
                    table[i][column] = number;
            }
            if (is_appeared('^', right_first))  //空串则看FOLLOW集合
            {
                TEMP_NOTERMINAL = get_noterminal(row_char[i]);
                if (TEMP_NOTERMINAL != NULL)
                    for (temp = TEMP_NOTERMINAL->FOLLOW; temp != NULL && *temp != 0; temp++)
                    {
                        column = terminal_column(*temp);
                        if (column >= 0 && table[i][column] == 0)
                            table[i][column] = number;
                    }
            }
        }
    }

    //输出预测分析表
    printf("\n预测分析表\n\n");
    printf(" \t");
    for (j = 0; j < count; j++)
        printf("%c\t", terminal_all[j]);
    printf("\n");
    for (i = 0; i < rows; i++)
    {
        printf("%c\t", row_char[i]);
        for (j = 0; j < count; j++)
        {
            if (table[i][j] != 0)
                printf("%d\t", table[i][j]);
            else
                printf(" \t");
        }
        printf("\n");
    }
    printf("\n\n\n");

    //用预测分析法分析输入串
    if (TEST_LIST[0] == NULL)
        return;
    init_stack();
    STACK_PUSH(NOTERMINAL_HEAD.next->character);    //开始符号入栈
    pointer = 0;
    while (1)
    {
        //输出当前分析栈与剩余输入串
        for (k = 0; k <= amount; k++)
            stack_string[k] = TEST_STACK[k];
        stack_string[amount + 1] = 0;
        printf("分析栈\t%s\t剩余字符串\t%s#\n", stack_string, TEST_LIST[0] + pointer);

        top = TEST_STACK[amount];
        input_char = (TEST_LIST[0][pointer] == 0) ? '#' : TEST_LIST[0][pointer];

        if (top == '#' && input_char == '#')        //匹配成功
        {
            printf("\n合法输入\n");
            break;
        }
        if (get_noterminal(top) == NULL)            //栈顶是终结符
        {
            if (top == input_char)
            {
                STACK_POP();
                pointer++;
                continue;
            }
            printf("非法输入\n");
            break;
        }

        //栈顶是非终结符，查分析表
        row = -1;
        for (k = 0; k < rows; k++)
            if (row_char[k] == top)
                row = k;
        column = terminal_column(input_char);
        if (row < 0 || column < 0 || table[row][column] == 0)
        {
            printf("非法输入\n");
            break;
        }

        number = 0;
        result = NULL;
        for (TEMP_PRODUCTION = PRODUCTION_HEAD.next; TEMP_PRODUCTION != NULL; TEMP_PRODUCTION = TEMP_PRODUCTION->next)
        {
            number++;
            if (number == table[row][column])
            {
                result = TEMP_PRODUCTION->result;
                break;
            }
        }
        if (result == NULL)
        {
            printf("非法输入\n");
            break;
        }
        STACK_POP();                                //弹出栈顶非终结符
        if (strcmp(result, "^") != 0)               //右部从右往左压栈，空前部不操作
            for (k = (int)strlen(result) - 1; k >= 0; k--)
                STACK_PUSH(result[k]);
    }
    return;
}
/******End******/

void STACK_POP(void)
{
    if (STACK_EMPTY())
    {
        printf("栈空\n");
        emergency(2);
    }
    amount--;
    return;
}
size_t STACK_EMPTY()
{
    return amount == 0 ? success : !success;
}
size_t STACK_FULL()
{
    return amount == 19 ? success : !success;
}
void STACK_PUSH(char source)
{
    if (STACK_FULL())
    {
        printf("栈满\n");
        emergency(2);
    }
    TEST_STACK[++amount] = source;
    return;
}

void init_stack(void)
{
    amount = 0;
    TEST_STACK[amount] = '#';
    return;
}
