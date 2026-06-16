//
// Created by wjh on 2026/6/15.
//
#include"parser.h"
#include<map>
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
,select_query_(std::move(select_query)),has_values_(false)
{
}

minidb::InsertStatement::InsertStatement(std::string table_name, std::vector<Value> raw_values)
:SQLStatement(StatementType::INSERT),table_name_(std::move(table_name))
,select_query_(nullptr),raw_values_(std::move(raw_values)),has_values_(true)
{
}

minidb::CreateTableStatement::CreateTableStatement(std::string table_name, std::vector<Column> columns)
    :SQLStatement(StatementType::CREATE_TABLE),table_name_(table_name),columns_(std::move(columns))
{

}

std::unique_ptr<minidb::SQLStatement> minidb::Parser::Parse() {
    TokenType first_type = Peek().type_;
    std::unique_ptr<SQLStatement> ast = nullptr;

    if (first_type == TokenType::KW_SELECT) {
        ast = ParseSelect();
    } else if (first_type == TokenType::KW_INSERT) {
        ast = ParseInsert();
    }else if (first_type==TokenType::KW_CREATE) {
        ast=ParseCreateTable();
    }
    else {
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
    Consume(TokenType::KW_INSERT, "Expected 'INSERT'");
    Consume(TokenType::KW_INTO, "Expected 'INTO' after 'INSERT'");
    std::string  table_name =
        Consume(TokenType::IDENTIFIER, "Expected target table name after 'INTO'").text_;
    if (Peek().type_ == TokenType::KW_SELECT) {
        std::unique_ptr<SelectStatement> select_query = ParseSelect();
        return std::make_unique<InsertStatement>(table_name,std::move(select_query));
    }
    else if (Peek().type_==TokenType::KW_VALUES) {
        std::vector<Value>values;
        Consume(TokenType::KW_VALUES, "Expected 'VALUES'");
        Consume(TokenType::TK_LPAREN, "Expected '(' after VALUES");
        while (true) {
            if (Peek().type_ == TokenType::NUMBER) {
                values.emplace_back(std::stoi(Advance().text_));
            } else if (Peek().type_ == TokenType::STRING) {
                values.emplace_back(Advance().text_);
            }
            else if (Peek().type_==TokenType::KW_TRUE) {
                values.emplace_back(true);
                Advance();
            }
            else if (Peek().type_==TokenType::KW_FALSE) {
                values.emplace_back(false);
                Advance();
            }
            else {
                throw std::runtime_error("Syntax Error: Only numbers and strings are supported in VALUES");
            }

            if (Peek().type_ == TokenType::TK_COMMA) {
                Advance(); // 吃掉逗号继续
            }
            if (Peek().type_ == TokenType::TK_RPAREN) {
                break;     // 遇到右括号结束
            }
        }
        Consume(TokenType::TK_RPAREN, "Expected ')'");
        return std::make_unique<InsertStatement>(table_name,values);
    }
    throw std::runtime_error("Syntax Error: INSERT expects SELECT or VALUES");
}

std::unique_ptr<minidb::CreateTableStatement> minidb::Parser::ParseCreateTable() {
    Consume(TokenType::KW_CREATE, "Expected 'CREATE'");
    Consume(TokenType::KW_TABLE, "Expected 'TABLE' after 'CREATE'");
    std::string table_name=Consume(TokenType::IDENTIFIER, "Expected table name").text_;
    Consume(TokenType::TK_LPAREN, "Expected '(' after table name");
    std::vector<Column>columns;
    while (true) {
        // 解析列名
        std::string col_name = Consume(TokenType::IDENTIFIER, "Expected column name").text_;

        // 解析列类型
        TypeId col_type;
        Token type_token = Advance();
        if (type_token.type_ == TokenType::KW_INT) {
            col_type = TypeId::INTEGER;
        } else if (type_token.type_ == TokenType::KW_VARCHAR) {
            col_type = TypeId::VARCHAR;
        }else if (type_token.type_==TokenType::KW_BOOL) {
            col_type=TypeId::BOOLEAN;
        }
        else {
            throw std::runtime_error("Syntax Error: Unsupported column type '" + type_token.text_ + "'");
        }

        // 保存列定义
        columns.emplace_back(col_name, col_type);

        // 判断是否还有下一列
        if (Peek().type_ == TokenType::TK_COMMA) {
            Advance(); // 吃掉逗号，继续循环
        } else if (Peek().type_ == TokenType::TK_RPAREN) {
            break;     // 遇到右括号，列定义结束
        } else {
            throw std::runtime_error("Syntax Error: Expected ',' or ')' in column definition");
        }
    }
    Consume(TokenType::TK_RPAREN, "Expected ')' at the end of column definition");
    return std::make_unique<CreateTableStatement>(table_name,columns);
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
    else if (ast->GetType()==StatementType::CREATE_TABLE) {
        static std::map<TypeId,std::string>type_strs={
            {TypeId::INTEGER,"INT"},
            {TypeId::BOOLEAN,"BOOL"},
            {TypeId::VARCHAR,"VARCHAR"}
        };
        auto create_stmt = static_cast<CreateTableStatement*>(ast);
        std::cout << prefix << "[CreateTableStatement] 表名: " << create_stmt->table_name_ << "\n";
        std::cout << prefix << "  └── 列定义:\n";

        for (const auto& col : create_stmt->columns_) {
            std::string type_str = type_strs[col.column_type_];
            std::cout << prefix << "      - " << col.column_name_ << " [" << type_str << "]\n";
        }
    }
}
