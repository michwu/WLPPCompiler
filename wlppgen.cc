// Starter code for CS241 assignments 9-11
//
// C++ translation by Simon Parent (Winter 2011),
// based on Java code by Ondrej Lhotak,
// which was based on Scheme code by Gord Cormack.
// Modified July 3, 2012 by Gareth Davies
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
using namespace std;

// The set of terminal symbols in the WLPP grammar.
const char *terminals[] = {
  "BOF", "BECOMES", "COMMA", "ELSE", "EOF", "EQ", "GE", "GT", "ID",
  "IF", "INT", "LBRACE", "LE", "LPAREN", "LT", "MINUS", "NE", "NUM",
  "PCT", "PLUS", "PRINTLN", "RBRACE", "RETURN", "RPAREN", "SEMI",
  "SLASH", "STAR", "WAIN", "WHILE", "AMP", "LBRACK", "RBRACK", "NEW",
  "DELETE", "NULL"
};
int isTerminal(const string &sym) {
  int idx;
  for(idx=0; idx<sizeof(terminals)/sizeof(char*); idx++)
    if(terminals[idx] == sym) return 1;
  return 0;
}

// Data structure for storing the parse tree.
class tree {
  public:
    string rule;
    vector<string> tokens;
    vector<tree*> children;
    ~tree() { for(int i=0; i<children.size(); i++) delete children[i]; }
};

// Call this to display an error message and exit the program.
void bail(const string &msg) {
  // You can also simply throw a string instead of using this function.
  throw string(msg);
}

// Read and return wlppi parse tree.
tree *readParse(const string &lhs) {
  // Read a line from standard input.
  string line;
  getline(cin, line);
  if(cin.fail())
    bail("ERROR: Unexpected end of file.");
  tree *ret = new tree();
  // Tokenize the line.
  stringstream ss;
  ss << line;
  while(!ss.eof()) {
    string token;
    ss >> token;
    if(token == "") continue;
    ret->tokens.push_back(token);
  }
  // Ensure that the rule is separated by single spaces.
  for(int idx=0; idx<ret->tokens.size(); idx++) {
    if(idx>0) ret->rule += " ";
    ret->rule += ret->tokens[idx];
  }
  // Recurse if lhs is a nonterminal.
  if(!isTerminal(lhs)) {
    for(int idx=1/*skip the lhs*/; idx<ret->tokens.size(); idx++) {
      ret->children.push_back(readParse(ret->tokens[idx]));
    }
  }
  return ret;
}

tree *parseTree;
map<string, string> symbol;
map<string, string>::iterator it;

string term(tree *t)
{
	string type = "";
	if (t->rule == "lvalue STAR factor")
	{
		type = term(t->children[1]);
		if (type == "int*")
		{
			return "int";
		}
		else
		{bail("ERROR:");return "";}
	}
	else if (t->rule == "lvalue LPAREN lvalue RPAREN")
	{
		return term(t->children[1]);
	}
	else if (t->rule == "lvalue ID")
	{
		type = symbol.find(t->children[0]->tokens[1])->second;
		return type;
	}


	else if (t->rule == "term term STAR factor" || t->rule == "term term SLASH factor" || t->rule == "term term PCT factor")
	{
		string type1 = term(t->children[0]);
		string type2 = term(t->children[2]);
			if (type1 == "int" && type2 == "int") {return "int";}
			else {bail("ERROR:"); return "";}
	}
	else if (t->rule == "term factor")
	{
		return term(t->children[0]);
	}
	else if (t->rule == "factor NEW INT LBRACK expr RBRACK")
	{
		type = term(t->children[3]);
		if (type == "int")
		{
			return "int*";
		}
		else
		{bail("ERROR:"); return "";}
	}

	else if (t->rule == "factor ID")
	{
		type = symbol.find(t->children[0]->tokens[1])->second;
		return type;
	}
	else if (t->rule == "factor NUM")
	{
		return "int";
	}
	else if (t->rule == "factor NULL")
	{
		return "int*";
	}
	else if (t->rule == "factor LPAREN expr RPAREN")
	{
		type = term(t->children[1]);
		return type;
	}
	else if (t->rule == "factor AMP lvalue")
	{
		type = term(t->children[1]);
		if (type == "int")
		{
			return "int*";
		}
		else
		{bail("ERROR:"); return "";}
	}
	else if (t->rule == "factor STAR factor")
	{
		type = term(t->children[1]);
		if (type == "int*")
		{
			return "int";
		}
		else
		{bail("ERROR:"); return "";}
	}
	else if (t->rule == "expr expr PLUS term")
	{
		string type1 = term(t->children[0]);
		string type2 = term(t->children[2]);
			if (type1 == "int" && type2 == "int") {return "int";}
			else if ((type1 == "int" && type2 == "int*") || (type1 == "int*" && type2 == "int")) {return "int*";}
			else {bail("ERROR:"); return "";}
	}
	else if (t->rule == "expr expr MINUS term")
	{
		string type1 = term(t->children[0]);
		string type2 = term(t->children[2]);
		if (type1 == "int" && type2 == "int") {return "int";}
			else if ((type1 == "int*" && type2 == "int*") || (type1 == "int*" && type2 == "int")) {return "int*";}
			else {bail("ERROR:"); return "";}
	}
	else if (t->rule == "expr term")
	{
		return term(t->children[0]);
	}
	else{bail("ERROR:");return "";}
}

// Compute symbols defined in t.
void genSymbols(tree *t) {
	if (t->tokens[0] == "expr" || t->tokens[0] == "lvalue")
	{
		string vartype = term(t);
	}
	if (t->rule == "statement lvalue BECOMES expr SEMI")
	{
		string ltype = term(t->children[0]);
		string etype = term(t->children[2]);
		if (ltype != etype) {bail("ERROR:");}
	}
	else if (t->rule == "statement DELETE LBRACK RBRACK expr SEMI")
	{
		string etype = term(t->children[3]);
		if (etype != "int*")
		{bail("ERROR:");}
	}
	else if (t->tokens[0] == "test")
	{
		string type1 = term(t->children[0]);
		string type2 = term(t->children[2]);
		if (type1 != type2)
		{bail("ERROR:");}
	}
	else if (t->rule == "statement PRINTLN LPAREN expr RPAREN SEMI")
	{
		string etype = term(t->children[2]);
		if (etype != "int")
		{bail("ERROR:");}
	}

	if (t->tokens[0] == "dcl")
	{
		it = symbol.find(t->children[1]->tokens[1]);
		if (it == symbol.end()){
			if (t->children[0]->tokens.size() == 3){
				symbol.insert(pair<string,string>(t->children[1]->tokens[1], "int*"));
				cerr << t->children[1]->tokens[1] << " " << "int*" << endl;
			}
			else if (t->children[0]->tokens.size() == 2){
				symbol.insert(pair<string,string>(t->children[1]->tokens[1], "int"));
				cerr << t->children[1]->tokens[1] << " " << "int" << endl;
			}
			else
			{bail("ERROR:");}
		}
		else {bail("ERROR:");}
	}
	else if (t->tokens[0] == "ID")
	{
		it = symbol.find(t->tokens[1]);
		if (it == symbol.end()){
			bail("ERROR:");
		}
	}
	else{
		for (int i=0; i<t->children.size();i++){
			genSymbols(t->children[i]);
		}
	}
}

void second(tree *t)
{
	if (t->rule == "procedure INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE")
	{
		string e = term(t->children[11]);
		if (e != "int")
		{bail("ERROR:");}
		int typesize = t->children[5]->children[0]->tokens.size();
		if (typesize != 2)
		{bail("ERROR:");}
	}

	else if (t->rule == "dcls dcls dcl BECOMES NULL SEMI")
	{
		int typesize = t->children[1]->children[0]->tokens.size();
		if (typesize != 3)
		{bail("ERROR:");}
	}
	else if (t->rule == "dcls dcls dcl BECOMES NUM SEMI")
	{
		int typesize = t->children[1]->children[0]->tokens.size();
		if (typesize != 2)
		{bail("ERROR:");}
	}
	for (int i=0; i<t->children.size();i++){
			second(t->children[i]);
		}
}
void loadaddr(tree* t, int reg){
	tree* IDnode = t;
	while (t->tokens[0]!="ID"){
		t = t->children[1];
		IDnode = t;
	}
    cout<<"lis $" << reg << "\n" << ".word V" << IDnode->tokens[1] << ";loadaddr"<<"\n";
}
void lw(int a, int b){
    cout<<"lw $" << a << ", 0($" << b <<  ")"<<";lw $" << a<< ", 0($"<<b<<")"<<"\n";
}

void sw(int a, int b){
    cout<<"sw $" << a << ", 0($" << b << ")"<<";sw $" << a<< ", 0($"<<b<<")"<<"\n";
}

void push(int a){
    cout<<"sw $" << a << ", -4($30)\n" << "lis $" << a << "\n" << ".word 4\n" << "sub $30, $30, $" << a <<";push" <<a<< "\n";
}

void pop(int a){
    cout<<"lis $" << a << "\n" << ".word 4\n" << "add $30, $30, $" << a << "\n" << "lw $" << a << ", -4($30)"<<";pop" <<a<< "\n";
}
void factorcode(tree* t);
void exprcode(tree* t);
void termcode(tree* t);
void statementcode(tree* t);
void statementscode (tree* t);
void addresscode(tree* t);
void dclscode(tree* t);
void dclcode( tree* t);
void testcode(tree* t, int label);

void testcode(tree* t, int label){
	if (t->rule == "test expr LT expr"){
		exprcode(t->children[0]);
		push(3);
		exprcode (t->children[2]);
		pop(1);
		cout<<"slt $1, $1, $3\nbeq $1, $0, L"<<label<<"\n";
	}
	else if (t->rule == "test expr EQ expr"){
		exprcode(t->children[0]);
		push(3);
		exprcode (t->children[2]);
		pop(1);
		cout<<"bne $1, $3, L"<<label<<"\n";
	}
	else if (t->rule=="test expr NE expr"){
		exprcode(t->children[0]);
		push(3);
		exprcode (t->children[2]);
		pop(1);
		cout<<"beq $1, $3, L"<<label<<"\n";
	}
	else if (t->rule =="test expr LE expr"){
		exprcode(t->children[0]);
		push(3);
		exprcode (t->children[2]);
		pop(1);
		cout<<"slt $1, $3, $1\nbne $1, $0, L"<<label<<"\n";
	}
	else if (t->rule=="test expr GE expr"){
		exprcode(t->children[0]);
		push(3);
		exprcode (t->children[2]);
		pop(1);
		cout<<"slt $1, $1, $3\nbne $1, $0, L"<<label<<"\n";
	}
	else if (t->rule == "test expr GT expr"){
		exprcode(t->children[0]);
		push(3);
		exprcode (t->children[2]);
		pop(1);
		cout<<"slt $1, $3, $1\nbeq $1, $0, L"<<label<<"\n";
	}
}

void addresscode(tree* t){
	if (t->rule =="lvalue ID"){
		//tree* name = t->children[0];
		loadaddr(t->children[0], 3);
	}
	else if (t-> rule == "lvalue LPAREN lvalue RPAREN"){
		addresscode(t->children[1]);
	}
	else if (t->rule == "lvalue STAR factor"){
		factorcode(t->children[1]);
	}
}
//int counter = 4;
void dclscode(tree* t){
	if (t->rule =="dcls dcls dcl BECOMES NUM SEMI"){
		dclscode(t->children[0]);
		loadaddr(t->children[1], 3);
		cout<<"lis $1"<<"\n.word "<<t->children[3]->tokens[1]<<"\n";
		sw (1,3);
	}
	else if (t->rule == "dcls dcls dcl BECOMES NULL SEMI"){
		dclscode(t->children[0]);
		loadaddr(t->children[1], 3);
		cout<<"lis $1"<<"\n.word 1\n";
		sw (1,3);
	}
}
void statementscode(tree* t){
	if (t->rule=="statements statements statement"){
		statementscode(t->children[0]);
		statementcode (t->children[1]);
	}
}
int labelcount = 0;
int newlabel(){
	labelcount++;
	return labelcount;
}
void statementcode (tree* t){
	if (t->rule =="statement PRINTLN LPAREN expr RPAREN SEMI"){
		exprcode(t->children[2]);
		cout<<"add $1, $3, $0"<<endl<<"lis $29"<<endl<<".word print"<<endl;
		cout <<"jalr $29"<<endl;
	}
	else if (t->rule == "statement lvalue BECOMES expr SEMI"){
		exprcode(t->children[2]);
		push (3);
		addresscode(t->children[0]);
		pop(1);
		sw(1,3);
	}
	else if (t->rule=="statement WHILE LPAREN test RPAREN LBRACE statements RBRACE"){
		int begin = newlabel();
		int end = newlabel();
		cout<<"L" << begin<<":\n";
		testcode(t->children[2], end);
		statementscode( t->children[5]);
		cout<< "beq $0, $0, L" << begin<<"\nL"<<end<<":\n";
	}
	else if (t->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE"){
		int iff = newlabel();
		int elsee = newlabel();
		testcode(t->children[2], elsee);
		statementscode(t->children[5]);
		cout<<"beq $0, $0, L" <<iff<<"\n";
		cout<<"L"<<elsee<<":\n";
		statementscode(t->children[9]);
		cout<<"L"<<iff<<":\n";
	}
	else if (t->rule=="statement DELETE LBRACK RBRACK expr SEMI"){
		exprcode(t->children[3]);
		push(31);
		cout<<"add $1, $3, $0\nlis $29\n.word delete\njalr $29\n";
		pop(31);
	}
}
void termcode(tree* t){
	if (t-> rule == "term factor"){
		factorcode(t->children[0]);
	}
	else if (t->rule == "term term STAR factor"){
		termcode(t->children[0]);
		push(3);
		factorcode(t->children[2]);
		pop(1);
		cout<<"mult $3, $1\n"<<"mflo $3\n";
	}
	else if (t->rule == "term term SLASH factor"){
		termcode(t->children[0]);
		push(3);
		factorcode(t->children[2]);
		pop(1);
		cout<<"div $1, $3\n"<<"mflo $3\n";
	}
	else if (t->rule == "term term PCT factor"){
		termcode(t->children[0]);
		push(3);
		factorcode(t->children[2]);
		pop(1);
		cout<<"div $1, $3\n"<<"mfhi $3\n";
	}
}


//exprcode
void exprcode(tree *t){
	if (t->rule == "expr term"){
		termcode(t->children[0]);
	}
	else if (t->rule == "expr expr PLUS term"){
		string type1 = term(t->children[0]);
		string type2 = term(t->children[2]);
		if (type1 =="int" && type2 == "int"){
			exprcode(t->children[0]);
			push(3);
			termcode(t->children[2]);
			pop(1);
			cout<<"add $3, $1, $3\n";
		}
		else if(type1=="int*" &&type2=="int"){
			exprcode(t->children[0]);
			push(3);
			termcode(t->children[2]);
			pop(1);
			cout<<"lis $4\n.word 4\nmult $3, $4\nmflo $3\nadd $3, $1, $3\n";
		}
		else if (type1=="int" && type2=="int*"){
			exprcode(t->children[0]);
			push(3);
			termcode(t->children[2]);
			pop(1);
			cout<<"lis $4\n.word 4\nmult $1, $4\nmflo $1\nadd $3, $1, $3\n";
		}
	}
	else if (t->rule == "expr expr MINUS term"){
		string type1 = term(t->children[0]);
		string type2 = term(t->children[2]);
		if (type1 == "int" && type2=="int"){
			exprcode(t->children[0]);
			push(3);
			termcode(t->children[2]);
			pop(1);
			cout<<"sub $3, $1, $3\n";
		}
		else if (type1 =="int*" && type2 == "int"){
			exprcode(t->children[0]);
			push(3);
			termcode(t->children[2]);
			pop(1);
			cout<<"lis $4\n.word 4\nmult $3, $4\nmflo $3\nsub $3, $1, $3\n";
		}
		else if (type1=="int*" && type2 == "int*"){
			exprcode(t->children[0]);
			push(3);
			termcode(t->children[2]);
			pop(1);
			cout<<"lis $4\n.word 4\n sub $3, $1, $3\ndiv $3, $4\nmflo $3\n";
			//cout<<"lis $4\n.word 4\nmult $3, $4\nmflo $3\nmult $1, $4\nmflo $1\nsub $3, $1, $3\ndiv $3, $4\nmflo $3\n";
		}
	}
}
//factorcode
void factorcode (tree* t){
	if (t-> rule == "factor ID"){
		string name = t->children[0]->tokens[1];
        cout<<"lis $1\n" << ".word V"  << name << "\n" << "lw $3, 0($1)\n";
	}
	else if (t-> rule == "factor LPAREN expr RPAREN"){
		exprcode(t->children[1]);
	}
	else if (t-> rule =="factor NUM"){
		cout<<"lis $3\n.word " << t->children[0]->tokens[1]<<"\n";
	}
	else if (t->rule == "factor NULL"){
		cout<<"lis $3\n.word 1\n";
	}
	else if (t->rule == "factor AMP lvalue"){
		addresscode(t->children[1]);
	}
	else if (t->rule == "factor STAR factor"){
		factorcode(t->children[1]);
		lw(3,3);
	}
	else if (t->rule=="factor NEW INT LBRACK expr RBRACK"){
		exprcode(t->children[3]);
		push(31);
		cout<<"add $1, $3, $0\nlis $29\n.word new\njalr $29\n";
		pop(31);
	}
}

//procedurecode
void procedurecode(tree *t){
	if (t-> rule == "procedure INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE"){
		if (t->children[3]->children[0]->rule == "type INT"){
			cout<<"add $2, $0, $0 ;TO CLEAR REGISTER 2\n";
		}
		push(31);
		cout<<"lis $29\n.word init\njalr $29\n";
		pop(31);
		push(31);
		loadaddr(t->children[3], 3);
		sw (1,3);
		loadaddr(t->children[5], 3);
		sw (2,3);
		dclscode(t->children[8]);
		statementscode(t->children[9]);
		exprcode(t->children[11]);
		pop(31);
		cout<<"jr $31\n";
		for (it = symbol.begin(); it!=symbol.end(); it++){
			cout<< "V" << it->first<< ":  .word 0\n";
		}
		cout<<"print:sw $1, -4($30) ;print"<<endl<<"sw $2, -8($30)"<<endl<< "sw $3, -12($30)\nsw $4, -16($30)\nsw $5, -20($30)\nsw $6, -24($30)\nsw $7, -28($30)\nsw $8, -32($30)\n";
		cout<<"sw $9, -36($30)\nsw $10, -40($30)\nlis $3\n.word -40\nadd $30, $30, $3\nlis $3\n.word 0xffff000c\nlis $4\n.word 10\nlis $5\n.word 4\n";
		cout<<"add $6, $1, $0\nslt $7, $1, $0\nbeq $7, $0, IfDone\nlis $8\n.word 0x0000002d\nsw $8, 0($3)\nsub $6, $0, $6\nIfDone:add $9, $30, $0\n";
		cout<<"Loop:divu $6, $4\nmfhi $10\nsw $10, -4($9)\nmflo $6\nsub $9, $9, $5\nslt $10, $0, $6\nbne $10, $0, Loop\nlis $7\n.word 48\n";
		cout<<"Loop2:lw $8, 0($9)\nadd $8, $8, $7\nsw $8, 0($3)\nadd $9, $9, $5\nbne $9, $30, Loop2\nsw $4, 0($3)\nlis $3\n.word 40\nadd $30, $30, $3\n";
		cout<<"lw $1, -4($30)\nlw $2, -8($30)\nlw $3, -12($30)\nlw $4, -16($30)\nlw $5, -20($30)\nlw $6, -24($30)\nlw $7, -28($30)\nlw $8, -32($30)\n";
		cout<<"lw $9, -36($30)\nlw $10, -40($30)\njr $31 ;end of print\n";

		cout<<"init:\n \
sw $1, -4($30)\n \
sw $2, -8($30)\n \
sw $3, -12($30)\n \
sw $4, -16($30)\n \
sw $5, -20($30)\n \
sw $6, -24($30)\n \
sw $7, -28($30)\n \
sw $8, -32($30)\n \
lis $4\n \
.word 32\n \
sub $30, $30, $4\n \
lis $1\n \
.word end\n \
lis $3\n \
.word 1024       ; space for free list (way more than necessary)\n \
lis $6\n \
.word 16         ; size of bookkeeping region at end of program\n \
lis $7\n \
.word 4096       ; size of heap\n \
lis $8\n \
.word 1\n \
add $2, $2, $2   ; Convert array length to words (*4)\n \
add $2, $2, $2\n \
add $2, $2, $6   ; Size of OS added by loader\n \
add $5, $1, $6   ; end of program + length of bookkeeping\n \
add $5, $5, $2   ; + length of incoming array\n \
add $5, $5, $3   ; + length of free list\n \
sw $5, 0($1)     ; store address of heap at Mem[end]\n \
add $5, $5, $7   ; store end of heap at Mem[end+4]\n \
sw $5, 4($1)\n \
sw $8, 8($1)     ; store initial size of free list (1) at Mem[end+8]\n \
add $5, $1, $6\n \
add $5, $5, $2\n \
sw $5, 12($1)   ; store location of free list at Mem[end+12]\n \
sw $8, 0($5)    ; store initial contents of free list (1) at Mem[end+12]\n \
sw $0, 4($5)    ; zero-terminate the free list\n \
add $30, $30, $4\n \
lw $1, -4($30)\n \
lw $2, -8($30)\n \
lw $3, -12($30)\n \
lw $4, -16($30)\n \
lw $5, -20($30)\n \
lw $6, -24($30)\n \
lw $7, -28($30)\n \
lw $8, -32($30)\n \
jr $31\n \
new:\n \
sw $1, -4($30)\n \
sw $2, -8($30)\n \
sw $4, -12($30)\n \
sw $5, -16($30)\n \
sw $6, -20($30)\n \
sw $7, -24($30)\n \
sw $8, -28($30)\n \
sw $9, -32($30)\n \
sw $10, -36($30)\n \
sw $11, -40($30)\n \
sw $12, -44($30)\n \
lis $10\n \
.word 44\n \
sub $30, $30, $10\n \
slt $3, $0, $1\n \
beq $3, $0, cleanupN\n \
lis $11   ; $11 = 1\n \
.word 1\n \
add $1, $1, $11 ; One extra word to store deallocation info\n \
add $1, $1, $1  ; Convert $1 from words to bytes\n \
add $1, $1, $1\n \
add $2, $11, $11  ; $2 = 2\n \
add $4, $0, $0  ; $4 = counter, to accumulate ceil(log($1))\n \
sub $1, $1, $11  ; So subtract 1 from $1\n \
topN:  ; Repeatedly divide $1 by 2, and count iterations\n \
beq $1, $0, endloopN\n \
div $1, $2      ; $1 /= 2\n \
mflo $1\n \
add $4, $4, $11  ; $4++\n \
beq $0, $0, topN\n \
endloopN:\n \
add $1, $1, $11  ; Now add 1 to $1 to restore its value after previous sub\n \
add $4, $4, $11  ; And add 1 to $4 to complete ceil calculation (see above)\n \
lis $5     ; $5 = 14\n \
.word 14\n \
sub $4, $5, $4  ; $4 <- 14 - $4\n \
lis $5\n \
.word 9\n \
slt $6, $5, $4\n \
beq $6, $0, doNotFixN\n \
add $4, $5, $0\n \
doNotFixN:\n \
slt $3, $0, $4\n \
beq $3, $0, cleanupN\n \
add $6, $4, $0    ; countdown from $4 to 0\n \
add $7, $11, $0   ; accumulates result by doubling $4 times\n \
top2N:\n \
add $7, $7, $7    ; double $7\n \
sub $6, $6, $11   ; $6--\n \
bne $6, $0, top2N\n \
sub $7, $7, $11  ; At the end of the loop, $7 = 2^$4 - 1\n \
lis $8\n \
.word findWord\n \
sw $31, -4($30)\n \
lis $31\n \
.word 4\n \
sub $30, $30, $31\n \
jalr $8          ; call findWord\n \
lis $31\n \
.word 4\n \
add $30, $30, $31\n \
lw $31, -4($30)\n \
beq $3, $0, cleanupN  ; if allocation fails, clean up and return 0\n \
add $7, $7, $11\n \
div $7, $2\n \
mflo $7\n \
exactN:\n \
slt $6, $3, $7\n \
bne $6, $0, largerN\n \
beq $0, $0, convertN\n \
largerN:  ;; buddies are 2$3 and 2$3+1\n \
add $3, $3, $3 ;; double $3\n \
lis $6   ;; $6 = address of address of free list\n \
.word free\n \
lw $8, -4($6)  ;; $8 = length of free list\n \
lw $6, 0($6)   ;; $6 = address of free list\n \
add $8, $8, $8 ;; convert to words (*4)\n \
add $8, $8, $8\n \
add $6, $6, $8 ;; address of next spot in free list\n \
add $8, $3, $11 ;; $8 = buddy\n \
sw $8, 0($6)   ;; add to end of list\n \
sw $0, 4($6)\n \
lis $6\n \
.word free\n \
lw $8, -4($6)\n \
add $8, $8, $11\n \
sw $8, -4($6)\n \
beq $0, $0, exactN\n \
convertN:\n \
add $12, $3, $0  ; retain original freelist word\n \
add $7, $0, $0 ;; offset into heap\n \
lis $8\n \
.word end\n \
lw $9, 4($8)  ;; end of heap\n \
lw $8, 0($8)  ;; beginning of heap\n \
sub $9, $9, $8 ;; size of heap (bytes)\n \
top5N:\n \
beq $3, $11, doneconvertN\n \
div $3, $2\n \
mflo $3    ;; $3/2\n \
mfhi $10   ;; $3%2\n \
beq $10, $0, evenN\n \
add $7, $7, $9   ;; add size of heap to offset\n \
evenN:\n \
div $7, $2       ;; divide offset by 2\n \
mflo $7\n \
beq $0, $0, top5N\n \
doneconvertN:\n \
add $3, $8, $7  ;; add start of heap to offset to get address\n \
lis $4\n \
.word 4\n \
add $3, $3, $4  ;; advance one byte for deallocation info\n \
sw $12, -4($3)  ;; store deallocation info\n \
cleanupN:\n \
lis $10\n \
.word 44\n \
add $30, $30, $10\n \
lw $1, -4($30)\n \
lw $2, -8($30)\n \
lw $4, -12($30)\n \
lw $5, -16($30)\n \
lw $6, -20($30)\n \
lw $7, -24($30)\n \
lw $8, -28($30)\n \
lw $9, -32($30)\n \
lw $10, -36($30)\n \
lw $11, -40($30)\n \
lw $12, -44($30)\n \
jr $31\n \
delete:\n \
sw $1, -4($30)\n \
sw $2, -8($30)\n \
sw $3, -12($30)\n \
sw $4, -16($30)\n \
sw $5, -20($30)\n \
sw $6, -24($30)\n \
sw $11, -28($30)\n \
sw $12, -32($30)\n \
sw $14, -36($30)\n \
lis $6\n \
.word 36\n \
sub $30, $30, $6\n \
lis $11\n \
.word 1\n \
lis $12\n \
.word 2\n \
lis $14\n \
.word 4\n \
lw $2, -4($1) ;; buddy code for the allocated block\n \
nextBuddyD:\n \
beq $2, $11, notFoundD  ;; if there is no buddy (i.e. buddy code=1), bail out\n \
add $3, $2, $0\n \
div $3, $12   ; $4 = $3 % 2\n \
mfhi $4\n \
beq $4, $0, evenD\n \
sub $3, $3, $11\n \
beq $0, $0, doneParityD\n \
evenD:\n \
add $3, $3, $11\n \
doneParityD:\n \
lis $5\n \
.word findAndRemove\n \
sw $31, -4($30)\n \
sub $30, $30, $14\n \
add $1, $3, $0\n \
jalr $5\n \
add $30, $30, $14\n \
lw $31, -4($30)\n \
beq $3, $0, notFoundD\n \
div $2, $12\n \
mflo $2\n \
beq $0, $0, nextBuddyD\n \
notFoundD:\n \
lis $4   ;; address of address of free list\n \
.word free\n \
lw $5, -4($4) ; length of the free list\n \
lw $4, 0($4)  ;; address of the free list\n \
add $5, $5, $5  ; convert to offset\n \
add $5, $5, $5\n \
add $5, $4, $5  ; address of next spot in free list\n \
sw $2, 0($5)    ; put code back into free list\n \
sw $0, 4($5)    ; keep free list 0-terminated\n \
lis $4\n \
.word free\n \
lw $5, -4($4)\n \
add $5, $5, $11\n \
sw $5, -4($4)\n \
lis $6\n \
.word 36\n \
add $30, $30, $6\n \
lw $1, -4($30)\n \
lw $2, -8($30)\n \
lw $3, -12($30)\n \
lw $4, -16($30)\n \
lw $5, -20($30)\n \
lw $6, -24($30)\n \
lw $11, -28($30)\n \
lw $12, -32($30)\n \
lw $14, -36($30)\n \
jr $31\n \
findWord:\n \
sw $1, -4($30)\n \
sw $2, -8($30)\n \
sw $4, -12($30)\n \
sw $5, -16($30)\n \
sw $6, -20($30)\n \
sw $7, -24($30)\n \
sw $8, -28($30)\n \
sw $9, -32($30)\n \
sw $10, -36($30)\n \
lis $1\n \
.word 36\n \
sub $30, $30, $1\n \
lis $1  ;; address of address of the free list\n \
.word free\n \
lw $2, -4($1)\n \
lw $1, 0($1) ;; address of the free list\n \
lis $4   ; $4 = 4 (for looping increments over memory)\n \
.word 4\n \
lis $9   ; $9 = 1 (for loop decrements)\n \
.word 1\n \
add $3, $0, $0  ;; initialize output to 0 (not found)\n \
add $10, $0, $0 ;; for address of max word\n \
beq $2, $0, cleanupFW  ;; skip if no free memory\n \
add $5, $2, $0  ;; loop countdown to 0\n \
topFW:\n \
lw $6, 0($1)\n \
slt $8, $7, $6  ;; limit < current item (i.e. item ineligible?)\n \
bne $8, $0, ineligibleFW\n \
slt $8, $3, $6  ;; max < current item?\n \
beq $8, $0, ineligibleFW  ; if not, skip to ineligible\n \
add $3, $6, $0  ;; replace max with current\n \
add $10, $1, $0 ;; address of current\n \
ineligibleFW:\n \
add $1, $1, $4  ;; increment address\n \
sub $5, $5, $9  ;; decrement loop counter\n \
bne $5, $0, topFW     ;; if items left, continue looping\n \
beq $3, $0, cleanupFW\n \
top2FW:\n \
lw $6, 4($10)  ;; grab next element in array\n \
sw $6, 0($10)  ;; store in current position\n \
add $10, $10, $4 ;; increment address\n \
bne $6, $0, top2FW  ;; continue while elements nonzero\n \
lis $2\n \
.word end\n \
lw $4, 8($2)\n \
sub $4, $4, $9  ; $9 still 1\n \
sw $4, 8($2)\n \
cleanupFW:\n \
lis $1\n \
.word 36\n \
add $30, $30, $1\n \
lw $1, -4($30)\n \
lw $2, -8($30)\n \
lw $4, -12($30)\n \
lw $5, -16($30)\n \
lw $6, -20($30)\n \
lw $7, -24($30)\n \
lw $8, -28($30)\n \
lw $9, -32($30)\n \
lw $10, -36($30)\n \
jr $31\n \
findAndRemove:\n \
sw $1, -4($30)\n \
sw $2, -8($30)\n \
sw $4, -12($30)\n \
sw $5, -16($30)\n \
sw $6, -20($30)\n \
sw $7, -24($30)\n \
sw $8, -28($30)\n \
sw $9, -32($30)\n \
sw $11, -36($30)\n \
sw $14, -40($30)\n \
lis $9\n \
.word 40\n \
sub $30, $30, $9\n \
lis $11\n \
.word 1\n \
lis $14\n \
.word 4\n \
lis $2     ;; address of address of the free list\n \
.word free\n \
lw $4, -4($2) ;; length of the free list\n \
lw $2, 0($2)  ;; address of the free list\n \
add $3, $0, $0 ; success code\n \
add $6, $0, $0 ; address of found code\n \
add $7, $0, $0 ; loop counter\n \
topFaR:  ; loop through free list, looking for the code\n \
beq $4, $0, cleanupFaR\n \
lw $5, 0($2) ; next code in list\n \
bne $5, $1, notEqualFaR  ;; compare with input\n \
add $6, $6, $2  ; if code found, save its address\n \
beq $0, $0, removeFaR\n \
notEqualFaR:  ; current item not the one we're looking for; update counters\n \
add $2, $2, $14\n \
add $7, $7, $11\n \
bne $7, $4, topFaR\n \
removeFaR:\n \
beq $6, $0, cleanupFaR  ;; if code not found, bail out\n \
top2FaR:  ; now loop through the rest of the free list, moving each item one\n \
lw $8, 4($2)\n \
sw $8, 0($2)\n \
add $2, $2, $14  ; add 4 to current address\n \
add $7, $7, $11  ; add 1 to loop counter\n \
bne $7, $4, top2FaR\n \
add $3, $11, $0  ;; set success code\n \
lis $2\n \
.word free\n \
lw $5, -4($2)\n \
sub $5, $5, $11\n \
sw $5, -4($2)\n \
cleanupFaR:\n \
lis $9\n \
.word 40\n \
add $30, $30, $9\n \
lw $1, -4($30)\n \
lw $2, -8($30)\n \
lw $4, -12($30)\n \
lw $5, -16($30)\n \
lw $6, -20($30)\n \
lw $7, -24($30)\n \
lw $8, -28($30)\n \
lw $9, -32($30)\n \
lw $11, -36($30)\n \
lw $14, -40($30)\n \
jr $31\n \
printFreeList:\n \
sw $1, -4($30)\n \
sw $2, -8($30)\n \
sw $3, -12($30)\n \
sw $4, -16($30)\n \
sw $5, -20($30)\n \
sw $6, -24($30)\n \
sw $7, -28($30)\n \
sw $8, -32($30)\n \
lis $6\n \
.word 32\n \
sub $30, $30, $6\n \
lis $3   ; address of address of the start of the free list\n \
.word free\n \
lis $4\n \
.word 4\n \
lis $5   ; external print procedure\n \
.word print\n \
lis $6\n \
.word 1\n \
lw $2, -4($3) ; $2 = length of free list; countdown to 0 for looping\n \
lw $3, 0($3) ; $3 = address of the start of the free list\n \
topPFL:\n \
beq $2, $0, endPFL  ;; skip if free list empty\n \
lw $1, 0($3)     ; store in $1 the item to be printed\n \
sw $31, -4($30)\n \
sub $30, $30, $4\n \
jalr $5          ; call external print procedure\n \
add $30, $30, $4\n \
lw $31, -4($30)\n \
add $3, $3, $4   ; update current address and loop counter\n \
sub $2, $2, $6\n \
bne $2, $0, topPFL\n \
endPFL:\n \
lis $6\n \
.word 0xffff000c\n \
lis $5\n \
.word 10\n \
sw $5, 0($6)\n \
lis $6\n \
.word 32\n \
add $30, $30, $6\n \
lw $1, -4($30)\n \
lw $2, -8($30)\n \
lw $3, -12($30)\n \
lw $4, -16($30)\n \
lw $5, -20($30)\n \
lw $6, -24($30)\n \
lw $7, -28($30)\n \
lw $8, -32($30)\n \
jr $31\n \
end:\n \
.word 0 ;; beginnning of heap\n \
.word 0 ;; end of heap\n \
.word 0 ;; length of free list\n \
free: .word 0 ;; beginning of free list\n";
}

}
// Generate the code for the parse tree t.
void genCode(tree *t) {
	procedurecode(t->children[1]);
}

int main() {
  // Main program.
  try {
    parseTree = readParse("S");
    genSymbols(parseTree);
    genCode(parseTree);
  } catch(string msg) {
    cerr << msg << endl;
  }
  if (parseTree) delete parseTree;
  return 0;
}



