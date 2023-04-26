%option noyywrap bison-bridge bison-locations

%{
#include <stdio.h>
#include <stdlib.h>

#define TAG_IDENT 1
#define TAG_NUMERIC 2
#define TAG_OPERATION 3
#define TAG_ERROR 4

//Идентификаторы: последовательности латинских букв, начинающиеся с гласной буквы. 
//Числовые литералы: последовательности десятичных цифр, перед которыми может стоять знак «минус». 
//Операции: «--», «<», «<=».

char *tag_names[] = {
    "END_OF_PROGRAM", "IDENT", "NUMERIC", "OPERATION", "ERROR"
};

typedef struct Position Position; 

struct Position {
    int line, pos, index;
};

void print_pos(Position * p) {
    printf("(%d,%d)", p->line, p->pos);
}

struct Fragment {
    Position starting, following;
};

typedef struct Fragment YYLTYPE;
typedef struct Fragment Fragment; 

void print_frag(Fragment *f) {
    print_pos(&(f->starting));
    printf(" - ");
    print_pos(&(f->following));
}

union Token {
    long numericToken;
    int identToken;
    char *operationToken;  
};

typedef union Token YYSTYPE;

int continued;
struct Position cur;

#define YY_USER_ACTION           \
{                                \
    int i;                       \
    if (!continued)              \
        yylloc->starting = cur;  \
    continued = 0;               \
    for (i = 0; i < yyleng; i++) \
    {                            \
        if (yytext[i] == '\n')   \
        {                        \
            cur.line++;          \
            cur.pos = 1;         \
        }                        \
        else                     \
            cur.pos++;           \
        cur.index++;             \
    }                            \
    yylloc->following = cur;     \
}

typedef struct{
    int size;
    char** names;
} Map;

void create_ident_tabel(Map * t){
    t->size = 0;
    t->names = NULL;
}

int add_ident(Map* tabel, char* name){
    for (int i = 0; i < tabel->size; i++){
        if (strcmp(name, tabel->names[i]) == 0){
            return i;
        }
    }

    tabel->size++;
    if (tabel->size == 1){
        tabel->names = (char**)malloc(sizeof(char*) * (tabel->size));
    }
    else {
        tabel->names = (char**)realloc(tabel->names, sizeof(char*) * (tabel->size));
    }
    tabel->names[tabel->size - 1] = (char*)malloc(sizeof(char)*(strlen(name) + 1));
    strcpy(tabel->names[tabel->size - 1], name);
    return tabel->size-1;
}   

Map ident_map;
Map numeric_map;  

void init_scanner(const char* file) {
    continued = 0;
    cur.line = 1;
    cur.pos = 1;
    cur.index = 0;
    create_ident_tabel(&numeric_map);
    create_ident_tabel(&ident_map);
    yyin = fopen(file, "r");
}

void err(char *msg) {
    printf("Error ");
    print_pos(&cur);
    printf(": %s\n", msg);
}
%}

IDENT       [aeiouy|AEIOUY][a-zA-Z]+
NUMERIC     [-]?[0-9]+
OPERATION     (--|<|<=)

%% 

[\n\t ]+

{OPERATION} {
                yylval->operationToken = yytext;
                return TAG_OPERATION;
            }

{IDENT}     {
                yylval->identToken = add_ident(&ident_map, yytext);
                return TAG_IDENT;
            }

{NUMERIC}   {
                yylval->numericToken = atol(yytext);
                return TAG_NUMERIC;
            }

<<EOF>>     return 0;

.           {
                err("unexpected character");
                return TAG_ERROR;
            }

%%

int main(int argc, const char **argv) {
    init_scanner(argv[1]);

    int tag;
    YYSTYPE value;
    YYLTYPE coords;

    while ((tag = yylex(&value, &coords)) != 0) {
        if (tag != 0 && tag != TAG_ERROR) {
            printf("%s ", tag_names[tag]);
            print_frag(&coords);
            if (strcmp(tag_names[tag], "IDENT") == 0) {
                printf(": %d\n", value.identToken);
            } else if (strcmp(tag_names[tag], "NUMERIC") == 0) {
                printf(": %ld\n", value.numericToken);
            } else if (strcmp(tag_names[tag], "OPERATION") == 0) {
                printf(": %s\n", value.operationToken);
            }
        }
    }

    return 0;
}