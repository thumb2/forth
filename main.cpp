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
func_type func_arr[4096 * 1024];     //
func_type* here = func_arr;          // HERE in forth language
func_type* pc;                       // Program Counter
func_type* nx;                       // Next PC
func_type* lr;                       // Link Register


class Word
{
public:
    Word(string name, func_type* func, bool imme = false):
        name_(name),
        func_(func),
        is_immediate_(imme)
    {
    }
    string name_;
    func_type* func_;
    bool is_immediate_;
};

std::stack<void *> data_stack;
std::stack<void *> call_stack;
std::vector<Word> dict;
string input_word;

bool is_compiling = false;
void ret()
{
    pc = lr;
    nx = lr;
}

void print_info(string func_name)
{
    cout << setw(20) << left << func_name;
    cout << ": pc: " << setw(8) << (void *)(pc - func_arr);
    cout << ", nx: " << setw(8) << (void *)(nx - func_arr);
    cout << ", lr: " << setw(8) << (void *)(lr - func_arr);
    cout << ", hr: " << setw(8) << (void *)(here - func_arr)  << endl;
}

void plus_func()
{
    if (debug) print_info("do_plus");

    int a = (long int)(data_stack.top());
    data_stack.pop();
    int b = (long int)(data_stack.top());
    data_stack.pop();
    data_stack.push((void *)(long int)(a + b));
    ret();
}
void dot_func()
{
    if (debug) print_info("dot_func");
    int a = (long int)data_stack.top();
    data_stack.pop();
    cout << a << endl;
    ret();
}

bool is_number(const string &str)
{
    for (auto c: str) {
        if (c < '0' || c > '9') return false;
    }
    return true;
}


void interpreter();

void jump_and_link()
{
    lr = pc + 1;
    pc = nx;
}


void do_colon()
{
    if (debug) print_info("do_colon");
    call_stack.push((void *)lr);
    pc++;
    nx = pc;
}

void colon_func()
{
    is_compiling = true;
    string new_word_name;
    cin >> new_word_name;
    dict.push_back(Word(new_word_name, here));
    *here++ = do_colon;
    ret();
}
void do_semicolon()
{
    if (debug) print_info("do_semicolon");
    lr = (func_type *)call_stack.top();
    call_stack.pop();
    ret();
}

void semicolon_func()
{
    is_compiling = false;
    *here++ = do_semicolon;
    ret();
}

void interpreter()
{
    bool is_word_found = false;
    Word* word;

    cin >> input_word;
    for (auto &w: dict){
        //cout << input_word << " vs " << w.name_ << endl;
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
        *here++ = *(word->func_);
    } else {
        if (debug) {
            cout << "executing: " << input_word << endl;
        }
        if (is_word_found) {
            nx = word->func_;
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

void loop_interpreter()
{
    nx = func_arr;
}

void setup()
{
    here = func_arr;
    *here++ = interpreter;
    *here++ = loop_interpreter;
    nx = func_arr;
    pc = func_arr;
    lr = func_arr;
}

void main_loop()
{
    while(true) {
        if (debug) print_info("before jump and link");
        jump_and_link();
        if (debug) print_info("after  jump and link");
        (*pc)();
    }
}

void def_word(string name, void (*func)(), bool imme = false)
{
    *here = func;
    dict.push_back(Word(name, here, imme));
    here++;
}

int main()
{
    setup();
    def_word("+", plus_func);
    def_word(".", dot_func);
    def_word(":", colon_func);
    def_word(";", semicolon_func, true);
    main_loop();
}
