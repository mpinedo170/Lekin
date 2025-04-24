#include "StdAfx.h"

#include "Date.h"

TDate::TDate()
{
    iDate = 1;
    iMonth = 1;
    iYear = min_year;
    iHour = 0;
    iMinute = 0;
}

TDate::TDate(int iMonthI, int iDateI, int iYearI, int iHourI, int iMinuteI)
{
    iYearI = (iYearI < 100) ? (1900 + iYearI) : iYearI;
    iYear = (iYearI < min_year) ? min_year : iYearI;
    iMonthI = (iMonthI > 12) ? 12 : iMonthI;
    iMonth = (iMonthI < 1) ? 1 : iMonthI;
    iDate = (iDateI < 1) ? 1 : iDateI;
    if ((iDateI > 28) && (iMonth == 2))
    {
        iDate = (iDateI >= 29) ? 29 : iDateI;
        if ((iDate == 29) && (iYear % 4 != 0)) iDate = 28;
    }
    else if (iDate > days_in_mo[iMonth - 1])
        iDate = days_in_mo[iMonth - 1];
    if (iHourI > 23)
    {
        iHour = 23;
        iMinuteI = 0;
    }
    else
        iHour = (iHourI < 0) ? 0 : iHourI;
    iMinuteI = (iMinuteI > 59) ? 59 : iMinuteI;
    iMinute = (iMinuteI < 0) ? 0 : iMinuteI;
}

TDate::TDate(double x)
{
    double smallTime = 0.0002778;  // 0.4 minute
    x = (x < 0) ? 0 : x;
    x = (x > 500000) ? 0 : x;
    x += smallTime;  // adjust for a small error (.4 minute)
    iYear = 0;
    x++;
    while (x >= 1461)
    {
        iYear += 4;
        x -= 1461;
    }
    if (x > 366)
    {
        iYear++;
        x -= 366;
    }
    while (x > 365)
    {
        iYear++;
        x -= 365;
    }
    iYear += min_year;
    iMonth = 1;
    if (x > (days_in_mo[0]))
    {
        x -= days_in_mo[0];
        iMonth++;
    }
    int feb = 28;
    if (iYear % 4 == 0) feb++;
    if (x > feb)
    {
        x -= feb;
        iMonth++;
    }
    while (x > days_in_mo[iMonth - 1])
    {
        x -= days_in_mo[iMonth - 1];
        iMonth++;
    }
    iDate = (int)x;
    x -= iDate;
    iMinute = (int)(x * 24 * 60);
    iHour = iMinute / 60;
    iMinute %= 60;
    if (iDate == 0)
    {
        switch (iMonth)
        {
            case 1:
                iYear--;
                iMonth = 12;
                iDate = 31;
                break;
            case 3:
                iMonth = 2;
                iDate = feb;
                break;
            default:
                iMonth--;
                iDate = days_in_mo[iMonth - 1];
                break;
        }
    }
}

TDate::TDate(const TDate& dt)
{
    iDate = dt.iDate;
    iYear = dt.iYear;
    iHour = dt.iHour;
    iMinute = dt.iMinute;
}

void TDate::Set(int iMonthI, int iDateI, int iYearI, int iHourI, int iMinuteI)
{
    while (iMinuteI > 59)
    {
        iMinuteI -= 60;
        iHourI++;
    }
    iMinute = (iMinuteI < 0) ? 0 : iMinuteI;
    while (iHourI > 23)
    {
        iHourI -= 24;
        iDateI++;
    }
    iHour = (iHourI < 0) ? 0 : iHourI;
    iDate = (iDateI < 1) ? 1 : iDateI;
    if ((iDate > 28) && (iMonthI == 2))
    {
        if (iYearI % 4 == 0)
        {
            if (iDate > 29)
            {
                iDate -= 29;
                iMonthI++;
            }
        }
        else
        {
            iDate -= 28;
            iMonthI++;
        }
    }
    else if (iDate > days_in_mo[iMonthI - 1])
    {
        iDate -= days_in_mo[iMonthI - 1];
        iMonthI++;
    }
    while (iMonthI > 12)
    {
        iMonthI -= 12;
        iYearI++;
    }
    iMonth = (iMonthI < 0) ? 0 : iMonthI;
    iYearI = (iYearI < 100) ? ((iYearI > 80) ? (1900 + iYearI) : (2000 + iYearI)) : iYearI;
    iYear = (iYearI < min_year) ? min_year : iYearI;
}

void TDate::Set(double x)
{
    x = (x < 0) ? 0 : x;
    x = (x > 500000) ? 0 : x;
    iYear = 0;
    x++;
    while (x >= 1461)
    {
        iYear += 4;
        x -= 1461;
    }
    if (x > 366)
    {
        iYear++;
        x -= 366;
    }
    while (x > 365)
    {
        iYear++;
        x -= 365;
    }
    iYear += min_year;
    iMonth = 1;
    if (x > days_in_mo[0])
    {
        x -= days_in_mo[0];
        iMonth++;
    }
    int feb = 28;
    if (iYear % 4 == 0) feb++;
    if (x > feb)
    {
        x -= feb;
        iMonth++;
    }
    while (x > days_in_mo[iMonth - 1])
    {
        x -= days_in_mo[iMonth - 1];
        iMonth++;
    }
    iDate = (int)x;
    x -= iDate;
    iMinute = (int)(x * 24 * 60 + 0.5);  // round off to the nearest minute
    iHour = iMinute / 60;
    iMinute %= 60;
}

int TDate::Day()
{
    long temp;
    temp = (long)this->Double();
    temp %= 7;
    temp += day1;
    temp %= 7;
    int temp2;
    temp2 = (int)temp;
    return temp2;
}

double TDate::Double()
{
    double temp = 0;
    int repeat;
    repeat = (iYear - min_year + 3) / 4;
    if ((iYear % 4 == 0) && (iMonth > 2)) repeat++;
    temp = ((int)iYear - min_year) * 365 + repeat;
    if (iMonth > 1)
        for (int i = 1; i < iMonth; i++) temp += days_in_mo[i - 1];
    temp += iDate - 1;
    temp += (double)iHour / 24.0;
    temp += (double)(iMinute) / (60 * 24.0);
    return temp;
}

double TDate::operator-(TDate& x)
{
    double a, b, c;
    a = this->Double();
    b = x.Double();
    c = a - b;
    return c;
}

TDate& TDate::operator-(double x)
{
    double c;
    c = this->Double();
    c -= x;
    if (c < 0) c = 0;
    TDate* temp = new TDate(c);
    return *temp;
}

TDate& TDate::operator+(double x)
{
    double c;
    c = this->Double();
    c += x;
    TDate* temp = new TDate(c);
    return *temp;
}

void TDate::Now()
{
    time_t aclock;
    struct tm ctime;
    time(&aclock);
    ctime = *localtime(&aclock);
    iDate = ctime.tm_mday;
    iMonth = ctime.tm_mon + 1;
    iYear = ctime.tm_year + 1900;
    iHour = ctime.tm_hour;
    iMinute = ctime.tm_min;
}

void TDate::Char(LPTSTR szX)
{
    LPTSTR cCurrent = szX;
    /*** month ***/
    *cCurrent = (char)(48 + (iMonth / 10));
    cCurrent++;
    *cCurrent = (char)(48 + (iMonth % 10));
    cCurrent++;
    *cCurrent = '/';
    cCurrent++;
    /*** date ***/
    *cCurrent = (char)(48 + (iDate / 10));
    cCurrent++;
    *cCurrent = (char)(48 + (iDate % 10));
    cCurrent++;
    *cCurrent = '/';
    cCurrent++;
    /*** year ***/
    int iY = iYear % 100;
    *cCurrent = (char)(48 + (iY / 10));
    cCurrent++;
    *cCurrent = (char)(48 + (iY % 10));
    cCurrent++;
    *cCurrent = ' ';
    cCurrent++;
    /*** hour ***/
    *cCurrent = (char)(48 + (iHour / 10));
    cCurrent++;
    *cCurrent = (char)(48 + (iHour % 10));
    cCurrent++;
    *cCurrent = ':';
    cCurrent++;
    /*** minute ***/
    *cCurrent = (char)(48 + (iMinute / 10));
    cCurrent++;
    *cCurrent = (char)(48 + (iMinute % 10));
    cCurrent++;
    *cCurrent = '\0';
}