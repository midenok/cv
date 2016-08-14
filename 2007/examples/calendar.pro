database - calendar
    dow(integer, string)
    mdays(integer, integer)    
predicates
    nondeterm days_since_epoch(integer, integer, integer, integer)
    nondeterm days_since_year(integer, integer, integer, integer)
    nondeterm cond_days(integer, integer, integer)
    nondeterm ndow_by_date(integer, integer, integer, integer)
    cond_dow(integer, integer)
    dow_by_date(symbol, integer, integer, integer)
    week_num(integer, integer, integer, integer)
    cond_week_num(integer, integer, integer)
clauses
    dow(1, mon).
    dow(2, tue).
    dow(3, wed).
    dow(4, thu).
    dow(5, fri).
    dow(6, sat).
    dow(7, sun).
    mdays(1, 31).
    mdays(2, 28).
    mdays(3, 31).
    mdays(4, 30).
    mdays(5, 31).
    mdays(6, 30).
    mdays(7, 31).
    mdays(8, 31).
    mdays(9, 30).
    mdays(10, 31).
    mdays(11, 30).
    mdays(12, 31).
    cond_days(X, MONTH, 0) :- mdays(MONTH, X).
    cond_days(DAY, _, DAY).
    /* Количество дней с начала года */
    days_since_year(0, _, 0, 0).
    days_since_year(59, YEAR, 2, 0) :-
        YEAR mod 4 > 0.
    days_since_year(60, YEAR, 2, 0) :-
        YEAR mod 4 = 0.
    days_since_year(X, YEAR, MONTH, DAY) :-
        MONTH > 0,
        MONTH2 = MONTH-1,
        days_since_year(Y, YEAR, MONTH2, 0),
        cond_days(Z, MONTH, DAY),
        X=Y+Z.
    /* Количество дней с 01.01.1970 */
    days_since_epoch(X, YEAR, MONTH, DAY) :-
        FullYears = YEAR-1970,
        VisDays = (FullYears+1) div 4,
        days_since_year(Y, YEAR, MONTH, DAY),
        X = Y + FullYears * 365 + VisDays.
    cond_dow(X, D) :-
        D > 7,
        X = D - 7.
    cond_dow(X, X) :-
        X < 8.
    /* Номер дня недели по дате */
    ndow_by_date(X, YEAR, MONTH, DAY) :-
        days_since_epoch(D, YEAR, MONTH, DAY),
        D2 = (D - 1) mod 7 + 4,
        cond_dow(X, D2).
    /* Имя дня недели по дате */
    dow_by_date(X, Year, Month, Day) :-
        ndow_by_date(X2, Year, Month, Day),
        dow(X2, X).
    cond_week_num(X, X, Z) :-
        Z = 0.
    cond_week_num(X, Y, Z) :-
        Z > 0,
        X = Y + 1.
    cond_week_num(X, Y, Z) :-
        Z < 0,
        X = Y - 1.
    /* Сколько недель между заданной и текущей датой */
    week_num(X, Year, Month, Day) :-
        ndow_by_date(XDow, curr_year, curr_month, curr_day),
        ndow_by_date(XDow2, Year, Month, Day),
        days_since_epoch(XCurr, curr_year, curr_month, curr_day),
        days_since_epoch(XDate, Year, Month, Day),
        D=XDate-XDow2-XCurr+XDow,
        Y=D div 7,
        Z=D mod 7,
        cond_week_num(X, Y, Z).
