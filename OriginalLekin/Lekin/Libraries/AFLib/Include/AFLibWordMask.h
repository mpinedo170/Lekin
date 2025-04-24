#pragma once

namespace AFLib
{ class AFLIB TWordMask
  { public:
      class TMask
      { private:
          bool m_bNot;
          CString m_data;

          int FindIndex(TCHAR ch) const;

        public:
          TMask();

          void SetNot(bool bNot);
          void Add(TCHAR ch);
          void Add(TCHAR chFrom, TCHAR chTo);

          bool Matches(TCHAR ch) const;
          bool IsStar() const;
      };

    private:
      CArray<TMask> m_arrMask;

    public:
      explicit TWordMask(LPCTSTR mask);

      int GetMaskCount() const
        { return m_arrMask.GetSize(); }

      const TMask& GetMask(int index) const
        { return m_arrMask[index]; }
  };
}
