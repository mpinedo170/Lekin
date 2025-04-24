#pragma once

class TTokenSet2;

class TSetup
{
private:
    CUIntArray m_a;
    int m_dim;

    void Copy(const TSetup& setup);

public:
    explicit TSetup(int dim = 0);
    TSetup(const TSetup& setup);

    TSetup& operator=(const TSetup& setup);
    bool operator==(const TSetup& setup);
    bool operator!=(const TSetup& setup);

    void Write(CArchive& ar);
    void Read(TTokenSet2& ts);

    void Save(TStringWriter& sw) const;
    void Load(TStringSpacer& sp);

    int GetDim() const;
    int GetAt(int i, int j) const;

    void SetDim(int dim);
    void SetAt(int i, int j, int setup);
    void SetAtGrow(int i, int j, int setup);
};
