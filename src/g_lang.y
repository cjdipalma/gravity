/**
 * g_lang.y
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
  #include "g_ir.h"
%}

%union {
  long l;
  double d;
  char *s;
  struct t { long a, b, c; } t;
}

%left '+' '-'
%left '*' '/' '%'
%right N

%token G_MODULE
%token G_PREFIX
%token G_PRECISION
%token G_COSTFNC
%token G_BATCH
%token G_ETA
%token G_INPUT
%token G_OUTPUT
%token G_HIDDEN
%token G_FLOAT
%token G_DOUBLE
%token G_FIXED
%token G_QUADRATIC
%token G_EXPONENTIAL
%token G_CROSS_ENTROPY
%token G_RELU
%token G_LINEAR
%token G_SOFTMAX
%token G_SIGMOID
%token <s> G_LONG
%token <s> G_REAL
%token <s> G_STRING
%token G_ERROR

%type <t> _precision1_
%type <l> _costfnc1_
%type <l> _activation_
%type <l> _expr_
%type <l> _long_
%type <d> _real_
%type <s> _string_

%%

top
  : _phrase_ { if (g_ir_top()) YYABORT; }
  | G_ERROR  { yyerror(0); YYABORT;  }
  ;

_phrase_
  : _phrase1_
  | _phrase1_ _phrase_
  ;

_phrase1_
  : _module_
  | _prefix_
  | _precision_
  | _costfnc_
  | _batch_
  | _eta_
  | _input_
  | _output_
  | _hidden_
  ;

/*---------------------------------------------------------------------------------------------------------------------------------------*/

_module_
  : G_MODULE _string_ { if (g_ir_module($2)) YYABORT; }
  ;

_prefix_
  : G_PREFIX _string_ { if (g_ir_prefix($2)) YYABORT; }
  ;

_precision_
  : G_PRECISION _precision1_ _precision1_ { if (g_ir_precision($2.a, $2.b, $2.c, $3.a, $3.b, $3.c)) YYABORT; }
  ;

_precision1_
  : G_FLOAT                           { struct t t = {  0,  0, G_IR_PRECISION_FLOAT  }; $$ = t; }
  | G_DOUBLE                          { struct t t = {  0,  0, G_IR_PRECISION_DOUBLE }; $$ = t; }
  | G_FIXED '[' _expr_ ',' _expr_ ']' { struct t t = { $3, $5, G_IR_PRECISION_FIXED  }; $$ = t; }
  ;

_costfnc_
  : G_COSTFNC _costfnc1_ { if (g_ir_costfnc($2)) YYABORT; }
  ;

_costfnc1_
  : G_QUADRATIC     { $$ = G_IR_COSTFNC_QUADRATIC;     }
  | G_EXPONENTIAL   { $$ = G_IR_COSTFNC_EXPONENTIAL;   }
  | G_CROSS_ENTROPY { $$ = G_IR_COSTFNC_CROSS_ENTROPY; }
  ;

_batch_
  : G_BATCH _expr_ { if (g_ir_batch($2)) YYABORT; }
  ;

_eta_
  : G_ETA _real_ { if (g_ir_eta($2)) YYABORT; }
  ;

_input_
  : G_INPUT _expr_ { if (g_ir_input($2)) YYABORT; }
  ;

_output_
  : G_OUTPUT _expr_ _activation_ { if (g_ir_output($2, $3)) YYABORT; }
  ;

_hidden_
  : G_HIDDEN _expr_ _activation_ { if (g_ir_hidden($2, $3)) YYABORT; }
  ;

_activation_
  : G_RELU    { $$ = G_IR_ACTIVATION_RELU;    }
  | G_LINEAR  { $$ = G_IR_ACTIVATION_LINEAR;  }
  | G_SOFTMAX { $$ = G_IR_ACTIVATION_SOFTMAX; }
  | G_SIGMOID { $$ = G_IR_ACTIVATION_SIGMOID; }
  ;

/*---------------------------------------------------------------------------------------------------------------------------------------*/

_expr_
  : _long_             { $$ = $1;      }
  | '(' _expr_ ')'     { $$ = $2;      }
  | '-' _expr_ %prec N { $$ = -$2;     }
  | _expr_ '+' _expr_  { $$ = $1 + $3; }
  | _expr_ '-' _expr_  { $$ = $1 - $3; }
  | _expr_ '*' _expr_  { $$ = $1 * $3; }
  | _expr_ '/' _expr_  { if(!$3) { yyerror("divide by zero near line %d", yylineno); G_DEBUG(G_ERR_SYNTAX); YYABORT; } $$ = $1 / $3; }
  | _expr_ '%' _expr_  { if(!$3) { yyerror("divide by zero near line %d", yylineno); G_DEBUG(G_ERR_SYNTAX); YYABORT; } $$ = $1 % $3; }
  ;

_long_
  : G_LONG
  {
    char *e = 0;
    $$ = strtol($1, &e, 10);
    if ((e == $1) || (*e)) {
      yyerror("invalid numeric value '%s' near line %d", $1, yylineno);
      G_DEBUG(G_ERR_SYNTAX);
      YYABORT;
    }
  }
  ;

_real_
  : G_REAL
  {
    char *e = 0;
    $$ = strtod($1, &e);
    if ((e == $1) || (*e)) {
      yyerror("invalid numeric value '%s' near line %d", $1, yylineno);
      G_DEBUG(G_ERR_SYNTAX);
      YYABORT;
    }
  }
  ;

_string_
  : G_STRING
  {
    if (!($$ = g_ir_strdup($1 + 1))) {
      yyerror("out of memory");
      G_DEBUG(0);
      YYABORT;
    }
    $$[g_strlen($$)-1] = 0;
  }
  ;

%%
