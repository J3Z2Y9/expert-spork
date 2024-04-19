#include "tokens.h"
#include<bits/stdc++.h>

extern int yylex();
extern char* yytext;
extern FILE* yyin;
int lookahead;

//建立全局符号表
struct SymbolValue {
  //  enum { INT, LONG } type;  // 支持的类型

    int VarSpace;
    // 初始化为 int
    SymbolValue(int varspace=0) : VarSpace(varspace) {}


    
};


class SymbolTable {
public:
    int varNum;
    int space;
    SymbolTable():varNum(0),space(0){}
    void addSymbol(const std::string& name, SymbolValue initialValue) {
        symbolTable[name] = initialValue ;
    }

    bool isDefined(const std::string& name) {
        return symbolTable.find(name) != symbolTable.end();
    }

    // 更新变量的值
    void setSymbolValue(const std::string& name, SymbolValue value) {
        if (isDefined(name)) {
            symbolTable[name] = value;
        }
        else {
            std::cerr << "Variable " << name << " not defined." << std::endl;
        }
    }

    // 获取变量的值
    SymbolValue getSymbolValue(const std::string& name) {
        if (isDefined(name)) {
            return symbolTable[name];
        }
        throw std::runtime_error("Variable not defined");
    }

private:
    std::map<std::string, SymbolValue> symbolTable;
    
};

SymbolTable globalSymbolTable;

void statement();


void match(int expected) {
    if (lookahead == expected) {
        lookahead = yylex();
    }
    else {
        std::cerr << "Syntax error at function match when lookahead="<<lookahead;
        std::cerr << std::endl << "yytext=" << yytext<<" "<<"expected="<<expected;
        exit(1);
    }
}
//想要匹配所有运算符
bool is_operator(int num) {
    switch (num) {
    case TOK_AND:
    case TOK_OR:
    case TOK_XOR:
    case TOK_PLUS:
    case TOK_MINUS:
    case TOK_MULTIPLY:
    case TOK_DIVIDE:
    case TOK_MOD:
        return true;
        break;
    default:
        return false;
    }
}
std::string tok2operator(int TOK) {
    std::string s;
    switch (TOK)
    {
    case TOK_PLUS:
        s = "+";
        break;
    case TOK_MINUS:
        s = '-';
        break;
    case TOK_MULTIPLY:
        s = '*';
        break;
    case TOK_DIVIDE:
        s = "/";
        break;
    case TOK_AND:
        s = '&';
        break;
    case TOK_OR:
        s = '|';
        break;
    case TOK_XOR:
        s = '^';
        break;
    default:
        std::cerr << "unknown operator";
        exit(1);
        break;
    }
    return s;
}
int precedence(int op) {
    if (op == TOK_MULTIPLY || op == TOK_DIVIDE || op == TOK_MOD) {
        return 3;
    }
    if (op == TOK_PLUS || op == TOK_MINUS) {
        return 4;
    }
    if (op == TOK_AND)
        return 8;
    if (op == TOK_XOR)
        return 9;
    if (op == TOK_OR)
        return 10;
}
void evaluatePostfixExpression(std::string varname, const std::vector<std::string>& postfixTokens) {
    int varIndex = globalSymbolTable.getSymbolValue(varname).VarSpace;
    for (const auto& token : postfixTokens) {
        if (token == "+" || token == "-" || token == "*" || token == "/" || token=="&" ||token=="|"||token=="^") {
            std::cout << "pop ebx" << std::endl;
            std::cout << "pop eax" << std::endl;
            if (token == "+") std::cout << "add eax, ebx" << std::endl;
            else if (token == "-") std::cout << "sub eax, ebx" << std::endl;
            else if (token == "*") std::cout << "imul eax, ebx" << std::endl;
            else if (token == "/") {
                std::cout << "cdq" << std::endl;
                std::cout << "idiv ebx" << std::endl;
            }
            else if (token == "&")std::cout << " and eax, ebx" << std::endl;
            else if (token == "|" )std::cout << "or eax,ebx" << std::endl;
            else if (token == "^")std::cout << "xor eax,ebx" << std::endl;
            else if (token == "%") {
                std::cout << "cdq" << std::endl;
                std::cout << "idiv ebx" << std::endl;
                std::cout << "mov eax,edx" << std::endl;
            }
            std::cout << "push eax" << std::endl;
        }
        else if (std::isalpha(token[0])) {
            auto varValue = globalSymbolTable.getSymbolValue(token);
                
            std::cout << "mov eax, [ebp-" << varValue.VarSpace << "]" << std::endl;
            std::cout << "push eax" << std::endl;
        }
        else if (std::isdigit(token[0])) {
            std::cout << "mov eax, " << token << std::endl;
            std::cout << "push eax" << std::endl;

        }
    }
    std::cout << "pop eax" << std::endl;
    std::cout << "mov [ebp-" << varIndex << "], eax" << std::endl;
}
void expression(std::string varname) {
    std::vector<std::string> postfixTokens;
    std::stack<int>operatorStack;
    for (; lookahead != TOK_SEMICOLON ; lookahead=yylex() ) {
        if (lookahead == TOK_INTEGER) {
            postfixTokens.push_back(yytext);
        }
        else if (lookahead == TOK_IDENTIFIER) {
            if (!globalSymbolTable.isDefined(yytext)) {
                std::cerr << "The Symbol is not defined";
                exit(1);
            }
            else postfixTokens.push_back(yytext);
        }
        
        else if (lookahead == TOK_LPAREN) {
            operatorStack.push(lookahead);
        }
        else if (lookahead==TOK_RPAREN) {
            while(!operatorStack.empty()&&operatorStack.top()!=TOK_LPAREN){
                postfixTokens.push_back(tok2operator(operatorStack.top()));
                operatorStack.pop();
            }
            operatorStack.pop();
        }

        else if (is_operator(lookahead)) {
            while (!operatorStack.empty() && precedence(operatorStack.top()) <= precedence(lookahead)) {
                postfixTokens.push_back(tok2operator(operatorStack.top()));
                operatorStack.pop();
            }
            operatorStack.push(lookahead);
        }

    }
    while (!operatorStack.empty()) {
        postfixTokens.push_back(tok2operator(operatorStack.top()));
        operatorStack.pop();
    }
    evaluatePostfixExpression(varname, postfixTokens);
}

void declaration_int(SymbolValue initValue) {
    globalSymbolTable.varNum += 1;
    //!!!注意，这里的4写死了，以后如果要添加long double等不同大小的变量类型，需要重构
    globalSymbolTable.space += 4;
    initValue = SymbolValue(globalSymbolTable.space);
    std::string identifier_name = yytext; // 获取标识符名称
    if (!globalSymbolTable.isDefined(identifier_name)) {
        globalSymbolTable.addSymbol(identifier_name, initValue); // 添加到符号表
        
        match(TOK_IDENTIFIER);
    }
    else {
        std::cerr << "Semantic error: Variable '" << identifier_name << "' is already defined\n";
        exit(1);
    }
}
//定义变量
void declaration() {
    SymbolValue initValue(0);

     if (lookahead == TOK_INT) {
        match(TOK_INT);
        
        

        if (lookahead == TOK_IDENTIFIER) {

            declaration_int(initValue);
        }
        else if (lookahead == TOK_MAIN) {
            match(TOK_MAIN);
            std::cout << ".intel_syntax noprefix" << std::endl;
            std::cout << ".global main" << std::endl;
            std::cout << ".extern printf" << std::endl;
            std::cout << ".data" << std::endl;
            std::cout << "format_str:" << std::endl;
            std::cout << ".asciz \"%d\\n\"" << std::endl;
            std::cout << ".text " << std::endl;
            std::cout << "main:" << std::endl;
            std::cout << " push ebp" << std::endl;
            std::cout << "mov ebp, esp" << std::endl;
            std::cout << "sub esp, 0x100" << std::endl;
            match(TOK_LPAREN);
            //注意这里是int argc,int argv
            if (lookahead == TOK_INT) {
                match(TOK_INT);
                initValue = SymbolValue(globalSymbolTable.space); // 默认初始化为0
                
                declaration_int(initValue);
                match(TOK_COMMA);

                match(TOK_INT);
                initValue = SymbolValue(globalSymbolTable.space); // 默认初始化为0
                declaration_int(initValue);

            }

            match(TOK_RPAREN);
            match(TOK_LBRACE);
            statement();
            match(TOK_RBRACE);
            if (lookahead == 0)
                return;
        }
        
        else {
            std::cerr << "unknown int;";
        }

    }
    /*else if (lookahead == TOK_LONG) {
        type = "long";
       initValue = SymbolValue(0L); // 默认初始化为0L
        match(TOK_LONG);
    }*/
    else {
        std::cerr << "Syntax error, expected 'int' or 'long'\n";
        exit(1);
    }

    

    if (lookahead == TOK_ASSIGN) {
        match(TOK_ASSIGN);
        expression(yytext); // 假设现在可以返回正确的SymbolValue

    }

    match(TOK_SEMICOLON);

}

void statement() {
    
    std::string tmp;
    if (strcmp(yytext , "println_int")==0) {
        match(TOK_IDENTIFIER);
        match(TOK_LPAREN);
        std::cout << " push DWORD PTR [ebp-" << globalSymbolTable.getSymbolValue(yytext).VarSpace << "]" << std::endl;
        std::cout << "push offset format_str" << std::endl;
        std::cout << "call printf" << std::endl;
        std::cout << " add esp, 8" << std::endl;
        match(TOK_IDENTIFIER);
        match(TOK_RPAREN);
        match(TOK_SEMICOLON);
    }
    else if (lookahead == TOK_INT) {
        declaration();  // 处理声明语句
    }
    else if (lookahead == TOK_IDENTIFIER) {
        tmp = yytext;  // 假设yytext是当前标识符的文本
        match(TOK_IDENTIFIER);
        match(TOK_ASSIGN);
        expression(tmp);  // 假设expression()现在接受一个字符串参数
        match(TOK_SEMICOLON);
    }
    else if (lookahead == TOK_RETURN) {
        match(TOK_RETURN);
        std::cout << "mov eax," << yytext<<std::endl;
        std::cout << "leave"  << std::endl;
        std::cout << "ret"  << std::endl;
        match(TOK_INTEGER);
        match(TOK_SEMICOLON);
    }
    else if (lookahead == TOK_SEMICOLON) {
        match(TOK_SEMICOLON);
    }
    else {
        std::cerr << "Invalid statement\n";
    }

    if (lookahead != TOK_RBRACE && lookahead != 0) {
        statement();  // 递归调用处理下一个语句
    }

}
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_code_file_path>" << std::endl;
        return 1;
    }
    yyin = fopen(argv[1], "r");
    //std::string filePath = argv[1];
    //int token;
    //
    //while ((token = yylex()) != 0) {
    //    printf("Token: %d, Text: %s\n", token, yytext);
    //}
    lookahead = yylex();
    while (lookahead != 0) {
        statement();
    }
    return 0;
}
