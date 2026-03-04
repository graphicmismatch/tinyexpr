/*
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 *
 * Copyright (c) 2015-2020 Lewis Van Winkle
 *
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgement in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "tinyexpr.h"
#include <stdio.h>
#include "minctest.h"


typedef struct {
    const char *expr;
    te_real answer;
} test_case;

typedef struct {
    const char *expr1;
    const char *expr2;
} test_equ;



void test_results() {
    test_case cases[] = {
        {"1", 1},
        {"1 ", 1},
        {"(1)", 1},

        {"pi", 3.14159},
        {"atan(1)*4 - pi", 0},
        {"e", 2.71828},

        {"2+1", 2+1},
        {"(((2+(1))))", 2+1},
        {"3+2", 3+2},

        {"3+2+4", 3+2+4},
        {"(3+2)+4", 3+2+4},
        {"3+(2+4)", 3+2+4},
        {"(3+2+4)", 3+2+4},

        {"3*2*4", 3*2*4},
        {"(3*2)*4", 3*2*4},
        {"3*(2*4)", 3*2*4},
        {"(3*2*4)", 3*2*4},

        {"3-2-4", 3-2-4},
        {"(3-2)-4", (3-2)-4},
        {"3-(2-4)", 3-(2-4)},
        {"(3-2-4)", 3-2-4},

        {"3/2/4", 3.0/2.0/4.0},
        {"(3/2)/4", (3.0/2.0)/4.0},
        {"3/(2/4)", 3.0/(2.0/4.0)},
        {"(3/2/4)", 3.0/2.0/4.0},

        {"(3*2/4)", 3.0*2.0/4.0},
        {"(3/2*4)", 3.0/2.0*4.0},
        {"3*(2/4)", 3.0*(2.0/4.0)},

        {"asin sin .5", 0.5},
        {"sin asin .5", 0.5},
        {"ln exp .5", 0.5},
        {"exp ln .5", 0.5},

        {"asin sin-.5", -0.5},
        {"asin sin-0.5", -0.5},
        {"asin sin -0.5", -0.5},
        {"asin (sin -0.5)", -0.5},
        {"asin (sin (-0.5))", -0.5},
        {"asin sin (-0.5)", -0.5},
        {"(asin sin (-0.5))", -0.5},

        {"log10 1000", 3},
        {"log10 1e3", 3},
        {"log10 1000", 3},
        {"log10 1e3", 3},
        {"log10(1000)", 3},
        {"log10(1e3)", 3},
        {"log10 1.0e3", 3},
        {"10^5*5e-5", 5},

#ifdef TE_NAT_LOG
        {"log 1000", 6.9078},
        {"log e", 1},
        {"log (e^10)", 10},
#else
        {"log 1000", 3},
#endif

        {"ln (e^10)", 10},
        {"100^.5+1", 11},
        {"100 ^.5+1", 11},
        {"100^+.5+1", 11},
        {"100^--.5+1", 11},
        {"100^---+-++---++-+-+-.5+1", 11},

        {"100^-.5+1", 1.1},
        {"100^---.5+1", 1.1},
        {"100^+---.5+1", 1.1},
        {"1e2^+---.5e0+1e0", 1.1},
        {"--(1e2^(+(-(-(-.5e0))))+1e0)", 1.1},

        {"sqrt 100 + 7", 17},
        {"sqrt 100 * 7", 70},
        {"sqrt (100 * 100)", 100},

        {"1,2", 2},
        {"1,2+1", 3},
        {"1+1,2+2,2+1", 3},
        {"1,2,3", 3},
        {"(1,2),3", 3},
        {"1,(2,3)", 3},
        {"-(1,(2,3))", -3},

        {"2^2", 4},
        {"pow(2,2)", 4},

        {"atan2(1,1)", 0.7854},
        {"atan2(1,2)", 0.4636},
        {"atan2(2,1)", 1.1071},
        {"atan2(3,4)", 0.6435},
        {"atan2(3+3,4*2)", 0.6435},
        {"atan2(3+3,(4*2))", 0.6435},
        {"atan2((3+3),4*2)", 0.6435},
        {"atan2((3+3),(4*2))", 0.6435},

    };


    int i;
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const te_real answer = cases[i].answer;

        int err;
        const te_real ev = te_interp(expr, &err);
        lok(!err);
        lfequal(ev, answer);

        if (err) {
            printf("FAILED: %s (%d)\n", expr, err);
        }
    }
}


void test_syntax() {
    test_case errors[] = {
        {"", 1},
        {"1+", 2},
        {"1)", 2},
        {"(1", 2},
        {"1**1", 3},
        {"1*2(+4", 4},
        {"1*2(1+4", 4},
        {"a+5", 1},
        {"!+5", 1},
        {"_a+5", 1},
        {"#a+5", 1},
        {"1^^5", 3},
        {"1**5", 3},
        {"sin(cos5", 8},
    };


    int i;
    for (i = 0; i < sizeof(errors) / sizeof(test_case); ++i) {
        const char *expr = errors[i].expr;
        const int e = errors[i].answer;

        int err;
        const te_real r = te_interp(expr, &err);
        lequal(err, e);
        lok(r != r);

        te_expr *n = te_compile(expr, 0, 0, &err);
        lequal(err, e);
        lok(!n);

        if (err != e) {
            printf("FAILED: %s\n", expr);
        }

        const te_real k = te_interp(expr, 0);
        lok(k != k);
    }
}


void test_nans() {

    const char *nans[] = {
        "0/0",
        "1%0",
        "1%(1%0)",
        "(1%0)%1",
        "fac(-1)",
        "ncr(2, 4)",
        "ncr(-2, 4)",
        "ncr(2, -4)",
        "npr(2, 4)",
        "npr(-2, 4)",
        "npr(2, -4)",
    };

    int i;
    for (i = 0; i < sizeof(nans) / sizeof(const char *); ++i) {
        const char *expr = nans[i];

        int err;
        const te_real r = te_interp(expr, &err);
        lequal(err, 0);
        lok(r != r);

        te_expr *n = te_compile(expr, 0, 0, &err);
        lok(n);
        lequal(err, 0);
        const te_real c = te_eval(n);
        lok(c != c);
        te_free(n);
    }
}


void test_infs() {

    const char *infs[] = {
            "1/0",
            "log(0)",
            "pow(2,10000000)",
            "fac(300)",
            "ncr(300,100)",
            "ncr(300000,100)",
            "ncr(300000,100)*8",
            "npr(3,2)*ncr(300000,100)",
            "npr(100,90)",
            "npr(30,25)",
    };

    int i;
    for (i = 0; i < sizeof(infs) / sizeof(const char *); ++i) {
        const char *expr = infs[i];

        int err;
        const te_real r = te_interp(expr, &err);
        lequal(err, 0);
        lok(r == r + 1);

        te_expr *n = te_compile(expr, 0, 0, &err);
        lok(n);
        lequal(err, 0);
        const te_real c = te_eval(n);
        lok(c == c + 1);
        te_free(n);
    }
}


void test_variables() {

    te_real x, y, test;
    te_variable lookup[] = {{"x", &x}, {"y", &y}, {"te_st", &test}};

    int err;

    te_expr *expr1 = te_compile("cos x + sin y", lookup, 2, &err);
    lok(expr1);
    lok(!err);

    te_expr *expr2 = te_compile("x+x+x-y", lookup, 2, &err);
    lok(expr2);
    lok(!err);

    te_expr *expr3 = te_compile("x*y^3", lookup, 2, &err);
    lok(expr3);
    lok(!err);

    te_expr *expr4 = te_compile("te_st+5", lookup, 3, &err);
    lok(expr4);
    lok(!err);

    for (y = 2; y < 3; ++y) {
        for (x = 0; x < 5; ++x) {
            te_real ev;

            ev = te_eval(expr1);
            lfequal(ev, cosl(x) + sinl(y));

            ev = te_eval(expr2);
            lfequal(ev, x+x+x-y);

            ev = te_eval(expr3);
            lfequal(ev, x*y*y*y);

            test = x;
            ev = te_eval(expr4);
            lfequal(ev, x+5);
        }
    }

    te_free(expr1);
    te_free(expr2);
    te_free(expr3);
    te_free(expr4);



    te_expr *expr5 = te_compile("xx*y^3", lookup, 2, &err);
    lok(!expr5);
    lok(err);

    te_expr *expr6 = te_compile("tes", lookup, 3, &err);
    lok(!expr6);
    lok(err);

    te_expr *expr7 = te_compile("sinn x", lookup, 2, &err);
    lok(!expr7);
    lok(err);

    te_expr *expr8 = te_compile("si x", lookup, 2, &err);
    lok(!expr8);
    lok(err);
}



#define cross_check(a, b) do {\
    if ((b)!=(b)) break;\
    expr = te_compile((a), lookup, 2, &err);\
    lfequal(te_eval(expr), (b));\
    lok(!err);\
    te_free(expr);\
}while(0)

void test_functions() {

    te_real x, y;
    te_variable lookup[] = {{"x", &x}, {"y", &y}};

    int err;
    te_expr *expr;

    for (x = -5; x < 5; x += .2) {
        cross_check("abs x", fabsl(x));
        cross_check("acos x", acosl(x));
        cross_check("asin x", asinl(x));
        cross_check("atan x", atanl(x));
        cross_check("ceil x", ceill(x));
        cross_check("cos x", cosl(x));
        cross_check("cosh x", coshl(x));
        cross_check("exp x", expl(x));
        cross_check("floor x", floorl(x));
        cross_check("ln x", logl(x));
        cross_check("log10 x", log10l(x));
        cross_check("sin x", sinl(x));
        cross_check("sinh x", sinhl(x));
        cross_check("sqrt x", sqrtl(x));
        cross_check("tan x", tanl(x));
        cross_check("tanh x", tanhl(x));

        for (y = -2; y < 2; y += .2) {
            if (fabsl(x) < 0.01) break;
            cross_check("atan2(x,y)", atan2l(x, y));
            cross_check("pow(x,y)", powl(x, y));
        }
    }
}


te_real sum0() {
    return 6;
}
te_real sum1(te_real a) {
    return a * 2;
}
te_real sum2(te_real a, te_real b) {
    return a + b;
}
te_real sum3(te_real a, te_real b, te_real c) {
    return a + b + c;
}
te_real sum4(te_real a, te_real b, te_real c, te_real d) {
    return a + b + c + d;
}
te_real sum5(te_real a, te_real b, te_real c, te_real d, te_real e) {
    return a + b + c + d + e;
}
te_real sum6(te_real a, te_real b, te_real c, te_real d, te_real e, te_real f) {
    return a + b + c + d + e + f;
}
te_real sum7(te_real a, te_real b, te_real c, te_real d, te_real e, te_real f, te_real g) {
    return a + b + c + d + e + f + g;
}


void test_dynamic() {

    te_real x, f;
    te_variable lookup[] = {
        {"x", &x},
        {"f", &f},
        {"sum0", sum0, TE_FUNCTION0},
        {"sum1", sum1, TE_FUNCTION1},
        {"sum2", sum2, TE_FUNCTION2},
        {"sum3", sum3, TE_FUNCTION3},
        {"sum4", sum4, TE_FUNCTION4},
        {"sum5", sum5, TE_FUNCTION5},
        {"sum6", sum6, TE_FUNCTION6},
        {"sum7", sum7, TE_FUNCTION7},
    };

    test_case cases[] = {
        {"x", 2},
        {"f+x", 7},
        {"x+x", 4},
        {"x+f", 7},
        {"f+f", 10},
        {"f+sum0", 11},
        {"sum0+sum0", 12},
        {"sum0()+sum0", 12},
        {"sum0+sum0()", 12},
        {"sum0()+(0)+sum0()", 12},
        {"sum1 sum0", 12},
        {"sum1(sum0)", 12},
        {"sum1 f", 10},
        {"sum1 x", 4},
        {"sum2 (sum0, x)", 8},
        {"sum3 (sum0, x, 2)", 10},
        {"sum2(2,3)", 5},
        {"sum3(2,3,4)", 9},
        {"sum4(2,3,4,5)", 14},
        {"sum5(2,3,4,5,6)", 20},
        {"sum6(2,3,4,5,6,7)", 27},
        {"sum7(2,3,4,5,6,7,8)", 35},
    };

    x = 2;
    f = 5;

    int i;
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const te_real answer = cases[i].answer;

        int err;
        te_expr *ex = te_compile(expr, lookup, sizeof(lookup)/sizeof(te_variable), &err);
        lok(ex);
        lfequal(te_eval(ex), answer);
        te_free(ex);
    }
}


te_real clo0(void *context) {
    if (context) return *((te_real*)context) + 6;
    return 6;
}
te_real clo1(void *context, te_real a) {
    if (context) return *((te_real*)context) + a * 2;
    return a * 2;
}
te_real clo2(void *context, te_real a, te_real b) {
    if (context) return *((te_real*)context) + a + b;
    return a + b;
}

te_real cell(void *context, te_real a) {
    te_real *c = context;
    return c[(int)a];
}

void test_closure() {

    te_real extra;
    te_real c[] = {5,6,7,8,9};

    te_variable lookup[] = {
        {"c0", clo0, TE_CLOSURE0, &extra},
        {"c1", clo1, TE_CLOSURE1, &extra},
        {"c2", clo2, TE_CLOSURE2, &extra},
        {"cell", cell, TE_CLOSURE1, c},
    };

    test_case cases[] = {
        {"c0", 6},
        {"c1 4", 8},
        {"c2 (10, 20)", 30},
    };

    int i;
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const te_real answer = cases[i].answer;

        int err;
        te_expr *ex = te_compile(expr, lookup, sizeof(lookup)/sizeof(te_variable), &err);
        lok(ex);

        extra = 0;
        lfequal(te_eval(ex), answer + extra);

        extra = 10;
        lfequal(te_eval(ex), answer + extra);

        te_free(ex);
    }


    test_case cases2[] = {
        {"cell 0", 5},
        {"cell 1", 6},
        {"cell 0 + cell 1", 11},
        {"cell 1 * cell 3 + cell 4", 57},
    };

    for (i = 0; i < sizeof(cases2) / sizeof(test_case); ++i) {
        const char *expr = cases2[i].expr;
        const te_real answer = cases2[i].answer;

        int err;
        te_expr *ex = te_compile(expr, lookup, sizeof(lookup)/sizeof(te_variable), &err);
        lok(ex);
        lfequal(te_eval(ex), answer);
        te_free(ex);
    }
}

void test_optimize() {

    test_case cases[] = {
        {"5+5", 10},
        {"pow(2,2)", 4},
        {"sqrt 100", 10},
        {"pi * 2", 6.2832},
    };

    int i;
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const te_real answer = cases[i].answer;

        int err;
        te_expr *ex = te_compile(expr, 0, 0, &err);
        lok(ex);

        /* The answer should be know without
         * even running eval. */
        lfequal(ex->value, answer);
        lfequal(te_eval(ex), answer);

        te_free(ex);
    }
}

void test_pow() {
#ifdef TE_POW_FROM_RIGHT
    test_equ cases[] = {
        {"2^3^4", "2^(3^4)"},
        {"-2^2", "-(2^2)"},
        {"--2^2", "(2^2)"},
        {"---2^2", "-(2^2)"},
        {"-(2*1)^2", "-(2^2)"},
        {"-2^2", "-4"},
        {"2^1.1^1.2^1.3", "2^(1.1^(1.2^1.3))"},
        {"-a^b", "-(a^b)"},
        {"-a^-b", "-(a^-b)"},
        {"1^0", "1"},
        {"(1)^0", "1"},
        {"-(2)^2", "-(2^2)"}
        /* TODO POW FROM RIGHT IS STILL BUGGY
        {"(-2)^2", "4"},
        {"(-1)^0", "1"},
        {"(-5)^0", "1"},
        {"-2^-3^-4", "-(2^(-(3^-4)))"}*/
    };
#else
    test_equ cases[] = {
        {"2^3^4", "(2^3)^4"},
        {"-2^2", "(-2)^2"},
        {"(-2)^2", "4"},
        {"--2^2", "2^2"},
        {"---2^2", "(-2)^2"},
        {"-2^2", "4"},
        {"2^1.1^1.2^1.3", "((2^1.1)^1.2)^1.3"},
        {"-a^b", "(-a)^b"},
        {"-a^-b", "(-a)^(-b)"},
        {"1^0", "1"},
        {"(1)^0", "1"},
        {"(-1)^0", "1"},
        {"(-5)^0", "1"},
        {"-2^-3^-4", "((-2)^(-3))^(-4)"}
    };
#endif

    te_real a = 2, b = 3;

    te_variable lookup[] = {
        {"a", &a},
        {"b", &b}
    };

    int i;
    for (i = 0; i < sizeof(cases) / sizeof(test_equ); ++i) {
        const char *expr1 = cases[i].expr1;
        const char *expr2 = cases[i].expr2;

        te_expr *ex1 = te_compile(expr1, lookup, sizeof(lookup)/sizeof(te_variable), 0);
        te_expr *ex2 = te_compile(expr2, lookup, sizeof(lookup)/sizeof(te_variable), 0);

        lok(ex1);
        lok(ex2);

        te_real r1 = te_eval(ex1);
        te_real r2 = te_eval(ex2);

        fflush(stdout);
        const int olfail = lfails;
        lfequal(r1, r2);
        if (olfail != lfails) {
            printf("Failed expression: %s <> %s\n", expr1, expr2);
        }

        te_free(ex1);
        te_free(ex2);
    }

}

void test_combinatorics() {
    test_case cases[] = {
            {"fac(0)", 1},
            {"fac(0.2)", 1},
            {"fac(1)", 1},
            {"fac(2)", 2},
            {"fac(3)", 6},
            {"fac(4.8)", 24},
            {"fac(10)", 3628800},

            {"ncr(0,0)", 1},
            {"ncr(10,1)", 10},
            {"ncr(10,0)", 1},
            {"ncr(10,10)", 1},
            {"ncr(16,7)", 11440},
            {"ncr(16,9)", 11440},
            {"ncr(100,95)", 75287520},

            {"npr(0,0)", 1},
            {"npr(10,1)", 10},
            {"npr(10,0)", 1},
            {"npr(10,10)", 3628800},
            {"npr(20,5)", 1860480},
            {"npr(100,4)", 94109400},
    };


    int i;
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const te_real answer = cases[i].answer;

        int err;
        const te_real ev = te_interp(expr, &err);
        lok(!err);
        lfequal(ev, answer);

        if (err) {
            printf("FAILED: %s (%d)\n", expr, err);
        }
    }
}


int main(int argc, char *argv[])
{
    lrun("Results", test_results);
    lrun("Syntax", test_syntax);
    lrun("NaNs", test_nans);
    lrun("INFs", test_infs);
    lrun("Variables", test_variables);
    lrun("Functions", test_functions);
    lrun("Dynamic", test_dynamic);
    lrun("Closure", test_closure);
    lrun("Optimize", test_optimize);
    lrun("Pow", test_pow);
    lrun("Combinatorics", test_combinatorics);
    lresults();

    return lfails != 0;
}
