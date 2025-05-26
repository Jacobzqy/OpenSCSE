#pragma once
// Memory
#define MAX_SIZE 100
#define MAX_STACK_SIZE 10
#define MAX_LINE_SIZE 25
#define MAX_FILENAME_LEN 25

// Kind
#define BEGIN 1
#define END 2
#define INTEGER 3
#define IF 4
#define THEN 5
#define ELSE 6
#define FUNCTION 7
#define READ 8
#define WRITE 9
#define ID 10
#define CONST 11
#define EQUAL 12
#define NEQUAL 13
#define LE 14
#define LT 15
#define GE 16
#define GT 17
#define SUB 18
#define MUL 19
#define ASSIGN 20
#define LPAR 21
#define RPAR 22
#define SEM 23
#define EOLN 24
#define EOF 25

// Var
#define VARIABLE 0
#define FORMAL_PARAMETER 1

// ErrorProc
#define SIGN_UNDEFINED_ERROR 0
#define SIGN_REDEFINED_ERROR 1
#define SIGN_UNMATCHED_ERROR 2
#define SIGN_ABSENCE_ERROR 3
#define FATAL_ERROR 4