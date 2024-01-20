#include <PalmOS.h> // standard Palm OS Header
#include "NaivePalmdayMitigation.h"

#define CREATORID 'NPDP'
#define HACKTYPE 'HACK'
#define ftrOldTrapAddress 1000 

typedef UInt16 (*DateTemplateToAsciiP)(const Char *templateP, UInt8 months, UInt8 days, UInt16 years, Char *stringP, Int16 stringLen);

const UInt32 MIN_PALM_OS = sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0);
const UInt32 UNSUPPORTED_AFTER_PALM_OS = sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0);

UInt16 HackEntrypoint(const Char *templateP, UInt8 months, UInt8 days, UInt16 years, Char *stringP, Int16 stringLen)
{
	DateTemplateToAsciiP originalDateTemplateToAscii;
	UInt32 romVersion;
	FtrGet(CREATORID, ftrOldTrapAddress, (UInt32*)&originalDateTemplateToAscii); //get old trap address from HackMaster


	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

	if (romVersion < MIN_PALM_OS || romVersion >= UNSUPPORTED_AFTER_PALM_OS)
		return originalDateTemplateToAscii(templateP, months, days, years, stringP, stringLen);

	return originalDateTemplateToAscii(templateP, months, days, years + YEARS_TO_ADD, stringP, stringLen);
}
