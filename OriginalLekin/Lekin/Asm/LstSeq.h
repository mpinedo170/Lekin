#pragma once

class CLstSeq : public CLstOwner
{
private:
    typedef CLstOwner super;

protected:
    virtual void Draw(CDC* pDC);
    virtual int Measure(int index);
};
