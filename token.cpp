//
// Created by wjh on 2026/6/15.
//
#include"token.h"
#include <cctype>
#include <stdexcept>
#include <algorithm>
#include<format>
#include<cmath>
#include<cassert>
std::string minidb::Token::ToString() const {
    std::string type_str;
    switch (type_) {
        case TokenType::KW_SELECT: type_str ="KW_SELECT";break;
        case TokenType::KW_FROM: type_str = "KW_FROM";break;
        case TokenType::KW_INSERT: type_str = "KW_INSERT";break;
        case TokenType::KW_INTO: type_str = "KW_INTO";break;
        case TokenType::TK_STAR: type_str = "TK_STAR";break; // *
        case TokenType::TK_SEMI: type_str = "TK_SEMI";break; // ;
        case TokenType::IDENTIFIER: type_str = "IDENTIFIER";break; // 表名或列名，比如 users
        case TokenType::END_OF_FILE: type_str = "END_OF_FILE";break; // 解析结束
        default: assert(false);
    }
    return std::format("[{}]->{}",type_str,text_);
}

minidb::Tokenizer::Tokenizer(std::string sql)
    :sql_(std::move(sql))
{
}

std::vector<minidb::Token> minidb::Tokenizer::Tokenize() {
    cursor_=0;
    std::vector<Token>tokens;
    while (!IsAtEnd()) {
        SkipWhitespace();
        if (IsAtEnd()) break;

        char c = Peek();

        if (std::isalpha(c) || c == '_') {
            tokens.push_back(ConsumeKeywordOrIdentifier());
        }
        else if (std::isdigit(c)) {
            tokens.push_back(ConsumeNumber());
        }
        else if (c=='\'') {
            tokens.push_back(ConsumeString());
        }
        else tokens.push_back(ConsumeSymbol());
    }
    return tokens;
}

void minidb::Tokenizer::SkipWhitespace() {
    while (std::isspace(Peek())) {
        Advance();
    }
}

minidb::Token minidb::Tokenizer::ConsumeKeywordOrIdentifier() {
    std::string text;
    while (std::isalnum(Peek())||Peek()=='_') {
        text+=Advance();
    }
    std::string upper_text=text;
    std::transform(upper_text.begin(), upper_text.end(), upper_text.begin(), ::toupper);
    if (upper_text == "SELECT") return {TokenType::KW_SELECT, upper_text};
    if (upper_text == "FROM") return {TokenType::KW_FROM, upper_text};
    if (upper_text == "INSERT") return {TokenType::KW_INSERT, upper_text};
    if (upper_text == "INTO") return {TokenType::KW_INTO, upper_text};
    if (upper_text == "CREATE") return {TokenType::KW_CREATE, upper_text};
    if (upper_text == "TABLE") return {TokenType::KW_TABLE, upper_text};
    if (upper_text == "INT" || upper_text == "INTEGER") return {TokenType::KW_INT, upper_text};
    if (upper_text == "VARCHAR") return {TokenType::KW_VARCHAR, upper_text};
    if (upper_text=="BOOL"||upper_text=="BOOLEAN")return {TokenType::KW_BOOL,upper_text};
    if (upper_text=="TRUE")return {TokenType::KW_TRUE,upper_text};
    if (upper_text=="FALSE")return {TokenType::KW_FALSE,upper_text};
    if (upper_text=="VALUES")return {TokenType::KW_VALUES,upper_text};
    if (upper_text=="WHERE")return {TokenType::KW_WHERE,upper_text};
    return {TokenType::IDENTIFIER, text};
}

minidb::Token minidb::Tokenizer::ConsumeSymbol() {
    char c=Advance();
    switch (c) {
        case '*':return {TokenType::TK_STAR,"*"};
        case ';':return {TokenType::TK_SEMI,";"};
        case '(':return {TokenType::TK_LPAREN,"("};
        case ')':return {TokenType::TK_RPAREN,")"};
        case ',':return {TokenType::TK_COMMA,","};
        case '<': {
            if (Peek()=='=') {
                Advance();
                return {TokenType::TK_LE, "<="};
            }
            return {TokenType::TK_LT, "<"};

        }
        case '>': {
            if (Peek()=='=') {
                Advance();
                return {TokenType::TK_GE, ">="};
            }
            return {TokenType::TK_GT, ">"};
        }

        case '!': {
            if (Advance()=='=')
            return {TokenType::TK_NEQ, "!="};
            break;
        }
        case '=': {
            return {TokenType::TK_EQ,"="};
            break;
        }
    }
    throw std::runtime_error(std::string("Lexer error: Unexpected character '") + c + "'");
}

minidb::Token minidb::Tokenizer::ConsumeNumber() {
    std::string text;
    while (std::isdigit(Peek())) {
        text+=Advance();
    }
    return Token{TokenType::NUMBER,text};
}

minidb::Token minidb::Tokenizer::ConsumeString() {
    Advance();
    std::string text;
    while (Peek()!='\''&&!IsAtEnd()) {
        text+=Advance();
    }
    Advance();
    return Token{TokenType::STRING,text};
}

char minidb::Tokenizer::Peek() const {
    if (IsAtEnd()) return '\0';
    return sql_[cursor_];
}

char minidb::Tokenizer::Advance()  {
    if (IsAtEnd())return '\0';
    return sql_[cursor_ ++];
}

bool minidb::Tokenizer::IsAtEnd() const {
    return cursor_>=sql_.length();
}
