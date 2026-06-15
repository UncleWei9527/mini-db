#pragma  once
#include<vector>
#include"token.h"
#include<memory>
#include<iostream>
namespace minidb {
    enum class StatementType {
        SELECT,
        INSERT
    };
    struct SQLStatement {
        SQLStatement(StatementType type);
        StatementType GetType()const;
        virtual ~SQLStatement()=default;
    private:
        StatementType type_;
    };
    struct SelectStatement:public SQLStatement {
        SelectStatement(std::string table_name);
        std::string table_name_;
    };
    struct InsertStatement:public SQLStatement {
        InsertStatement(std::string table_name,std::unique_ptr<SelectStatement> select_query);
        std::string table_name_;
        std::unique_ptr<SelectStatement>select_query_;
    };
    void PrintAST(SQLStatement* ast, int indent = 0) ;




    class Parser {
    public:
        explicit Parser(const std::vector<Token>& tokens) : tokens_(tokens), cursor_(0) {}
        std::unique_ptr<SQLStatement> Parse();

    private:
        std::vector<Token> tokens_;
        size_t cursor_;


        Token Peek() const;
        Token Advance();
        bool IsAtEnd() const;
        Token Consume(TokenType expected_type, const std::string& error_message);

        // --- 具体语句的推导逻辑 (递归下降逻辑) ---
        std::unique_ptr<SelectStatement> ParseSelect();
        std::unique_ptr<InsertStatement> ParseInsert();
    };
}