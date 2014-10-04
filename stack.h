

struct Stack {
    int data;
    struct Stack *top;
};


struct Stack* create_stack();
int push(struct Stack*, int);
int  pop(struct Stack*);
int empty(struct Stack*);
int delete_stack(struct Stack*);
