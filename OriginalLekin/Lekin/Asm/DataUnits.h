#pragma once

class TIDDataUnit : public TStrDataUnit
{
private:
    typedef TStrDataUnit super;

    UINT m_ids2;

protected:
    void DDV(CDataExchange* pDX, bool bUndefOk) const;

public:
    TIDDataUnit(LPCTSTR key, UINT idc, CString& data, LPCTSTR init, UINT ids2);
    virtual void Load(LPCTSTR section) const;
    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};

class TCbsIDDataUnit : public TIDDataUnit
{
private:
    typedef TIDDataUnit super;

public:
    TCbsIDDataUnit(LPCTSTR key, UINT idc, CString& data, UINT ids2);
    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};

class TCountDataUnit : public TSpnDataUnit
{
private:
    typedef TSpnDataUnit super;

    int m_maxCount;
    int m_curCount;
    UINT m_ids2;

public:
    TCountDataUnit(
        LPCTSTR key, UINT idc, UINT idcSpin, int& data, int init, int maxCount, int curCount, UINT ids2);
    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};

class TStatusDataUnit : public TSDataUnit
{
private:
    typedef TSDataUnit super;

public:
    TStatusDataUnit(LPCTSTR key, UINT idc, CString& data);
    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};
