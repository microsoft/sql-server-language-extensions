#include <stdio.h>
#include <R.h>
#include <Rembedded.h>
#include <Rdefines.h>
#include <R_ext/Parse.h>
#include "Rextension.h"
using namespace std;

int executeUserScript(char *script);

extern "C"
{
    int initR(int argc, char *argv[])
    {
        Rf_initEmbeddedR(argc, argv);
        int error = executeUserScript(argv[0]);
        Rf_endEmbeddedR(0);
        return error;
    }
}

int executeUserScript(char *script)
{
    if(!script)
    {
        return 0;
    }

    SEXP cmdSexp, cmdexpr = R_NilValue;
    ParseStatus status;
    int error = 0;

    // create and evaluate
    PROTECT(cmdSexp = mkString(script));
    PROTECT(cmdexpr = R_ParseVector(cmdSexp, -1, &status, R_NilValue));
    if ( status != PARSE_OK ) {
        UNPROTECT(2);
        // handle error
        printf("Error parsing user script with status: %d", status);
        return status;
    }

    for(int i = 0; i < length(cmdexpr); i++)
    {
         R_tryEval(VECTOR_ELT(cmdexpr, i), R_GlobalEnv, &error);
         if (error)
         {
             printf("Error evaluating : %d", error);
             UNPROTECT(2);
             return error;
         }
    }

    UNPROTECT(2);
    return error;
}
