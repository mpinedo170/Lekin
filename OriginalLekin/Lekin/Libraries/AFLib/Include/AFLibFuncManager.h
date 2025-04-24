#pragma once

#include "AFLibArrays.h"
#include "AFLibDefine.h"
#include "AFLibFuncPrototype.h"
#include "AFLibMaps.h"

/////////////////////////////////////////////////////////////////////////////
// TFuncManager class
// Holds function identifiers with their prototypes for TCompiler

namespace AFLibCalc {
class TCalculator;

class AFLIB TFuncManager
{
private:
    DEFINE_COPY_AND_ASSIGN(TFuncManager);

public:
    // single identifier definition
    struct TIDDef
    {
        TFuncPrototype m_fpr;  // function prototype
        int m_pos;             // position in calculator

        TIDDef(const TFuncPrototype& fpr, int pos) : m_fpr(fpr), m_pos(pos)
        {}
    };

private:
    // the actual map
    AFLib::CS2PMap<TIDDef> m_map;

    bool m_bIgnoreCase;         // ignore lettercase in identifiers?
    TIDDef* m_pIdCurFunc;       // currently compiled function
    int m_maxLocalRef;          // max local var in the current function
    int m_nUndefFunc;           // number of undefined functions
    AFLib::CIntArray m_arrPos;  // array of positions for previously undefined functions

    // helper for PrivateStartFunc()
    void PrivateStartFuncHelper(LPCTSTR id, LPCTSTR id2, const TFuncPrototype& fpr, int pos);

    // helper for StartFunc() and DeclareFunc()
    void PrivateStartFunc(LPCTSTR id, const TFuncPrototype& fpr, int pos);

public:
    explicit TFuncManager(bool bIgnoreCase);
    ~TFuncManager();

    // add a function and enter into it; throws if ID already exists
    void StartFunc(LPCTSTR id, const TFuncPrototype& fpr, TCalculator& calc);

    // exit function
    void EndFunc(TCalculator& calc);

    // declare a function without a definition
    void DeclareFunc(LPCTSTR id, const TFuncPrototype& fpr);

    // get definition for identifier
    const TIDDef* GetFuncDef(LPCTSTR id) const;

    // add local variable to the current function
    void AddLocalVar(int ref);

    // translate a "function prototype" reference into a real function offset
    int GetNewFuncPos(int ref) const;

    // get definition for identifier
    const TIDDef* GetCurFuncDef() const
    {
        return m_pIdCurFunc;
    }

    // ignore lettercase?
    bool GetIC() const
    {
        return m_bIgnoreCase;
    }
};
}  // namespace AFLibCalc
