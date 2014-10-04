

struct Stack {
    void *data;
    struct Stack *top;
};


struct Stack* create_stack();
int push(struct Stack*, void*);
void* pop(struct Stack*);
int empty(struct Stack*);
int delete_stack(struct Stack*,int);
