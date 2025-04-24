#pragma once

class TOperation;
class TJob;

class TRoute
{
private:
    CStringArray m_a;
    friend class CDlgOppy;

public:
    void Load(const TJob& job);
    void Save(TJob& job);
    void CreateDefault();

    TRoute& operator=(const TRoute& route);
    bool operator==(const TRoute& route);
    bool operator!=(const TRoute& route);
    bool IsValid();

    void Clear();
    void Add(LPCTSTR idWkc, int proc, TCHAR status);
    void Add(const TOperation* pOp);

    CString GetWkcID(int index) const;
    int GetProcTime(int index) const;
    TCHAR GetStatus(int index) const;

    int GetCount() const;
    int GetTotal() const;

    void DrawRect(CDC* pDC, const CRect& rect) const;
};
