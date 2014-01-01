/*tarter code for the CS 241 assembler (assignments 3 and 4).
    Code contained here may be included in submissions for CS241
    assignments at the University of Waterloo.

    ---------------------------------------------------------------

    To compile on a CSCF linux machine, use:

            g++ -g asm.cc -o asm

    To run:
            ./asm           < source.asm > program.mips
            valgrind ./asm  < source.asm > program.mips
 */

#include <string>
#include <vector>
#include <sstream>
#include <bitset>
#include <iostream>
#include <cmath>
#include <cstdio>
using namespace std;

//======================================================================
//========= Declarations for the scan() function =======================
//======================================================================

// Each token has one of the following kinds.

enum Kind {
    ID,                 // Opcode or identifier (use of a label)
    INT,                // Decimal integer
    HEXINT,             // Hexadecimal integer
    REGISTER,           // Register number
    COMMA,              // Comma
    LPAREN,             // (
    RPAREN,             // )
    LABEL,              // Declaration of a label (with a colon)
    DOTWORD,            // .word directive
    WHITESPACE,         // Whitespace
    NUL                 // Bad/invalid token
};

// kindString(k) returns string a representation of kind k
// that is useful for error and debugging messages.
string kindString(Kind k);

// Each token is described by its kind and its lexeme.

struct Token {
    Kind      kind;
    string    lexeme;
    /* toInt() returns an integer representation of the token. For tokens
     * of kind INT (decimal integer constant) and HEXINT (hexadecimal integer
     * constant), returns the integer constant. For tokens of kind
     * REGISTER, returns the register number.
     */
    int       toInt();
};

// scan() separates an input line into a vector of Tokens.
vector<Token> scan(string input);

// =====================================================================
// The implementation of scan() and associated type definitions.
// If you just want to use the scanner, skip to the next ==== separator.

// States for the finite-state automaton that comprises the scanner.

enum State {
    ST_NUL,
    ST_START,
    ST_DOLLAR,
    ST_MINUS,
    ST_REGISTER,
    ST_INT,
    ST_ID,
    ST_LABEL,
    ST_COMMA,
    ST_LPAREN,
    ST_RPAREN,
    ST_ZERO,
    ST_ZEROX,
    ST_HEXINT,
    ST_COMMENT,
    ST_DOT,
    ST_DOTW,
    ST_DOTWO,
    ST_DOTWOR,
    ST_DOTWORD,
    ST_WHITESPACE
};

// The *kind* of token (see previous enum declaration)
// represented by each state; states that don't represent
// a token have stateKinds == NUL.

Kind stateKinds[] = {
    NUL,            // ST_NUL
    NUL,            // ST_START
    NUL,            // ST_DOLLAR
    NUL,            // ST_MINUS
    REGISTER,       // ST_REGISTER
    INT,            // ST_INT
    ID,             // ST_ID
    LABEL,          // ST_LABEL
    COMMA,          // ST_COMMA
    LPAREN,         // ST_LPAREN
    RPAREN,         // ST_RPAREN
    INT,            // ST_ZERO
    NUL,            // ST_ZEROX
    HEXINT,         // ST_HEXINT
    WHITESPACE,     // ST_COMMENT
    NUL,            // ST_DOT
    NUL,            // ST_DOTW
    NUL,            // ST_DOTWO
    NUL,            // ST_DOTWOR
    DOTWORD,        // ST_DOTWORD
    WHITESPACE      // ST_WHITESPACE
};

State delta[ST_WHITESPACE+1][256];

#define whitespace "\t\n\r "
#define letters    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define digits     "0123456789"
#define hexDigits  "0123456789ABCDEFabcdef"
#define oneToNine  "123456789"

void setT(State from, string chars, State to) {
    for(int i = 0; i < chars.length(); i++ ) delta[from][chars[i]] = to;
}

void initT(){
    int i, j;

    // The default transition is ST_NUL (i.e., no transition
    // defined for this char).
    for ( i=0; i<=ST_WHITESPACE; i++ ) {
        for ( j=0; j<256; j++ ) {
            delta[i][j] = ST_NUL;
        }
    }
    // Non-null transitions of the finite state machine.
    // NB: in the third line below, letters digits are macros
    // that are replaced by string literals, which the compiler
    // will concatenate into a single string literal.
    setT( ST_START,      whitespace,     ST_WHITESPACE );
    setT( ST_WHITESPACE, whitespace,     ST_WHITESPACE );
    setT( ST_START,      letters,        ST_ID         );
    setT( ST_ID,         letters digits, ST_ID         );
    setT( ST_START,      oneToNine,      ST_INT        );
    setT( ST_INT,        digits,         ST_INT        );
    setT( ST_START,      "-",            ST_MINUS      );
    setT( ST_MINUS,      digits,      	 ST_INT        );
    setT( ST_START,      ",",            ST_COMMA      );
    setT( ST_START,      "(",            ST_LPAREN     );
    setT( ST_START,      ")",            ST_RPAREN     );
    setT( ST_START,      "$",            ST_DOLLAR     );
    setT( ST_DOLLAR,     digits,         ST_REGISTER   );
    setT( ST_REGISTER,   digits,         ST_REGISTER   );
    setT( ST_START,      "0",            ST_ZERO       );
    setT( ST_ZERO,       "x",            ST_ZEROX      );
    setT( ST_ZERO,       digits,      	 ST_INT        );
    setT( ST_ZEROX,      hexDigits,      ST_HEXINT     );
    setT( ST_HEXINT,     hexDigits,      ST_HEXINT     );
    setT( ST_ID,         ":",            ST_LABEL      );
    setT( ST_START,      ";",            ST_COMMENT    );
    setT( ST_START,      ".",            ST_DOT        );
    setT( ST_DOT,        "w",            ST_DOTW       );
    setT( ST_DOTW,       "o",            ST_DOTWO      );
    setT( ST_DOTWO,      "r",            ST_DOTWOR     );
    setT( ST_DOTWOR,     "d",            ST_DOTWORD    );

    for ( j=0; j<256; j++ ) delta[ST_COMMENT][j] = ST_COMMENT;
}

static int initT_done = 0;

vector<Token> scan(string input){
    // Initialize the transition table when called for the first time.
    if(!initT_done) {
        initT();
        initT_done = 1;
    }

    vector<Token> ret;

    int i = 0;
    int startIndex = 0;
    State state = ST_START;

    if(input.length() > 0) {
        while(true) {
            State nextState = ST_NUL;
            if(i < input.length())
                nextState = delta[state][(unsigned char) input[i]];
            if(nextState == ST_NUL) {
                // no more transitions possible
                if(stateKinds[state] == NUL) {
                    throw("ERROR in lexing after reading " + input.substr(0, i));
                }
                if(stateKinds[state] != WHITESPACE) {
                    Token token;
                    token.kind = stateKinds[state];
                    token.lexeme = input.substr(startIndex, i-startIndex);
                    ret.push_back(token);
                }
                startIndex = i;
                state = ST_START;
                if(i >= input.length()) break;
            } else {
                state = nextState;
                i++;
            }
        }
    }

    return ret;
}

int Token::toInt() {
    if(kind == INT) {
        long long l;
        sscanf( lexeme.c_str(), "%lld", &l );
	if (lexeme.substr(0,1) == "-") {
            if(l < -2147483648LL)
                throw("ERROR: constant out of range: "+lexeme);
	} else {
	    unsigned long long ul = l;
            if(ul > 4294967295LL)
                throw("ERROR: constant out of range: "+lexeme);
	}
        return l;
    } else if(kind == HEXINT) {
        long long l;
        sscanf( lexeme.c_str(), "%llx", &l );
	unsigned long long ul = l;
        if(ul > 0xffffffffLL)
            throw("ERROR: constant out of range: "+lexeme);
        return l;
    } else if(kind == REGISTER) {
        long long l;
        sscanf( lexeme.c_str()+1, "%lld", &l );
	unsigned long long ul = l;
        if(ul > 31)
            throw("ERROR: constant out of range: "+lexeme);
        return l;
    }
    throw("ERROR: attempt to convert non-integer token "+lexeme+" to Int");
}

// kindString maps each kind to a string for use in error messages.

string kS[] = {
    "ID",           // ID
    "INT",          // INT
    "HEXINT",       // HEXINT
    "REGISTER",     // REGISTER
    "COMMA",        // COMMA
    "LPAREN",       // LPAREN
    "RPAREN",       // RPAREN
    "LABEL",        // LABEL
    "DOTWORD",      // DOTWORD
    "WHITESPACE",   // WHITESPACE
    "NUL"           // NUL
};

string kindString( Kind k ){
    if ( k < ID || k > NUL ) return "INVALID";
    return kS[k];
}



//======================================================================
//======= A sample program demonstrating the use of the scanner. =======
//======================================================================

int main() {

    try {
        vector<string> srcLines;

        // Read the entire input file, storing each line as a
        // single string in the array srcLines.
        while(true) {
            string line;
            getline(cin, line);
            if(cin.fail()) break;
            srcLines.push_back(line);
        }

        // Tokenize each line, storing the results in tokLines.
        vector<vector<Token> > tokLines;

        for(int line = 0; line < srcLines.size(); line++) {
            tokLines.push_back(scan(srcLines[line]));
        }

//for a3p4

        vector<string> lbllist;
        vector <int> location;
        int count = 0;
        int offset;
        int difference;
        int currentmem;
        int line = 0;
        int line2 = line;
        int danger = 0;
        for (line = 0; line < tokLines.size(); line++)
        {
            line2 = line;
            if (tokLines[line].size() == 0) count = count -4;
            for (int j = 0; j< tokLines[line].size(); j++)
            {
                if (tokLines[line][j].kind!=LABEL && tokLines[line][j].kind != ID && tokLines[line][j].kind!=DOTWORD &&tokLines[line][j].kind!=INT &&tokLines[line][j].kind!=HEXINT&&tokLines[line][j].kind!=REGISTER)
                {
                    cerr<<"ERROR"<<endl;
                    danger =1;
                    break;
                }

                if (tokLines[line][j].kind==LABEL){
                    if (j == (tokLines[line].size())-1)
                        {count = count-4;
                        line2 = line +1;
                    }
                }
                else
                    break;
            }

            if (danger ==1){
                danger =0;
                break;
            }
            for (int j = 0; j< tokLines[line].size(); j++)
            {
                if (tokLines[line][j].kind ==LABEL)
                {
                    for (int m =0; m<lbllist.size(); m++)     //check for duplicate labels
                    {
                        if ((lbllist[m]+":")==tokLines[line][j].lexeme){
                            cerr <<"ERROR"<<endl;
                            danger = 1;
                        }
                    }
                    if (danger ==1)
                    {
                        break;
                    }
                    string temp =tokLines[line][j].lexeme;   //remove colons from labels
                    string temp2 = "";
                    for (int k=0; k<temp.length()-1; k++)
                    {
                        temp2 += temp[k];
                    }
                    lbllist.push_back(temp2);           //construct symbol table
                    location.push_back((line2) * 4 + count);
                }
            }
                if (danger ==1)
                {
                    danger =0;
                    break;
                }
        }

        for (int i = 0; i < lbllist.size(); i ++)
        {
            cerr <<lbllist[i] << " " << location[i] <<endl;   //cerr the symbol table
        }

        count = 0;
        for(int line=0; line < tokLines.size(); line++ ) {
            line2 = line;
            for (int k = 0; k< tokLines[line].size(); k++)
            {
                    if (tokLines[line][k].kind==LABEL){
                            if (k == (tokLines[line].size())-1)
                            {
                                count = count-4;
                                line2 = line +1;
                            }
                    }
            }

            if (tokLines[line].size() == 0) count = count -4; //empty line do nothing

            else
            {
                for (int j = 0; j<tokLines[line].size(); j++)
                {
                    if (tokLines[line][j].kind==DOTWORD){      //for .word
                        if (j==tokLines[line].size()-1)
                        {
                            cerr<<"ERROR"<<endl;
                        }
                        else if (tokLines[line][j+1].kind == ID)
                        {
                            if ((j+2) !=tokLines[line].size())
                            {
                                cerr <<"ERROR"<<endl;
                                break;
                            }
                            string operand = tokLines[line][j+1].lexeme;
                            danger =1;
                            for (int n = 0; n<lbllist.size(); n ++)
                            {
                                if (operand == lbllist[n])
                                {
                                    int output = location[n];
                                    putchar((output>>24) & 0xff);
                                    putchar((output>>16) & 0xff);
                                    putchar((output>>8) & 0xff);
                                    putchar (output & 0xff);
                                    danger =0;
                                    break;
                                }
                                else danger = 1;
                            }
                            if (danger == 1)
                            {
                                cerr<<"ERROR"<<endl;
                                danger = 0;
                                break;
                            }
                            else if (danger ==0)
                            {
                                break;
                            }
                        }

                        else if(tokLines[line][j+1].kind==INT || tokLines[line][j+1].kind ==HEXINT)
                        {
                            if ((j+2) !=tokLines[line].size())
                            {
                                cerr <<"ERROR"<<endl;
                                break;
                            }
                            int output = tokLines[line][j+1].toInt();
                            putchar((output>>24) & 0xff);
                            putchar((output>>16) & 0xff);
                            putchar((output>>8) & 0xff);
                            putchar (output & 0xff);
                            break;
                        }
                        else
                        {
                            cerr<<"ERROR"<<endl;
                        }
                    }

                    else if (tokLines[line][j].kind == ID)
                    {
                        if (tokLines[line][j].lexeme == "jr")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&(j+2) == tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int output = (0<<26)|(reg<<21)|(8 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }
                        else if (tokLines[line][j].lexeme == "jalr")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&(j+2)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int output = (0<<26)|(reg<<21)|(9 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }
                        else if (tokLines[line][j].lexeme == "add")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&tokLines[line][j+2].kind==COMMA&&tokLines[line][j+3].kind==REGISTER&& tokLines[line][j+4].kind == COMMA && tokLines[line][j+5].kind == REGISTER&&(j+6)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int reg2 = tokLines[line][j+3].toInt();
                                int reg3 = tokLines[line][j+5].toInt();
                                int output = (0<<26)|(reg2<<21)|(reg3<<16)|(reg<<11)|(32 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }
                        else if (tokLines[line][j].lexeme == "sub")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&tokLines[line][j+2].kind==COMMA&&tokLines[line][j+3].kind==REGISTER&& tokLines[line][j+4].kind == COMMA && tokLines[line][j+5].kind == REGISTER&&(j+6)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int reg2 = tokLines[line][j+3].toInt();
                                int reg3 = tokLines[line][j+5].toInt();
                                int output = (0<<26)|(reg2<<21)|(reg3<<16)|(reg<<11)|(34 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }
                        else if (tokLines[line][j].lexeme == "slt")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&tokLines[line][j+2].kind==COMMA&&tokLines[line][j+3].kind==REGISTER&& tokLines[line][j+4].kind == COMMA && tokLines[line][j+5].kind == REGISTER&&(j+6)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int reg2 = tokLines[line][j+3].toInt();
                                int reg3 = tokLines[line][j+5].toInt();
                                int output = (0<<26)|(reg2<<21)|(reg3<<16)|(reg<<11)|(42 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }
                        else if (tokLines[line][j].lexeme == "sltu")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&tokLines[line][j+2].kind==COMMA&&tokLines[line][j+3].kind==REGISTER&& tokLines[line][j+4].kind == COMMA && tokLines[line][j+5].kind == REGISTER&&(j+6)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int reg2 = tokLines[line][j+3].toInt();
                                int reg3 = tokLines[line][j+5].toInt();
                                int output = (0<<26)|(reg2<<21)|(reg3<<16)|(reg<<11)|(43 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }
                        else if (tokLines[line][j].lexeme == "beq")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&tokLines[line][j+2].kind==COMMA&&tokLines[line][j+3].kind==REGISTER&& tokLines[line][j+4].kind == COMMA && (tokLines[line][j+5].kind ==ID || tokLines[line][j+5].kind == INT || tokLines[line][j+5].kind == HEXINT)&&(j+6)==tokLines[line].size())
                            {
                                if (tokLines[line][j+5].kind ==ID)
                                {
                                    int memory = -1;     //invalid memory check
                                    for (int i =0; i < lbllist.size(); i++)
                                    {
                                        if (lbllist[i]==tokLines[line][j+5].lexeme)
                                        {
                                            memory = location[i];
                                            break;
                                        }
                                    }
                                    if (memory==-1)        //label not  found in symbol table
                                    {
                                        cerr<<"ERROR"<<endl;
                                        break;
                                    }
                                    currentmem = line * 4 + count;
                                    difference = memory - currentmem;
                                    offset = difference/4-1;
                                    if (offset>32767 || offset<-32768)
                                    {
                                        cerr<<"ERROR"<<endl;
                                        break;
                                    }
                                }

                               else{
                                    if (tokLines[line][j+5].kind == INT &&(tokLines[line][j+5].toInt()>32767 || tokLines[line][j+5].toInt()<-32768))
                                    {
                                        cerr<<"ERROR"<<endl;
                                        break;
                                    }
                                    else if (tokLines[line][j+5].kind == HEXINT &&(tokLines[line][j+5].toInt()>65535 || tokLines[line][j+5].toInt()<0))
                                    {
                                        cerr<<"ERROR"<<endl;
                                        break;
                                    }
                                }
                                int reg = tokLines[line][j+1].toInt();
                                int reg2 = tokLines[line][j+3].toInt();
                                int reg3;
                                if (tokLines[line][j+5].kind == ID)
                                {
                                    reg3 = offset;
                                }
                                else
                                {
                                    reg3 = tokLines[line][j+5].toInt();
                                }

                                int output = (4<<26)|(reg<<21)|(reg2<<16)|(reg3 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }
                        else if (tokLines[line][j].lexeme == "bne")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&tokLines[line][j+2].kind==COMMA&&tokLines[line][j+3].kind==REGISTER&& tokLines[line][j+4].kind == COMMA && (tokLines[line][j+5].kind ==ID || tokLines[line][j+5].kind == INT || tokLines[line][j+5].kind == HEXINT)&&(j+6)==tokLines[line].size())
                            {
                                if (tokLines[line][j+5].kind ==ID)
                                {
                                    int memory = -1;     //invalid memory check
                                    for (int i =0; i < lbllist.size(); i++)
                                    {
                                        if (lbllist[i]==tokLines[line][j+5].lexeme)
                                        {
                                            memory = location[i];
                                            break;
                                        }
                                    }
                                    if (memory==-1)        //label not  found in symbol table
                                    {
                                        cerr<<"ERROR"<<endl;
                                        break;
                                    }
                                    currentmem = line * 4 + count;
                                    difference = memory - currentmem;
                                    offset = difference/4-1;
                                    if (offset>32767 || offset<-32768)
                                    {
                                        cerr<<"ERROR"<<endl;
                                        break;
                                    }
                                }

                                else{
                                    if (tokLines[line][j+5].kind == INT &&(tokLines[line][j+5].toInt()>32767 || tokLines[line][j+5].toInt()<-32768))
                                    {
                                        cerr<<"ERROR"<<endl;
                                        break;
                                    }
                                    else if (tokLines[line][j+5].kind == HEXINT &&(tokLines[line][j+5].toInt()>65535 || tokLines[line][j+5].toInt()<0))
                                    {
                                        cerr<<"ERROR"<<endl;
                                        break;
                                    }
                                }
                                int reg = tokLines[line][j+1].toInt();
                                int reg2 = tokLines[line][j+3].toInt();
                                int reg3;
                                if (tokLines[line][j+5].kind == ID)
                                {
                                    reg3 = offset;
                                }
                                else
                                {
                                    reg3 = tokLines[line][j+5].toInt();
                                }

                                int output = (5<<26)|(reg<<21)|(reg2<<16)|(reg3 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }

                        else if (tokLines[line][j].lexeme == "lis")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&(j+2)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int output = (0<<16)|(reg<<11)|(20 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }

                        else if (tokLines[line][j].lexeme == "mflo")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&(j+2)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int output = (0<<16)|(reg<<11)|(18 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }

                        else if (tokLines[line][j].lexeme == "mfhi")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&(j+2)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int output = (0<<16)|(reg<<11)|(16 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }
                        else if (tokLines[line][j].lexeme == "mult")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&tokLines[line][j+2].kind == COMMA && tokLines[line][j+3].kind==REGISTER &&(j+4)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int reg2 = tokLines[line][j+3].toInt();
                                int output = (0<<26)|(reg<<21)|(reg2<<16)|(24 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }
                        else if (tokLines[line][j].lexeme == "multu")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&tokLines[line][j+2].kind == COMMA && tokLines[line][j+3].kind==REGISTER &&(j+4)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int reg2 = tokLines[line][j+3].toInt();
                                int output = (0<<26)|(reg<<21)|(reg2<<16)|(25 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }
                        else if (tokLines[line][j].lexeme == "div")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&tokLines[line][j+2].kind == COMMA && tokLines[line][j+3].kind==REGISTER &&(j+4)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int reg2 = tokLines[line][j+3].toInt();
                                int output = (0<<26)|(reg<<21)|(reg2<<16)|(26 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }
                        else if (tokLines[line][j].lexeme == "divu")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&tokLines[line][j+2].kind == COMMA && tokLines[line][j+3].kind==REGISTER &&(j+4)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int reg2 = tokLines[line][j+3].toInt();
                                int output = (0<<26)|(reg<<21)|(reg2<<16)|(27 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }
                        else if (tokLines[line][j].lexeme == "sw")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&tokLines[line][j+2].kind == COMMA && (tokLines[line][j+3].kind==INT || tokLines[line][j+3].kind==HEXINT) &&tokLines[line][j+4].kind==LPAREN && tokLines[line][j+5].kind==REGISTER && tokLines[line][j+6].kind==RPAREN&&(j+7)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int reg2 = tokLines[line][j+3].toInt();
                                int reg3 = tokLines[line][j+5].toInt();

                                if (tokLines[line][j+3].kind == INT && (reg2>32767||reg2<-32768))
                                {
                                    cerr<<"ERROR"<<endl;
                                    break;
                                }
                                else if (tokLines[line][j+3].kind == HEXINT && (reg2>65535||reg2<0))
                                {
                                    cerr<<"ERROR"<<endl;
                                    break;
                                }
                                int output = (43<<26)|(reg3<<21)|(reg<<16)|(reg2 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }
                        else if (tokLines[line][j].lexeme == "lw")
                        {
                            if (tokLines[line][j+1].kind==REGISTER&&tokLines[line][j+2].kind == COMMA && (tokLines[line][j+3].kind==INT || tokLines[line][j+3].kind==HEXINT) &&tokLines[line][j+4].kind==LPAREN && tokLines[line][j+5].kind==REGISTER && tokLines[line][j+6].kind==RPAREN&&(j+7)==tokLines[line].size())
                            {
                                int reg = tokLines[line][j+1].toInt();
                                int reg2 = tokLines[line][j+3].toInt();
                                int reg3 = tokLines[line][j+5].toInt();

                                if (tokLines[line][j+3].kind == INT && (reg2>32767||reg2<-32768))
                                {
                                    cerr<<"ERROR"<<endl;
                                    break;
                                }
                                else if (tokLines[line][j+3].kind == HEXINT && (reg2>65535||reg2<0))
                                {
                                    cerr<<"ERROR"<<endl;
                                    break;
                                }
                                int output = (35<<26)|(reg3<<21)|(reg<<16)|(reg2 & 0xffff);
                                putchar((output>>24) & 0xff);
                                putchar((output>>16) & 0xff);
                                putchar((output>>8) & 0xff);
                                putchar (output & 0xff);
                                break;
                            }
                            else
                            {
                                cerr<<"ERROR"<<endl;
                            }
                        }

                    }
                    else if (tokLines[line][j].kind==LABEL){
                        continue;
                    }



                    else
                        {
                            cerr<<"ERROR"<<endl;
                        }

            }
        }
        }

    } catch(string msg) {
        cerr << msg << endl;
    }

    return 0;
}

