#ifndef T_STACK_H
#define T_STACK_H

#define NULL_STACK_POSITION -1

template <class T>
class Stack
{
    T* pointer[SIZE_STACK];
    // array of pointers. Makes sence when
    // the size of T is big and we don't want
    // to rewrite it

    short position;

public:
    Stack()
    {
        position = NULL_STACK_POSITION;
    };
    void Flush()
    {
        position = NULL_STACK_POSITION;
    };
    int Empty()
    {
        if (position == NULL_STACK_POSITION)
            return (TRUE);
        else
            return (FALSE);
    };

    void Push(T* element)
    {
        if (position >= (SIZE_STACK - 1))
        {
            printf("error: Stack overflow\n");
            exit(1);
        }
        else
        {
            position++;
            pointer[position] = element;
        };
    };
    T* Pop()
    {
        if (Empty())
        {
            printf("error: Stack empty\n");
            exit(1);
            return NULL;
        }
        else
        {
            position--;
            return (pointer[(position + 1)]);
        };
    };
};

#endif
