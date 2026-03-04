#include "tinyexpr.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    const char *c = "sqrt(5^2+7^2+11^2+(8-2)^2)";
    te_real r = te_interp(c, 0);
    printf("The expression:\n\t%s\nevaluates to:\n\t%Lf\n", c, r);
    return 0;
}
