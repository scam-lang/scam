#include <stdlib.h>
#include <assert.h>

#include "stack.h"

struct Stack*
create_stack()
{
    struct Stack *s = (struct Stack*)malloc(sizeof(struct Stack));
    if( s == 0) {
        exit(-1);
    }
    s->top = NULL;
    return s;
    }

int
push(struct Stack* s, int item)
    {
    if( s == 0) 
        {
        return -1;
        }

    struct Stack *n = (struct Stack*)malloc(sizeof(struct Stack));
    if( n == 0 )
    {
        return -1;
    }
    n->data = item;
    n->top = s->top;

    s->top = n;

    return 0;
    }

int
pop(struct Stack *s)
    {

    if( s == 0)
        {
        return 0;
        }

    int d;
    struct Stack* n;

    // Get data
    n = s->top;
    d = n->data;

    // Fixup
    s->top = n->top;

    // Cleanup
    free(n);

    return d;
    }

int
empty(struct Stack *s)
    {
    if( s != 0) 
    {
        return s->top == NULL;
    }
    return -1;
    }

int
delete_stack(struct Stack *s)
    {
    if( s == 0 ) return -1;

    // While we have something on the stack remove it.
    while( s->top != 0)
        {
        // Get data
        struct Stack *n = s->top;

        // Fixup stack
        s->top = n->top;

        // Free node
        free(n);
        }
    free(s);
    return 0;
    }
