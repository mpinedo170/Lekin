// date.h
//

#pragma once

const int days_in_mo[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const int min_year = 1992;  // note: min_year%4 = 0
const int day1 = 3;         // January 1, 1992 = Wednesday

/* TDate class

   This class manages time information. The minimum data = 1/1/92.
   Commands
   ========
   TDate(mm,dd,yy,hh,mn)
   TDate(double)
 - Set(mm,dd,yy,hh=0,mn=0) : set date
 - Set(fx)                 : set date; fx=days from 1/1/92 (min_year)
 i Day()        : return day; 0=Sun, 1=Mon, 2=Tue, ...
 i Date()       : return date
 i Month()      : return month
 i Year()       : return year
 i Hour()       : return hour
 i Minute()     : return minute
 d Double()     : return days(double) from midnight of Jan 1, 1992
 - Char(sx)     : put the date in LPTSTR sx (**len(sx)>=16)
 d operator-(xx): return days(double) difference from xx (TDate) to current time
 x operator-(fx): return time(TDate) of current time - fx days
 x operator+(fx): return time(TDate) of current time + fx days
 - Now()        : get the current time from the machine
*/

class TDate
{
private:
    int iDate,    // date   1-28,29,30, or 31
        iMonth,   // month  1-12
        iYear,    // year     1992-..
        iHour,    // hour   1-24
        iMinute;  // minute 1-60
public:
    TDate();
    TDate(int iMonth, int iDate, int iYear, int iHour = 0, int iMinute = 0);
    TDate(double x);
    TDate(const TDate& dt);
    void Set(int iMonth, int iDate, int iYear, int iHour = 0, int iMinute = 0);  // 0:not valid TDate
    void Set(double x);
    int Day();  // 1=Mon,2=Tue,..,0=Sun
    int Date()
    {
        return iDate;
    };
    int Month()
    {
        return iMonth;
    };
    int Year()
    {
        return iYear;
    };
    int Hour()
    {
        return iHour;
    };
    int Minute()
    {
        return iMinute;
    };
    double Double();  // days from midnight of Jan 1, 1995
    void Char(LPTSTR szX);
    double operator-(TDate& x);
    TDate& operator-(double x);
    TDate& operator-(int x)
    {
        return operator-((double)x);
    };
    TDate& operator-(long x)
    {
        return operator-((double)x);
    };
    TDate& operator+(double x);
    TDate& operator+(int x)
    {
        return operator+((double)x);
    };
    TDate& operator+(long x)
    {
        return operator+((double)x);
    };
    operator double()
    {
        return Double();
    };
    void Now();
};
