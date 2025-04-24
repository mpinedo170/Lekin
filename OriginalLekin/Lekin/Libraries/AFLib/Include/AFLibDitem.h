#pragma once

namespace AFLibIO {
class TStringSpacer;
class TStringWriter;
}  // namespace AFLibIO

namespace AFLibGui {
class TSuperText;
}

/////////////////////////////////////////////////////////////////////////////
// TDitem
// A member of a "directory" tree view

namespace AFLibGui {
class AFLIB TDitem
{
private:
    // helper for LoadPath() and LoadPathNoTest()
    void PrivateLoadPath(LPCTSTR path, bool bTest);

protected:
    CString m_title;  // item title
    CString m_dir;    // directory of the item (contains backslashes)

    // divide path into directory and title
    void LoadPath(LPCTSTR path);

    // LoadPath(), do NOT test the title
    void LoadPathNoTest(LPCTSTR path);

    // test title: not empty, no starting or ending whitespace
    virtual void TestTitle() const;

public:
    virtual ~TDitem();

    // create a clone of the Ditem
    virtual TDitem* Clone() const = 0;

    // get the name of this Ditem, like "scan"
    virtual const CString& GetDitemType() const = 0;

    // save into a string
    virtual void Save(AFLibIO::TStringWriter& sw) const = 0;

    // screen i/o
    virtual void DDX(CDataExchange* pDX) = 0;

    // describe the criterion in ft
    virtual void Describe(TSuperText& superText) const;

    // check if tho ditems are identical
    virtual bool Equals(const TDitem& ditem) const;

    // set ditem title
    void SetTitle(LPCTSTR title);

    // set ditem dir
    void SetDir(LPCTSTR dir);

    // get title
    const CString& GetTitle() const
    {
        return m_title;
    }

    // get "directory"
    const CString& GetDir() const
    {
        return m_dir;
    }

    // get path
    CString GetPath() const
    {
        return m_dir + m_title;
    }

    // key for sorting
    CString GetGLKey() const
    {
        return GetPath();
    }
};
}  // namespace AFLibGui
