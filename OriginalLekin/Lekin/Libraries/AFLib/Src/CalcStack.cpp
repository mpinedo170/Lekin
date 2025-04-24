#include "StdAfx.h"

#include "AFLibCalcStack.h"

using AFLibCalc::TCalcStack;

TCalcStack::TCalcStack()
{  // always keep 1 element in the stack to avoid memory freeing and reallocation
    m_arrStk.SetSize(1, 1 << 8);
    m_arrGlobal.SetSize(0, 1 << 8);
}

TCalcStack::TCalcStack(const TCalcStack& stk)
{
    m_arrStk.Copy(stk.m_arrStk);
    m_arrGlobal.Copy(stk.m_arrGlobal);
}

void TCalcStack::Pop(int count)
{
    m_arrStk.SetSize(GetHeight() - count);
}

void TCalcStack::ClearStack()
{
    m_arrStk.SetSize(1);
}

void TCalcStack::ExtendVar(int ref)
{
    if (m_arrGlobal.GetSize() <= ref) m_arrGlobal.SetSize(ref + 1);
}
