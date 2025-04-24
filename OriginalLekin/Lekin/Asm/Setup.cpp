#include "StdAfx.h"

#include "AppAsm.h"
#include "Setup.h"

#include "ID.h"
#include "Misc.h"
#include "TokenSet.h"

////////////////////////////////////////////
// TSetup

namespace {
const CString keySetup = _T("Setup");
}

TSetup::TSetup(int dim)
{
    m_dim = dim;
    m_a.SetSize(dim * dim);
}

TSetup::TSetup(const TSetup& setup)
{
    m_dim = 0;
    *this = setup;
}

void TSetup::Write(CArchive& ar)
{
    CString buf;

    for (int i = 0; i < m_dim; ++i)
        for (int j = 0; j < m_dim; ++j)
        {
            int setup = GetAt(i, j);
            if (!setup) continue;
            if (buf.GetLength() > 0) buf += ' ';
            buf += TCHAR(i + 'A');
            buf += strSemicolon;
            buf += TCHAR(j + 'A');
            buf += strSemicolon;
            buf += IntToStr(setup);
        }

    if (TID::IsShort() || !buf.IsEmpty()) WriteToken(ar, 2, keySetup, buf);
}

void TSetup::Read(TTokenSet2& ts)
{
    CString buf = ts.GetAt(keySetup);
    LPCTSTR ss = buf;

    while (true)
    {
        CString x = NextToken(ss, chrSemicolon);
        if (x.IsEmpty()) break;
        CheckStatus(x);
        int x2 = toupper(x[0]) - 'A';

        CString y = NextToken(ss, chrSemicolon);
        CheckStatus(y);
        int y2 = toupper(y[0]) - 'A';

        int setup = StrToInt(NextToken(ss, ' '));
        if (setup < 0) ThrowMessage(IDP_BAD_SETUP);

        SetAtGrow(x2, y2, setup);
    }
}

void TSetup::Save(TStringWriter& sw) const
{
    if (m_dim == 0) return;
    sw.Write(keySetup + _T(" ="));
    sw.NewLine();

    for (int i = 0; i < m_dim; ++i)
    {
        CString s = i == 0 ? _T("( ") : _T("  ");
        s += _T("( ");
        for (int j = 0; j < m_dim; ++j) s += IntToStr(GetAt(i, j)) + strSpace;
        s += _T(")");
        if (i == m_dim - 1) s += _T(" )");
        sw.Write(s);
        sw.NewLine();
    }
}

void TSetup::Load(TStringSpacer& sp)
{
    SetDim(0);
    if (!sp.TestSymbol(keySetup)) return;

    sp.TestSymbolHard(_T("("));

    for (int row = 0; sp.TestSymbol(_T("(")); ++row)
    {
        for (int col = 0; !sp.TestSymbol(_T(")")); ++col)
        {
            int a = sp.ReadInt();
            CheckInt(a, 0, 9999);
            if (a == 0) continue;
            SetAtGrow(row, col, a);
        }
    }

    sp.TestSymbolHard(_T(")"));
}

TSetup& TSetup::operator=(const TSetup& setup)
{
    m_a.Copy(setup.m_a);
    m_dim = setup.m_dim;
    return *this;
}

bool TSetup::operator==(const TSetup& setup)
{
    if (m_dim != setup.m_dim) return false;
    for (int i = m_dim * m_dim; --i >= 0;)
        if (m_a[i] != setup.m_a[i]) return false;
    return true;
}

bool TSetup::operator!=(const TSetup& setup)
{
    return !(*this == setup);
}

void TSetup::Copy(const TSetup& setup)
{
    int m = min(GetDim(), setup.GetDim());
    if (m_dim > m) memset(m_a.GetData(), 0, sizeof(DWORD) * m_dim * m_dim);

    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j) SetAt(i, j, setup.GetAt(i, j));
}

int TSetup::GetDim() const
{
    return m_dim;
}

int TSetup::GetAt(int i, int j) const
{
    ASSERT(i >= 0 && j >= 0);
    return i < m_dim && j < m_dim ? m_a[i * m_dim + j] : 0;
}

void TSetup::SetDim(int dim)
{
    ASSERT(dim >= 0 && dim < 26);
    if (dim == m_dim) return;

    TSetup setup(*this);
    m_a.SetSize(0);
    m_dim = dim;
    m_a.SetSize(dim * dim);
    Copy(setup);
}

void TSetup::SetAt(int i, int j, int setup)
{
    ASSERT(i >= 0 && j < m_dim && j >= 0 && j < m_dim);
    m_a[i * m_dim + j] = setup;
}

void TSetup::SetAtGrow(int i, int j, int setup)
{
    int dim = 1 + max(i, j);
    if (m_dim < dim) SetDim(dim);
    SetAt(i, j, setup);
}
