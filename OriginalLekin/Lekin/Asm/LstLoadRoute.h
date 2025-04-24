#pragma once

class CLstLoadRoute : public CLstOwner
{
private:
    typedef CLstOwner super;

public:
    int m_total;

protected:
    virtual void OnInit();
    virtual void Draw(CDC* pDC);
    virtual int Measure(int index);
};
