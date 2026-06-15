//
// Created by wjh on 2026/6/15.
//
#include"token.h"
#include <cctype>
#include <stdexcept>
#include <algorithm>
#include<format>
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
    return {TokenType::IDENTIFIER, text};
}

minidb::Token minidb::Tokenizer::ConsumeSymbol() {
    char c=Advance();
    switch (c) {
        case '*':return {TokenType::TK_STAR,"*"};
        case ';':return {TokenType::TK_SEMI,";"};
    }
    throw std::runtime_error(std::string("Lexer error: Unexpected character '") + c + "'");
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
