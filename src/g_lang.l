/**
 * g_lang.l
 * Copyright (C) Tony Givargis, 2019
 *
 * This file is part of The Gravity Compiler.
 *
 * The Gravity Compiler is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. The Gravity Compiler is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with Foobar.
 * If not, see <https://www.gnu.org/licenses/>.
 */

%{
#define YY_NO_LEAKS
#include "y.tab.h"
#include "g_ir.h"
%}

%option never-interactive
%option nodefault
%option noyywrap
%option yylineno
%option nounput
%option noinput

%%
".module"                        { return G_MODULE;                    }
".prefix"                        { return G_PREFIX;                    }
".precision"                     { return G_PRECISION;                 }
".costfnc"                       { return G_COSTFNC;                   }
".batch"                         { return G_BATCH;                     }
".eta"                           { return G_ETA;                       }
".input"                         { return G_INPUT;                     }
".output"                        { return G_OUTPUT;                    }
".hidden"                        { return G_HIDDEN;                    }
"float"                          { return G_FLOAT;                     }
"double"                         { return G_DOUBLE;                    }
"fixed"                          { return G_FIXED;                     }
"quadratic"                      { return G_QUADRATIC;                 }
"exponential"                    { return G_EXPONENTIAL;               }
"cross_entropy"                  { return G_CROSS_ENTROPY;             }
"relu"                           { return G_RELU;                      }
"linear"                         { return G_LINEAR;                    }
"softmax"                        { return G_SOFTMAX;                   }
"sigmoid"                        { return G_SIGMOID;                   }
","                              { return ',';                         }
"["                              { return '[';                         }
"]"                              { return ']';                         }
"("                              { return '(';                         }
")"                              { return ')';                         }
"+"                              { return '+';                         }
"-"                              { return '-';                         }
"*"                              { return '*';                         }
"/"                              { return '/';                         }
"%"                              { return '%';                         }
-?[0-9]+                         { yylval.s = yytext; return G_LONG;   }
-?[0-9]+"."[0-9]* |
-?"."[0-9]+ |
-?[0-9]+E[-+]?[0-9]+ |
-?[0-9]+"."[0-9]*E[-+]?[0-9]+ |
-?"."[0-9]*E[-+]?[0-9]+          { yylval.s = yytext; return G_REAL;   }
'(\\.|''|[^'\n])*' |
\"(\\.|\"\"|[^"\n])*\"           { yylval.s = yytext; return G_STRING; }
[ \t\n]                          ;
\/\/.*\n                         ;
.                                { return G_ERROR;                     }
%%

void yyerror(const char *format, ...) {
        va_list va;

        fprintf(stderr, "error: ");
        va_start(va, format);
        vfprintf(stderr, format ? format : "syntax error", va);
        va_end(va);
        fprintf(stderr, " near line %d\n", yylineno);
}
