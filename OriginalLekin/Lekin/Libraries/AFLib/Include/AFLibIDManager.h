#pragma once

#include "AFLibArrays.h"
#include "AFLibDefine.h"
#include "AFLibMaps.h"
#include "AFLibSimpleType.h"

/////////////////////////////////////////////////////////////////////////////
// TIDManager class
// Holds identifiers with their definitions; supports nesting through a linked list.
// Also supports nested loops and break/continue operators for them.

namespace AFLibCalc {
class TCalculator;

class AFLIB TIDManager
{
private:
    DEFINE_COPY_AND_ASSIGN(TIDManager);

public:
    // single identifier definition
    struct TIDDef
    {
        TSimpleType m_stp;  // variable type (reference is on)
        int m_ref;          // local reference
        bool m_bGlobal;     // is the variable global?

        TIDDef(const TSimpleType& stp, int ref, bool bGlobal) : m_stp(stp), m_ref(ref), m_bGlobal(bGlobal)
        {}
    };

private:
    // the actual map
    AFLib::CS2PMap<TIDDef> m_map;

    // optional label of this ID map
    CString m_label;

    // pointer to the upper-level map
    const TIDManager* m_pIDMPrev;

    // pointer to the map that defines the uppermost loop
    TIDManager* m_pIDMLoop;

    // pointer to the map that defines the second-uppermost loop
    TIDManager* m_pIDMLoopPrev;

    // array of "goto break" commands
    AFLib::CIntArray m_arrBreak;

    // array of "goto continue" commands
    AFLib::CIntArray m_arrContinue;

    bool m_bGlobal;          // is this the "global" ID manager
    bool m_bIgnoreCase;      // ignore lettercase in identifiers?
    bool m_bAcceptContinue;  // "continue" operator accepted (false for "switch")
    int m_nRef;              // current number of variables

    // helper for constructors
    void PrivateInit(const TIDManager* pIDMPrev, bool bIgnoreCase);

protected:
    // helper for FindLoopedIDM()
    TIDManager* ProtectedFindLoopedIDM(LPCTSTR label);

public:
    TIDManager(const TIDManager* pIDMPrev, bool bIgnoreCase);
    TIDManager(const TIDManager* pIDMPrev, bool bIgnoreCase, LPCTSTR label);
    ~TIDManager();

    // does not let this IDM accept the "continue" operator
    void RejectContinue()
    {
        m_bAcceptContinue = false;
    }

    // make the ID Manager local
    void SetLocal();

    // add a variable; throws if ID already exists
    int AddVar(LPCTSTR id, TSimpleType stp, int nRef = -1);

    // get definition for identifier
    const TIDDef* GetVarDef(LPCTSTR id) const;

    // convert defined variables into function arguments (with negative ref's)
    void ConvertFuncArgs();

    // find IDM for Break or Continue
    TIDManager* FindLoopedIDM(LPCTSTR label, bool bContinue);

    // process the "break" operator
    void AddBreak(int pos);

    // process the "continue" operator
    void AddContinue(int pos);

    // the loop is finished; update the goto's
    void FinalizeLoop(TCalculator& calc, int posBreak, int posContinue);

    // check if the ID Manager is global
    bool IsGlobal() const
    {
        return m_bGlobal;
    }

    // can define functions on this level?
    bool IsTopLevel() const
    {
        return m_pIDMPrev == NULL;
    }

    // return the current number of variables
    int GetNRef() const
    {
        return m_nRef;
    }

    // ignore lettercase?
    bool GetIC() const
    {
        return m_bIgnoreCase;
    }

    // accepts "continue" (or only "break")?
    bool AcceptsContinue() const
    {
        return m_bAcceptContinue;
    }
};
}  // namespace AFLibCalc
