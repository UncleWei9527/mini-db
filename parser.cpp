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

minidb::SelectStatement::SelectStatement(std::string table_name,std::unique_ptr<AbstractExpression>cond)
    :SQLStatement(StatementType::SELECT),table_name_(std::move(table_name)),cond_(std::move(cond))
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

std::unique_ptr<minidb::SQLStatement> minidb::Parser::ParseStatement() {
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
    PrintAST(ast.get());
    return ast;
}

std::unique_ptr<minidb::AbstractExpression> minidb::Parser::ParseExpression() {
    auto left=ParsePrimary();
    while (IsCompareOp(Peek().type_)) {
        auto compare_op=ToCompareOp(Advance().type_);
        auto right=ParsePrimary();
        left=std::make_unique<ComparisonExpression>(left.release(),right.release(),compare_op);
    }
    return left;
}

std::unique_ptr<minidb::AbstractExpression> minidb::Parser::ParsePrimary() {
    if (Match(TokenType::IDENTIFIER)) {
        std::string id=Last().text_;
        return std::make_unique<ColumnValueExpression>(id);
    }
    else if (Match(TokenType::STRING)) {
        return std::make_unique<ConstantValueExpression>(Value(Last().text_));
    }
    else if (Match(TokenType::NUMBER)) {
        return std::make_unique<ConstantValueExpression>(Value(std::stoi(Last().text_)));
    }
    else if (Match(TokenType::KW_TRUE)) {
        return std::make_unique<ConstantValueExpression>(Value(true));

    }
    else if (Match(TokenType::KW_FALSE)) {
        return std::make_unique<ConstantValueExpression>(Value(false));
    }
    throw std::logic_error("unknown expression");
}

minidb::CompareOp minidb::Parser::ToCompareOp(TokenType tok_ty) {
    switch (tok_ty) {
        case TokenType::TK_LT:
            return CompareOp::Lt;
        case TokenType::TK_LE:
            return CompareOp::Le;
        case TokenType::TK_GT:
            return CompareOp::Gt;
        case TokenType::TK_GE:
            return CompareOp::Ge;
        case TokenType::TK_EQ:
            return CompareOp::Eq;
        case TokenType::TK_NEQ:
            return CompareOp::Ne;
        default:
            throw std::runtime_error(" token is not compare op");
    }

}

bool minidb::Parser::IsCompareOp(TokenType tok_ty) {
    switch (tok_ty) {
        case TokenType::TK_LT:
        case TokenType::TK_LE:
        case TokenType::TK_GT:
        case TokenType::TK_GE:
        case TokenType::TK_EQ:
        case TokenType::TK_NEQ:
            return true;
        default:
            return false;
    }

}

minidb::Token minidb::Parser::Last() const {
    return tokens_[cursor_-1];
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

bool minidb::Parser::Match(TokenType expected_type) {
    if (Peek().type_ == expected_type) {
        Advance();
        return true;
    }
    return false;
}

std::unique_ptr<minidb::SelectStatement> minidb::Parser::ParseSelect() {
    Consume(TokenType::KW_SELECT, "Expected 'SELECT'");
    Consume(TokenType::TK_STAR, "Currently only 'SELECT *' is supported");
    Consume(TokenType::KW_FROM, "Expected 'FROM' after '*'");
    Token table_name_token = Consume(TokenType::IDENTIFIER, "Expected table name after 'FROM'");
    if (Match(TokenType::KW_WHERE)) {
        return std::make_unique<SelectStatement>(table_name_token.text_,ParseExpression());
    }
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
#include<format>
void minidb::PrintAST(SQLStatement *ast, int indent)
{
    printf("======语法树节点======\n");
    std::string prefix(indent, ' ');
    if (ast->GetType() == StatementType::SELECT) {
        auto sel = static_cast<SelectStatement*>(ast);
        if (sel->cond_) {
            std::cout<<std::format("{}[SelectStatement] 扫描表: {}\n",prefix,sel->table_name_);
            std::cout<<std::format("{}  └── 筛选条件节点:{}\n",prefix,sel->cond_->ToString());
        }
        else
        std::cout<<std::format("{}[SelectStatement] 扫描表: {}\n",prefix,sel->table_name_);

    } else if (ast->GetType() == StatementType::INSERT) {
        auto ins = static_cast<InsertStatement*>(ast);
        std::cout<<std::format("{}[InsertStatement] 写入目标表: {}\n",prefix,ins->table_name_);


        if (ins->has_values_) {
            std::string data_str="VALUES(";
            for (auto  &value:ins->raw_values_) {
                data_str+=value.ToString();
                data_str+=",";
            }
            data_str+=")";
            std::cout<<std::format("{}  └── 数据来源子节点:{}\n",prefix,data_str);
        }
        else {
            std::cout<<std::format("{}  └── 数据来源子节点:\n",prefix);
            PrintAST(ins->select_query_.get(), indent + 6);
        }

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
