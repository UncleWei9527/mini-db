#pragma  once
#include<string>
#include<vector>
namespace minidb {
    enum class TokenType {
        INVALID,
        KW_SELECT,
        KW_FROM,
        KW_INSERT,
        KW_INTO,
        KW_CREATE,
        KW_TABLE,
        KW_WHERE,
        KW_DELETE,
        KW_INT,
        KW_VARCHAR,
        KW_BOOL,
        KW_TRUE,
        KW_FALSE,
        KW_VALUES,
        TK_LPAREN,//(
        TK_RPAREN,// )
        TK_COMMA,
        TK_STAR,    // *
        TK_SEMI,    // ;
        TK_LT,      // <
        TK_LE,      // <=
        TK_GT,      // >
        TK_GE,      // >=
        TK_EQ,      // ==
        TK_NEQ,     // !=
        IDENTIFIER,     // 表名或列名，比如 users
        NUMBER,
        STRING,
        END_OF_FILE     // 解析结束
    };

    struct Token {
        TokenType type_;
        std::string  text_;
        std::string ToString()const;
    };
    class Tokenizer {
    public:
        explicit Tokenizer(std::string sql);
        std::vector<Token>Tokenize();
    private:
        std::string sql_;
        size_t cursor_;
        void SkipWhitespace();
        Token ConsumeKeywordOrIdentifier();
        Token ConsumeSymbol();
        Token ConsumeNumber();
        Token ConsumeString();
        char Peek()const;
        char Advance();
        bool IsAtEnd()const;
    };
}