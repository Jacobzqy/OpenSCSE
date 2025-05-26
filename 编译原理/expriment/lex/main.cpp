#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<cstdio>
#include<cstring>
#include"parameters.h"

using namespace std;

FILE* inFile = NULL;
FILE* outFile = NULL;
FILE* errFile = NULL;

const int MAX_SIZE = 200;
const int TEXT_MAX_SIZE = 1000;

struct Tuple{
	int c;
	int value;
};

char ch, prevCh;
char strToken[MAX_SIZE];
char text[TEXT_MAX_SIZE];
int idx, position;
int lineNum = 1;

void GetChar() {
	ch = text[position++];
}

void GetBC() {
	do GetChar(); while (ch == ' ');
}

void Concat() {
	strToken[idx++] = ch;
}

bool IsLetter() {
	if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z') {
		return true;
	}
	else {
		return false;
	}
}

bool IsDigit() {
	if (ch >= '0' && ch <= '9') {
		return true;
	}
	else {
		return false;
	}
}

void Retract() {
	position--;
}

int Reserve() {
	if (strcmp(strToken, "begin") == 0) {
		return BEGIN;
	}
	else if (strcmp(strToken, "end") == 0) {
		return END;
	}
	else if (strcmp(strToken, "integer") == 0) {
		return INTEGER;
	}
	else if (strcmp(strToken, "if") == 0) {
		return IF;
	}
	else if (strcmp(strToken, "then") == 0) {
		return THEN;
	}
	else if (strcmp(strToken, "else") == 0) {
		return ELSE;
	}
	else if (strcmp(strToken, "function") == 0) {
		return FUNCTION;
	}
	else if (strcmp(strToken, "read") == 0) {
		return READ;
	}
	else if (strcmp(strToken, "write") == 0) {
		return WRITE;
	}
	else {
		return ID;
	}
}

int InsertId() {
	return 0;
}

int InsertConst() {
	return 0;
}

int read() {
	position = 0;
	return fgets(text, 1000, inFile) == NULL ? 0 : 1;
}

void ProcError(int errorType) {
	if (errorType == SIGN_UNDEFINED_ERROR) {
		fprintf(errFile, "LINE%d£ºillegal character '%c'\n", lineNum, ch);
	}
	else if (errorType == SIGN_UNMATCHED_ERROR) {
		fprintf(errFile, "LINE%d£ºexpected '=' after ':'\n", lineNum);
	}
	else if (errorType == ID_OVERFLOW_ERROR) {
		fprintf(errFile, "LINE%d£ºidentifier '%s' is too long(Don't exceed 16 characters)\n", lineNum, strToken);
	}
}

Tuple lex(FILE* stream) {
	memset(strToken, 0, sizeof(strToken));
	idx = 0;
	int code;
	Tuple returnValue;
	GetBC();
	if(IsLetter()){
		while (IsLetter() || IsDigit()) {
			Concat();
			GetChar();
		}
		Retract();
		code = Reserve();
		if (code == ID) {
			if (strlen(strToken) > 16) {
				ProcError(ID_OVERFLOW_ERROR);
				returnValue.c = ERROR;
				return returnValue;
			}
			else {
				returnValue.c = ID;
				returnValue.value = InsertId();
				return returnValue;
			}
		}
		else {
			returnValue.c = code;
			returnValue.value = 0;
			return returnValue;
		}
	}
	else if (IsDigit()) {
		while (IsDigit()) {
			Concat();
			GetChar();
		}
		Retract();
		returnValue.c = CONST;
		returnValue.value = InsertConst();
		return returnValue;
	}
	else if (ch == '=') {
		returnValue.c = EQUAL;
		returnValue.value = 0;
		Concat();
		return returnValue;
	}
	else if (ch == '<') {
		Concat();
		GetChar();
		if (ch == '>') {
			returnValue.c = NEQUAL;
			returnValue.value = 0;
			Concat();
			return returnValue;
		}
		else if (ch == '=') {
			returnValue.c = LE;
			returnValue.value = 0;
			Concat();
			return returnValue;
		}
		Retract();
		returnValue.c = LT;
		returnValue.value = 0;
		return returnValue;
	}
	else if (ch == '>') {
		Concat();
		GetChar();
		if (ch == '=') {
			returnValue.c = GE;
			returnValue.value = 0;
			Concat();
			return returnValue;
		}
		Retract();
		returnValue.c = GT;
		returnValue.value = 0;
		return returnValue;
	}
	else if (ch == '-') {
		returnValue.c = SUB;
		returnValue.value = 0;
		Concat();
		return returnValue;
	}
	else if (ch == '*') {
		returnValue.c = MUL;
		returnValue.value = 0;
		Concat();
		return returnValue;
	}
	else if (ch == ':') {
		Concat();
		GetChar();
		if (ch == '=') {
			returnValue.c = ASSIGN;
			returnValue.value = 0;
			Concat();
			return returnValue;
		}
		Retract();
		ProcError(SIGN_UNMATCHED_ERROR);
		returnValue.c = ERROR;
		return returnValue;
	}
	else if (ch == '(') {
		returnValue.c = LPAR;
		returnValue.value = 0;
		Concat();
		return returnValue;
	}
	else if (ch == ')') {
		returnValue.c = RPAR;
		returnValue.value = 0;
		Concat();
		return returnValue;
	}
	else if (ch == ';') {
		returnValue.c = SEM;
		returnValue.value = 0;
		Concat();
		return returnValue;
	}
	else if (ch == '\n') {
		strcpy(strToken, "EOLN");
		returnValue.c = EOLN;
		returnValue.value = 0;
		lineNum++;
		if(!read())returnValue.c = EOF;
		return returnValue;
	}
	else if (ch == '\0') {
		memset(strToken, 0, sizeof(strToken));
		strcpy(strToken, "EOF");
		returnValue.c = EOF;
		return returnValue;
	}
	else {
		ProcError(SIGN_UNDEFINED_ERROR);
		returnValue.c = ERROR;
		return returnValue;
	}
}

void lexerInit();

void lexerFinalize();

int main() {
	lexerInit();
	Tuple ret;
	ret.c = 0;

	read();
	while (ret.c != EOF) {
		ret = lex(inFile);
		fprintf(outFile, "%16s %2d\n", strToken, ret.c);
	}
	
	lexerFinalize();
	return 0;
}

void lexerInit() {
	char inFileName[MAX_FILENAME_LEN] = "Source.pas";
	char outFileName[MAX_FILENAME_LEN] = "../parser/Source.dyd";
	char errFileName[MAX_FILENAME_LEN] = "Source.err";

	inFile = fopen(inFileName, "r");
	outFile = fopen(outFileName, "w");
	errFile = fopen(errFileName, "w");
}

void lexerFinalize() {
	fclose(inFile);
	fclose(outFile);
	fclose(errFile);
}