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
        TK_STAR,    // *
        TK_SEMI,    // ;
        IDENTIFIER,     // 表名或列名，比如 users
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
        char Peek()const;
        char Advance();
        bool IsAtEnd()const;
    };
}