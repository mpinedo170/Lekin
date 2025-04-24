#pragma once

class TTokenSet2;

class TID
{
private:
    static CString GetToken2(UINT id);

public:
    TType m_ty;

    CString m_id;
    CString m_idGen;
    CString m_comment;
    COLORREF m_color;

    TID(TType ty);
    TID(TType ty, LPCTSTR id, COLORREF color);
    virtual ~TID();

    const CString& GetGLKey() const
    {
        return m_id;
    }

    void Write(CArchive& ar);
    void Read(TTokenSet2& ts);

    void Save(TStringWriter& sw) const;
    void Load(TStringSpacer& sp);

    static CString GetToken(TType ty);
    static CString GetTokenWkc2();

    void RandomColor();
    void DrawRect(
        CDC* pDC, const CRect& rect, LPCTSTR text = NULL, bool bCross = false, bool bLate = false) const;

    COLORREF GetTextColor(bool bLate) const;
    COLORREF GetHatchColor(bool bLate) const;
    static int GetGray(COLORREF color);

    int GetDiff(COLORREF color) const;

    static TObjective m_IOFlag;
    static void SetIOFlag(TObjective IOFlag);
    static bool IsShort();

    CString GetToken() const
    {
        return GetToken(m_ty);
    }

    int GetGray() const
    {
        return GetGray(m_color);
    }
};

template <class T>
class TIDArray : public CKeyedStrArray<T>
{
private:
    typedef CKeyedStrArray<T> super;

public:
    T* FindGen(LPCTSTR id)
    {
        int index = StrTailToInt(id);
        return index >= 0 && index < GetSize() ? GetAt(index) : NULL;
    }

    int Replace(T* pT)
    {
        int index = LSearch(pT->m_id);
        if (index < 0)
        {
            return Add(pT);
        }
        else
        {
            delete (*this)[index];
            (*this)[index] = pT;
            return index;
        }
    }

    bool Exists(LPCTSTR id, T* pExclude = NULL)
    {
        return LSearch(id, pExclude) >= 0;
    }

    void AlterName(CString& name)
    {
        while (Exists(name)) ::AlterName(name);
    }

    void GenerateIDs(LPCTSTR prefix)
    {
        for (int i = 0; i < GetSize(); ++i) GetAt(i)->m_idGen.Format(_T("%s%03d"), prefix, i);
    }

    void ClearIDs()
    {
        for (int i = 0; i < GetSize(); ++i) GetAt(i)->m_idGen.Empty();
    }
};

class TCollection : public TIDArray<TID>
{};
