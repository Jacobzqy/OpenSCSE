#pragma once
#define MAX_FILENAME_LEN 25

#define NONE 0
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
#define ERROR 99

#define SIGN_UNDEFINED_ERROR 0
#define SIGN_UNMATCHED_ERROR 1
#define ID_OVERFLOW_ERROR 2