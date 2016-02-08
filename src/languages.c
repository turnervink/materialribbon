#include <pebble.h>
#include "languages.h"

/*
LANGUAGE CODES

english 0
french 1
spanish 2
german 3
russian 4

*/

// Days of the week
const char* dayNames[5][7] = {
	{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},
	{"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"},
	{"Do", "Lu", "Ma", "Mi", "Ju", "Vi", "Sa"},
	{"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"},
	{"ВСК", "ПНД", "ВТР", "СРД", "ЧТВ", "ПТН", "СБТ"},
};

// Codes for fetching weather
char* langCodes[5] = {
	"en",
	"fr",
	"es",
	"de",
	"ru"
};