#pragma once

#include "AFLibArrays.h"
#include "AFLibDefine.h"

namespace AFLib {
class T3State;
}

/////////////////////////////////////////////////////////////////////////////
// TDataMass
// Set of data units.  Loads data from the registry, updates, saves...

namespace AFLibGui {
class TDataUnit;

class AFLIB TDataMass
{
private:
    DEFINE_COPY_AND_ASSIGN(TDataMass);

    // array of data (user interface) units
    AFLib::CSmartArray<TDataUnit> m_arrDU;

protected:
    CString m_section;  // registry section to load/save the data units

    // add a data unit
    void AddDU(TDataUnit* pDu);

    // insert a data unit in the beginning
    void InsertDU(TDataUnit* pDu);

    // number of data units in the array
    int GetDUCount();

    // index-th data unit
    const TDataUnit* GetDU(int index);

    // find data unit for a control IDC
    const TDataUnit* FindDU(UINT idc);

    // m_section not empty & pDu->IsLoadable()
    bool IsDULoadable(const TDataUnit* pDu);

    // assign the "unneeded" value (m_zero)
    virtual void ZeroDU(const TDataUnit* pDu);

    // write data unit into registry
    virtual void SaveDU(const TDataUnit* pDu);

    // DDX and data integrity tests
    virtual void DDXDU(CDataExchange* pDX, const TDataUnit* pDu);

    // check if this data unit is necessary (in combination with other data)
    //   e.g., if Top Holdings are not checked, the number of top assets is not needed
    virtual bool IsDUNeeded(const TDataUnit* pDu);

    // check if this data unit serves a "multiple selection"
    virtual bool IsDUMultiSel(const TDataUnit* pDu);

    // check if this data unit is undefined
    virtual bool IsDUUndefined(const TDataUnit* pDu);

public:
    TDataMass();
    ~TDataMass();

    // initialize all data with default values
    virtual void InitData();

    // load all data from registry
    virtual void LoadData();

    // save all data into registry
    virtual void SaveData();

    // Do DDX and data integrity tests for a subset of items
    virtual void DDXData(CDataExchange* pDX, int iStart, int iEnd, bool bGoodOnly);

    // reset data that is "not needed"
    virtual void ValidateData();

    // the following functions fill "multiple selection" data
    // if all items in "selection" are identical, dataDlg is set to that value;
    // otherwise it is set to an "undefined" state

    static void FillMultiSel(int selCount, int& dataDlg, int dataItem);
    static void FillMultiSel(int selCount, AFLib::T3State& dataDlg, bool dataItem);
    static void FillMultiSel(int selCount, CString& dataDlg, LPCTSTR dataItem);
    static void FillMultiSel(int selCount, CString& dataDlg, TCHAR dataItem);
    static void FillMultiSel(int selCount, double& dataDlg, double dataItem);

    // the following functions read "multiple selection" data
    // if dataDlg is undefined, dataItem is unchanged;
    // otherwise dataItem is set to dataDlg

    static void ReadMultiSel(int dataDlg, int& dataItem);
    static void ReadMultiSel(const AFLib::T3State& dataDlg, bool& dataItem);
    static void ReadMultiSel(LPCTSTR dataDlg, CString& dataItem);
    static void ReadMultiSel(LPCTSTR dataDlg, TCHAR& dataItem);
    static void ReadMultiSel(double dataDlg, double& dataItem);
};
}  // namespace AFLibGui
