#include <iostream>
#include <string>
#include <iomanip>
#include <stack>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
using namespace std;
//bool debug = true;
bool debug = false;

typedef void(* func_type)();
func_type func_arr[4096 * 1024];     // 工作区，用于存储词的执行代码等
func_type* here = func_arr;          // here指针，指向当前空白的工作区单元
func_type* pc;                       // 程序计数器（Program Counter）
func_type* lr;                       // 链接寄存器（Link Register）


class Word
{
public:
    Word(string name, func_type* func, bool imme = false):
        name_(name),
        func_(func),
        is_immediate_(imme)
    {
    }
    string name_;               // 词的名字
    func_type* func_;           // 词的执行区域
    bool is_immediate_;         // 是否是立即词
};

std::stack<void *> data_stack;  // 数据栈（又称参数栈）
std::stack<void *> call_stack;  // 调用栈（又称返回栈）
std::vector<Word> dict;         // 词典
bool is_compiling = false;      // 是否处于编译态

// **************************************************************
// 以下部分是一些工具
// **************************************************************

// 输出调试信息
void print_info(string func_name)
{
    if (debug) {
        cout << setw(20) << left << func_name;
        cout << ": pc: " << setw(8) << (void *)(pc - func_arr);
        cout << ", lr: " << setw(8) << (void *)(lr - func_arr);
        cout << ", hr: " << setw(8) << (void *)(here - func_arr)  << endl;
    }
}

// 在C语言中定义词的工具
void def_word(string name, void (*func)(), bool imme = false)
{
    *here = func;
    dict.push_back(Word(name, here, imme));
    here++;
}

// 判断是否是数
bool is_number(const string &str)
{
    for (auto c: str) {
        if (c < '0' || c > '9') return false;
    }
    return true;
}

// **************************************************************
// 以下部分是虚拟机相关的代码
// **************************************************************

// 虚拟机的跳转并链接
void jump_and_link()
{
    print_info(__FUNCTION__);
    lr = pc + 1;
    pc = (func_type*)(*pc);
}

// 虚拟机的返回
void ret()
{
    pc = lr;
}

// 虚拟机主循环
void vm_loop()
{
    while(true) {
        print_info(__FUNCTION__);
        (*pc++)();
    }
}

// **************************************************************
// 以下部分是定义的词
// **************************************************************

// + (a b -- a+b )
void plus_func()
{
    print_info(__FUNCTION__);
    long int a = (long int)(data_stack.top());
    data_stack.pop();
    long int b = (long int)(data_stack.top());
    data_stack.pop();
    data_stack.push((void *)(long int)(a + b));
    ret();
}

// - (a b -- a-b )
void minus_func()
{
    print_info(__FUNCTION__);
    long int a = (long int)(data_stack.top());
    data_stack.pop();
    long int b = (long int)(data_stack.top());
    data_stack.pop();
    data_stack.push((void *)(long int)(b - a));
    ret();
}

// 弹出并打印栈顶 (a -- )
void dot_func()
{
    print_info(__FUNCTION__);
    long int a = (long int)data_stack.top();
    data_stack.pop();
    cout << dec << a << endl;
    ret();
}

// 执行时的:
void do_colon()
{
    print_info(__FUNCTION__);
    call_stack.push((void *)lr);
}

// 编译时的:
void colon_func()
{
    is_compiling = true;
    string new_word_name;
    cin >> new_word_name;
    dict.push_back(Word(new_word_name, here));
    *here++ = do_colon;
    ret();
}

// 执行时的;
void do_semicolon()
{
    print_info(__FUNCTION__);
    lr = (func_type *)call_stack.top();
    call_stack.pop();
    ret();
}

// 执行时的;
void semicolon_func()
{
    is_compiling = false;
    *here++ = do_semicolon;
    ret();
}

// 执行时的literal（读取数）
void do_lit()
{
    print_info(__FUNCTION__);
    data_stack.push((void *)(long int)(*pc));
    pc++;
}

// **************************************************************
// 以下部分是编译、解释器
// **************************************************************

// 编译、解释器
void interpreter()
{
    string input_word;
    bool is_word_found = false;
    Word* word;
    cin >> input_word;
    for (auto &w: dict){
        if (input_word.compare(w.name_) == 0) {
            is_word_found = true;
            word = &w;
        }
    }
    lr = func_arr;
    if (is_compiling && (is_word_found && !(word->is_immediate_))) {
        if (debug) {
            cout << "compiling: " << input_word << endl;
        }
        *here++ = jump_and_link;
        *here++ = (func_type)(word->func_);
    } else if (is_compiling && !is_word_found) {
        if (is_number(input_word)) {
            *here++ = do_lit;
            *here++ = (func_type)(long int)atoi(input_word.c_str());
        } else {
            cout << input_word << " not found." << endl;
        }
    } else {
        if (debug) {
            cout << "executing: " << input_word << endl;
        }
        if (is_word_found) {
            pc = word->func_;
            return;
        } else {
            if (is_number(input_word)) {
                data_stack.push((void *)(long int)atoi(input_word.c_str()));
            } else {
                cout << input_word << " not found." << endl;
            }
        }
    }
    lr = func_arr;
    ret();
}

// 不一定会到这里，但到这里的话，就回到interpreter
void loop_interpreter()
{
    pc = func_arr;
}

// **************************************************************
// 以下部分是初始化和程序入口
// **************************************************************

// 初始化
void setup()
{
    here = func_arr;
    *here++ = interpreter;
    *here++ = loop_interpreter;
    pc = func_arr;
    lr = func_arr;
}

int main()
{
    setup();
    def_word("+", plus_func);
    def_word("-", minus_func);
    def_word(".", dot_func);
    def_word(":", colon_func);
    def_word(";", semicolon_func, true);
    vm_loop();
}
