#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<cstdio>
#include<cstring>
#include "parameters.h"
using namespace std;

int kind[MAX_SIZE];
char input[MAX_SIZE][17];
int pToken;
int varCount, proCount, inputCount;
int lineNum;
bool lineErr;

FILE* inFile = NULL;
FILE* outFile = NULL;
FILE* errFile = NULL;
FILE* varFile = NULL;
FILE* proFile = NULL;

enum types {
	integer
};

struct Var {
	char name[17];
	char proc[17];
	int kind;
	types type;
	int level;
	int addr;
}vars[MAX_SIZE], pVar;

struct Proc {
	char name[17];
	types type;
	int level;
	int faddr;
	int laddr;
	int varNum;
	char parameter[20];
	bool parameterIsDefined;
}procs[MAX_SIZE], pProc;

bool nextToken();
bool isVarExisted(char* vname, char* vporc, int vkind);
bool isProcExisted(char* pname);
void ErrorProc(int errorType, const char* token);
void Program();
void SubProgram();
void StatementList();
void _StatementList();
void Statement();
void _Statement();
void Parameter();
void Function();
void ExecutionList();
void _ExecutionList();
void Execution();
void Read();
void Write();
void Assign();
void ArithExpression();
void _ArithExpression();
void Item();
void _Item();
void Factor();
void Constant();
void If();
void IfExpression();
void RelationalOperator();
void FunctionCall();
void Var();
void Identifier();
void parserInit();
void parserFinalize();

int main() {
	parserInit();
	Program();
	parserFinalize();
	return 0;
}

bool nextToken() {
	pToken++;
	if (kind[pToken] == EOF) {
		return false;
	}
	while (kind[pToken] == EOLN) {
		pToken++;
		lineNum++;
		lineErr = false;
	}
	return true;
}

bool isVarExisted(char* vname, char* vproc, int vkind) {
	for (int i = 0; i < varCount; i++) {
		if (strcmp(vname, vars[i].name) == 0 && strcmp(vproc, vars[i].proc) == 0 && vkind == vars[i].kind) {
			return true;
		}
	}
	return false;
}

bool isProcExisted(char* pname) {
	for (int i = 0; i < proCount; i++) {
		if (strcmp(pname, procs[i].name) == 0) {
			return true;
		}
	}
	return false;
}

void ErrorProc(int errorType, const char* token) {
	if (errorType == SIGN_UNDEFINED_ERROR && !lineErr) {
		fprintf(errFile, "LINE%d£º'%s' was not declared in this scope\n", lineNum, token);
	}
	else if (errorType == SIGN_REDEFINED_ERROR && !lineErr) {
		fprintf(errFile, "LINE%d: redeclaration of '%s'\n", lineNum, input[pToken]);
	}
	else if (errorType == SIGN_UNMATCHED_ERROR && !lineErr) {
		fprintf(errFile, "LINE%d£ºexpected '%s', found '%s'\n", lineNum, token, input[pToken]);
	}
	else if (errorType == SIGN_ABSENCE_ERROR && !lineErr) {
		fprintf(errFile, "LINE%d£ºexpected '%s' before '%s'\n", lineNum, token, input[pToken]);
	}
	else if (errorType == FATAL_ERROR) {
		fprintf(errFile, "LINE%d: [fatal error] expected '%s', found '%s'\n", lineNum, token, input[pToken]);
		parserFinalize();
		exit(0);
	}
	lineErr = true;
}

void Program() {
	SubProgram();
}

void SubProgram() {
	if (kind[pToken] == BEGIN) {
		nextToken();
	}
	else {
		if (kind[pToken] == INTEGER) {
			ErrorProc(SIGN_ABSENCE_ERROR, "begin");
		}
		else {
			ErrorProc(SIGN_UNMATCHED_ERROR, "begin");
			nextToken();
		}
	}
	StatementList();
	ExecutionList();
	if (kind[pToken] == END) {
		nextToken();
	}
	else {
		ErrorProc(SIGN_ABSENCE_ERROR, "end");
	}
}

void StatementList() {
	Statement();
	if (kind[pToken] == SEM) {
		nextToken();
	}
	else {
		ErrorProc(SIGN_ABSENCE_ERROR, ";");
		lineErr = false;
	}
	_StatementList();
}

void _StatementList() {
	if (kind[pToken] == INTEGER) {
		Statement();
		if (kind[pToken] == SEM) {
			nextToken();
		}
		else {
			ErrorProc(SIGN_ABSENCE_ERROR, ";");
		}
		_StatementList();
	}
}

void Statement() {
	if (kind[pToken] == INTEGER) {
		nextToken();
	}
	else {
		ErrorProc(SIGN_UNMATCHED_ERROR, "integer");
		nextToken();
	}
	_Statement();
}

void _Statement() {
	if (kind[pToken] == FUNCTION) {
		nextToken();
		Proc backupProc = pProc;
		strcpy(pProc.name, input[pToken]);
		pProc.type = integer;
		pProc.level++;
		pProc.varNum = 0;
		pProc.parameterIsDefined = false;
		if (isProcExisted(input[pToken])) {
			ErrorProc(SIGN_REDEFINED_ERROR, "");
		}
		Identifier();
		if (kind[pToken] == LPAR) {
			nextToken();
		}
		else {
			ErrorProc(SIGN_ABSENCE_ERROR, "(");
		}
		strcpy(pProc.parameter, input[pToken]);
		Parameter();
		if (kind[pToken] == RPAR) {
			nextToken();
		}
		else {
			ErrorProc(SIGN_ABSENCE_ERROR, ")");
		}
		if (kind[pToken] == SEM) {
			nextToken();
		}
		else {
			ErrorProc(SIGN_ABSENCE_ERROR, ";");
		}
		Function();
		pProc = backupProc;
	}
	else {
		strcpy(pVar.name, input[pToken]);
		strcpy(pVar.proc, pProc.name);
		if (strcmp(input[pToken], pProc.parameter) == 0) {
			pVar.kind = FORMAL_PARAMETER;
			pProc.parameterIsDefined = true;
		}
		else {
			pVar.kind = VARIABLE;
		}
		pVar.type = integer;
		pVar.level = pProc.level;
		pVar.addr = varCount;
		if (isVarExisted(pVar.name, pVar.proc, pVar.kind)) {
			ErrorProc(SIGN_REDEFINED_ERROR, "");
			Var();
		}
		else {
			if (pProc.varNum == 0) {
				pProc.faddr = pVar.addr;
			}
			pProc.laddr = pVar.addr;
			pProc.varNum++;
			Var();
			vars[varCount++] = pVar;
		}
	}
}

void Parameter() {
	Var();
}

void Function() {
	if (kind[pToken] == BEGIN) {
		nextToken();
	}
	else {
		if (kind[pToken] == INTEGER) {
			ErrorProc(SIGN_ABSENCE_ERROR, "begin");
		}
		else {
			ErrorProc(SIGN_UNMATCHED_ERROR, "begin");
			nextToken();
		}
	}
	StatementList();
	if (!pProc.parameterIsDefined) {
		ErrorProc(SIGN_UNDEFINED_ERROR, pProc.parameter);
	}
	procs[proCount++] = pProc;
	ExecutionList();
	if (kind[pToken] == END) {
		nextToken();
	}
	else {
		ErrorProc(SIGN_ABSENCE_ERROR, "end");
	}
}

void ExecutionList() {
	Execution();
	_ExecutionList();
}

void _ExecutionList() {
	if (kind[pToken] == SEM) {
		nextToken();
		Execution();
		_ExecutionList();
	}
	else if (kind[pToken] != END) {
		ErrorProc(SIGN_ABSENCE_ERROR, ";");
		Execution();
		_ExecutionList();
	}
}

void Execution() {
	if (kind[pToken] == READ) {
		nextToken();
		Read();
	}
	else if (kind[pToken] == WRITE) {
		nextToken();
		Write();
	}
	else if (kind[pToken] == IF) {
		nextToken();
		If();
	}
	else if (kind[pToken] == ID) {
		Assign();
	}
	else {
		ErrorProc(FATAL_ERROR, "execution");
	}
}

void Read() {
	if (kind[pToken] == LPAR) {
		nextToken();
	}
	else {
		ErrorProc(SIGN_ABSENCE_ERROR, "(");
	}
	Var();
	if (!isVarExisted(input[pToken - 1], pProc.name, FORMAL_PARAMETER) && !isVarExisted(input[pToken - 1], pProc.name, VARIABLE)) {
		pToken--;
		ErrorProc(SIGN_UNDEFINED_ERROR, input[pToken]);
		pToken++;
	}
	if (kind[pToken] == RPAR) {
		nextToken();
	}
	else {
		ErrorProc(SIGN_ABSENCE_ERROR, ")");
	}
}

void Write() {
	if (kind[pToken] == LPAR) {
		nextToken();
	}
	else {
		ErrorProc(SIGN_ABSENCE_ERROR, "(");
	}
	Var();
	if (!isVarExisted(input[pToken - 1], pProc.name, FORMAL_PARAMETER) && !isVarExisted(input[pToken - 1], pProc.name, VARIABLE)) {
		pToken--;
		ErrorProc(SIGN_UNDEFINED_ERROR, input[pToken]);
		pToken++;
	}
	if (kind[pToken] == RPAR) {
		nextToken();
	}
	else {
		ErrorProc(SIGN_ABSENCE_ERROR, ")");
	}
}

void Assign() {
	Var();
	if (!isVarExisted(input[pToken - 1], pProc.name, FORMAL_PARAMETER) && !isVarExisted(input[pToken - 1], pProc.name, VARIABLE) && !isProcExisted(input[pToken - 1])) {
		pToken--;
		ErrorProc(SIGN_UNDEFINED_ERROR, input[pToken]);
		pToken++;
	}
	if (kind[pToken] == ASSIGN) {
		nextToken();
	}
	else {
		ErrorProc(SIGN_ABSENCE_ERROR, ":=");
	}
	ArithExpression();
}

void ArithExpression() {
	Item();
	_ArithExpression();
}

void _ArithExpression() {
	if (kind[pToken] == SUB) {
		nextToken();
		Item();
		_ArithExpression();
	}
}

void Item() {
	Factor();
	_Item();
}

void _Item() {
	if (kind[pToken] == MUL) {
		nextToken();
		Factor();
		_Item();
	}
}

void Factor() {
	if (kind[pToken] == CONST) {
		Constant();
	}
	else if (isProcExisted(input[pToken])) {
		FunctionCall();
	}
	else {
		Var();
		if (!isVarExisted(input[pToken - 1], pProc.name, FORMAL_PARAMETER) && !isVarExisted(input[pToken - 1], pProc.name, VARIABLE)) {
			pToken--;
			ErrorProc(SIGN_UNDEFINED_ERROR, input[pToken]);
			pToken++;
		}
	}
}

void Constant() {
	if (kind[pToken] == CONST) {
		nextToken();
	}
	else {
		ErrorProc(FATAL_ERROR, "constant");
	}
}

void If() {
	IfExpression();
	if (kind[pToken] == THEN) {
		nextToken();
	}
	else {
		ErrorProc(SIGN_ABSENCE_ERROR, "then");
	}
	Execution();
	if (kind[pToken] == ELSE) {
		nextToken();
	}
	else {
		ErrorProc(SIGN_ABSENCE_ERROR, "else");
	}
	Execution();
}

void IfExpression() {
	ArithExpression();
	RelationalOperator();
	ArithExpression();
}

void RelationalOperator() {
	if (kind[pToken] == EQUAL || kind[pToken] == NEQUAL || kind[pToken] == LE || kind[pToken] == LT || kind[pToken] == GE || kind[pToken] == GT) {
		nextToken();
	}
	else {
		ErrorProc(FATAL_ERROR, "relational operator");
	}
}

void FunctionCall() {
	Identifier();
	if (!isProcExisted(input[pToken - 1])) {
		pToken--;
		ErrorProc(SIGN_UNDEFINED_ERROR, input[pToken]);
		pToken++;
	}
	if (kind[pToken] == LPAR) {
		nextToken();
	}
	else {
		ErrorProc(SIGN_ABSENCE_ERROR, "(");
	}
	ArithExpression();
	if (kind[pToken] == RPAR) {
		nextToken();
	}
	else {
		ErrorProc(SIGN_ABSENCE_ERROR, ")");
	}
}

void Var() {
	Identifier();
}

void Identifier() {
	if (kind[pToken] == ID) {
		nextToken();
	}
	else {
		ErrorProc(FATAL_ERROR, "identifier");
	}
}

void parserInit() {
	char inFileName[MAX_FILENAME_LEN] = "Source.dyd";
	char outFileName[MAX_FILENAME_LEN] = "Source.dys";
	char errFileName[MAX_FILENAME_LEN] = "Source.err";
	char varFileName[MAX_FILENAME_LEN] = "Source.var";
	char proFileName[MAX_FILENAME_LEN] = "Source.pro";

	inFile = fopen(inFileName, "r");
	outFile = fopen(outFileName, "w");
	errFile = fopen(errFileName, "w");
	varFile = fopen(varFileName, "w");
	proFile = fopen(proFileName, "w");

	pToken = 0;
	lineNum = 1;
	strcpy(pProc.name, "main");
	pProc.level = 0;
	inputCount = 0;
	varCount = 0;
	proCount = 0;

	while (!feof(inFile)) {
		int i;
		char lineStr[MAX_LINE_SIZE];
		char lineName[MAX_LINE_SIZE], lineType[MAX_LINE_SIZE];
		memset(lineStr, 0, sizeof(lineStr));
		memset(lineName, 0, sizeof(lineName));
		memset(lineType, 0, sizeof(lineType));
		fgets(lineStr, MAX_LINE_SIZE, inFile);
		strncpy(lineName, lineStr, 16);
		strncpy(lineType, lineStr + 17, 2);
		lineType[2] = '\0';
		for (i = 0; lineName[i] == ' '; i++);
		strcpy(input[inputCount], lineName + i);
		kind[inputCount++] = atoi(lineType);
	}
	inputCount--;
}

void parserFinalize() {
	for (int i = 0; i < varCount; i++) {
		fprintf(varFile, "%16s  %16s  %d  %s  %d  %d\n", vars[i].name, vars[i].proc, vars[i].kind, "integer", vars[i].level, vars[i].addr);
	}
	for (int i = 0; i < proCount; i++) {
		fprintf(proFile, "%16s  %s  %d  %d  %d\n", procs[i].name, "integer", procs[i].level, procs[i].faddr, procs[i].laddr);
	}
	fseek(inFile, 0, 0);
	int c = fgetc(inFile);
	while (c != -1) {
		fputc(c, outFile);
		c = fgetc(inFile);
	}
	
	fclose(inFile);
	fclose(outFile);
	fclose(errFile);
	fclose(varFile);
	fclose(proFile);
}