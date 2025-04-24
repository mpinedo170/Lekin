#pragma once

#include "AFLibDefine.h"
#include "AFLibTestMaps.h"

namespace AFLibDB {
class CRecordsetAF;
}

/////////////////////////////////////////////////////////////////////////////
// TClusterSet
// Turns a set of match pairs into a set of clusters

namespace AFLibMath {
class AFLIB TClusterSet
{
private:
    DEFINE_COPY_AND_ASSIGN(TClusterSet);

    // internal structure to keep clusters
    struct TCluster
    {
        CString m_ID;                  // cluster ID
        AFLib::CSTestMap m_mapMember;  // cluster members

        explicit TCluster(LPCTSTR ID);
        void AddMember(LPCTSTR ID);
    };

    // custer map (contains duplicate objects)
    AFLib::CS2PMap<TCluster> m_mapCluster;

    // cluster set: no duplicates
    AFLib::CPtrTestMap<TCluster> m_setCluster;

public:
    TClusterSet();
    ~TClusterSet();

    // add association, merge clusters
    void AddPair(LPCTSTR ID1, LPCTSTR ID2);

    // read recordset of pairs
    void ReadRs(const AFLibDB::CRecordsetAF& rs);

    // output structure
    struct TPair
    {
        CString m_ID;        // item ID
        CString m_IDParent;  // ID of the containing cluster

        TPair();
        TPair(LPCTSTR ID, LPCTSTR IDParent);
    };

    // get the total number of clusters
    int GetClusterCount() const
    {
        return m_setCluster.GetCount();
    }

    // get the number of elements
    int GetElementCount() const
    {
        return m_mapCluster.GetCount();
    }

    // get results in an array
    void GetResults(CArray<TPair>& arrPair) const;

    // get results in a recordset
    void GetResults(AFLibDB::CRecordsetAF& rs) const;
};
}  // namespace AFLibMath
