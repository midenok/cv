nowarnings
/* shorttrace query4 */
constants
    curr_year = 2006
    curr_month = 1
    curr_day = 18

include "calendar.pro"

domains
    LX=symbol*
    LX2=symbol*
    list=symbol*
    lists=list*

database
    doctor(symbol, symbol).
    workhours(symbol, symbol, integer, integer).
    patient(symbol, integer, integer, integer, integer, symbol).

predicates
    member(symbol, list)
    member2(symbol, lists)

clauses
    member(X, [X|_]).
    member(X, [_|Y]) :- member(X, Y).
    member2(X, [L|_]) :- member(X, L).
    member2(X, [_|L]) :- member2(X, L).

predicates
    query1(symbol, symbol, integer, symbol)    

clauses
    query1(X, Dow, WeekNum, Profile) :-
        patient(X, Year, Month, Day, Hour, Doctor),
        week_num(WeekNum, Year, Month, Day),
        doctor(Doctor, Profile),
        workhours(Doctor, Dow, SHour, EHour),
        dow_by_date(Dow, Year, Month, Day).
/*      Hour > SHour,
        Hour < Ehour. */

predicates
    hours_intersect(integer, integer, integer, integer)
    query2(symbol, integer, integer, integer, integer, integer)

clauses
    hours_intersect(HBegin, HEnd, HStart, _) :-
        HStart >= HBegin,
        HStart < HEnd.

    hours_intersect(HBegin, HEnd, _, HStop) :-
        HStop > HBegin,
        HStop <= HEnd.

    query2(XDoctor, Year, Month, Day, HBegin, HEnd) :-
        dow_by_date(Dow, Year, Month, Day),
        workhours(XDoctor, Dow, HStart, HStop),
        hours_intersect(HBegin, HEnd, HStart, HStop).

predicates
    query3(symbol)
    hour_not_in_range(integer, integer, integer)

clauses
    hour_not_in_range(Hour, HStart, _) :-
        Hour < HStart.

    hour_not_in_range(Hour, _, HStop) :-
        Hour > HStop.

    /* TODO: return not just patient, but registration record */
    query3(XPatient) :-
        patient(XPatient, Year, Month, Day, Hour, Doctor),
        workhours(Doctor, Dow, HStart, HStop),
        dow_by_date(Dow, Year, Month, Day),
        /* ѕредполагаетс€, что за день врач работает без перерыва */
        hour_not_in_range(Hour, HStart, HStop).

predicates
    query4(lists)
    query4_1(lists, lists)
    query4_2(list,list)

clauses
    query4(XLists) :-
        query4_1(XLists, []).
        
    query4_1(XLists, Lists) :-
        patient(Patient, _, _, _, _, _),
        not(member2(Patient, Lists)),
        query4_2(XList, [Patient]),
        query4_1(XLists, [XList|Lists]).
    
    query4_1(XLists, XLists).

    query4_2(XList, PatientList) :-
        patient(Patient, Year, Month, Day, Hour, Doctor),
        not(member(Patient, PatientList)),
        PatientList = [Pat|_],
        patient(Pat, Y, M, D, H, Doc),
        Year = Y,
        Month = M,
        SomeDay = D,
        Hour = H,
        Doctor = Doc,
        query4_2(XList, [Patient|PatientList]).
        
    query4_2(PatientList, PatientList) :-
        PatientList = [_,_|_].

domains
    pat_docs = pd(symbol, list)
    pd_list = pat_docs*

predicates
    member_pd(symbol, pd_list)
    query5(pd_list)
    query5_1(pd_list, pd_list)
    query5_2(list, symbol, list)

clauses
    member_pd(X, [pd(X,_)|_]).
    member_pd(X, [_|Y]) :- member_pd(X, Y).

    query5(XPDList) :-
        query5_1(XPDList, []).
        
    query5_1(XPDList, PDList) :-
        patient(Patient, _, _, _, _, _),
        not(member_pd(Patient, PDList)),
        query5_2(DocList, Patient, []),
        query5_1(XPDList, [pd(Patient, DocList)|PDList]).

    query5_1(PDList, PDList).

    query5_2(XDocList, Patient, DocList) :-
        patient(Patient, _, _, _, _, Doctor),
        not(member(Doctor, DocList)),
        query5_2(XDocList, Patient, [Doctor|DocList]).

    query5_2(DocList, _, DocList) :-
        DocList = [_,_|_].

        
    
goal
    consult("clinic.pro"),
/*    /* ѕоказать всех пациентов, прошедших приЄм у терапевта
       на прошлой неделе во вторник */
    findall(X, query1(X, tue, -1, terapeut), LX), write("1: ", LX, "\n"),
    /* ѕоказать всех врачей, работающих 26.01.2006 в интервале
       с 8:00 до 13:00 */
    findall(X, query2(X, 2006, 1, 26, 8, 13), LX2), write("2: ", LX2, "\n"),
    /* ѕоказать всех пациентов, записанных не по расписанию врача. */
    findall(X, query3(X), LX3), write("3: ", LX3, "\n").
    /* ѕоказать пациентов, записанных на одно и то же врем€. */
    query4(LX), write(LX). */
    /* ѕоказать пациентов, записанным к нескольким докторам.
       ѕоказать к каким докторам записаны эти пациенты. */
    query5(LX), write(LX).