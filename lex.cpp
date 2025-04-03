#include "lex.h"
#include <algorithm>

bool istheree(string& lexeme) {
    for (auto i : lexeme)
    {
        if (i == 'E') {
            return true;
        }
    }
    return false;
    
}

bool hasdot(string& lexeme) {
    for (auto i : lexeme)
    {
        if (i == '.') {
            return true;
        }
    }
    return false;
    
}

bool hasintafterdot(string& lexeme) {
    for (int i = 0; i < lexeme.size(); i++)
    {
        if (i != lexeme.size()-1) {
            if (lexeme[i] == '.' && isdigit(lexeme[i+1])) {
                return true;
            }
        } 
    }
    return false;
    
}

// the lexer itself
LexItem getNextToken(istream& in, int& linenum) {

    enum TokState {
        START, INID, ININT, INFLOAT, INSTRING, INCHAR, INCOMMENT, INEXP, INNEQ, INASSOP, INLTE, INGTE, DONE, ERR
    };

    TokState lexstate = START;
    string lexeme;
    char ch;
    
    while(in.get(ch)) {
        switch (lexstate) {
        case START:
            if (isspace(ch)) {
                if (ch == '\n') linenum++;
                continue;
            }
            if (isalpha(ch)) {
                lexeme += ch;
                lexstate = INID;
            } else if (isdigit(ch)) {
                lexeme += ch;
                lexstate = ININT;
            } else if (ch == '"') {
                lexeme += ch;
                lexstate = INSTRING;
            } else if (ch == '\'') {
                lexeme += ch;
                lexstate = INCHAR;
            } else if (ch == '-') {
                if (in.peek() == '-') lexstate = INCOMMENT;
                else return LexItem(MINUS, lexeme, linenum);
            } else if (ch == '+') {
                return LexItem(PLUS, lexeme, linenum);
            } else if (ch == '*') {
                if (in.peek() == '*') lexstate = INEXP;
                else return LexItem(MULT, lexeme, linenum);
            } else if (ch == '/') {
                if (in.peek() == '=') lexstate = INNEQ;
                else return LexItem(DIV, lexeme, linenum);
            } else if (ch == ':') {
                if (in.peek() == '=') lexstate = INASSOP;
                else return LexItem(COLON, lexeme, linenum);
            } else if (ch == '=') {
                return LexItem(EQ, lexeme, linenum);
            } else if (ch == '<') {
                if (in.peek() == '=') lexstate = INLTE;
                else return LexItem(LTHAN, lexeme, linenum);
            } else if (ch == '>') {
                if (in.peek() == '=') lexstate = INGTE;
                else return LexItem(GTHAN, lexeme, linenum);
            } else if (ch == '&') {
                return LexItem(CONCAT, lexeme, linenum);
            } else if (ch == ',') {
                return LexItem(COMMA, lexeme, linenum);
            } else if (ch == ';') {
                return LexItem(SEMICOL, lexeme, linenum);
            } else if (ch == '(') {
                return LexItem(LPAREN, lexeme, linenum);
            } else if (ch == ')') {
                return LexItem(RPAREN, lexeme, linenum);
            } else if (ch == '.') {
                return LexItem(DOT, lexeme, linenum);
            } else {
                return LexItem(Token::ERR, string(1, ch), linenum);
            }
            break;

        case INID:
            if (isalnum(ch) || ch == '_') {
                if (ch == '_' && lexeme[lexeme.size()-1] == '_') {
                    return LexItem(Token::ERR, string(1, ch), linenum);
                } else if (in.peek() == '_' && !(lexeme[lexeme.size()-1] == '_')) {
                    lexeme += ch;
                    std::transform(lexeme.begin(), lexeme.end(), lexeme.begin(), [](unsigned char c){ return std::tolower(c); });
                    return LexItem(IDENT, lexeme, linenum);
                }
                lexeme += ch;
            } else {
                in.putback(ch);
                return id_or_kw(lexeme, linenum);
            }
            break;

        case ININT:
            if (isdigit(ch)) {
                lexeme += ch;
            } else if ((ch == 'E' || ch == 'e') && !istheree(lexeme) && !isalpha(in.peek())) {
                lexeme += ch;
            } else if ((ch == '+' || ch == '-') && (lexeme[lexeme.size() - 1] == 'E' || lexeme[lexeme.size() - 1] == 'e')) {
                lexeme += ch;
            } else if (ch == '.') {
                lexeme += ch;
                lexstate = INFLOAT;
            } else {
                in.putback(ch);
                return LexItem(ICONST, lexeme, linenum);
            }
            break;
        
        case INFLOAT:
            if (isdigit(ch)) {
                lexeme += ch;
            } else if (ch == 'E' || ch == 'e') {
                if (in.peek() == '+' || in.peek() == '-' || isdigit(in.peek())) {
                    lexeme += ch;
                } else {
                    in.putback(ch);
                    return LexItem(FCONST, lexeme, linenum);
                }
            } else if (ch == '+' || ch == '-') {
                lexeme += ch;
            } else if (ch == '.' && !hasdot(lexeme)) {
                lexeme += ch;
                return LexItem(Token::ERR, lexeme, linenum);
            } else if (ch == '.' && hasdot(lexeme)) {
                if (hasintafterdot(lexeme)) {
                    lexeme += ch;
                    return LexItem(Token::ERR, lexeme, linenum);
                }
                lexeme.erase(std::remove(lexeme.begin(), lexeme.end(), '.'), lexeme.end());
                in.putback(ch);
                in.putback(ch);
                return LexItem(ICONST, lexeme, linenum);
            } else {
                in.putback(ch);
                return LexItem(FCONST, lexeme, linenum);
            }
            break;

        case INSTRING:
            if (ch == '"') {
                lexeme += ch;
                return LexItem(SCONST, lexeme, linenum);
            } else if (ch == '\'') {
                lexeme += ch;
                return LexItem(Token::ERR, string(" Invalid string constant " + lexeme), linenum);
            } else if (ch == '\n') {
                return LexItem(Token::ERR, string(" Invalid string constant " + lexeme), linenum);
            }
            lexeme += ch;
            break;

        case INCHAR:
            lexeme += ch;
            if (ch == '\'') {
                return LexItem(CCONST, lexeme, linenum);
            } else if (ch == '\n') {
                linenum++;
                return LexItem(Token::ERR, string("New line is an invalid character constant."), linenum);
            } else if (lexeme.size() > 2) {
                return LexItem(Token::ERR, string(" Invalid character constant " + lexeme + "\'"), linenum);
            }
            break;

        case INEXP:
            return LexItem(EXP, lexeme, linenum);
            break;

        case INNEQ:
            return LexItem(NEQ, lexeme, linenum);
            break;

        case INASSOP:
            return LexItem(ASSOP, lexeme, linenum);
            break;

        case INLTE:
            return LexItem(LTE, lexeme, linenum);
            break;

        case INGTE:
            return LexItem(GTE, lexeme, linenum);
            break;

        case INCOMMENT:
            if (ch == '\n') {
                linenum++;
                lexstate = START;
            }
            break;

        default:
            break;
        }
    }

    // Handle EOF case when a token is still being processed
    if (!lexeme.empty()) {
        if (lexstate == INID) {
            return id_or_kw(lexeme, linenum);
        } else if (lexstate == ININT) {
            return LexItem(ICONST, lexeme, linenum);
        } else if (lexstate == INFLOAT) {
            return LexItem(FCONST, lexeme, linenum);
        }
    }

    return LexItem(Token::DONE, "", linenum);
}

LexItem id_or_kw(const string& lexeme, int linenum) {
    string original = lexeme;
    std::transform(original.begin(), original.end(), original.begin(), [](unsigned char c){ return std::tolower(c); });
    if (original == "if") {
        return LexItem(IF, original, linenum);
    } else if (original == "else") {
        return LexItem(ELSE, original, linenum);
    } else if (original == "elsif") {
        return LexItem(ELSIF, original, linenum);
    } else if (original == "put") {
        return LexItem(PUT, original, linenum);
    } else if (original == "putline") {
        return LexItem(PUTLN, original, linenum);
    } else if (original == "get") {
        return LexItem(GET, original, linenum);
    } else if (original == "integer") {
        return LexItem(INT, original, linenum);
    } else if (original == "float") {
        return LexItem(FLOAT, original, linenum);
    } else if (original == "character") {
        return LexItem(CHAR, original, linenum);
    } else if (original == "string") {
        return LexItem(STRING, original, linenum);
    } else if (original == "boolean") {
        return LexItem(BOOL, original, linenum);
    } else if (original == "procedure") {
        return LexItem(PROCEDURE, original, linenum);
    } else if (original == "true") {
        return LexItem(BCONST, original, linenum);
    } else if (original == "false") {
        return LexItem(BCONST, original, linenum);
    } else if (original == "end") {
        return LexItem(END, original, linenum);
    } else if (original == "is") {
        return LexItem(IS, original, linenum);
    } else if (original == "begin") {
        return LexItem(BEGIN, original, linenum);
    } else if (original == "then") {
        return LexItem(THEN, original, linenum);
    } else if (original == "constant") {
        return LexItem(CONST, original, linenum);
    } else if (original == "and") {
        return LexItem(AND, original, linenum);
    } else if (original == "or") {
        return LexItem(OR, original, linenum);
    } else if (original == "not") {
        return LexItem(NOT, original, linenum);
    } else if (original == "mod") {
        return LexItem(MOD, original, linenum);
    } else {
        std::transform(original.begin(), original.end(), original.begin(), [](unsigned char c){ return std::tolower(c); });
        return LexItem(IDENT, original, linenum);
    }
}

ostream& operator<<(ostream& out, const LexItem& tok) {
    switch (tok.GetToken()) {
    case 0:
        out << "IF";
        return out;
        break;
    case 1:
        out << "ELSE";
        return out;
        break;
    case 2:
        out << "ELSIF";
        return out;
        break;
    case 3:
        out << "PUT";
        return out;
        break;
    case 4:
        out << "PUTLN";
        return out;
        break;
    case 5:
        out << "GET";
        return out;
        break;
    case 6:
        out << "INT";
        return out;
        break;
    case 7:
        out << "FLOAT";
        return out;
        break;
    case 8:
        out << "CHAR";
        return out;
        break;
    case 9:
        out << "STRING";
        return out;
        break;
    case 10:
        out << "BOOL";
        return out;
        break;
    case 11:
        out << "PROCEDURE";
        return out;
        break;
    case 12:
        out << "TRUE";
        return out;
        break;
    case 13:
        out << "FALSE";
        return out;
        break;
    case 14:
        out << "END";
        return out;
        break;
    case 15:
        out << "IS";
        return out;
        break;
    case 16:
        out << "BEGIN";
        return out;
        break;
    case 17:
        out << "THEN";
        return out;
        break;
    case 18:
        out << "CONST";
        return out;
        break;
    case 19:
        out << "IDENT: " << "<" << tok.GetLexeme() << ">";
        return out;
        break;
    case 20:
        out << "ICONST: " << "(" << tok.GetLexeme() << ")";
        return out;
        break;
    case 21:
        out << "FCONST: " << "(" << tok.GetLexeme() << ")";
        return out;
        break;
    case 22:
        out << "SCONST: " << tok.GetLexeme();
        return out;
        break;
    case 23:
        out << "BCONST: " << "(" << tok.GetLexeme() << ")";
        return out;
        break;
    case 24:
        out << "CCONST: " << tok.GetLexeme();
        return out;
        break;
    case 25:
        out << "PLUS";
        return out;
        break;
    case 26:
        out << "MINUS";
        return out;
        break;
    case 27:
        out << "MULT";
        return out;
        break;
    case 28:
        out << "DIV";
        return out;
        break;
    case 29:
        out << "ASSOP";
        return out;
        break;
    case 30:
        out << "EQ";
        return out;
        break;
    case 31:
        out << "NEQ";
        return out;
        break;
    case 32:
        out << "EXP";
        return out;
        break;
    case 33:
        out << "CONCAT";
        return out;
        break;
    case 34:
        out << "GTHAN";
        return out;
        break;
    case 35:
        out << "LTHAN";
        return out;
        break;
    case 36:
        out << "LTE";
        return out;
        break;
    case 37:
        out << "GTE";
        return out;
        break;
    case 38:
        out << "AND";
        return out;
        break;
    case 39:
        out << "OR";
        return out;
        break;
    case 40:
        out << "NOT";
        return out;
        break;
    case 41:
        out << "MOD";
        return out;
        break;
    case 42:
        out << "COMMA";
        return out;
        break;
    case 43:
        out << "SEMICOL";
        return out;
        break;
    case 44:
        out << "LPAREN";
        return out;
        break;
    case 45:
        out << "RPAREN";
        return out;
        break;
    case 46:
        out << "DOT";
        return out;
        break;
    case 47:
        out << "COLON";
        return out;
        break;
    case 48:
        out << "ERR: In line " << tok.GetLinenum() << ", Error Message {" << tok.GetLexeme() << "}";
        return out;
        break;
    case 49:
        out << "DONE";
        return out;
        break;
    
    default:
        break;
    }
    return out;
}