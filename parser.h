#pragma  once
#include<vector>
#include"token.h"
#include"catalog.h"
#include<memory>
#include<iostream>
#include"expression.h"
namespace minidb {
    enum class StatementType {
        SELECT,
        INSERT,
        DELETE,
        UPDATE,
        CREATE_TABLE,

    };
    struct SQLStatement {
        SQLStatement(StatementType type);
        StatementType GetType()const;
        virtual ~SQLStatement()=default;
    private:
        StatementType type_;
    };
    struct SelectStatement:public SQLStatement {
        SelectStatement(std::string table_name,std::unique_ptr<AbstractExpression>cond=nullptr);
        std::unique_ptr<AbstractExpression>cond_;
        std::string table_name_;
    };
    struct InsertStatement:public SQLStatement {
        InsertStatement(std::string table_name,std::unique_ptr<SelectStatement> select_query);
        // INSERT INTO TABLE SELECT ...
        InsertStatement(std::string table_name,std::vector<Value>raw_values);
        std::string table_name_;
        std::unique_ptr<SelectStatement>select_query_;
        //INSERT INTO TABLE VALUES(value1,value2...)
        bool has_values_;
        std::vector<Value>raw_values_;
    };
    struct CreateTableStatement:public SQLStatement {
        CreateTableStatement(std::string table_name,std::vector<Column>columns);
        std::string table_name_;
        std::vector<Column>columns_;
    };
    struct DeleteStatement:public SQLStatement {
        DeleteStatement(std::string table_name,std::unique_ptr<AbstractExpression>cond);
        std::string table_name_;
        std::unique_ptr<AbstractExpression>cond_;
    };
    struct UpdateStatement:public SQLStatement {
        UpdateStatement(std::string table_name,std::vector<std::pair<std::string, std::unique_ptr<AbstractExpression>>> updates,
            std::unique_ptr<AbstractExpression>cond);
        std::string table_name_;
        std::vector<std::pair<std::string, std::unique_ptr<AbstractExpression>>> updates_;
        std::unique_ptr<AbstractExpression> cond_;
    };
    void PrintAST(SQLStatement* ast, int indent = 0) ;
    class Parser {
    public:
        explicit Parser(const std::vector<Token>& tokens) : tokens_(tokens), cursor_(0) {}
        std::unique_ptr<SQLStatement> ParseStatement();

    private:
        std::vector<Token> tokens_;
        size_t cursor_;

        Token Last()const;
        Token Peek() const;
        Token Advance();
        bool IsAtEnd() const;
        Token Consume(TokenType expected_type, const std::string& error_message);
        bool Match(TokenType expected_type);
        // --- 具体语句的推导逻辑 (递归下降逻辑) ---
        std::unique_ptr<SelectStatement> ParseSelect();
        std::unique_ptr<InsertStatement> ParseInsert();
        std::unique_ptr<DeleteStatement>ParseDelete();
        std::unique_ptr<UpdateStatement>ParseUpdate();
        std::unique_ptr<CreateTableStatement>ParseCreateTable();

        // --- 解析表达式
        std::unique_ptr<AbstractExpression>ParseExpression();
        std::unique_ptr<AbstractExpression>ParsePrimary();

        CompareOp ToCompareOp(TokenType tok_ty);
        bool IsCompareOp(TokenType tok_ty);
    };
}