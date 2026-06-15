//
// Created by wjh on 2026/6/15.
//
#include"Parser.h"

minidb::SQLStatement::SQLStatement(StatementType type)
    :type_(type)
{
}

minidb::StatementType minidb::SQLStatement::GetType() const {
    return type_;
}

minidb::SelectStatement::SelectStatement(std::string table_name)
    :SQLStatement(StatementType::SELECT),table_name_(std::move(table_name))
{
}

minidb::InsertStatement::InsertStatement(
    std::string table_name, std::unique_ptr<SelectStatement> select_query)
:SQLStatement(StatementType::INSERT),table_name_(std::move(table_name))
,select_query_(std::move(select_query))
{
}

std::unique_ptr<minidb::SQLStatement> minidb::Parser::Parse() {
    TokenType first_type = Peek().type_;
    std::unique_ptr<SQLStatement> ast = nullptr;

    if (first_type == TokenType::KW_SELECT) {
        ast = ParseSelect();
    } else if (first_type == TokenType::KW_INSERT) {
        ast = ParseInsert();
    } else {
        throw std::runtime_error("Syntax Error: Unsupported statement starting with '" + Peek().text_ + "'");
    }
    if (Peek().type_ == TokenType::TK_SEMI) {
        Advance();
    }
    if (!IsAtEnd()) {
        throw std::runtime_error("Syntax Error: Unexpected tokens at the end of the statement.");
    }

    return ast;
}

minidb::Token minidb::Parser::Peek() const {
    if (IsAtEnd()) return tokens_.back(); // 返回最后的 EOF
    return tokens_[cursor_];
}

minidb::Token minidb::Parser::Advance() {
    if (!IsAtEnd()) cursor_++;
    return tokens_[cursor_ - 1];
}

bool minidb::Parser::IsAtEnd() const {
    return cursor_ >= tokens_.size() || tokens_[cursor_].type_ == TokenType::END_OF_FILE;
}

minidb::Token minidb::Parser::Consume(TokenType expected_type, const std::string &error_message) {
    if (Peek().type_ == expected_type) {
        return Advance();
    }
    throw std::runtime_error("Syntax Error: " + error_message + " (Got: '" + Peek().text_ + "')");
}

std::unique_ptr<minidb::SelectStatement> minidb::Parser::ParseSelect() {
    Consume(TokenType::KW_SELECT, "Expected 'SELECT'");
    Consume(TokenType::TK_STAR, "Currently only 'SELECT *' is supported");
    Consume(TokenType::KW_FROM, "Expected 'FROM' after '*'");
    Token table_name_token = Consume(TokenType::IDENTIFIER, "Expected table name after 'FROM'");

    return std::make_unique<SelectStatement>(table_name_token.text_);
}

std::unique_ptr<minidb::InsertStatement> minidb::Parser::ParseInsert() {
    TokenType first_type = Peek().type_;
    std::unique_ptr<SQLStatement> ast = nullptr;
    Consume(TokenType::KW_INSERT, "Expected 'INSERT'");
    Consume(TokenType::KW_INTO, "Expected 'INTO' after 'INSERT'");
    Token table_name_token = Consume(TokenType::IDENTIFIER, "Expected target table name after 'INTO'");
    std::unique_ptr<SelectStatement>select_query=nullptr;
    if (Peek().type_ == TokenType::KW_SELECT) {
        select_query = ParseSelect();
    } else {
        throw std::runtime_error("Syntax Error: Currently INSERT only supports 'INSERT INTO ... SELECT ...'");
    }

    return std::make_unique<InsertStatement>(table_name_token.text_,std::move(select_query));
}

void minidb::PrintAST(SQLStatement *ast, int indent)
{
    std::string prefix(indent, ' ');
    if (ast->GetType() == StatementType::SELECT) {
        auto sel = static_cast<SelectStatement*>(ast);
        std::cout << prefix << "[SelectStatement] 扫描表: " << sel->table_name_ << "\n";
    } else if (ast->GetType() == StatementType::INSERT) {
        auto ins = static_cast<InsertStatement*>(ast);
        std::cout << prefix << "[InsertStatement] 写入目标表: " << ins->table_name_ << "\n";
        std::cout << prefix << "  └── 数据来源子节点:\n";
        PrintAST(ins->select_query_.get(), indent + 6);
    }
}