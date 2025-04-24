#pragma once

/////////////////////////////////////////////////////////////////////////////
// TRandom
// Random number generator.
// User can use the same generator throughout the program (TRandom::Get())
//   or create several independent ones by calling constructors.

namespace AFLibMath {
class AFLIB TRandom
{
private:
    int m_iseed;  // seed

    // class for destruction of m_pRndObj
    struct TInit
    {
        ~TInit();
    };

    // standard TRandom object
    static TRandom* m_pRndObj;

    // destructor of this object deletes m_pRndObj
    static TInit m_init;

public:
    TRandom();
    explicit TRandom(int iseed);

    // get standard random number generator
    static TRandom& Get();

    // set seed -- fixes following sequence of random numbers
    void SetSeed(int iseed);

    // get random number from U(0,1)
    double Uniform();

    // get integer number uniformly distributed in { 0, ..., x-1 }
    int Uniform(int x);

    // get random number from U(from, to)
    double Uniform(double from, double to);
};
}  // namespace AFLibMath
