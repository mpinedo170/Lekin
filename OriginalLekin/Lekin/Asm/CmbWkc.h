#pragma once

class CCmbWkc : public CCmbOwner
{
private:
    typedef CCmbOwner super;

protected:
    virtual void OnInit();
    virtual void Draw(CDC* pDC);
    virtual int Measure(int index);
};
