#include <PalmOS.h> // standard Palm OS Header
#include "NaivePalmdayMitigation.h"

#define CREATORID 'NPDP'
#define HACKTYPE 'HACK'
#define ftrOldTrapAddress 1001

// Date Selector Dialog
#define MyDateSelectorForm					10100
#define MyDateSelectorYearLabel				10102
#define MyDateSelectorPriorYearButton		10103
#define MyDateSelectorNextYearButton		10104
#define MyDateSelectorTodayButton			10118
#define MyDateSelectorCancelButton			10119
#define MyDateSelectorDayGadget				10120
#define MyDateSelectorThisWeekButton		10121
#define MyDateSelectorThisMonthButton		10122

#define myMonthGroup			1

#define myFirstYear				1904
#define myLastYear				2031
#define myDaySelectorMinYear	myFirstYear
#define myDaySelectorMaxYear	myLastYear

typedef Boolean (*SelectDayP)(SelectDayType selectDayBy, Int16 *month, Int16 *day, Int16 *year, const Char * title);

const UInt32 MIN_PALM_OS = sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0);
const UInt32 UNSUPPORTED_AFTER_PALM_OS = sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0);

// This code was adapted from Palm OS 4.0
static Boolean MySelectDay(SelectDayType selectDayBy, Int16 *month, Int16 *day, Int16 *year, const Char * title)
{
	char yearString[5];
	FormType *originalForm, *frm;
	EventType event;
	Boolean confirmed = false;
	DateTimeType today;
	DaySelectorType daySelectorInfo;
	UInt16 buttonToUse;
	UInt8 weekStartDay;
	Int16 selectedDayOfWeek;
	Int16 todayDayOfWeek;

	ErrNonFatalDisplayIf ((*year < myFirstYear) || (*year > myLastYear), "Invalid year");
	ErrNonFatalDisplayIf ((*month < january) || (*month > december), "Invalid month");
	ErrNonFatalDisplayIf ((*day < 1) || (*day > DaysInMonth(*month, *year)), "Invalid day");
	ErrNonFatalDisplayIf (title == NULL, "Invalid title");
	
	daySelectorInfo.selectDayBy = selectDayBy;

	daySelectorInfo.selected.month = *month;
	daySelectorInfo.selected.day = *day;
	daySelectorInfo.selected.year = *year;
	daySelectorInfo.visibleMonth = *month;
	daySelectorInfo.visibleYear = *year;


	// init the time info to 0 (no time)
	daySelectorInfo.selected.second = 0;
	daySelectorInfo.selected.minute = 0;
	daySelectorInfo.selected.hour = 0;

	originalForm = FrmGetActiveForm();
	frm = (FormType *) FrmInitForm (MyDateSelectorForm);
	FrmSetTitle (frm, (Char *) title);

	StrIToA(yearString, daySelectorInfo.selected.year + YEARS_TO_ADD);
	FrmCopyLabel(frm, MyDateSelectorYearLabel, yearString);

	FrmSetControlGroupSelection (frm, myMonthGroup, daySelectorInfo.selected.month);
	
	// Enable either the Today, This Week, or This Month button
	switch (selectDayBy)
		{
		case selectDayByDay:
			buttonToUse = MyDateSelectorTodayButton;
			break;
		
		case selectDayByWeek:
			buttonToUse = MyDateSelectorThisWeekButton;
			break;
		
		case selectDayByMonth:
			buttonToUse = MyDateSelectorThisMonthButton;
			break;
		
		default:
			ErrNonFatalDisplay ("Invalid SelectDayType");
			break;
		}
	FrmShowObject(frm, FrmGetObjectIndex(frm, buttonToUse));

	FrmSetActiveForm (frm);
	FrmDrawForm (frm);

	RctSetRectangle (&(daySelectorInfo.bounds), 7, 63, 140, 70);
	TimSecondsToDateTime(TimGetSeconds(), &today);
	daySelectorInfo.visible = true;
	DayDrawDaySelector (&daySelectorInfo);

	FrmSetGadgetData(frm, FrmGetObjectIndex(frm, MyDateSelectorDayGadget), &daySelectorInfo);

	Boolean redrawYear = false;
	
	while (true)
	{
		EvtGetEvent (&event, evtWaitForever);
		if (! SysHandleEvent ((EventType *)&event))	
			FrmDispatchEvent (&event); 

		DayHandleEvent (&daySelectorInfo, &event);

		if (event.eType == ctlSelectEvent)
		{
			if (selectDayBy == selectDayByMonth &&
				event.data.ctlSelect.controlID >= january &&
				event.data.ctlSelect.controlID <= december)
				{
				// Return with the user's choice of month.
				*month = daySelectorInfo.visibleMonth;
				*year = daySelectorInfo.visibleYear;
				
				// Make sure that the day is within the month (some months are
				// shorter than others).
				today.day = DaysInMonth(today.month, today.year);
				if (*day > today.day)
					*day = today.day;
				confirmed = true;
				break;
				}
			
			if (event.data.ctlSelect.controlID == MyDateSelectorCancelButton)
				break;

			// Today
			else if (event.data.ctlSelect.controlID == MyDateSelectorTodayButton)
				{
				// The Today button now exits the dialog.
				*month = today.month;
				*day = today.day;
				*year = today.year;
				confirmed = true;
				break;
				}

			// This week
			else if (event.data.ctlSelect.controlID == MyDateSelectorThisWeekButton)
				{
				// Tapping this button should return a day from the current week.
				// The day returned is the same day of the week as the selected day.
				// Just returning today is easier, but it loses the context of the 
				// original day.
				
				// Take the week start day into account, otherwise we may end up
				// returning the wrong week.
				// Here's an example: suppose the weekStartDay is monday, today is a
				// tuesday, and the selected day is a sunday.  Then
				//    selectedDayOfWeek - todayDayOfWeek = 0 - 2 = -2.
				// So we end up with the day two days before today, which is the
				// sunday belonging to the previous week.  We really want the sunday
				// a week later.
				weekStartDay = (UInt8) PrefGetPreference(prefWeekStartDay);
				selectedDayOfWeek = (Int16) DayOfWeek(*month, *day, *year);
				todayDayOfWeek = (Int16) DayOfWeek(today.month, today.day, today.year);
				
				// Put both days of the week into the range [weekStartDay,weekStartDay+6].
				if (selectedDayOfWeek < weekStartDay)
					{
					selectedDayOfWeek += daysInWeek;
					}
				if (todayDayOfWeek < weekStartDay)
					{
					todayDayOfWeek += daysInWeek;
					}
				
				TimAdjust(&today, (selectedDayOfWeek - todayDayOfWeek) * daysInSeconds);
				
				// check that the new day is on the same day of week as the selected day
				ErrNonFatalDisplayIf((Int16) DayOfWeek(today.month, today.day, today.year)
											!= (Int16) DayOfWeek(*month, *day, *year),
											"new day is on wrong day of week");
				
				// The This Week button exits the dialog.
				*month = today.month;
				*day = today.day;
				*year = today.year;
				
				confirmed = true;
				break;
				}

			// This month
			else if (event.data.ctlSelect.controlID == MyDateSelectorThisMonthButton)
				{
				// The This Month button exits the dialog.
				*month = today.month;
				*year = today.year;
				
				// Make sure that the day is within the month (some months are
				// shorter than others).
				today.day = DaysInMonth(today.month, today.year);
				if (*day > today.day)
					*day = today.day;
				confirmed = true;
				break;
				}
			else if (event.data.ctlSelect.controlID >= january && event.data.ctlSelect.controlID <= december)
				{
					daySelectorInfo.visibleMonth = (Int16) event.data.ctlSelect.controlID;
					FrmSetControlGroupSelection (frm, myMonthGroup, daySelectorInfo.visibleMonth);
					DayDrawDays(&daySelectorInfo);
				}
			}

		else if (event.eType == frmUpdateEvent)
		{
			FrmDrawForm (frm);
			DayDrawDays(&daySelectorInfo);
		}

		else if (event.eType == ctlRepeatEvent)
		{
		if (event.data.ctlRepeat.controlID == MyDateSelectorPriorYearButton)
			{
			if (daySelectorInfo.visibleYear > myDaySelectorMinYear)
				{
				daySelectorInfo.visibleYear--;
				DayDrawDays(&daySelectorInfo);
				redrawYear = true;
				}
			}
		else if (event.data.ctlRepeat.controlID == MyDateSelectorNextYearButton)
			{
			if (daySelectorInfo.visibleYear < myDaySelectorMaxYear)
				{
				daySelectorInfo.visibleYear++;
				DayDrawDays(&daySelectorInfo);
				redrawYear = true;
				}
			}
		}

		else if (event.eType == daySelectEvent)
		{
			if (daySelectorInfo.visibleMonth != daySelectorInfo.selected.month)
			{
				daySelectorInfo.visibleMonth = daySelectorInfo.selected.month;
				FrmSetControlGroupSelection (frm, myMonthGroup, daySelectorInfo.selected.month);
				DayDrawDays(&daySelectorInfo);
			}
			if (daySelectorInfo.visibleYear != daySelectorInfo.selected.year)
			{
				daySelectorInfo.visibleYear = daySelectorInfo.selected.year;
				DayDrawDays(&daySelectorInfo);
				redrawYear = true;
			}

			// We have a selection.  Now return it.
			if (event.data.daySelect.useThisDate)
				{
				*month = daySelectorInfo.selected.month;
				*day = daySelectorInfo.selected.day;
				*year = daySelectorInfo.selected.year;
				confirmed = true;
				break;
				}
			}
			
		else if (event.eType == appStopEvent)
		{
			EvtAddEventToQueue (&event);
			break;
		}
	
		else if	((event.eType == keyDownEvent) &&(!TxtCharIsHardKey(event.data.keyDown.modifiers, event.data.keyDown.chr)) && (EvtKeydownIsVirtual(&event)))
		{		
			switch (event.data.keyDown.chr)
			{
				case vchrPageUp:
					if (daySelectorInfo.visibleMonth == january)
					{
						if (daySelectorInfo.visibleYear > myDaySelectorMinYear)
						{
							daySelectorInfo.visibleYear--;
							daySelectorInfo.visibleMonth = december;
							FrmSetControlGroupSelection (frm, myMonthGroup, daySelectorInfo.visibleMonth);
							DayDrawDays(&daySelectorInfo);
							redrawYear = true;
						}
					} else {
						daySelectorInfo.visibleMonth--;
						FrmSetControlGroupSelection (frm, myMonthGroup, daySelectorInfo.visibleMonth);
						DayDrawDays(&daySelectorInfo);
						redrawYear = true;
					}
					break;
				case vchrPageDown:
					if (daySelectorInfo.visibleMonth == december)
					{
						if (daySelectorInfo.visibleYear < myDaySelectorMaxYear)
						{
							daySelectorInfo.visibleYear++;
							daySelectorInfo.visibleMonth = january;
							FrmSetControlGroupSelection (frm, myMonthGroup, daySelectorInfo.visibleMonth);
							DayDrawDays(&daySelectorInfo);
							redrawYear = true;
						}
					} else {
						daySelectorInfo.visibleMonth++;
						FrmSetControlGroupSelection (frm, myMonthGroup, daySelectorInfo.visibleMonth);
						DayDrawDays(&daySelectorInfo);
						redrawYear = true;
					}
					break;
			}
		}

		if (redrawYear)
		{
			// Change year label
			StrIToA(yearString, daySelectorInfo.visibleYear + YEARS_TO_ADD);
			FrmCopyLabel(frm, MyDateSelectorYearLabel, yearString);
			redrawYear = false;
		}

	}

	FrmEraseForm (frm);
	FrmDeleteForm (frm);

	FrmSetActiveForm(originalForm);

	return confirmed;
}

Boolean HackEntrypoint(SelectDayType selectDayBy, Int16 *month, Int16 *day, Int16 *year, const Char * title)
{
	SelectDayP originalSelectDay;
	UInt32 romVersion;
	FtrGet(CREATORID, ftrOldTrapAddress, (UInt32*)&originalSelectDay); //get old trap address from HackMaster


	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

	if (romVersion < MIN_PALM_OS || romVersion >= UNSUPPORTED_AFTER_PALM_OS)
		return originalSelectDay(selectDayBy, month, day, year, title);

	return MySelectDay(selectDayBy, month, day, year, title);
}
