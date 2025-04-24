#include "StdAfx.h"

#include "AFLibClusterSet.h"

#include "AFLibDlgRun.h"
#include "AFLibRecordsetAF.h"
#include "AFLibRowNC.h"

using AFLibMath::TClusterSet;
using namespace AFLibDB;

TClusterSet::TCluster::TCluster(LPCTSTR ID)
{
    m_ID = ID;
    m_mapMember.Include(ID);
}

void TClusterSet::TCluster::AddMember(LPCTSTR ID)
{
    m_mapMember.Include(ID);
    if (m_ID > ID) m_ID = ID;
}

//***********************************************

TClusterSet::TPair::TPair()
{}

TClusterSet::TPair::TPair(LPCTSTR ID, LPCTSTR IDParent) : m_ID(ID), m_IDParent(IDParent)
{}

//***********************************************

TClusterSet::TClusterSet()
{}

TClusterSet::~TClusterSet()
{
    m_setCluster.DestroyAll();
}

void TClusterSet::AddPair(LPCTSTR ID1, LPCTSTR ID2)
{
    TCluster* pCluster1 = m_mapCluster.GetAt(ID1);
    TCluster* pCluster2 = m_mapCluster.GetAt(ID2);

    if (pCluster1 == NULL)
    {
        pCluster1 = new TCluster(ID1);
        m_mapCluster.SetAt(ID1, pCluster1);
        m_setCluster.Include(pCluster1);
    }

    if (pCluster2 == NULL)
    {
        pCluster1->AddMember(ID2);
        m_mapCluster.SetAt(ID2, pCluster1);
    }
    else if (pCluster2 != pCluster1)
    {
        for (POSITION pos = pCluster2->m_mapMember.GetStartPosition(); pos != NULL;)
        {
            const CString& ID = pCluster2->m_mapMember.GetNextAssoc(pos);
            pCluster1->AddMember(ID);
            m_mapCluster.SetAt(ID, pCluster1);
        }

        m_setCluster.Destroy(pCluster2);
    }
}

void TClusterSet::ReadRs(const CRecordsetAF& rs)
{
    for (int r = 0; r < rs.GetRowCount(); ++r)
    {
        CRow row(rs, r);
        AddPair(row.GetString(0), row.GetString(1));
        if (rs.m_pDlgRun != NULL) rs.m_pDlgRun->AddClick();
    }
}

void TClusterSet::GetResults(CArray<TPair>& arrPair) const
{
    for (POSITION pos = m_mapCluster.GetStartPosition(); pos != NULL;)
    {
        CString ID;
        TCluster* pCluster = NULL;
        m_mapCluster.GetNextAssoc(pos, ID, pCluster);
        arrPair.Add(TPair(ID, pCluster->m_ID));
    }
}

void TClusterSet::GetResults(CRecordsetAF& rs) const
{
    for (POSITION pos = m_mapCluster.GetStartPosition(); pos != NULL;)
    {
        CString ID;
        TCluster* pCluster = NULL;
        m_mapCluster.GetNextAssoc(pos, ID, pCluster);

        CRowNC row(rs, AFLibDB::CRowNC::newRow);
        row.SetString(0, ID);
        row.SetString(1, pCluster->m_ID);

        if (rs.m_pDlgRun != NULL) rs.m_pDlgRun->AddClick();
    }
}
