#ifndef __PARSER_H__
#define __PARSER_H__

#include "lex.h"
#include "ast.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

using namespace std;

enum class Priority { 
    LOWEST, //
    EQUALS,  // == 
    LESSGREATER, // > or <
    SUM, // + 
    PRODUCT, // * 
    PREFIX,  // - or !
    CALL, // myfunction();
};

struct EnumClassHash {
    template <typename T>
    std::size_t operator()(T t) const {
        return static_cast<std::size_t>(t);
    }
};

class Parser {
    public: 
        using prefixParserFn = std::function<Expression*()>;
        using infixParserFn = std::function<Expression*(Expression*)>;

        const static unordered_map<TokenType, Priority, EnumClassHash> sPriority;
        

        Parser(const string& s):l_(std::move(unique_ptr<Lex>(new Lex(s)))) { 
            nextToken();
            nextToken();
            registerPrefixFn(TokenType::IDENTIFIER, std::bind(&Parser::parseIdenifier, this));
            registerPrefixFn(TokenType::INT, std::bind(&Parser::parseIntegerLiteral, this));
            registerPrefixFn(TokenType::BANG, std::bind(&Parser::parsePrefixExpression, this));
            registerPrefixFn(TokenType::MINUES, std::bind(&Parser::parsePrefixExpression, this));
            registerPrefixFn(TokenType::TRUE, std::bind(&Parser::parseBooleanExpression, this));
            registerPrefixFn(TokenType::FALSE, std::bind(&Parser::parseBooleanExpression, this));
            registerPrefixFn(TokenType::LPAREN, std::bind(&Parser::parseGroupedExpression, this));
            registerPrefixFn(TokenType::IF, std::bind(&Parser::parseIfExpression, this));
            registerPrefixFn(TokenType::FUNCTION, std::bind(&Parser::parseFunctionLiteral, this));

            registerInfixFn(TokenType::PLUS, std::bind(&Parser::parseInprefixExpression, this, placeholders::_1));
            registerInfixFn(TokenType::MINUES, std::bind(&Parser::parseInprefixExpression, this, placeholders::_1));
            registerInfixFn(TokenType::PRODUCT, std::bind(&Parser::parseInprefixExpression, this, placeholders::_1));
            registerInfixFn(TokenType::DIVIDE, std::bind(&Parser::parseInprefixExpression, this, placeholders::_1));
            registerInfixFn(TokenType::EQUAL, std::bind(&Parser::parseInprefixExpression, this, placeholders::_1));
            registerInfixFn(TokenType::NOTEQUAL, std::bind(&Parser::parseInprefixExpression, this, placeholders::_1));
            registerInfixFn(TokenType::LESS, std::bind(&Parser::parseInprefixExpression, this, placeholders::_1));
            registerInfixFn(TokenType::GREAT, std::bind(&Parser::parseInprefixExpression, this, placeholders::_1));
            registerInfixFn(TokenType::LPAREN, std::bind(&Parser::parseCallExpression, this, placeholders::_1));
            registerInfixFn(TokenType::GREATEQUAL, std::bind(&Parser::parseInprefixExpression, this, placeholders::_1));
            registerInfixFn(TokenType::LESSEQUAL, std::bind(&Parser::parseInprefixExpression, this, placeholders::_1));
        }

        Token nextToken() { 
            cur_token_ = peek_token_;
            peek_token_ = l_->getNextToken();
            return cur_token_;
        } 
    public:
        bool currentTokenIs(TokenType t) const {
            return cur_token_.type == t;
        }
        bool peekTokenIs(TokenType t) const {
            return peek_token_.type == t;
        }
        void registerPrefixFn(TokenType t, prefixParserFn fn) {
            int type = static_cast<int>(t);
            pre_parser_fn_.insert(std::pair<int, prefixParserFn>(type, fn));
        }

        void registerInfixFn(TokenType t, infixParserFn fn) {
            int type = static_cast<int>(t);
            in_parser_fn_.insert(std::pair<int, infixParserFn>(type, fn));
        }

        bool expectPeek(TokenType t) {
            if(peekTokenIs(t)) {
                nextToken();
                return true;
            } else {
                peekError(t);
                return false;
            }
        } 
        void peekError(TokenType t) { 
            char buf[1024];
            ::snprintf(buf, sizeof(buf), "expect next token to be %s, got %s instead\n", 
                    convertTokenType(t).c_str(), convertTokenType(peek_token_.type).c_str()); 
            errors_.push_back(string(buf));
        }

        const vector<string>& getErrors() {
            return errors_;
        }

    public: 
        Priority peekPriority() const ; 
        Priority currentPrioriy() const ;

    public:
        Program* parseProgram();
        Statement* parseStatement();
        LetStatement* parseLetStatement();
        ReturnStatement* parseReturnStatement(); 
        ExpressionStatement* parseExpressionStatement();
        Expression* parseExpression(Priority p);
        Expression* parseIdenifier(); 
        Expression* parseIntegerLiteral();
        Expression* parsePrefixExpression();
        Expression* parseInprefixExpression(Expression* );
        Expression* parseBooleanExpression();
        Expression* parseGroupedExpression();
        void noPrefixParserFnError(TokenType t);
        Expression* parseIfExpression();
        BlockStatement* parseBlockStatement();
        Expression* parseFunctionLiteral();
        vector<IdentifierNode*> parseFunctionParams(); 
        Expression* parseCallExpression(Expression* func);
        vector<Expression*> parseCallArguments(); 
    private:
        unique_ptr<Lex> l_;
        vector<string> errors_;
        std::unordered_map<int, prefixParserFn>pre_parser_fn_;
        std::unordered_map<int, infixParserFn>in_parser_fn_; 
        Token cur_token_;
        Token peek_token_;
}; 

#endif
