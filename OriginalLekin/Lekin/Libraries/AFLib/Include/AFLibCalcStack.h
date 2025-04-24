#pragma once

#include "AFLibOleVariantArray.h"

/////////////////////////////////////////////////////////////////////////////
// TCalcStack class
// Stack and local variables.
// Separate from TCalculator so that the same Calculator could be executed
//   on different threads simultaneously.
// A dummy element is always kept in the bootom of the stack to make sure
//   memory is not deallocated from the array.

namespace AFLibCalc {
class AFLIB TCalcStack
{
public:
    AFLib::COleVariantArray m_arrStk;     // stack
    AFLib::COleVariantArray m_arrGlobal;  // global variables

    TCalcStack();
    TCalcStack(const TCalcStack& stk);

    // get the height of the stack
    int GetHeight() const
    {
        return m_arrStk.GetSize();
    }

    // get the top element of the stack
    const COleVariant& GetTop() const
    {
        return m_arrStk[GetHeight() - 1];
    }
    COleVariant& GetTop()
    {
        return m_arrStk[GetHeight() - 1];
    }

    // is the stack empty?
    bool IsEmpty() const
    {
        return GetHeight() <= 1;
    }

    // pop one or more elements from the stack
    void Pop(int count = 1);

    // clear the stack (keep global variables in place)
    void ClearStack();

    // make sure variable exists in the local variable array
    void ExtendVar(int ref);
};
}  // namespace AFLibCalc
