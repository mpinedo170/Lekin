#define INCLUDE_AFLIB_IO
#define INCLUDE_AFLIB_GUI
#define INCLUDE_AFLIB_GUI_COMMON_CTL
#define INCLUDE_AFLIB_GUI_ATL
#define INCLUDE_AFLIB_MATH
#define USE_AFLIB_NAMESPACES
#include "../Basic.h"
#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CAppAsm: main application class

class CDocTmpSmart;
class CDocTmpAlg;
class CDocSmart;
struct TPlugin;
class CDlgJob;
class CDlgPerform;
class CFrmMain;
class CFrmSmart;
class CViewSmart;

class TOperation;
class TSchedule;
class TWorkcenterArray;
class TMachineArray;
class TJobArray;
class TScheduleArray;

class CAppAsm : public CAppAF
{
private:
    typedef CAppAF super;

    HWND m_hWndCon;

    CString m_faceFont;
    int m_sizeFont;
    int m_sortSch;
    int m_sortJob;
    int m_maskObj;
    bool m_bNormalRun;

public:
    CAppAsm();
    CDocTmpSmart* m_pTmpMch;
    CDocTmpSmart* m_pTmpJob;
    CDocTmpSmart* m_pTmpSeq;
    CDocTmpSmart* m_pTmpGantt;
    CDocTmpSmart* m_pTmpObj;
    CDocTmpSmart* m_pTmpLog;
    CDocTmpAlg* m_pTmpAlg;

    TJobArray* m_pArrJobSorted;
    TScheduleArray* m_pArrSchSorted;

    CDlgJob* m_pDlgJob;
    CDlgPerform* m_pDlgPerform;

    // global initialization
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    void RegisterLekin();
    void UnregisterLekin();

    // access
private:
    CRecentFileList* GetFileList(UINT nID, int& index);

public:
    CDocTmpSmart* GetTmp(int ID);
    CDocSmart* GetDoc(int ID);
    CFrmSmart* GetFrm(int ID);
    CViewSmart* GetView(int ID);
    CFrmMain* GetMainFrame();
    CFrmSmart* GetActiveFrame();
    TJobArray& GetJobList();
    TScheduleArray& GetSchList();

    // floaters
    void OpenDlgJob();
    void CloseDlgJob();
    void CloseDlgPerform();
    void CloseFloaters();
    void UpdateFloaters(TOperation* pOp = NULL);

    // window
private:
    void OpenMainFrame();
    int FillWindows(bool bIconic, bool arrBOk[]);
    void TileHor(const CRect& rect, bool arrBOk[]);
    bool PrivateOpenConsole();

public:
    bool OpenConsole();
    bool CloseConsole();

    CRect ArrangeIcons();
    void ShowSomething();
    void ActivateSomething();
    bool ZoomFit();
    void SavePosition();

    // font
    void CreateAppFont(CFont& font, CDC* pDC, double coef = 1, bool bBold = false, bool bItalic = false);
    void CreateSysFont(CFont& font, CDC* pDC);

    // confirmation
    bool AskConfirmation(UINT promptID, int level, LPCTSTR prompt = NULL);
    bool AskConfirmDel(UINT ids2);

    // edit tree views
    void EditWkc(TWorkcenterArray& arrWkc);
    void EditMch(TMachineArray& arrMch);
    void EditJob(TJobArray& arrJob);

    // workspace save/load
    void FinishSaving(bool bSaveAs);
    bool ClearWorkspace();
    void LoadWorkspace(LPCTSTR lpszPathName, bool bSpecial = false);

    // misc
private:
    void ResetObjMask();
    void ShowAboutDlg(bool bAnyway);

public:
    int ScheduleMenu();
    void SetSchActive(TSchedule* pSch, bool bModify);
    bool CheckSequences(bool bCancel);
    void RunExtAlgor(TPlugin& plug);

    afx_msg void OnAppAbout();
    afx_msg void OnWktNew();
    afx_msg void OnWktSave();
    afx_msg void OnWktSaveAs();
    afx_msg void OnWktPrint();
    afx_msg void OnWktSettings();
    afx_msg void OnToolOptions();
    afx_msg void OnWinCascade();
    afx_msg void OnWinTile();
    afx_msg void OnToolPrf();
    afx_msg void OnRuleManual();
    afx_msg void OnToolObj();
    afx_msg void OnLogAdd();
    afx_msg void OnLogDel();
    afx_msg void OnLogEdit();
    afx_msg void OnUpdateLogs(CCmdUI* pCmdUI);
    afx_msg void OnUpdateLogDel(CCmdUI* pCmdUI);
    afx_msg void OnUpdateWktSave(CCmdUI* pCmdUI);
    afx_msg void OnToolPlugin();
    afx_msg void OnToolGeneric();
    afx_msg void OnToolConvert();

    afx_msg void OnWin(UINT nID);
    afx_msg void OnWinShow(UINT nID);
    afx_msg void OnJobSort(UINT nID);
    afx_msg void OnUpdateJobSort(CCmdUI* pCmdUI);
    afx_msg void OnSchSort(UINT nID);
    afx_msg void OnUpdateSchSort(CCmdUI* pCmdUI);

    afx_msg void OnRule(UINT nID);
    afx_msg void OnLogEntry(UINT nID);
    afx_msg void OnPlugin(UINT nID);
    afx_msg void OnUpdatePlugin(CCmdUI* pCmdUI);
    afx_msg void OnUpdateRecentFileMenu(CCmdUI* pCmdUI);
    afx_msg void OnOpenRecentFile(UINT nID);

    DECLARE_MESSAGE_MAP()
};

extern const CString secVit;
extern const CString secHeu;
extern const CString secView;
extern const CString secSettings;

extern const CString keyLicense;
extern const CString keyFace;
extern const CString keySize;
extern const CString keySortJob;
extern const CString keySortSch;

extern const CString keyID;
extern const CString keyCount;
extern const CString keyConfirm;
extern const CString keyObj;

extern const CString strFntTimes;
extern const CString strFntFancy;

extern const CString keyX;
extern const CString keyY;
extern const CString keyCX;
extern const CString keyCY;
extern const CString keyState;

extern const CString tokRGB;
extern const CString tokOper;
extern const CString tokComment;
extern const CString tokStatus;
extern const CString tokBatch;
extern const CString tokRelease;
extern const CString tokDue;
extern const CString tokWeight;

extern CAppAsm theApp;
extern CString ExePath;
extern CString AlgPath;
extern CString Company;
extern CString License;

enum TMode
{
    modeAdd,
    modeSingle,
    modeMultiple
};

enum
{
    timerTimeout = 2,
    timerUpdate,
    timerSize,
    timerStyle,
    timerRoute
};

enum TTypeView
{
    tvwMch,
    tvwJob,
    tvwSeq,
    tvwGantt,
    tvwObj,
    tvwLog,
    tvwN
};

const int MRU_COUNT = 4;
const int MX9 = 9999;

const int MaxJob = 3000;
const int MaxWkc = 50;
const int MaxMch = 750;

enum TShop
{
    OneShop,
    FlowShop,
    JobShop
};

enum TType
{
    tyJob,
    tyWkc,
    tyMch,
    tySch,
    tySpec
};

enum TObjective
{
    objTime,
    objCmax,
    objLmax,
    objUj,
    objCj,
    objTj,
    objWCj,
    objWTj,
    objN
};

enum TRule
{
    ruleATCS,
    ruleEDD,
    ruleMS,
    ruleFCFS,
    ruleLPT,
    ruleSPT,
    ruleWSPT,
    ruleCR,
    ruleN
};

const int ALL_OBJ = (1 << objN) - 2;

#define EnumerateWkc                                         \
    {                                                        \
        for (int wi = 0; wi < arrWorkcenter.GetSize(); ++wi) \
        {                                                    \
            TWorkcenter* pWkc = arrWorkcenter[wi];

#define EnumerateJob                                  \
    {                                                 \
        for (int ji = 0; ji < arrJob.GetSize(); ++ji) \
        {                                             \
            TJob* pJob = arrJob[ji];

#define EnumerateSch                                       \
    {                                                      \
        for (int si = 0; si < arrSchedule.GetSize(); ++si) \
        {                                                  \
            TSchedule* pSch = arrSchedule[si];

#define EnumerateEnd \
    }                \
    }
