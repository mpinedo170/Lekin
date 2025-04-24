#pragma once

/////////////////////////////////////////////////////////////////////////////
// T3State
// Implements a boolean variable with a third ("undefined") state.
// Free conversion to bool and back.
// Conversion into bool raises assertion failure if in undefined state.

namespace AFLib {
class AFLIB T3State
{
private:
    int m_value;

public:
    T3State()
    {
        m_value = 2;
    }

    T3State(int value, int)
    {
        m_value = value;
    }

    T3State(bool value)
    {
        m_value = value ? 1 : 0;
    }

    int ToInt() const
    {
        return m_value;
    }

    operator bool() const
    {
        ASSERT(m_value == 0 || m_value == 1);
        return m_value == 1;
    }

    bool operator==(T3State state) const
    {
        return m_value == state.m_value;
    }

    bool operator==(bool state) const
    {
        return m_value == (state ? 1 : 0);
    }

    bool operator!=(T3State state) const
    {
        return m_value != state.m_value;
    }

    bool operator!=(bool state) const
    {
        return m_value != (state ? 1 : 0);
    }
};

AFLIB extern const T3State t3Start;
AFLIB extern const T3State t3Undef;
}  // namespace AFLib
