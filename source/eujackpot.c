/**
 * Euro Jackpot Lotto Statistics and Ball Draw Algorithms
 * Author: İbrahim Tıpırdamaz  <itipirdamaz@gmail.com>
 * Copyright 2023
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(SOLARIS) || defined(WIN32)
#include <libgen.h>
#include <unistd.h>
#endif

#if defined(__MSDOS__) || defined(WIN32)
#include <conio.h>
#endif

#if defined(__MSDOS__)
#include <dos.h>
#endif

#ifdef WIN32
#include <windows.h> /* GetModuleFileName */
#endif

#ifdef __APPLE__
#include <sys/param.h>
#include <mach-o/dyld.h> /* _NSGetExecutablePath : must add -framework CoreFoundation to link line */
#ifndef PATH_MAX
#define PATH_MAX MAXPATHLEN
#endif
#endif

#ifndef PATH_MAX
#if defined(__MSDOS__) || defined(WIN32)
# define PATH_MAX 260
#else
# define PATH_MAX 2048
#endif
#endif


#if defined(__MSDOS__)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif


#define TOTAL_BALL 50				// total ball count
#define DRAW_BALL 5					// number of balls to be drawn
#define TOTAL_BALL_EN 12			// total euro number count
#define DRAW_BALL_EN 2				// number of euro numbers to be drawn


#ifdef __MSDOS__
#define FILESTATS "eurojack.txt"	// statistics file (winning numbers, euro numbers)
#else
#define FILESTATS "eujackpot.txt"	// statistics file (winning numbers, euro numbers)
#endif

#define OUTPUTFILE "output.txt"		// file to write results


/* MS-DOS swap files for large FILESTATS due to lack of memory */
#define LBL2SWAPFILE "luckybl2.swp"	// luckyBalls2 swap file
#define LBL3SWAPFILE "luckybl3.swp"	// luckyBalls3 swap file
#define LBL4SWAPFILE "luckybl4.swp"	// luckyBalls4 swap file
#define EUNMSWAPFILE "euronums.swp"	// euroNumbers  swap file


#define UINT16MAX 65535	// max file rows



/* TYPE DEFINITIONS */


typedef unsigned char UINT8;

#if defined(__MSDOS__)
typedef unsigned int UINT16;
typedef unsigned long UINT32;
typedef long SINT32;
#else
typedef short unsigned int UINT16;
typedef unsigned int UINT32;
typedef int SINT32;
#endif



/* GLOBAL VARIABLES */


/* Today's Date */
UINT8 currDay;
UINT8 currMon;
UINT16 currYear;



/* current working directory */
char *cwd = NULL;
char *fileStats = NULL;		// cwd + PATH_SEPARATOR + FILESTATS
char *outputFile = NULL;	// cwd + PATH_SEPARATOR + OUTPUTFILE



/**
* It was named "List" because the data structure was previously defined as a fully linked list.
* But the column (X) widths (number of balls) are fixed, so the columns were converted to char arrays.
* Thus, inter-column access is faster and takes up less space.
* File line counts are variable, so lines (Y) are linked by a linked list.
*/

struct ListX {			/* List (drawn balls, statistics file row etc) */
	char *label;		// list label (for new drawns)
	UINT16 year;		// old drawn year
	UINT8 mon;			// old drawn month
	UINT8 day;			// old drawn day (year, month, day for statistics file rows)
	UINT16 val;			// other value (how many times the numbers that drawn together, for lucky numbers)
	UINT16 val2;		// other value (how many days apart on average, for lucky numbers)
	UINT8 *balls;		// ball array
	UINT8 index;		// last item index in array (added items count, if index==0 array is empty)
	UINT8 size;			// allocated total size of array
	struct ListX *next;	// next list (next row, if two dimensions)
};



struct ListX2 {			/* List (globe, ballSortOrder: Sorting of balls by how many times each ball has been drawn in past draws) */
	UINT8 *balls;		// ball array
	UINT16 *vals;		// other info such as how many times the balls has been drawn so far
	UINT8 index;		// last item index in array (added items count, if index==0 array is empty)
	UINT8 size;			// allocated total size of array
};



struct ListXY {			/* 2 dimensions List. Multi draw (coupon or drawn balls from file has been drawn so far) */
	struct ListX *list;	// list
};



/* Drawn balls lists from file has been drawn so far */

struct ListXY *winningDrawnBallsList = NULL;
struct ListXY *euNumberDrawnBallsList = NULL;
UINT16 winningBallRows = 0;
UINT16 euNumberBallRows = 0;


/* Old drawn dates between dateStart and dateEnd */

char dateStart[11], dateEnd[11];

/* Old drawn days count between dateStart and dateEnd */

UINT16 drawnDays;


/* How many times were the winning numbers drawn in the previous draws? */

struct ListX2 *winningBallsDrawCount = NULL;
struct ListX2 *euNumberBallsDrawCount = NULL;


/* Numbers that drawn together (how many times the numbers drawn together) */

struct ListXY *luckyBalls2 = NULL;
struct ListXY *luckyBalls3 = NULL;
struct ListXY *luckyBalls4 = NULL;
struct ListXY *euroNumbers = NULL;


/* Matched combinations of numbers from previous draws */

UINT32 match2comb = 0;
UINT32 match3comb = 0;
UINT32 match4comb = 0;
UINT32 match5comb = 0;



/* FUNCTION DEFINITIONS */


/**
 * Returns the difference between two dates in days
 * Date 1 : y1-m1-d1 (yyyy-mm-dd)
 * Date 2 : y2-m2-d2 (yyyy-mm-dd)
 *
 * @param {Integer} d1  : day 1
 * @param {Integer} m1  : month 1
 * @param {Integer} y1  : year 1
 * @param {Integer} d2  : day 2
 * @param {Integer} m2  : month 2
 * @param {Integer} y2  : year 2
 *
 * @return {Integer}    : Returns the difference between two dates in days
*/
UINT16 dateDiff(UINT8 d1, UINT8 m1, UINT16 y1, UINT8 d2, UINT8 m2, UINT16 y2);



/**
 * Date format yyyy-mm-dd
 *
 * @param {char *} date     : char array (date assign as string to this variable (yyyy-mm-dd))
 * @param {Integer} day     : day
 * @param {Integer} mon     : month
 * @param {Integer} year    : year
*/
void formatDate(char* date, UINT8 day, UINT8 mon, UINT16 year);



/**
 * Clear Screen
 *
 */
void clearScreen();



/**
 * Percent of progress
 *
 */
void printPercentOfProgress(char *label, UINT32 completed, UINT32 all);



/**
 * Get application full path
 *
 */
int get_app_path (char *pname, size_t pathsize);



/**
 * It waits until you press any key and exits the program.
 *
 */
void pressAnyKeyToExit();



/**
 * It checks whether the value entered from the keyboard is an integer.
 *
 */
UINT8 isIntString(char* input);



/**
 * Create Empty List (1 dimension)
 * 
 * @param {struct ListX *} pl : refers to a ball list
 * @param {Integer} size      : list size
 * @param {char *} label      : list label (for new drawns lists, otherwise NULL)
 * @param {Integer} val       : other value (how many times the numbers that drawn together, for lucky numbers)
 * @param {Integer} val2      : other value (how many days apart on average, for lucky numbers)
 * @param {Integer} year      : year
 * @param {Integer} mon       : month
 * @param {Integer} day       : day
 * @return {struct ListX *}   : refers to the ball list (memory allocated)
 */
struct ListX *createListX(struct ListX *pl, UINT8 size, char *label, UINT16 val, UINT16 val2, UINT16 year, UINT8 mon, UINT8 day);



/**
 * Create Empty List (1 dimension)
 *
 * @param {struct ListX2 *} pl : refers to a ball list
 * @param {Integer} size       : list size
 * @return {struct ListX2 *}   : refers to the ball list (memory allocated)
 */
struct ListX2 *createListX2(struct ListX2 *pl, UINT8 size);



/**
 * Create Empty 2 dimensions List
 * 
 * @param {struct ListXY *}    : refers to a 2 dimensions ball list
 * @return {struct ListXY *}   : refers to the ball list (memory allocated)
 */
struct ListXY *createListXY(struct ListXY *pl);



/** 
 * Add an item (ball) to the beginning of the list
 *
 * @param {struct ListX2 *}    : refers to a ball list
 * @param {Integer} key        : the key to which the ball will be added
 */
void insertItem2(struct ListX2 *pl, UINT8 key);



/**
 * Add an item (ball) to the end of the list
 * 
 * @param {struct ListX *}     : refers to a ball list
 * @param {Integer} key        : the key to which the ball will be added
 */
void appendItem(struct ListX *pl, UINT8 key);



/** 
 * Add an item (ball) with key->val pair to the end of the list
 *
 * @param {struct ListX2 *}    : refers to a ball list
 * @param {Integer} key        : the key to which the ball will be added
 * @param {Integer} val        : value
 */
void appendItem2(struct ListX2 *pl, UINT8 key, UINT16 val);



/** 
 * Add multiple items (balls) to the end of the list
 * 
 * @param {struct ListX *}     : refers to a ball list
 * @param {Integer *} keys     : the keys to which the ball will be added
 */
void appendItems(struct ListX *pl, UINT8 *keys);



/** 
 * Insert a list to the beginning of the 2 dimensions list
 * 
 * @param {struct ListXY *}    : refers to a 2 dimensions ball list (multiple draws)
 * @param {struct ListX *}     : refers to a ball list
 */
void insertList(struct ListXY *pl, struct ListX *newList);



/** 
 * Add a list to the end of the 2 dimensions list
 * 
 * @param {struct ListXY *}    : refers to a 2 dimensions ball list (multiple draws)
 * @param {struct ListX *}     : refers to a ball list
 */
void appendList(struct ListXY *pl, struct ListX *plNext);



/** 
 * Removes item and returns the key (ball number) at the specified index from the list
 * 
 * @param {struct ListX2 *}    : refers to a ball list
 * @param {Integer} ind        : the index to which the ball will be removed
 * @return {Integer}           : returns key (ball number)
 */
UINT8 removeItemByIndex2(struct ListX2 *pl, UINT8 ind);



/** 
 * If the specified key is not in the list, it adds to the specified index and returns 1
 *
 * @param {struct ListX2 *}    : refers to a ball list
 * @param {Integer} ind        : the index to which the ball will be added
 * @param {Integer} key        : key (ball number)
 * @return {Integer}           : If the key is in the list, it does not add it to the list and return 0
 */
UINT8 addItemByIndex2(struct ListX2 *pl, UINT8 ind, UINT8 key);



/** 
 * It adds to the specified index
 * 
 * @param {struct ListXY *}    : refers to a 2 dimensions ball list
 * @param {struct ListX *}     : new list
 * @param {Integer} ind        : the index to which the list will be added
 * @return {Integer}           : It adds to the specified index
 */
void addListByIndex(struct ListXY *pl, struct ListX *newList, UINT16 ind);



/** 
 * Removes list and returns at the specified index from the 2 dimensions list
 *
 * @param {struct ListXY *}    : refers to a 2 dimensions ball list
 * @param {Integer} ind        : the index to which the ball will be removed
 * @return {Integer}           : returns key (ball number)
 */
struct ListX * removeListByIndex(struct ListXY *pl, UINT16 ind);



/** 
 * Shuffle a 2 dimensions list
 *
 * @param {struct ListXY *}    : refers to a 2 dimensions ball list
 * @return {struct ListXY *}   : return list
 */
struct ListXY * shuffleListXY(struct ListXY *pl);



/**
 * Remove all items in the list
 * 
 * @param {struct ListX *}     : refers to a ball list
 */
void removeAllX(struct ListX *pl);



/** 
 * Remove all items in the list
 * 
 * @param {struct ListX2 *}     : refers to a ball list
 */
void removeAllX2(struct ListX2 *pl);



/** 
 * Remove all list in the 2 dimensions list
 *
 * @param {struct ListXY *}    : refers to a 2 dimensions ball list (multiple draws)
 */
void removeAllXY(struct ListXY *pl);



/** 
 * Return the number of list in the 2 dimensions list
 * 
 * @param {struct ListXY *}    : refers to a 2 dimensions ball list (multiple draws)
 * @return {Integer}           : Return the number of list in the 2 dimensions list
 */
UINT16 lengthY(struct ListXY *pl);



/** 
 * Find the pointer of the list at the end of the 2 dimensions list
 * 
 * @param {struct ListXY *}    : refers to a 2 dimensions ball list (multiple draws)
 * @return {struct ListX *}    : returns pointer of the list at the end of the 2 dimensions list
 */
struct ListX *atEndY(struct ListXY *pl);



/**
 * Searches for {size} keys (ball numbers) from the list.
 * 
 * @param {struct ListX *}       : refers to a ball list
 * @param {unsigned char *} keys : keys (ball numbers) to be search
 * @param {Integer} size         : keys array size (ball count)
 * @return {Integer}             : Returns the number of keys found.
 */
UINT8 seqSearchX(struct ListX *pl, UINT8 *keys, UINT8 size);



/**
 * Searches for a key (ball number) from the list.
 * 
 * @param {struct ListX *}     : refers to a ball list
 * @param {Integer} key        : key (ball number) to be search
 * @return {Integer}           : It returns the index of the number if it finds it, or -1 if it doesn't.
 */
int seqSearchX1(struct ListX *pl, UINT8 key);



/** 
 * Searches for a key (ball number) from the list.
 * 
 * @param {struct ListX2 *}    : refers to a ball list
 * @param {Integer} key        : key (ball number) to be search
 * @return {Integer}           : It returns the index of the number if it finds it, or -1 if it doesn't.
 */
int seqSearchX2(struct ListX2 *pl, UINT8 key);



/**
 * Searches for {size} keys (ball numbers) from the 2 dimensions list.
 * 
 * @param {struct ListXY *}      : refers to a 2 dimensions ball list
 * @param {unsigned char *} keys : keys (ball numbers) to be search
 * @param {Integer} size         : keys array size (ball count)
 * @return {Integer}             : It returns the index of the numbers if it finds it, or -1 if it doesn't.
 *                                 This is index of the first list containing the key in the 2 dimensions list
 */
int seqSearchXY(struct ListXY *pl, UINT8 *keys, UINT8 size);



/** 
 * Increments the value (the number of times the ball has drawn so far) of the item at the specified index by 1
 *
 * @param {struct ListX2 *}    : refers to a ball list (ballSortOrder)
 * @param {Integer} ind        : index of an item (ball) in the ball list
 */
void incVal2(struct ListX2 *pl, UINT8 ind);



/**
 * Increments the value (how many times the numbers that drawn together) of the list at the specified index by 1
 * 
 * @param {struct ListXY *}    : refers to a 2 dimensions ball list (luckyBalls)
 * @param {Integer} ind        : index of a list in the 2 dimensions ball list
 */
void incValXY(struct ListXY *pl, UINT16 ind);



/** 
 * Returns the value of the list at the specified index
 *
 * @param {struct ListXY *}    : refers to a 2 dimensions ball list (luckyBalls)
 * @param {Integer} ind        : index of a list in the 2 dimensions ball list
 * @return {Integer}           : Returns the value of the list at the specified index
 */
UINT16 getValXY(struct ListXY *pl, UINT16 ind);



/** 
 * Returns the key (ball number) of the item at the specified index
 * 
 * @param {struct ListX *}     : refers to a ball list
 * @param {Integer} ind        : index of an item (ball) in the ball list
 * @return {Integer}           : Returns the key (ball number)
 */
UINT8 getKey(struct ListX *pl, UINT8 index);



/**
 * Returns the key (ball number) of the item at the specified index
 * 
 * @param {struct ListX2 *}    : refers to a ball list
 * @param {Integer} ind        : index of an item (ball) in the ball list
 * @return {Integer}           : Returns the key (ball number)
 */
UINT8 getKey2(struct ListX2 *pl, UINT8 index);



/** 
 * Returns the keys (ball numbers) of the "count" items starting from the "index"
 * 
 * @param {struct ListX *}     : refers to a ball list
 * @param {Integer *} keys     : refers to the keys (ball numbers)
 * @param {Integer} index      : starting index
 * @param {Integer} count      : ball count (if count==0 until end of array)
 */
void getKeys(struct ListX *pl, UINT8 *keys, UINT8 index, UINT8 count);



/**
 * Returns the list in the 2 dimensions list at the specified index
 * 
 * @param {struct ListXY *}    : refers to a 2 dimensions ball list (multiple draws)
 * @param {Integer} ind        : index of 1 dimension list (a draw) in the 2 dimensions list (multi draws)
 * @return {struct ListX *}    : refers to a ball list at the specified index
 */
struct ListX * getListXByIndex(struct ListXY *pl, UINT16 ind);



/** 
 * Bubble sort by val the items (balls) in the ball list
 *
 * @param {struct ListX2 *}    : refers to a ball list (ballSortOrder)
 * @param {Integer} inc        : If inc=1, sort from smallest to greager. If inc=-1, sort from greater to smallest
 */
void bubbleSortX2ByVal(struct ListX2 *pl, int inc);



/**
 * Bubble sort by val the lists in the 2 dimensions list
 * 
 * @param {struct ListXY *}   : refers to a 2 dimensions ball list
 * @param {Integer} inc       : If inc=1, sort from smallest to greager. If inc=-1, sort from greater to smallest
 */
void bubbleSortYByVal(struct ListXY *pl, int inc);



/**
 * Bubble sort by val2 the lists in the 2 dimensions list
 * 
 * @param {struct ListXY *}   : refers to a 2 dimensions ball list
 * @param {Integer} inc       : If inc=1, sort from smallest to greager. If inc=-1, sort from greater to smallest
 */
void bubbleSortYByVal2(struct ListXY *pl, int inc);



/** 
 * Bubble sort by key (ball number) from smallest to greager the items (ball) in the ball list
 *
 * @param {struct ListX *}    : refers to a ball list
 */
void bubbleSortXByKey(struct ListX *pl);



/** 
 * Function to swap data of two list a and b
 *
 * @param {struct ListX *} a   : refers to a ball list
 * @param {struct ListX *} b   : refers to a ball list
 */
void swapY(struct ListX *a, struct ListX *b);



/**
 * Print keys (ball numbers) of the items in the list
 * 
 * @param {struct ListX *}    : refers to a ball list
 * @param {Integer} printTo   : 0: print to screen and output file. 1: print to screen only. 2: print to output file only.
 * @param {FILE *} fp         : refers to output file. If fp != NULL print to output file
 */
void printListXByKey(struct ListX *pl, UINT8 printTo, FILE *fp);



/** 
 * print ListXY With EuroNumber ByKey
 * Print keys of the items (row by row) in the 2 dimensions lists
 * 
 * @param {struct ListXY *}    : refers to 2 dimensions ball list (winning numbers)
 * @param {struct ListXY *}    : refers to 2 dimensions ball list (euroNumber)
 * @param {FILE *} fp          : refers to output file. If fp != NULL print to output file
 */
void printListXYWithENByKey(struct ListXY *pl1, struct ListXY *pl2, FILE *fp);



/** 
 * How many times the balls has been drawn so far 
 * (assign values to winningBallsDrawCount and euNumberBallsDrawCount global variables)
 * 
 */
void getDrawnBallCount();



/**
 * Print key-val pair of the items in the list by ball statistics (How many times the balls has been drawn so far)
 * 
 * @param {struct ListX2 *} ballSortOrder     : refers to balls sorted by statistics from past draws.
 *                                            : Balls are sorted by the number of draws from previous draws.
 */
void printDrawnBallCount(struct ListX2 *ballSortOrder);



/**
 * print numbers (double combinations or triple combinations) that drawn together (lucky balls)

 * @param {struct ListXY *}     : refers to 2 dimensions ball list (2*y or 3*y)
 * @param {FILE *} fp           : refers to output file. If fp != NULL print to output file
*/
void printLuckyBalls(struct ListXY *pl, FILE *fp);



/**
 * Return number of term

 * @param {Integer}     : sum of numbers from 1 to n
 * @return {Integer}    : n
*/
UINT16 numberOfTerm(UINT16 sum);



/**
 * ballSortOrder: Sorting of balls by how many times each ball has been drawn in past draws.
 * Balls sorted by statistics from past draws (eg, normal distribution, side-stacked, left-stacked).
 * 
 * normal distribution: the most drawn numbers in the middle and the least drawn numbers on the sides.
 * side stacked: the most drawn numbers on the sides and the least drawn numbers in the middle.
 * left stacked: the most drawn number on the left and the least drawn number on the right.
 * 
 * ballSortOrder are arranged at the base of the pascal's triangle and the ball is dropped on 
 * (with gaussIndex function) it and the ball hit is drawn. The gaussIndex function returns the index (random) of one of the balls arranged 
 * at the base of pascal's triangle. If a ball is dropped from the top node of Pascal's triangle, it drops to the left or right each time 
 * it hits the node (like tossing a coin). The gaussIndex function moves towards the base of the triangle, making a random selection at
 * each node. When it reaches the base of the triangle, it returns the index whichever node (index) it hits. 
 * 
 * @param {Integer} ballCount      : Ball count in the globe
 * @return {Integer}               : Returns the random ball number (or ballSortOrder index, you substract 1 from the return value for index)
 *                                   For example, ballCount = 90, it returns number between 1-90 (for index 0-89) (theoretically)
 *                                   Usually retuns the middle numbers
 */
UINT8 gaussIndex(UINT8 ballCount);



/**
 * Draw random numbers
 * 
 * @param {struct ListX *} drawnBallsRand : refers to the balls to be drawn. 
 * @param {struct ListX2 *} ballSortOrder : refers to balls sorted by statistics from past draws.
 * @param {Integer} totalBall             : Total ball count in the globe
 * @param {Integer} drawBallCount      	  : Number of balls to be drawn
 * @param {Integer} matchComb             : Number of combinations in which the drawn numbers must match any of the previous draws.
 * @param {Integer} elimComb              : If a combination of the drawn numbers matched with any of the previous draws, specified by the
 *                                          elimComb parameter, the draw is renewed.
 *                                          For example, a previous draw is [1, 2, 4, 6, 8, 9]
 *                                          the new draw (drawnBallsRand) is [1, 2, 5, 7, 9, 10]
 *                                          If matchComb = 2 and elimComb = 0, draw is OK, because (1,2) (1,9) or (2,9) matched.
 *                                          If matchComb = 2 and elimComb = 3, draw is renewed, because (1,2,9) eliminated. 
 * @return {struct ListX *} drawnBallsRand: Returns new drawn balls.
 */
struct ListX * drawBallByRand(struct ListX *drawnBallsRand, struct ListX2 *ballSortOrder, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb);



/**
 * ballSortOrder are arranged at the base of the pascal triangle
 * (stacked to the left) according to the number that comes out the most from the number that comes out the least, 
 * and the ball is dropped on (with gaussIndex function) it and the ball hit is drawn.
 *
 * @param {struct ListX *} drawnBallsLeft : refers to the balls to be drawn. 
 * @param {struct ListX2 *} ballSortOrder : refers to balls sorted by statistics from past draws.
 * @param {Integer} totalBall             : Total ball count in the globe
 * @param {Integer} drawBallCount      	  : Number of balls to be drawn
 * @param {Integer} matchComb             : Number of combinations in which the drawn numbers must match any of the previous draws.
 * @param {Integer} elimComb              : If a combination of the drawn numbers matched with any of the previous draws, specified by the
 *                                          elimComb parameter, the draw is renewed.
 * @return {struct ListX *} drawnBallsLeft: Returns new drawn balls.
 */
struct ListX * drawBallByLeft(struct ListX *drawnBallsLeft, struct ListX2 *ballSortOrder, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb);



/** 
 * Blend 1
 * Dividing the list of numbers (ballSortOrder) in the middle, inverting the left and right parts and combining them.
 * ballSortOrder are arranged at the base of the
 * pascal triangle and the ball is dropped on (with gaussIndex function) it and the ball hit is drawn.
 * 
 * @param {struct ListX *} drawnBallsBlend1 : refers to the balls to be drawn.
 * @param {struct ListX2 *} ballSortOrder   : refers to balls sorted by statistics from past draws.
 * @param {Integer} totalBall               : Total ball count in the globe
 * @param {Integer} drawBallCount           : Number of balls to be drawn
 * @param {Integer} matchComb               : Number of combinations in which the drawn numbers must match any of the previous draws.
 * @param {Integer} elimComb                : If a combination of the drawn numbers matched with any of the previous draws, 
 *                                            specified by the elimComb parameter, the draw is renewed.
 * @return {struct ListX *} drawnBallsBlend1: Returns new drawn balls.
 */
struct ListX * drawBallByBlend1(struct ListX *drawnBallsBlend1, struct ListX2 *ballSortOrder, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb);



/** 
 * Blend 2
 * The balls are taken from the left and right of the ballSortOrder and placed from the middle of the globe 
 * (The base of the Pascal's triangle) to the edges
 * 
 * @param {struct ListX *} drawnBallsBlend2 : refers to the balls to be drawn. 
 * @param {struct ListX2 *} ballSortOrder   : refers to balls sorted by statistics from past draws.
 * @param {Integer} totalBall               : Total ball count in the globe
 * @param {Integer} drawBallCount           : Number of balls to be drawn
 * @param {Integer} matchComb               : Number of combinations in which the drawn numbers must match any of the previous draws.
 * @param {Integer} elimComb                : If a combination of the drawn numbers matched with any of the previous draws, specified by the
 *                                            elimComb parameter, the draw is renewed.
 * @return {struct ListX *} drawnBallsBlend2: Returns new drawn balls.
 */
struct ListX * drawBallByBlend2(struct ListX *drawnBallsBlend2, struct ListX2 *ballSortOrder, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb);



/**
 * The balls that drawn the least are placed in the middle of the globe, and the balls that drawn the most are placed on the edges.
 * 
 * @param {struct ListX *} drawnBallsSide   : refers to the balls to be drawn. 
 * @param {struct ListX2 *} ballSortOrder   : refers to balls sorted by statistics from past draws.
 * @param {Integer} totalBall               : Total ball count in the globe
 * @param {Integer} drawBallCount           : Number of balls to be drawn
 * @param {Integer} matchComb               : Number of combinations in which the drawn numbers must match any of the previous draws.
 * @param {Integer} elimComb                : If a combination of the drawn numbers matched with any of the previous draws, specified by the
 *                                            elimComb parameter, the draw is renewed.
 * @return {struct ListX *} drawnBallsSide  : Returns new drawn balls.
 */
struct ListX * drawBallBySide(struct ListX *drawnBallsSide, struct ListX2 *ballSortOrder, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb);



/** 
 * The most drawn balls are placed in the center of the globe,
 * the least drawn balls are placed at the edges (normal distribution)
 * 
 * @param {struct ListX *} drawnBallsNorm   : refers to the balls to be drawn.
 * @param {struct ListX2 *} ballSortOrder   : refers to balls sorted by statistics from past draws.
 * @param {Integer} totalBall               : Total ball count in the globe
 * @param {Integer} drawBallCount           : Number of balls to be drawn
 * @param {Integer} matchComb               : Number of combinations in which the drawn numbers must match any of the previous draws.
 * @param {Integer} elimComb                : If a combination of the drawn numbers matched with any of the previous draws, specified by the
 *                                            elimComb parameter, the draw is renewed.
 *                                            For example, a previous draw is [1, 2, 4, 6, 8, 9]
 *                                            the new draw (drawnBallsNorm) is [1, 2, 5, 7, 9, 10]
 *                                            If matchComb = 2 and elimComb = 0, draw is OK, because (1,2) (1,9) or (2,9) matched.
 *                                            If matchComb = 2 and elimComb = 3, draw is renewed, because (1,2,9) eliminated. 
 * @return {struct ListX *} drawnBallsNorm  : Returns new drawn balls.
 */
struct ListX * drawBallByNorm(struct ListX *drawnBallsNorm, struct ListX2 *ballSortOrder, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb);



/** 
 * The search5CombXY function searches for 5 combinations in all previous draws. 
 * 
 * @param {struct ListXY *} prvDrawnsList : refers to the balls has been drawn so far. 
 * @param {struct ListX *} drawnBalls     : refers to balls drawn in a new draw. 
 * @return {Integer}                      : Returns 1 if found, 0 if not. 
 */
UINT8 search5CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls);



/** 
 * The search5CombX function searches for 5 combinations in a previous draw. 
 * 
 * @param {struct ListX *} aPrvDrawn      : refers to a drawn balls in the previous draws.
 * @param {struct ListX *} drawnBalls     : refers to balls drawn in a new draw. 
 * @return {Integer}                      : Returns 1 if found, 0 if not. 
 */
UINT8 search5CombX(struct ListX *aPrvDrawn, struct ListX *drawnBalls);



/** 
 * The search4CombXY function searches for 4 combinations in all previous draws. 
 * 
 * @param {struct ListXY *} prvDrawnsList : refers to the balls has been drawn so far. 
 * @param {struct ListX *} drawnBalls     : refers to balls drawn in a new draw. 
 * @param {struct ListXY *} foundComb	  : refers to founded combinations.
 * @return {Integer}                      : Returns 1 if found, 0 if not.
 */
UINT8 search4CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls, struct ListXY *foundComb);



/** 
 * The search4CombX function searches for 4 combinations in a previous draw. 
 * 
 * @param {struct ListX *} aPrvDrawn      : refers to a drawn balls in the previous draws.
 * @param {struct ListX *} drawnBalls     : refers to balls drawn in a new draw. 
 * @param {struct ListXY *} luckyBalls    : quartet combinations numbers list that drawn together (lucky numbers)
 * @param {char *} buf                    : If this parameter is not set to NULL, matching combinations are assigned to this address as string
 * @return {Integer}                      : Returns 1 if found, 0 if not.
 */
UINT8 search4CombX(struct ListX *aPrvDrawn, struct ListX *drawnBalls, struct ListXY *luckyBalls, char *buf);



/** 
 * The search3CombXY function searches for 3 combinations in all previous draws.
 *
 * @param {struct ListXY *} prvDrawnsList : refers to the balls has been drawn so far. 
 * @param {struct ListX *} drawnBalls     : refers to balls drawn in a new draw. 
 * @param {struct ListXY *} foundComb	  : refers to founded combinations.
 * @return {Integer}                      : Returns 1 if found, 0 if not. 
 */
UINT8 search3CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls, struct ListXY *foundComb);



/** 
 * The search3CombX function searches for 3 combinations in a previous draw. 
 * 
 * @param {struct ListX *} aPrvDrawn      : refers to a drawn balls in the previous draws.
 * @param {struct ListX *} drawnBalls     : refers to balls drawn in a new draw. 
 * @param {struct ListXY *} luckyBalls    : triple combinations numbers list that drawn together (lucky numbers)
 * @param {char *} buf                    : If this parameter is not set to NULL, matching combinations are assigned to this address as string
 * @return {Integer}                      : Returns 1 if found, 0 if not. 
 */
UINT8 search3CombX(struct ListX *aPrvDrawn, struct ListX *drawnBalls, struct ListXY *luckyBalls, char *buf);



/** 
 * The search2CombXY function searches for 2 combinations in all previous draws. 
 *
 * @param {struct ListXY *} prvDrawnsList : refers to the balls has been drawn so far. 
 * @param {struct ListX *} drawnBalls     : refers to balls drawn in a new draw. 
 * @param {struct ListXY *} foundComb	  : refers to founded combinations.
 * @return {Integer}                      : Returns 1 if found, 0 if not. 
 */
UINT8 search2CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls, struct ListXY *foundComb);



/** 
 * The search2CombX function searches for 2 combinations in a previous draw. 
 * 
 * @param {struct ListX *} aPrvDrawn      : refers to a drawn balls in the previous draws.
 * @param {struct ListX *} drawnBalls     : refers to balls drawn in a new draw. 
 * @param {struct ListXY *} luckyBalls    : double combinations numbers list that drawn together (lucky numbers)
 * @param {char *} buf                    : If this parameter is not set to NULL, matching combinations are assigned to this address as string
 * @return {Integer}                      : Returns 1 if found, 0 if not.
 */
UINT8 search2CombX(struct ListX *aPrvDrawn, struct ListX *drawnBalls, struct ListXY *luckyBalls, char *buf);



/** 
 * The search1BallXY function searches for 1 ball in a draws list. 
 *
 * @param {struct ListXY *} couponList    : refers to the balls has been new drawns. 
 * @param {struct ListX *} drawnBalls     : refers to balls drawn in a new draw. 
 * @param {Integer} drawBallCount     	  : DRAW_BALL for normal ball list or 1 for superStar list (coupon column count)
 * @return {Integer}                      : Returns 1 if found, 0 if not.
 */
UINT8 search1BallXY(struct ListXY *couponList, struct ListX *drawnBalls, UINT8 drawBallCount);



/** 
 * Matched combinations are assigned to {luckyBalls} and {buf}
 * 
 * @param {struct ListX *} aPrvDrawn      : refers to a drawn balls in the previous draws.
 * @param {struct ListX *} drawnBalls     : refers to balls drawn in a new draw. 
 * @param {struct ListXY *} luckyBalls    : matched combinations numbers list that drawn together (lucky numbers)
 * @param {char *} buf                    : matched combinations are assigned to this address as string
 * @param {char *} balls                  : matched numbers
 * @param {Integer} comb                  : double, triple, quartet, quintuple, or six combinations
 * @param {Integer} foundPrev             : If found previous search foundPrev > 1
 */
void foundComb(struct ListX *aPrvDrawn, struct ListX *drawnBalls, struct ListXY *luckyBalls, char *buf, UINT8 *balls, UINT8 comb, UINT8 foundPrev);



/**
 * Initialization
 * @return {Integer}	: returns 1 on success, 0 otherwise.
*/
UINT8 init();



/**
 * Calculate matching combinations count of numbers from previous draws
 * it prints the number of matching double, triple, quartet, quintuple, and six combinations.
*/
void calcMatchCombCount();



/**
 * Calculate matching combinations of numbers from previous draws
 * 
 * @param {Integer} comb    : If 2, it prints matching double combinations along with their dates
 *                            If 3, it prints matching triple combinations along with their dates
 *                            If 4, it prints matching quartet combinations along with their dates
 *                            If 5, it prints matching quintuple combinations along with their dates
 * @param {FILE *} fp       : refers to output file. If fp != NULL print to output file
*/
void calcMatchComb(UINT8 comb, FILE *fp);



/**
 * Get the numbers that drawn together from euro numbers
 * 
 * @param {struct ListXY *} euroNumbers    : refers to the euro numbers list
 * @return {struct ListXY *} euroNumbers   : refers to the euro numbers list
*/
struct ListXY * getEuroNumbers(struct ListXY *euroNumbers);



/**
 * Save the numbers that drawn together to file
 *
 * @param {struct ListXY *} euroNumbers : refers to the euro numbers list
 * @return {Integer}                    : returns 1 if success, otherwise returns 0
*/
UINT8 saveEuroNumbersToFile(struct ListXY *euroNumbers);



/**
 * Get the numbers that drawn together from file
 * 
 * @param {struct ListXY *} euroNumbers    : refers to the euroNumbers balls list
 * @return {struct ListXY *} euroNumbers   : refers to the euroNumbers balls list
*/
struct ListXY * getEuroNumbersFromFile(struct ListXY *euroNumbers);



/**
 * Get the numbers that drawn together from winning numbers (lucky numbers)
 *
 * @param {struct ListXY *} luckyBalls    : refers to the lucky balls list
 * @param {Integer} comb                  : double, triple or quartet (2, 3 or 4) combinations
 * @return {struct ListXY *} luckyBalls   : refers to the lucky balls list
*/
struct ListXY * getLuckyBalls(struct ListXY *luckyBalls, UINT8 comb);



/**
 * Save the numbers that drawn together to file
 * 
 * @param {struct ListXY *} luckyBalls  : refers to the lucky balls list
 * @param {Integer} comb                : double, triple or quartet (2, 3 or 4) combinations
 * @return {Integer}                    : returns 1 if success, otherwise returns 0
*/
UINT8 saveLuckyBallsToFile(struct ListXY *luckyBalls, UINT8 comb);



/**
 * Get the numbers that drawn together from file
 * 
 * @param {struct ListXY *} luckyBalls    : refers to the lucky balls list
 * @param {Integer} comb                  : double, triple or quartet (2, 3 or 4) combinations
 * @return {struct ListXY *} luckyBalls   : refers to the lucky balls list
*/
struct ListXY * getLuckyBallsFromFile(struct ListXY *luckyBalls, UINT8 comb);



/**
 * Draw balls by lucky numbers (the numbers that drawn together)
 *
 * @param {struct ListXY *} drawnBallsLucky : refers to the balls to be drawn. 
 * @param {Integer} drawNum                 : 1: lucky 3, 2: 2 of lucky3, 3: lucky 2
 * @param {Integer} totalBall               : Total ball count in the globe
 * @param {Integer} drawBallCount           : Number of balls to be drawn
 * @return {struct ListXY *} drawnBallsLucky: Returns new drawn balls.
*/
struct ListX * drawBallsByLucky(struct ListX *drawnBallsLucky, UINT8 drawNum, UINT8 totalBall, UINT8 drawBallCount);



/**
 * Draw balls
 * 
 * @param {struct ListXY *} coupon	: refers to the 2 dimensions balls list (coupon)
 * @param {Integer} totalBall		: total ball count
 * @param {Integer} drawBallCount	: number of balls to be drawn
 * @param {Integer} drawRowCount	: how many draws will be made
 * @param {Integer} drawByNorm		: If 1 draw, if 0 don't draw.
 * @param {Integer} drawByLeft		: If 1 draw, if 0 don't draw.
 * @param {Integer} drawByBlend1	: If 1 draw, if 0 don't draw.
 * @param {Integer} drawByBlend2	: If 1 draw, if 0 don't draw.
 * @param {Integer} drawBySide		: If 1 draw, if 0 don't draw.
 * @param {Integer} drawByRand		: If 1 draw, if 0 don't draw.
 * @param {Integer} drawByLucky		: If 1 draw, if 0 don't draw.
*/
void drawBalls(struct ListXY *coupon, UINT8 totalBall, UINT8 drawBallCount, UINT8 drawRowCount, UINT8 drawByNorm, UINT8 drawByLeft, UINT8 drawByBlend1, UINT8 drawByBlend2, UINT8 drawBySide, UINT8 drawByRand, UINT8 drawByLucky);



/** 
 * Get drawn balls lists from file has been drawn so far
 * The lists is assigned to the global variables winningDrawnBallsList or euNumberDrawnBallsList
 * 
 * @param {struct ListXY *}  : refer to 2 dimensions list of winning numbers (winningDrawnBallsList or euNumberDrawnBallsList)
 * @param {char *}           : Drawn list file
 * @return {Integer}         : returns 0 if fileName or record not found, otherwise returns the number of records.
 */
UINT16 getDrawnBallsList(struct ListXY *ballList, char *fileName);



/* FUNCTIONS */


struct ListX *createListX(struct ListX *pl, UINT8 size, char *label, UINT16 val, UINT16 val2, UINT16 year, UINT8 mon, UINT8 day)
{
	pl = (struct ListX *) malloc(sizeof(struct ListX));
	pl->balls = (UINT8 *) malloc(sizeof(UINT8)*size);
	pl->next = NULL;
	pl->label = NULL;
    
	if (label) {
		pl->label = (char *) malloc(sizeof(char)*22);
		strcpy(pl->label, label);
	}

	pl->index = 0;
	pl->size = size;
	pl->val = val;
	pl->val2 = val2;
	pl->year = year;
	pl->mon = mon;
	pl->day = day;

	return pl;
}



struct ListX2 *createListX2(struct ListX2 *pl, UINT8 size)
{
	pl = (struct ListX2 *) malloc(sizeof(struct ListX2));
	pl->balls = (UINT8 *) malloc(sizeof(UINT8)*size);
	pl->vals = (UINT16 *) malloc(sizeof(UINT16)*size);

	pl->index = 0;
	pl->size = size;

	return pl;
}



struct ListXY *createListXY(struct ListXY *pl)
{
	pl = (struct ListXY *) malloc(sizeof(struct ListXY));
	pl->list = NULL;

	return pl;
}



void insertItem2(struct ListX2 *pl, UINT8 key)
{
	UINT8 i;

	if (pl->index < pl->size) 
	{
		for (i=pl->index; i>0; i--) {
			pl->balls[i] = pl->balls[i-1];
		}

		pl->balls[0] = key;
		pl->index++;
	}
}



struct ListX *atEndY(struct ListXY *pl)
{
	struct ListX *curr;

	if (pl == NULL)
		return NULL;

	curr = pl->list;

	while (curr->next) {
		curr = curr->next;
	}

	return curr;
}



void appendItem(struct ListX *pl, UINT8 key)
{
	if (pl->index < pl->size) {
		pl->balls[pl->index] = key;
		pl->index++;
	}
}



void appendItem2(struct ListX2 *pl, UINT8 key, UINT16 val)
{
	if (pl->index < pl->size) {
		pl->balls[pl->index] = key;
		pl->vals[pl->index] = val;
		pl->index++;
	}
}



void appendItems(struct ListX *pl, UINT8 *keys)
{
	UINT8 i, size = strlen((char *)keys);
	
	if (pl->index + size <= pl->size) 
	{
		for (i=0; i< size && keys[i] != '\0'; i++) {
			pl->balls[pl->index+i] = keys[i];
		}

		pl->index += size;
	}
}



void insertList(struct ListXY *pl, struct ListX *newList)
{
	newList->next = pl->list;
	pl->list = newList;
}



void appendList(struct ListXY *pl, struct ListX *plNext)
{
	if (pl == NULL) return;

	if (pl->list == NULL) {
		pl->list = plNext;
	} else {
		(atEndY(pl))->next = plNext;
		plNext->next = NULL;
	}
}



UINT8 removeItemByIndex2(struct ListX2 *pl, UINT8 ind)
{
	UINT8 i, key = pl->balls[ind];

	for(i=ind; i<pl->index; i++) {
		pl->balls[i] = pl->balls[i+1];
	}

	pl->balls[i] = '\0';
	pl->index--;

	return key;
}



struct ListX * removeListByIndex(struct ListXY *pl, UINT16 ind)
{
	struct ListX *prv, *pt = pl->list;
	UINT16 i;
	struct ListX *list;

	if (pt) 
	{
		if (ind == 0) {
			list = pt;
			pl->list = pt->next;
		} else {
			for (i=1, prv=pt, pt=pt->next; (pt) && (i<ind); prv=prv->next, pt=pt->next, i++);

			if (i == ind) {
				list = pt;
				prv->next = pt->next;
			}
		}
	}

	return list;
}



struct ListXY * shuffleListXY(struct ListXY *pl)
{
	UINT16 i, index;
	struct ListX *list;
	UINT16 listLen = lengthY(pl);
	UINT16 shuffleList = (UINT16) ((listLen * (rand() % 6 + (UINT16) ceil(listLen/2) - 5) * (rand() % 6 + (UINT16) ceil(listLen/3) - 5)));

	for (i=0; i<shuffleList; i++)
	{
		index = rand() % listLen;
		list = removeListByIndex(pl, index);

		if (index < (UINT16) ceil(listLen/3)) {
			insertList(pl, list);
		}
		else if (index < (UINT16) ceil(listLen/2)) {
			addListByIndex(pl, list, (UINT16) ceil(2*listLen/3));
		}
		else if (index < (UINT16) ceil(2*listLen/3)) {
			addListByIndex(pl, list, (UINT16) ceil(listLen/3));
		}
		else {
			appendList(pl, list);
		}
	}

	return pl;
}



UINT8 addItemByIndex2(struct ListX2 *pl, UINT8 ind, UINT8 key)
{
	UINT8 i;

	if (pl->index < pl->size) 
	{
		for(i=pl->index; i>ind; i--) {
			pl->balls[i] = pl->balls[i-1];
		}

		pl->balls[i] = key;
		pl->index++;
	} 
	else {
		return 0;
	}

	return 1;
}



void addListByIndex(struct ListXY *pl, struct ListX *newList, UINT16 ind)
{
	UINT16 i;
	struct ListX *right = NULL, *left = pl->list;

	if (left == NULL || ind == 0) 
	{
		pl->list = newList;
		newList->next = left;
	} 
	else 
	{
		right = left->next;

		for (i=1; i<ind; i++) {
			if (left != NULL) left = left->next;
			if (right != NULL) right = right->next;
		}

		if (left == NULL) return;

		left->next = newList;
		newList->next = right;
	}
}



void removeAllX(struct ListX *pl)
{
	UINT8 i;

	for (i=0; i<pl->size; i++) {
		pl->balls[i] = '\0';
	}
    
	pl->index = 0;
}



void removeAllX2(struct ListX2 *pl)
{
	UINT8 i;

	for (i=0; i<pl->size; i++) {
		pl->balls[i] = '\0';
		pl->vals[i] = '\0';
	}
    
	pl->index = 0;
}



void removeAllXY(struct ListXY *pl)
{
	struct ListX *prvList, *pList = pl->list;

	while (pList != NULL) 
	{
		prvList = pList;
		pList = pList->next;
		//removeAllX(prvList);
		free(prvList->balls);
		if (prvList->label) free(prvList->label);
		free(prvList);
	} 

	pl->list = NULL;
}



UINT16 lengthY(struct ListXY *pl)
{
	struct ListX *tmp = pl->list;
	UINT16 i=0;

	while (tmp)
	{
		i++;
		tmp = tmp->next;
	}

	return i;
}



UINT8 seqSearchX(struct ListX *pl, UINT8 *keys, UINT8 size)
{
	UINT8 found=0;
	UINT8 i, j;

	for (i=0; i < pl->index -(size-1); i++) 
	{
		for (j=0; j < size; j++) {
			if (pl->balls[i+j] == keys[j]) found++;
		}

		if (found == size) break;
	}

	return found;
}



int seqSearchX1(struct ListX *pl, UINT8 key)
{
	UINT8 i;

	for (i=0; i<pl->index; i++) 
	{
		if (pl->balls[i] == key) return i;
	}

	return -1;
}



int seqSearchX2(struct ListX2 *pl, UINT8 key)
{
	UINT8 i;

	for (i=0; i<pl->index; i++) 
	{
		if (pl->balls[i] == key) return i;
	}

	return -1;
}



int seqSearchXY(struct ListXY *pl, UINT8 *keys, UINT8 size)
{
	struct ListX *tmp = pl->list;
	UINT16 i=0;

	while (tmp)
	{
		if (seqSearchX(tmp, keys, size) == size) break;
		tmp = tmp->next;
		i++;
	}

	if (tmp) return i;
	else return -1;
}



void incVal2(struct ListX2 *pl, UINT8 ind)
{
    pl->vals[ind]++;
}



void incValXY(struct ListXY *pl, UINT16 ind)
{
	struct ListX *tmp = pl->list;
	UINT16 i;

	for (i=0; (tmp) && i<ind; i++) 
	{
		tmp=tmp->next;
	}

	tmp->val++;
}



UINT16 getValXY(struct ListXY *pl, UINT16 ind)
{
	struct ListX *tmp = pl->list;
	UINT16 i;

	for (i=0; (tmp) && i<ind; i++) 
	{
		tmp=tmp->next;
	}

	return tmp->val;
}



UINT8 getKey(struct ListX *pl, UINT8 index)
{
	return pl->balls[index];
}



UINT8 getKey2(struct ListX2 *pl, UINT8 index)
{
	return pl->balls[index];
}



void getKeys(struct ListX *pl, UINT8 *keys, UINT8 index, UINT8 count)
{
	UINT8 i, j;

	if (count == 0) count = pl->index;

	for (i=index, j=0; j<count; i++, j++) 
	{
		keys[j] = pl->balls[i];
	}

	keys[j] = '\0';
}



struct ListX * getListXByIndex(struct ListXY *pl, UINT16 ind)
{
	struct ListX *l = pl->list;
	UINT16 i;

	for (i=0; (l) && i<ind; i++) 
	{
		l = l->next;
	}

	return l;
}



void bubbleSortX2ByVal(struct ListX2 *pl, int inc)
{
	UINT8 i;
	UINT8 tmp, swapped;
	UINT16 tmp2;

	if (pl->index < 2) return;

	do {
		swapped = 0;

		for (i=0; i < pl->index -1; i++)
		{
			if ((inc == 1 && pl->vals[i] > pl->vals[i+1]) || (inc == -1 && pl->vals[i] < pl->vals[i+1])) 
			{
				tmp = pl->balls[i];
				pl->balls[i] = pl->balls[i+1];
				pl->balls[i+1] = tmp;

				tmp2 = pl->vals[i];
				pl->vals[i] = pl->vals[i+1];
				pl->vals[i+1] = tmp2;

				swapped = 1;
			}
		}
	} while (swapped);
}



void bubbleSortYByVal(struct ListXY *pl, int inc)
{
	UINT8 swapped;
	struct ListX *ptr1;
	struct ListX *lptr = NULL;

	if (pl->list == NULL) return;

	do {
		swapped = 0;
		ptr1 = pl->list;

		while (ptr1->next != lptr)
		{
			if ((inc == 1 && ptr1->val > ptr1->next->val) || (inc == -1 && ptr1->val < ptr1->next->val)) {
				swapY(ptr1, ptr1->next);
				swapped = 1;
			}
			ptr1 = ptr1->next;
		}

		lptr = ptr1;

	} while (swapped);
}



void bubbleSortYByVal2(struct ListXY *pl, int inc)
{
	UINT8 swapped;
	struct ListX *ptr1;
	struct ListX *lptr = NULL;

	if (pl->list == NULL) return;

	do {
		swapped = 0;
		ptr1 = pl->list;

		while (ptr1->next != lptr)
		{
			if ((inc == 1 && ptr1->val2 > ptr1->next->val2) || (inc == -1 && ptr1->val2 < ptr1->next->val2)) {
				swapY(ptr1, ptr1->next);
				swapped = 1;
			}
			ptr1 = ptr1->next;
		}

		lptr = ptr1;

	} while (swapped);
}



void bubbleSortXByKey(struct ListX *pl)
{
	UINT8 i;
	UINT8 tmp, swapped;

	if (pl->index < 2) return;

	do {
		swapped = 0;

		for (i=0; i < pl->index -1; i++)
		{
			if (pl->balls[i] > pl->balls[i+1]) {
				tmp = pl->balls[i];
				pl->balls[i] = pl->balls[i+1];
				pl->balls[i+1] = tmp;
				swapped = 1;
			}
		}
	} while (swapped);
}



void swapY(struct ListX *a, struct ListX *b)
{
	char *label;
	UINT8 *balls;
	UINT8 day, mon;
	UINT16 year, val, val2;

	balls = a->balls;
	a->balls = b->balls;
	b->balls = balls;

	label = a->label;
	a->label = b->label;
	b->label = label;

	day = a->day;
	a->day = b->day;
	b->day = day;

	mon = a->mon;
	a->mon = b->mon;
	b->mon = mon;

	year = a->year;
	a->year = b->year;
	b->year = year;

	val = a->val;
	a->val = b->val;
	b->val = val;

	val2 = a->val2;
	a->val2 = b->val2;
	b->val2 = val2;
}



void printListXByKey(struct ListX *pl, UINT8 printTo, FILE *fp)
{
	UINT8 i;
	char buf[5];

	for (i=0; i < pl->index; i++) 
	{
		sprintf(buf, "%2d ", pl->balls[i]);
		// If printTo == 0 print to screen and output file, 1: print to screen only, 2: print to output file only
		if (printTo == 0 || printTo == 1) {
			printf("%s", buf);
		}
		if ((printTo == 0 || printTo == 2) && fp != NULL) {
			fputs(buf, fp);
		}
	}
}



void printListXYWithENByKey(struct ListXY *pl1, struct ListXY *pl2, FILE *fp)
{
	struct ListX *nl1 = pl1->list;
	struct ListX *nl2 = pl2->list;
	char ioBuf[60];
	char buf[30];
	UINT8 i, j;

	puts("     Numbers       EuroNumbers\n");
	if (fp != NULL) fputs("     Numbers       EuroNumbers\n\n", fp);

	for (i=1; (nl1) && (nl2); i++)
	{
		ioBuf[0] = '\0';

		sprintf(buf, "%2d - ", i);
		strcat(ioBuf, buf);

		for (j=0; j < nl1->index; j++) {
			sprintf(buf, "%2d ", nl1->balls[j]);
			strcat(ioBuf, buf);
		}

		strcat(ioBuf, "  ");

		for (j=0; j < nl2->index; j++) {
			sprintf(buf, "%2d ", nl2->balls[j]);
			strcat(ioBuf, buf);
		}

		if (nl1->label) {
			sprintf(buf, "     %s", nl1->label);
			strcat(ioBuf, buf);
		}

		nl1 = nl1->next;
		nl2 = nl2->next;

		strcat(ioBuf, "\n");

		printf("%s", ioBuf);
		if (fp != NULL) fputs(ioBuf, fp);
	}
}



void printDrawnBallCount(struct ListX2 *ballSortOrder)
{
	UINT8 i;

#ifdef __MSDOS__
	UINT8 col = 10;
#else
	UINT8 col = 15;
#endif

	for (i=0; i < ballSortOrder->index; i++) 
	{
		printf("%2d:%3d", ballSortOrder->balls[i], ballSortOrder->vals[i]);

		if ((i+1) % col == 0) puts("");
		else printf("  ");
	}
}



void printLuckyBalls(struct ListXY *pl, FILE *fp)
{
	struct ListX *nl = NULL;
	UINT16 i = 0;
	UINT8 j;
	UINT8 col, len;
	char ioBuf[50];
	char buf[20];

	 if (pl == NULL || pl->list == NULL) 
	 {
		puts("No matched found.\n");
		if (fp != NULL) fputs("No matched found.\n", fp);

		return;
	 }

	nl = pl->list;
	len = nl->index;

#ifdef __MSDOS__
	col = 6-len;
	if (col<3) col = 3;
#else
	col = 8-len;
#endif

	while (nl)
	{
		ioBuf[0] = '\0';
		
		for (j=0; j < nl->index; j++) {
			sprintf(buf, "%2d ", nl->balls[j]);
			strcat(ioBuf, buf);
		}

		sprintf(buf, ": %2d times", nl->val);
		strcat(ioBuf, buf);

		if ((i+1) % col == 0) {
			strcat(ioBuf, "\n");
		} else {
			strcat(ioBuf, "    ");
		}

		i++;
		nl = nl->next;

		printf("%s", ioBuf);
		if (fp != NULL) fprintf(fp, "%s", ioBuf);
	}
}



UINT16 numberOfTerm(UINT16 sum)
{
	UINT16 n;
	SINT32 sum2 = (SINT32) sum;

	for (n=0; sum2>0; n++) 
	{
		sum2 = sum2 - (n+1);
	}

	return n;
}



UINT8 gaussIndex(UINT8 ballCount)
{
	UINT16 node=0, leftnode=0, level;

	for (level=1; level<ballCount; level++)
	{
		if (rand() % 100 < 49 + rand() % 2) {
			node = node + level;
		} else {
			node = node + level + 1;
		}

		leftnode = leftnode + level;
	}

	return node - (leftnode-1);
}



UINT8 search5CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls)
{
	UINT16 i, listRows;
	UINT8 balls[DRAW_BALL+1];
	struct ListX *aPrvDrawn = NULL;
	UINT8 found = 0;

	if (prvDrawnsList == winningDrawnBallsList) listRows = winningBallRows;
	else listRows = lengthY(prvDrawnsList);

	getKeys(drawnBalls, balls, 0, 0);

	aPrvDrawn = prvDrawnsList->list;

	for (i=0; aPrvDrawn && i<listRows; i++) 
	{
		if (aPrvDrawn->balls[0] <= balls[0] && seqSearchX(aPrvDrawn, balls, 5) == 5) {
			found=1;
			break;
		}

		aPrvDrawn = aPrvDrawn->next;
	}

	return found;
}



UINT8 search5CombX(struct ListX *aPrvDrawn, struct ListX *drawnBalls)
{
	UINT8 balls[DRAW_BALL+1];
	UINT8 found = 0;

	getKeys(drawnBalls, balls, 0, 0);

	if (aPrvDrawn->balls[0] <= balls[0] && seqSearchX(aPrvDrawn, balls, 5) == 5) found=1;

	return found;
}



UINT8 search4CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls, struct ListXY *foundComb)
{
	UINT16 i, listRows;
	UINT8 b1[DRAW_BALL+1];
	UINT8 b2[5];
	struct ListX *aPrvDrawn = NULL;
	struct ListX *fc = NULL;
	UINT8 found = 0;

	if (prvDrawnsList == winningDrawnBallsList) listRows = winningBallRows;
	else listRows = lengthY(prvDrawnsList);

	getKeys(drawnBalls, b1, 0, drawnBalls->size);

	if (foundComb != NULL) removeAllXY(foundComb);

	aPrvDrawn = prvDrawnsList->list;

	for (i=0; aPrvDrawn && i<listRows; i++)
	{
		b2[0] = b1[0];
		b2[1] = b1[1];
		b2[2] = b1[2];
		b2[3] = b1[3];
		b2[4] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 4) == 4) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 4, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		//b2[0] = b1[0];
		//b2[1] = b1[1];
		//b2[2] = b1[2];
		b2[3] = b1[4];
		//b2[4] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 4) == 4) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 4, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}


		//b2[0] = b1[0];
		//b2[1] = b1[1];
		b2[2] = b1[3];
		//b2[3] = b1[4];
		//b2[4] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 4) == 4) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 4, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}


		//b2[0] = b1[0];
		b2[1] = b1[2];
		//b2[2] = b1[3];
		//b2[3] = b1[4];
		//b2[4] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 4) == 4) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 4, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}


		b2[0] = b1[1];
		//b2[1] = b1[2];
		//b2[2] = b1[3];
		//b2[3] = b1[4];
		//b2[4] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 4) == 4) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 4, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		aPrvDrawn = aPrvDrawn->next;
	}

	return found;
}



UINT8 search4CombX(struct ListX *aPrvDrawn, struct ListX *drawnBalls, struct ListXY *luckyBalls, char *buf)
{
	UINT8 b1[DRAW_BALL+1];
	UINT8 b2[5];
	UINT8 found = 0;

	getKeys(drawnBalls, b1, 0, drawnBalls->size);

	if (buf) buf[0] = '\0';

	b2[0] = b1[0];
	b2[1] = b1[1];
	b2[2] = b1[2];
	b2[3] = b1[3];
	b2[4] = '\0';

	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 4) == 4)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 4, found);
		found++;
	}

	//b2[0] = b1[0];
	//b2[1] = b1[1];
	//b2[2] = b1[2];
	b2[3] = b1[4];
	//b2[4] = '\0';

	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 4) == 4)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 4, found);
		found++;
	}

	//b2[0] = b1[0];
	//b2[1] = b1[1];
	b2[2] = b1[3];
	//b2[3] = b1[4];
	//b2[4] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 4) == 4)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 4, found);
		found++;
	}

	//b2[0] = b1[0];
	b2[1] = b1[2];
	//b2[2] = b1[3];
	//b2[3] = b1[4];
	//b2[4] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 4) == 4)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 4, found);
		found++;
	}

	b2[0] = b1[1];
	//b2[1] = b1[2];
	//b2[2] = b1[3];
	//b2[3] = b1[4];
	//b2[4] = '\0';

	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 4) == 4)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 4, found);
		found++;
	}

	return found;
}



UINT8 search3CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls, struct ListXY *foundComb)
{
	UINT16 i, listRows;
	UINT8 b1[DRAW_BALL+1];
	UINT8 b2[4];
	struct ListX *aPrvDrawn = NULL;
	struct ListX *fc = NULL;
	UINT8 found = 0;

	if (prvDrawnsList == winningDrawnBallsList) listRows = winningBallRows;
	else listRows = lengthY(prvDrawnsList);

	getKeys(drawnBalls, b1, 0, drawnBalls->size);

	if (foundComb != NULL) removeAllXY(foundComb);

	aPrvDrawn = prvDrawnsList->list;

	for (i=0; aPrvDrawn && i<listRows; i++)
	{
		b2[0] = b1[0];
		b2[1] = b1[1];
		b2[2] = b1[2];
		b2[3] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 3, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		//b2[0] = b1[0];
		//b2[1] = b1[1];
		b2[2] = b1[3];
		//b2[3] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 3, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		//b2[0] = b1[0];
		//b2[1] = b1[1];
		b2[2] = b1[4];
		//b2[3] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 3, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}


		//b2[0] = b1[0];
		b2[1] = b1[2];
		b2[2] = b1[3];
		//b2[3] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 3, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		//b2[0] = b1[0];
		//b2[1] = b1[2];
		b2[2] = b1[4];
		//b2[3] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 3, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}


		//b2[0] = b1[0];
		b2[1] = b1[3];
		//b2[2] = b1[4];
		//b2[3] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 3, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}


		b2[0] = b1[1];
		b2[1] = b1[2];
		b2[2] = b1[3];
		//b2[3] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 3, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		//b2[0] = b1[1];
		//b2[1] = b1[2];
		b2[2] = b1[4];
		//b2[3] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 3, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}


		//b2[0] = b1[1];
		b2[1] = b1[3];
		//b2[2] = b1[4];
		//b2[3] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 3, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}


		b2[0] = b1[2];
		//b2[1] = b1[3];
		//b2[2] = b1[4];
		//b2[3] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 3, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		aPrvDrawn = aPrvDrawn->next;
	}

	return found;
}



UINT8 search3CombX(struct ListX *aPrvDrawn, struct ListX *drawnBalls, struct ListXY *luckyBalls, char *buf)
{
	UINT8 b1[DRAW_BALL+1];
	UINT8 b2[4];
	UINT8 found = 0;

	getKeys(drawnBalls, b1, 0, drawnBalls->size);

	if (buf) buf[0] = '\0';

	b2[0] = b1[0];
	b2[1] = b1[1];
	b2[2] = b1[2];
	b2[3] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 3, found);
		found++;
	}

	//b2[0] = b1[0];
	//b2[1] = b1[1];
	b2[2] = b1[3];
	//b2[3] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 3, found);
		found++;
	}

	//b2[0] = b1[0];
	//b2[1] = b1[1];
	b2[2] = b1[4];
	//b2[3] = '\0';

	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 3, found);
		found++;
	}

	//b2[0] = b1[0];
	b2[1] = b1[2];
	b2[2] = b1[3];
	//b2[3] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 3, found);
		found++;
	}

	//b2[0] = b1[0];
	//b2[1] = b1[2];
	b2[2] = b1[4];
	//b2[3] = '\0';

	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 3, found);
		found++;
	}

	//b2[0] = b1[0];
	b2[1] = b1[3];
	//b2[2] = b1[4];
	//b2[3] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 3, found);
		found++;
	}

	b2[0] = b1[1];
	b2[1] = b1[2];
	b2[2] = b1[3];
	//b2[3] = '\0';

	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 3, found);
		found++;
	}

	//b2[0] = b1[1];
	//b2[1] = b1[2];
	b2[2] = b1[4];
	//b2[3] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 3, found);
		found++;
	}

	//b2[0] = b1[1];
	b2[1] = b1[3];
	//b2[2] = b1[4];
	//b2[3] = '\0';

	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 3, found);
		found++;
	}

	b2[0] = b1[2];
	//b2[1] = b1[3];
	//b2[2] = b1[4];
	//b2[3] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 3) == 3)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 3, found);
		found++;
	}

	return found;
}



UINT8 search2CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls, struct ListXY *foundComb)
{
	UINT16 i, listRows;
	UINT8 b1[DRAW_BALL+1];
	UINT8 b2[3];
	struct ListX *aPrvDrawn = NULL;
	struct ListX *fc = NULL;
	UINT8 found = 0;

	if (prvDrawnsList == winningDrawnBallsList) listRows = winningBallRows;
	else listRows = lengthY(prvDrawnsList);

	getKeys(drawnBalls, b1, 0, drawnBalls->size);

	if (foundComb != NULL) removeAllXY(foundComb);

	aPrvDrawn = prvDrawnsList->list;

	for (i=0; aPrvDrawn && i<listRows; i++) 
	{
		b2[0] = b1[0];
		b2[1] = b1[1];
		b2[2] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 2, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		//b2[0] = b1[0];
		b2[1] = b1[2];
		//b2[2] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 2, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		//b2[0] = b1[0];
		b2[1] = b1[3];
		//b2[2] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 2, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		//b2[0] = b1[0];
		b2[1] = b1[4];
		//b2[2] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 2, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}


		b2[0] = b1[1];
		b2[1] = b1[2];
		//b2[2] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 2, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		//b2[0] = b1[1];
		b2[1] = b1[3];
		//b2[2] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 2, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		//b2[0] = b1[1];
		b2[1] = b1[4];
		//b2[2] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 2, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}


		b2[0] = b1[2];
		b2[1] = b1[3];
		//b2[2] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 2, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		//b2[0] = b1[2];
		b2[1] = b1[4];
		//b2[2] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 2, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}


		b2[0] = b1[3];
		//b2[1] = b1[4];
		//b2[2] = '\0';

		if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2) 
		{
			found++; 

			if (foundComb == NULL) {
				break;
			} else {
				fc = createListX(fc, 2, NULL, aPrvDrawn->val, aPrvDrawn->val2, aPrvDrawn->year, aPrvDrawn->mon, aPrvDrawn->day);
				appendItems(fc, b2);
				appendList(foundComb, fc);
			}
		}

		aPrvDrawn = aPrvDrawn->next;
	}

	return found;
}



UINT8 search2CombX(struct ListX *aPrvDrawn, struct ListX *drawnBalls, struct ListXY *luckyBalls, char *buf)
{
	UINT8 b1[DRAW_BALL+1];
	UINT8 b2[3];
	UINT8 found = 0;

	getKeys(drawnBalls, b1, 0, drawnBalls->size);

	if (buf) buf[0] = '\0';

	b2[0] = b1[0];
	b2[1] = b1[1];
	b2[2] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 2, found);
		found++;
	}

	//b2[0] = b1[0];
	b2[1] = b1[2];
	//b2[2] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 2, found);
		found++;
	}

	//b2[0] = b1[0];
	b2[1] = b1[3];
	//b2[2] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 2, found);
		found++;
	}

	//b2[0] = b1[0];
	b2[1] = b1[4];
	//b2[2] = '\0';

	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 2, found);
		found++;
	}

	b2[0] = b1[1];
	b2[1] = b1[2];
	//b2[2] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 2, found);
		found++;
	}

	//b2[0] = b1[1];
	b2[1] = b1[3];
	//b2[2] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 2, found);
		found++;
	}

	//b2[0] = b1[1];
	b2[1] = b1[4];
	//b2[2] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 2, found);
		found++;
	}

	b2[0] = b1[2];
	b2[1] = b1[3];
	//b2[2] = '\0';

	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 2, found);
		found++;
	}

	//b2[0] = b1[2];
	b2[1] = b1[4];
	//b2[2] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 2, found);
		found++;
	}

	b2[0] = b1[3];
	//b2[1] = b1[4];
	//b2[2] = '\0';
		
	if (aPrvDrawn->balls[0] <= b2[0] && seqSearchX(aPrvDrawn, b2, 2) == 2)
	{
		if (buf || luckyBalls) foundComb(aPrvDrawn, drawnBalls, luckyBalls, buf, b2, 2, found);
		found++;
	}

	return found;
}



UINT8 search1BallXY(struct ListXY *couponList, struct ListX *drawnBalls, UINT8 drawBallCount)
{
	UINT16 i;
	UINT8 j;
	struct ListX *aPrvDrawn = NULL;
	UINT8 balls[DRAW_BALL+1];
	UINT8 found = 0;

	getKeys(drawnBalls, balls, 0, drawBallCount);

	aPrvDrawn = couponList->list;

	for (i=0; aPrvDrawn && i<lengthY(couponList); i++) 
	{
		for (j=0; j<drawBallCount; j++) 
		{
			if (aPrvDrawn->balls[0] <= balls[j] && seqSearchX1(aPrvDrawn, balls[j]) >= 0) {
				found=1; 
				break;
			}
		}

		if (found) break;

		aPrvDrawn = aPrvDrawn->next;
	}

	return found;
}



void foundComb(struct ListX *aPrvDrawn, struct ListX *drawnBalls, struct ListXY *luckyBalls, char *buf, UINT8 *balls, UINT8 comb, UINT8 foundPrev)
{
	struct ListX *lb = NULL;
	int index;
	char fStr[14];
	UINT16 dDiff;

	if (buf) 
	{
		if (foundPrev) strcat(buf, ", ");

		switch (comb)
		{
		case 2 : sprintf(fStr, "(%2d,%2d)", balls[0], balls[1]);
		break;
		case 3 : sprintf(fStr, "(%2d,%2d,%2d)", balls[0], balls[1], balls[2]);
		break;
		case 4 : sprintf(fStr, "(%2d,%2d,%2d,%2d)", balls[0], balls[1], balls[2], balls[3]);
		break;
		default: break;
		}

		strcat(buf, fStr);
	}

	if (luckyBalls) 
	{
		if ((index = seqSearchXY(luckyBalls, balls, comb)) < 0) {
			dDiff = dateDiff(aPrvDrawn->day, aPrvDrawn->mon, aPrvDrawn->year, drawnBalls->day, drawnBalls->mon, drawnBalls->year);
			lb = createListX(lb, comb, NULL, 1, dDiff, drawnBalls->year, drawnBalls->mon, drawnBalls->day);
			appendItems(lb, balls);
			appendList(luckyBalls, lb);
		} else {
			lb = getListXByIndex(luckyBalls, index);
			dDiff = dateDiff(aPrvDrawn->day, aPrvDrawn->mon, aPrvDrawn->year, lb->day, lb->mon, lb->year);
			lb->val2 = (UINT16) ceil(dDiff / numberOfTerm(lb->val));
			lb->val++;
		}
	}
}



UINT8 init()
{
	time_t rawtime;
	struct tm *timeInfo;

	struct ListX *tmp = NULL;
	struct ListX *tmp2 = NULL;
	int err;
	char realPath[PATH_MAX];
	cwd = (char *) malloc(sizeof(char)*PATH_MAX);
	fileStats = (char *) malloc(sizeof(char)*PATH_MAX);
	outputFile = (char *) malloc(sizeof(char)*PATH_MAX);

	time(&rawtime);
	timeInfo = localtime(&rawtime);

	fileStats[0] = '\0';
	outputFile[0] = '\0';

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(SOLARIS) || defined(WIN32)
	err = get_app_path(realPath, PATH_MAX);
	if (err) {
		puts("App path lookup failed!");
		return 0;
	}
	else {
		cwd = dirname(realPath);

		strcat(fileStats, cwd);
		strcat(fileStats, PATH_SEPARATOR);
		strcat(outputFile, cwd);
		strcat(outputFile, PATH_SEPARATOR);
	}
#endif

	strcat(fileStats, FILESTATS);
	strcat(outputFile, OUTPUTFILE);

	puts("Initializing... Please wait.");

	winningDrawnBallsList = createListXY(winningDrawnBallsList);
	euNumberDrawnBallsList = createListXY(euNumberDrawnBallsList);

	if (!(winningBallRows = getDrawnBallsList(winningDrawnBallsList, fileStats))) {
		printf("%s file or record not found!\n", fileStats);
		return 0;
	}

	#ifndef __MSDOS__
	if (!(euNumberBallRows = getDrawnBallsList(euNumberDrawnBallsList, fileStats))) {
		printf("%s file or record not found!\n", fileStats);
		return 0;
	}
	#endif

	tmp = getListXByIndex(winningDrawnBallsList, winningBallRows-1);
	formatDate(dateStart, tmp->day, tmp->mon, tmp->year);
	tmp2 = winningDrawnBallsList->list;
	formatDate(dateEnd, tmp2->day, tmp2->mon, tmp2->year);

	drawnDays = dateDiff(tmp->day, tmp->mon, tmp->year, tmp2->day, tmp2->mon, tmp2->year);

	currDay = (UINT8) timeInfo->tm_mday;
	currMon = (UINT8) timeInfo->tm_mon +1;
	currYear = (UINT16) timeInfo->tm_year +1900;

	luckyBalls2 = createListXY(luckyBalls2);
	luckyBalls2 = getLuckyBalls(luckyBalls2, 2);
	bubbleSortYByVal(luckyBalls2, -1);

	#if defined(__MSDOS__)
	removeAllXY(winningDrawnBallsList);
	if (!saveLuckyBallsToFile(luckyBalls2, 2)) return 0;
	removeAllXY(luckyBalls2);
	if (!(winningBallRows = getDrawnBallsList(winningDrawnBallsList, fileStats))) {
		printf("%s file or record not found!\n", fileStats);
		return 0;
	}
	#endif

	luckyBalls3 = createListXY(luckyBalls3);
	luckyBalls3 = getLuckyBalls(luckyBalls3, 3);
	bubbleSortYByVal(luckyBalls3, -1);

	#if defined(__MSDOS__)
	removeAllXY(winningDrawnBallsList);
	if (!saveLuckyBallsToFile(luckyBalls3, 3)) return 0;
	removeAllXY(luckyBalls3);
	if (!(winningBallRows = getDrawnBallsList(winningDrawnBallsList, fileStats))) {
		printf("%s file or record not found!\n", fileStats);
		return 0;
	}
	#endif

	luckyBalls4 = createListXY(luckyBalls4);
	luckyBalls4 = getLuckyBalls(luckyBalls4, 4);
   	bubbleSortYByVal(luckyBalls4, -1);

	#if defined(__MSDOS__)
	removeAllXY(winningDrawnBallsList);
	if (!saveLuckyBallsToFile(luckyBalls4, 4)) return 0;
	removeAllXY(luckyBalls4);
	if (!(euNumberBallRows = getDrawnBallsList(euNumberDrawnBallsList, fileStats))) {
		printf("%s file or record not found!\n", fileStats);
		return 0;
	}
	#endif

	euroNumbers = createListXY(euroNumbers);
	euroNumbers = getEuroNumbers(euroNumbers);
	bubbleSortYByVal(euroNumbers, -1);

	#if defined(__MSDOS__)
	if (!saveEuroNumbersToFile(euroNumbers)) return 0;
	removeAllXY(euroNumbers);
	if (!(winningBallRows = getDrawnBallsList(winningDrawnBallsList, fileStats))) {
		printf("%s file or record not found!\n", fileStats);
		return 0;
	}
	#endif

	winningBallsDrawCount = createListX2(winningBallsDrawCount, TOTAL_BALL);
	euNumberBallsDrawCount = createListX2(euNumberBallsDrawCount, TOTAL_BALL_EN);
	getDrawnBallCount();

	calcMatchCombCount();
	clearScreen();

	#ifdef __MSDOS__
	free(cwd);
	free(fileStats);
	cwd = NULL;
	fileStats = NULL;
	#endif

	return 1;
}



struct ListXY * getEuroNumbers(struct ListXY *euroNumbers)
{
	UINT16 i, j;
	struct ListX *aPrvDrawn1 = NULL;
	struct ListX *aPrvDrawn2 = NULL;
	struct ListX *list = NULL;

	aPrvDrawn1 = euNumberDrawnBallsList->list;

	for (i=0; aPrvDrawn1 && i < euNumberBallRows; i++) 
	{
		aPrvDrawn2 = aPrvDrawn1->next;

		for (j=i+1; aPrvDrawn2 && j < euNumberBallRows; j++) 
		{
			search2CombX(aPrvDrawn2, aPrvDrawn1, euroNumbers, NULL);

			aPrvDrawn2 = aPrvDrawn2->next;
		}

		aPrvDrawn1 = aPrvDrawn1->next;
	}

	list = euroNumbers->list;

	while (list) {
		list->val = numberOfTerm(list->val) +1;
		list = list->next;
	}

	return euroNumbers;
}



UINT8 saveEuroNumbersToFile(struct ListXY *euroNumbers)
{
	struct ListX *nl = NULL;
	char lsFile[14];
	char ioBuf[20];
	char buf[5];
	UINT8 i;
	FILE *fp;

	strcpy(lsFile, EUNMSWAPFILE);

	if ((fp = fopen(lsFile, "w")) == NULL) {
		printf("Can't open file %s\n", lsFile);
		pressAnyKeyToExit();
		return 0;
	}

	if (euroNumbers == NULL || euroNumbers->list == NULL) {
		return 0;
	}

	nl = euroNumbers->list;

	while (nl)
	{
		ioBuf[0] = '\0';

		for (i=0; i < nl->index; i++) {
			sprintf(buf, "%02d", nl->balls[i]);
			strcat(ioBuf, buf);
		}

		sprintf(buf, "%d\n", nl->val);
		strcat(ioBuf, buf);

		nl = nl->next;

		fputs(ioBuf, fp);
	}

	fclose(fp);

	return 1;
}



struct ListXY * getEuroNumbersFromFile(struct ListXY *euroNumbers)
{
	UINT16 i;
	UINT8 j, k;
	int n1, n2, val;
	//UINT8 keys[DRAW_BALL+1];
	char lsFile[14];
	char ioBuf[20];
	char s[6];
	FILE *fp;

	struct ListX *drawList = NULL;

	strcpy(lsFile, EUNMSWAPFILE);


	if ((fp = fopen(lsFile, "r")) == NULL) {
		printf("Can't open file %s\n", lsFile);
		pressAnyKeyToExit();
		return NULL;
	}

	for (i=0; fgets(ioBuf, 20, fp) && i<UINT16MAX; i++)
	{
		s[0] = ioBuf[0];
		s[1] = ioBuf[1];
		s[2] = '\0';
		n1 = atoi(s);

		s[0] = ioBuf[2];
		s[1] = ioBuf[3];
		s[2] = '\0';
		n2 = atoi(s);

		for (j=0, k=4; !(ioBuf[k]=='\n'||ioBuf[k]=='\0'); j++, k++){
			s[j] = ioBuf[k];
		}
		s[j] = '\0';
		val = atoi(s);

		drawList = createListX(drawList, 2, NULL, (UINT16) val, 0, 0, 0, 0);
/*
		keys[0] = (UINT8) n1;
		keys[1] = (UINT8) n2;
		keys[2] = '\0';

		appendItems(drawList, keys);
*/
		drawList->balls[0] = (UINT8) n1;
		drawList->balls[1] = (UINT8) n2;
		drawList->index += 2;

		appendList(euroNumbers, drawList);
	}

	fclose(fp);

	return euroNumbers;
}



struct ListXY * getLuckyBalls(struct ListXY *luckyBalls, UINT8 comb)
{
	UINT16 i, j;
	struct ListX *aPrvDrawn1 = NULL;
	struct ListX *aPrvDrawn2 = NULL;
	struct ListX *list = NULL;

	aPrvDrawn1 = winningDrawnBallsList->list;

	for (i=0; aPrvDrawn1 && i<winningBallRows; i++) 
	{
		aPrvDrawn2 = aPrvDrawn1->next;

		for (j=i+1; aPrvDrawn2 && j<winningBallRows; j++) 
		{
			switch (comb)
			{
			case 2 : search2CombX(aPrvDrawn2, aPrvDrawn1, luckyBalls, NULL); break;
			case 3 : search3CombX(aPrvDrawn2, aPrvDrawn1, luckyBalls, NULL); break;
			case 4 : search4CombX(aPrvDrawn2, aPrvDrawn1, luckyBalls, NULL); break;
			default: break;
			}
	
			aPrvDrawn2 = aPrvDrawn2->next;
		}

		aPrvDrawn1 = aPrvDrawn1->next;
	}

	list = luckyBalls->list;

	while (list) {
		list->val = numberOfTerm(list->val) +1;
		list = list->next;
	}

	return luckyBalls;
}



UINT8 saveLuckyBallsToFile(struct ListXY *luckyBalls, UINT8 comb)
{
	struct ListX *nl = NULL;
	char lbFile[14];
	char ioBuf[20];
	char buf[5];
	UINT8 i;
	FILE *fp;

	switch (comb)
	{
	case 2 : strcpy(lbFile, LBL2SWAPFILE); break;
	case 3 : strcpy(lbFile, LBL3SWAPFILE); break;
	case 4 : strcpy(lbFile, LBL4SWAPFILE); break;
	default: break;
	}

	if ((fp = fopen(lbFile, "w")) == NULL) {
		printf("Can't open file %s\n", lbFile);
		pressAnyKeyToExit();
		return 0;
	}

	if (luckyBalls == NULL || luckyBalls->list == NULL) {
		return 0;
	}

	nl = luckyBalls->list;

	while (nl)
	{
		ioBuf[0] = '\0';

		for (i=0; i < nl->index; i++) {
			sprintf(buf, "%02d", nl->balls[i]);
			strcat(ioBuf, buf);
		}

		sprintf(buf, "%d\n", nl->val);
		strcat(ioBuf, buf);

		nl = nl->next;

		fputs(ioBuf, fp);
	}

	fclose(fp);

	return 1;
}



struct ListXY * getLuckyBallsFromFile(struct ListXY *luckyBalls, UINT8 comb)
{
	UINT16 i;
	UINT8 j, k;
	int n1, n2, n3, n4, val;
	//UINT8 keys[DRAW_BALL+1];
	char lbFile[14];
	char ioBuf[20];
	char s[6];

	FILE *fp;

	struct ListX *drawList = NULL;

	switch (comb)
	{
	case 2 : strcpy(lbFile, LBL2SWAPFILE); break;
	case 3 : strcpy(lbFile, LBL3SWAPFILE); break;
	case 4 : strcpy(lbFile, LBL4SWAPFILE); break;
	default: break;
	}

	if ((fp = fopen(lbFile, "r")) == NULL) {
		printf("Can't open file %s\n", lbFile);
		pressAnyKeyToExit();
		return NULL;
	}

	for (i=0; fgets(ioBuf, 20, fp) && i<UINT16MAX; i++)
	{
		drawList = createListX(drawList, comb, NULL, 0, 0, 0, 0, 0);

		if (comb == 2) 
		{
			s[0] = ioBuf[0];
			s[1] = ioBuf[1];
			s[2] = '\0';
			n1 = atoi(s);

			s[0] = ioBuf[2];
			s[1] = ioBuf[3];
			s[2] = '\0';
			n2 = atoi(s);

			for (j=0, k=4; !(ioBuf[k]=='\n'||ioBuf[k]=='\0'); j++, k++){
				s[j] = ioBuf[k];
			}
			s[j] = '\0';
			val = atoi(s);
/*			
			keys[0] = (UINT8) n1;
			keys[1] = (UINT8) n2;
			keys[2] = '\0';
*/
			drawList->balls[0] = (UINT8) n1;
			drawList->balls[1] = (UINT8) n2;
			drawList->index += 2;
		}
		else if (comb == 3) 
		{
			s[0] = ioBuf[0];
			s[1] = ioBuf[1];
			s[2] = '\0';
			n1 = atoi(s);

			s[0] = ioBuf[2];
			s[1] = ioBuf[3];
			s[2] = '\0';
			n2 = atoi(s);

			s[0] = ioBuf[4];
			s[1] = ioBuf[5];
			s[2] = '\0';
			n3 = atoi(s);

			for (j=0, k=6; !(ioBuf[k]=='\n'||ioBuf[k]=='\0'); j++, k++){
				s[j] = ioBuf[k];
			}
			s[j] = '\0';
			val = atoi(s);
/*
			keys[0] = (UINT8) n1;
			keys[1] = (UINT8) n2;
			keys[2] = (UINT8) n3;
			keys[3] = '\0';
*/
			drawList->balls[0] = (UINT8) n1;
			drawList->balls[1] = (UINT8) n2;
			drawList->balls[2] = (UINT8) n3;
			drawList->index += 3;
		}
		else if (comb == 4)
		{
			s[0] = ioBuf[0];
			s[1] = ioBuf[1];
			s[2] = '\0';
			n1 = atoi(s);

			s[0] = ioBuf[2];
			s[1] = ioBuf[3];
			s[2] = '\0';
			n2 = atoi(s);

			s[0] = ioBuf[4];
			s[1] = ioBuf[5];
			s[2] = '\0';
			n3 = atoi(s);

			s[0] = ioBuf[6];
			s[1] = ioBuf[7];
			s[2] = '\0';
			n4 = atoi(s);

			for (j=0, k=8; !(ioBuf[k]=='\n'||ioBuf[k]=='\0'); j++, k++){
				s[j] = ioBuf[k];
			}
			s[j] = '\0';
			val = atoi(s);
/*
			keys[0] = (UINT8) n1;
			keys[1] = (UINT8) n2;
			keys[2] = (UINT8) n3;
			keys[3] = (UINT8) n4;
			keys[4] = '\0';
*/
			drawList->balls[0] = (UINT8) n1;
			drawList->balls[1] = (UINT8) n2;
			drawList->balls[2] = (UINT8) n3;
			drawList->balls[3] = (UINT8) n4;
			drawList->index += 4;
		}

		drawList->val = (UINT16) val;
		//appendItems(drawList, keys);
		appendList(luckyBalls, drawList);
	}

	fclose(fp);

	return luckyBalls;
}



void calcMatchCombCount()
{
	UINT16 i, j;
	struct ListX *aPrvDrawn1 = NULL;
	struct ListX *aPrvDrawn2 = NULL;

	aPrvDrawn1 = winningDrawnBallsList->list;

	for (i=0; aPrvDrawn1 && i<winningBallRows; i++)
	{
		aPrvDrawn2 = aPrvDrawn1->next;

		for (j=i+1; aPrvDrawn2 && j<winningBallRows; j++) 
		{
			match2comb += search2CombX(aPrvDrawn2, aPrvDrawn1, NULL, NULL);
			match3comb += search3CombX(aPrvDrawn2, aPrvDrawn1, NULL, NULL);
			match4comb += search4CombX(aPrvDrawn2, aPrvDrawn1, NULL, NULL);
			match5comb += search5CombX(aPrvDrawn2, aPrvDrawn1);

			aPrvDrawn2 = aPrvDrawn2->next;
		}

		aPrvDrawn1 = aPrvDrawn1->next;
	}
}



void calcMatchComb(UINT8 comb, FILE *fp)
{
	UINT16 i, j;
	UINT32 k, x;
	UINT8 d1, m1, d2, m2;
	UINT16 y1, y2;
	UINT16 dDif;
	UINT32 matchComb;
	UINT8 found;
	struct ListX *aPrvDrawn1 = NULL;
	struct ListX *aPrvDrawn2 = NULL;
	char lbBuf[119];
	char pLabel[10];
	char date1[11], date2[11];
	char lastDate[11];

	strcpy(pLabel, "Progress");

	if (comb >= 2 && comb <= 5) 
	{
		puts("Matched combinations of numbers from previous draws:\n");
		if (fp != NULL) fprintf(fp, "Matched combinations of numbers from previous draws:\n\n");

		switch (comb)
		{
		case 2 : matchComb = match2comb; break;
		case 3 : matchComb = match3comb; break;
		case 4 : matchComb = match4comb; break;
		case 5 : matchComb = match5comb; break;
		default: break;
		}

        printf("Matched %d combinations: %lu\n\n", comb, (unsigned long) matchComb);
        if (fp != NULL) fprintf(fp, "Matched %d combinations: %lu\n\n", comb, (unsigned long) matchComb);

		aPrvDrawn1 = winningDrawnBallsList->list;

		for (i=0, k=0, x=0; x<matchComb && aPrvDrawn1 && i<winningBallRows; i++) 
		{
			if (comb >= 2 && comb <= 4) {
					 printPercentOfProgress(pLabel, k, (UINT32) ceil((UINT32) winningBallRows*((UINT32) winningBallRows-1)/2));
			}

			aPrvDrawn2 = aPrvDrawn1->next;

			for (j=i+1; aPrvDrawn2 && j<winningBallRows; j++, k++) 
			{
				switch (comb)
				{
				case 2 : found = search2CombX(aPrvDrawn2, aPrvDrawn1, NULL, lbBuf); break;
				case 3 : found = search3CombX(aPrvDrawn2, aPrvDrawn1, NULL, lbBuf); break;
				case 4 : found = search4CombX(aPrvDrawn2, aPrvDrawn1, NULL, lbBuf); break;
				case 5 : found = search5CombX(aPrvDrawn2, aPrvDrawn1); break;
				default: break;
				}

				if (found) 
				{
					x++;

					if (x == matchComb && comb < 5) printPercentOfProgress(pLabel, 1, 1);

					formatDate(date1, aPrvDrawn1->day, aPrvDrawn1->mon, aPrvDrawn1->year);

					if (strcmp(lastDate, date1) != 0) 
					{
						if (comb >= 5) {
							puts("\n--------------------------------------------------------------------------------");
							printf("\n%s : ", date1);
						}

						if (fp != NULL) {
							fputs("\n--------------------------------------------------------------------------------", fp);
							fprintf(fp, "\n%s : ", date1);
						}

						if (comb >= 5) printListXByKey(aPrvDrawn1, 0, fp);
						else  printListXByKey(aPrvDrawn1, 2, fp);
						
						if (comb >= 5) printf("   ");
						if (fp != NULL) fprintf(fp, "   ");
						
						y1 = aPrvDrawn1->year;
						m1 = aPrvDrawn1->mon;
						d1 = aPrvDrawn1->day;
						formatDate(date1, d1, m1, y1);
					}
					else {
						if (comb >= 5) printf("                               ");
						if (fp != NULL) fprintf(fp, "                               ");
					}
					
					formatDate(date2, aPrvDrawn2->day, aPrvDrawn2->mon, aPrvDrawn2->year);

					if (comb >= 5) printf("    %s : ", date2);
					if (fp != NULL) fprintf(fp, "    %s : ", date2);
			
					if (comb >= 5) printListXByKey(aPrvDrawn2, 0, fp);
					else  printListXByKey(aPrvDrawn2, 2, fp);
		
					y2 = aPrvDrawn2->year;
					m2 = aPrvDrawn2->mon;
					d2 = aPrvDrawn2->day;
		
					dDif = dateDiff(d2, m2, y2, d1, m1, y1);

					if (comb >= 5) printf("   %4d days", dDif);
					if (fp != NULL) fprintf(fp, "   %4d days", dDif);
		
					if (comb != 5) {
						if (comb >= 5) printf("   %s", lbBuf);
						if (fp != NULL) fprintf(fp, "   %s", lbBuf);
					}

					if (comb >= 5) puts("");
					if (fp != NULL) fprintf(fp, "\n");
					strcpy(lastDate, date1);
				}

				aPrvDrawn2 = aPrvDrawn2->next;
			}

			aPrvDrawn1 = aPrvDrawn1->next;
		}

		if (matchComb) {
			if (comb >= 5) puts("\n--------------------------------------------------------------------------------\n");
			if (fp != NULL) fputs("\n--------------------------------------------------------------------------------\n", fp);
		}
	}
}



struct ListX * drawBallsByLucky(struct ListX *drawnBallsLucky, UINT8 drawNum, UINT8 totalBall, UINT8 drawBallCount)
{
	UINT8 i, j, k, x;
	UINT16 lbsLen, index = 0;
	UINT16 lucky2MinVal = 0;
	UINT16 lucky2MaxVal = 0;
	int ind1, ind2, ind3;
	UINT8 ball1, ball2, ball3;
	struct ListXY *luckyBalls = NULL;
	struct ListX *luckyRow2 = NULL;
	struct ListX *luckyRow3 = NULL;
	UINT8 numOfAttempts = (UINT8) ceil(5*totalBall/drawBallCount);

	#if defined(__MSDOS__)
	if (drawBallCount == DRAW_BALL_EN) {
		euroNumbers = getEuroNumbersFromFile(euroNumbers);
	}
	else {
		switch (drawNum)
		{
		case 1 :
		case 2 : luckyBalls3 = getLuckyBallsFromFile(luckyBalls3, 3); break;
		default: luckyBalls2 = getLuckyBallsFromFile(luckyBalls2, 2); break;
		}
	}
	#endif

	if (drawBallCount == DRAW_BALL_EN) {
        luckyBalls = euroNumbers;
	}
	else {
		switch (drawNum)
		{
		case 1 :
		case 2 : luckyBalls = luckyBalls3; break;
		default: luckyBalls = luckyBalls2; break;
		}
	}

	lbsLen = lengthY(luckyBalls);

	bubbleSortYByVal(luckyBalls, -1);
	bubbleSortYByVal2(luckyBalls, -1);

	luckyRow2 = getListXByIndex(luckyBalls, lbsLen-1);
	lucky2MinVal = luckyRow2->val;
	luckyRow2 = getListXByIndex(luckyBalls, 0);
	lucky2MaxVal = luckyRow2->val;

	for (index=0; index < (UINT16) ceil(2*lbsLen/3) && getValXY(luckyBalls, index) > (UINT16) ceil((lucky2MinVal+lucky2MaxVal)/2); index++);

	if (index < 10) index = lbsLen;

	if (drawBallCount == DRAW_BALL_EN) {
		x = 1;
	} else {
		if (drawNum == 1) x = 3;
		else x = 2;
	}

	if (drawNum == 1 && drawBallCount > DRAW_BALL_EN) 
	{
		luckyBalls = shuffleListXY(luckyBalls);
		luckyRow3 = getListXByIndex(luckyBalls, (UINT16) rand() % lbsLen);
		ind1 = rand()%3;

		if (ind1 == 0) 
		{
			ind2 = rand()%2 +1;
			if (ind2 == 1) ind3 = 2;
			else ind3 = 1;
		}
		else if (ind1 == 1) 
		{
			ind2 = 0;
			ind3 = 2;
		}
		else 
		{
			ind2 = rand()%2;
			if (ind2 == 0) ind3 = 1;
			else ind3 = 0;
		}

		ball1 = getKey(luckyRow3, ind1);
		ball2 = getKey(luckyRow3, ind2);
		ball3 = getKey(luckyRow3, ind3);

		appendItem(drawnBallsLucky, ball1);
		appendItem(drawnBallsLucky, ball2);
		appendItem(drawnBallsLucky, ball3);
	}
	else
	{
		if (drawNum == 2) {
			luckyBalls = shuffleListXY(luckyBalls);
			luckyRow2 = getListXByIndex(luckyBalls, (UINT16) rand() % lbsLen);
		} else {
			luckyRow2 = getListXByIndex(luckyBalls, (UINT16) rand() % index);
		}

		ind1 = rand()%2;
		if (ind1 == 0) ind2 = 1;
		else ind2 = 0;

		ball1 = getKey(luckyRow2, ind1);
		appendItem(drawnBallsLucky, ball1);

		if (drawBallCount > DRAW_BALL_EN) {
			ball2 = getKey(luckyRow2, ind2);
			appendItem(drawnBallsLucky, ball2);
		}
	}

	bubbleSortYByVal(luckyBalls, -1);
	bubbleSortYByVal2(luckyBalls, -1);

	for (i=0; i<drawBallCount-x; i++)
	{
		j = 0;
		do {
			k = 0;
			do {
				luckyRow2 = getListXByIndex(luckyBalls, rand() % index);
				k++;
			} while ((ind1 = seqSearchX1(luckyRow2, ball2)) < 0 && k <= numOfAttempts);

			if (ind1 < 0) ind1 = 0;

			if (ind1 == 0) ind2 = 1;
			else ind2 = 0;

			ball2 = getKey(luckyRow2, ind2);
			j++;
		} while ((ind1 = seqSearchX1(drawnBallsLucky, ball2)) >= 0 && j <= numOfAttempts);

		if (ind1 >= 0) {
			do {
				ball2 = (rand() % drawBallCount) +1;
			} while(seqSearchX1(drawnBallsLucky, ball2) >= 0);
		}

		appendItem(drawnBallsLucky, ball2);
	}

	#if defined(__MSDOS__)
	removeAllXY(luckyBalls);
	#endif

	bubbleSortXByKey(drawnBallsLucky);

	return drawnBallsLucky;
}



void drawBalls(struct ListXY *coupon, UINT8 totalBall, UINT8 drawBallCount, UINT8 drawRowCount, UINT8 drawByNorm, UINT8 drawByLeft, UINT8 drawByBlend1, UINT8 drawByBlend2, UINT8 drawBySide, UINT8 drawByRand, UINT8 drawByLucky)
{
	UINT8 i, j, k;
	UINT8 found = 0;
	UINT8 matchComb = 0;
	UINT8 elimComb = 0;
	UINT8 autoCalc = 1;
	UINT8 noMatch, elim;
	char label[22];
	char pLabel[12];
	UINT8 drawCountDown = drawRowCount;
	UINT8 luckyNum = 0;
	UINT16 dDiff;
	UINT8 numOfAttempts;

	struct ListX *drawnBalls = NULL;
	struct ListX *fc = NULL;
	struct ListX2 *ballSortOrder = NULL;
	struct ListXY *foundComb = NULL;

	foundComb = createListXY(foundComb);

	numOfAttempts = (UINT8) ceil(5*totalBall/drawBallCount);

	if (drawBallCount == DRAW_BALL_EN) {
		ballSortOrder = euNumberBallsDrawCount;
		strcpy(pLabel, "EuroNumbers");
	}
	else {
		ballSortOrder = winningBallsDrawCount;
		strcpy(pLabel, "Numbers");
	}

	for (j=0; drawCountDown; j++)
	{
		if (j%2 == 0)
		{
			/* normal distribution */
			if (drawByNorm && drawCountDown)
			{
				strcpy(label, "(normal distribution)");
				drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0, 0);

				if (autoCalc && drawBallCount > DRAW_BALL_EN) {
					matchComb = 0;
					elimComb = 2;
				}

				for (i=0; i < numOfAttempts; i++)
				{
					drawnBalls = drawBallByNorm(drawnBalls, ballSortOrder, totalBall, drawBallCount, matchComb, elimComb);

					if (drawBallCount == DRAW_BALL_EN) break;

					noMatch = 0;

					if (matchComb == 3) {
						noMatch = !search3CombXY(luckyBalls3, drawnBalls, NULL);
					}

					elim = 0;

					switch (elimComb)
					{
					case 4 : elim = search4CombXY(luckyBalls4, drawnBalls, foundComb);
					case 3 : if (!elim) elim = search3CombXY(luckyBalls3, drawnBalls, foundComb);
					case 2 : if (!elim) elim = search2CombXY(luckyBalls2, drawnBalls, foundComb);
					default: break;
					}

					if (elim)
					{
						fc = foundComb->list;

						for (k=0; fc && k<lengthY(foundComb); k++) 
						{
							dDiff = dateDiff(fc->day, fc->mon, fc->year, currDay, currMon, currYear);

							if (dDiff >= fc->val2 && ((double) fc->val * (double) fc->val2 / (double) drawnDays) >= 0.49) {
								elim = 0;
								break;
							}
							
							fc = fc->next;
						}
					}

					if (!(noMatch || elim)) {
						if (i < ceil((double) numOfAttempts/4)) {
							found = search1BallXY(coupon, drawnBalls, drawBallCount);
						} else if (i < ceil((double) numOfAttempts/2)) {
							found = search2CombXY(coupon, drawnBalls, NULL);
						} else if (i < ceil(3* (double) numOfAttempts/4)) {
							if (drawBallCount == DRAW_BALL_EN) break;
							found = search3CombXY(coupon, drawnBalls, NULL);
						} else {
							found = search4CombXY(coupon, drawnBalls, NULL);
						}

						if(!found) break;
					}

					if (autoCalc && drawBallCount > DRAW_BALL_EN) {
						if (i < ceil((double) numOfAttempts/4)) {matchComb = 0; elimComb = 2;}
						else if (i < ceil((double) numOfAttempts/2)) {matchComb = 0; elimComb = 3;}
						else if (i < ceil(3* (double) numOfAttempts/4)) {matchComb = 3; elimComb = 4;}
						else {matchComb = 0; elimComb = 0;}
					}
				}

				appendList(coupon, drawnBalls);
				drawCountDown--;

				printPercentOfProgress(pLabel, (UINT32) (drawRowCount-drawCountDown), (UINT32) drawRowCount);
			}
		}
		else
		{
			/* Left Stacked */
			if (drawByLeft && drawCountDown)
			{
				strcpy(label, "(left stacked)");
				drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0, 0);

				if (autoCalc && drawBallCount > DRAW_BALL_EN) {
					matchComb = 0;
					elimComb = 2;
				}

				for (i=0; i < numOfAttempts; i++)
				{
					drawnBalls = drawBallByLeft(drawnBalls, ballSortOrder, totalBall, drawBallCount, matchComb, elimComb);

					if (drawBallCount == DRAW_BALL_EN) break;

					noMatch = 0;

					if (matchComb == 3) {
						noMatch = !search3CombXY(luckyBalls3, drawnBalls, NULL);
					}

					elim = 0;

					switch (elimComb)
					{
					case 4 : elim = search4CombXY(luckyBalls4, drawnBalls, foundComb);
					case 3 : if (!elim) elim = search3CombXY(luckyBalls3, drawnBalls, foundComb);
					case 2 : if (!elim) elim = search2CombXY(luckyBalls2, drawnBalls, foundComb);
					default: break;
					}

					if (elim)
					{
						fc = foundComb->list;

						for (k=0; fc && k<lengthY(foundComb); k++) 
						{
							dDiff = dateDiff(fc->day, fc->mon, fc->year, currDay, currMon, currYear);

							if (dDiff >= fc->val2 && ((double) fc->val * (double) fc->val2 / (double) drawnDays) >= 0.49) {
								elim = 0;
								break;
							}
							
							fc = fc->next;
						}
					}

					if (!(noMatch || elim)) {
						if (i < ceil((double) numOfAttempts/4)) {
							found = search1BallXY(coupon, drawnBalls, drawBallCount);
						} else if (i < ceil((double) numOfAttempts/2)) {
							found = search2CombXY(coupon, drawnBalls, NULL);
						} else if (i < ceil(3* (double) numOfAttempts/4)) {
							if (drawBallCount == DRAW_BALL_EN) break;
							found = search3CombXY(coupon, drawnBalls, NULL);
						} else {
							found = search4CombXY(coupon, drawnBalls, NULL);
						}

						if(!found) break;
					}

					if (autoCalc && drawBallCount > DRAW_BALL_EN) {
						if (i < ceil((double) numOfAttempts/4)) {matchComb = 0; elimComb = 2;}
						else if (i < ceil((double) numOfAttempts/2)) {matchComb = 0; elimComb = 3;}
						else if (i < ceil(3* (double) numOfAttempts/4)) {matchComb = 3; elimComb = 4;}
						else {matchComb = 0; elimComb = 0;}
					}
				}

				appendList(coupon, drawnBalls);
				drawCountDown--;

				printPercentOfProgress(pLabel, (UINT32) (drawRowCount-drawCountDown), (UINT32) drawRowCount);
			}
		}

		if (j%2 == 0)
		{
			/* Blend 1 */
			if (drawByBlend1 && drawCountDown)
			{
				strcpy(label, "(blend 1)");
				drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0, 0);

				if (autoCalc && drawBallCount > DRAW_BALL_EN) {
					matchComb = 0;
					elimComb = 2;
				}

				for (i=0; i < numOfAttempts; i++)
				{
					drawnBalls = drawBallByBlend1(drawnBalls, ballSortOrder, totalBall, drawBallCount, matchComb, elimComb);

					if (drawBallCount == DRAW_BALL_EN) break;

					noMatch = 0;

					if (matchComb == 3) {
						noMatch = !search3CombXY(luckyBalls3, drawnBalls, NULL);
					}

					elim = 0;

					switch (elimComb)
					{
					case 4 : elim = search4CombXY(luckyBalls4, drawnBalls, foundComb);
					case 3 : if (!elim) elim = search3CombXY(luckyBalls3, drawnBalls, foundComb);
					case 2 : if (!elim) elim = search2CombXY(luckyBalls2, drawnBalls, foundComb);
					default: break;
					}

					if (elim)
					{
						fc = foundComb->list;

						for (k=0; fc && k<lengthY(foundComb); k++) 
						{
							dDiff = dateDiff(fc->day, fc->mon, fc->year, currDay, currMon, currYear);

							if (dDiff >= fc->val2 && ((double) fc->val * (double) fc->val2 / (double) drawnDays) >= 0.49) {
								elim = 0;
								break;
							}
							
							fc = fc->next;
						}
					}

					if (!(noMatch || elim)) {
						if (i < ceil((double) numOfAttempts/4)) {
							found = search1BallXY(coupon, drawnBalls, drawBallCount);
						} else if (i < ceil((double) numOfAttempts/2)) {
							found = search2CombXY(coupon, drawnBalls, NULL);
						} else if (i < ceil(3* (double) numOfAttempts/4)) {
							if (drawBallCount == DRAW_BALL_EN) break;
							found = search3CombXY(coupon, drawnBalls, NULL);
						} else {
							found = search4CombXY(coupon, drawnBalls, NULL);
						}

						if(!found) break;
					}

					if (autoCalc && drawBallCount > DRAW_BALL_EN) {
						if (i < ceil((double) numOfAttempts/4)) {matchComb = 0; elimComb = 2;}
						else if (i < ceil((double) numOfAttempts/2)) {matchComb = 0; elimComb = 3;}
						else if (i < ceil(3* (double) numOfAttempts/4)) {matchComb = 3; elimComb = 4;}
						else {matchComb = 0; elimComb = 0;}
					}
				}

				appendList(coupon, drawnBalls);
				drawCountDown--;

				printPercentOfProgress(pLabel, (UINT32) (drawRowCount-drawCountDown), (UINT32) drawRowCount);
			}
		}
		else
		{
			/* Blend 2 */
			if (drawByBlend2 && drawCountDown)
			{
				strcpy(label, "(blend 2)");
				drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0, 0);

				if (autoCalc && drawBallCount > DRAW_BALL_EN) {
					matchComb = 0;
					elimComb = 2;
				}

				for (i=0; i < numOfAttempts; i++)
				{
					drawnBalls = drawBallByBlend2(drawnBalls, ballSortOrder, totalBall, drawBallCount, matchComb, elimComb);

					if (drawBallCount == DRAW_BALL_EN) break;

					noMatch = 0;

					if (matchComb == 3) {
						noMatch = !search3CombXY(luckyBalls3, drawnBalls, NULL);
					}

					elim = 0;

					switch (elimComb)
					{
					case 4 : elim = search4CombXY(luckyBalls4, drawnBalls, foundComb);
					case 3 : if (!elim) elim = search3CombXY(luckyBalls3, drawnBalls, foundComb);
					case 2 : if (!elim) elim = search2CombXY(luckyBalls2, drawnBalls, foundComb);
					default: break;
					}

					if (elim)
					{
						fc = foundComb->list;

						for (k=0; fc && k<lengthY(foundComb); k++) 
						{
							dDiff = dateDiff(fc->day, fc->mon, fc->year, currDay, currMon, currYear);

							if (dDiff >= fc->val2 && ((double) fc->val * (double) fc->val2 / (double) drawnDays) >= 0.49) {
								elim = 0;
								break;
							}
							
							fc = fc->next;
						}
					}

					if (!(noMatch || elim)) {
						if (i < ceil((double) numOfAttempts/4)) {
							found = search1BallXY(coupon, drawnBalls, drawBallCount);
						} else if (i < ceil((double) numOfAttempts/2)) {
							found = search2CombXY(coupon, drawnBalls, NULL);
						} else if (i < ceil(3* (double) numOfAttempts/4)) {
							if (drawBallCount == DRAW_BALL_EN) break;
							found = search3CombXY(coupon, drawnBalls, NULL);
						} else {
							found = search4CombXY(coupon, drawnBalls, NULL);
						}

						if(!found) break;
					}

					if (autoCalc && drawBallCount > DRAW_BALL_EN) {
						if (i < ceil((double) numOfAttempts/4)) {matchComb = 0; elimComb = 2;}
						else if (i < ceil((double) numOfAttempts/2)) {matchComb = 0; elimComb = 3;}
						else if (i < ceil(3* (double) numOfAttempts/4)) {matchComb = 3; elimComb = 4;}
						else {matchComb = 0; elimComb = 0;}
					}
				}

				appendList(coupon, drawnBalls);
				drawCountDown--;

				printPercentOfProgress(pLabel, (UINT32) (drawRowCount-drawCountDown), (UINT32) drawRowCount);
			}
		}

		/* Side Stacked */
		if (drawBySide && drawCountDown)
		{
			strcpy(label, "(side stacked)");
			drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0, 0);

			if (autoCalc && drawBallCount > DRAW_BALL_EN) {
				matchComb = 0;
				elimComb = 2;
			}

			for (i=0; i < numOfAttempts; i++)
			{
				drawnBalls = drawBallBySide(drawnBalls, ballSortOrder, totalBall, drawBallCount, matchComb, elimComb);

				if (drawBallCount == DRAW_BALL_EN) break;

				noMatch = 0;

				if (matchComb == 3) {
					noMatch = !search3CombXY(luckyBalls3, drawnBalls, NULL);
				}

				elim = 0;

				switch (elimComb)
				{
				case 4 : elim = search4CombXY(luckyBalls4, drawnBalls, foundComb);
				case 3 : if (!elim) elim = search3CombXY(luckyBalls3, drawnBalls, foundComb);
				case 2 : if (!elim) elim = search2CombXY(luckyBalls2, drawnBalls, foundComb);
				default: break;
				}

				if (elim)
				{
					fc = foundComb->list;

					for (k=0; fc && k<lengthY(foundComb); k++) 
					{
						dDiff = dateDiff(fc->day, fc->mon, fc->year, currDay, currMon, currYear);

						if (dDiff >= fc->val2 && ((double) fc->val * (double) fc->val2 / (double) drawnDays) >= 0.49) {
							elim = 0;
							break;
						}
							
						fc = fc->next;
					}
				}

				if (!(noMatch || elim)) {
					if (i < ceil((double) numOfAttempts/4)) {
						found = search1BallXY(coupon, drawnBalls, drawBallCount);
					} else if (i < ceil((double) numOfAttempts/2)) {
						found = search2CombXY(coupon, drawnBalls, NULL);
					} else if (i < ceil(3* (double) numOfAttempts/4)) {
						if (drawBallCount == DRAW_BALL_EN) break;
						found = search3CombXY(coupon, drawnBalls, NULL);
					} else {
						found = search4CombXY(coupon, drawnBalls, NULL);
					}

					if(!found) break;
				}

				if (autoCalc && drawBallCount > DRAW_BALL_EN) {
					if (i < ceil((double) numOfAttempts/4)) {matchComb = 0; elimComb = 2;}
					else if (i < ceil((double) numOfAttempts/2)) {matchComb = 0; elimComb = 3;}
					else if (i < ceil(3* (double) numOfAttempts/4)) {matchComb = 3; elimComb = 4;}
					else {matchComb = 0; elimComb = 0;}
				}
			}

			appendList(coupon, drawnBalls);
			drawCountDown--;

			printPercentOfProgress(pLabel, (UINT32) (drawRowCount-drawCountDown), (UINT32) drawRowCount);
		}

		/* Random */
		if (drawByRand && drawCountDown)
		{
			strcpy(label, "(random)");
			drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0, 0);

			if (autoCalc && drawBallCount > DRAW_BALL_EN) {
				matchComb = 0;
				elimComb = 2;
			}

			for (i=0; i < numOfAttempts; i++)
			{
				drawnBalls = drawBallByRand(drawnBalls, ballSortOrder, totalBall, drawBallCount, matchComb, elimComb);

				if (drawBallCount == DRAW_BALL_EN) break;

				noMatch = 0;

				if (matchComb == 3) {
					noMatch = !search3CombXY(luckyBalls3, drawnBalls, NULL);
				}

				elim = 0;

				switch (elimComb)
				{
				case 4 : elim = search4CombXY(luckyBalls4, drawnBalls, foundComb);
				case 3 : if (!elim) elim = search3CombXY(luckyBalls3, drawnBalls, foundComb);
				case 2 : if (!elim) elim = search2CombXY(luckyBalls2, drawnBalls, foundComb);
				default: break;
				}

				if (elim)
				{
					fc = foundComb->list;

					for (k=0; fc && k<lengthY(foundComb); k++) 
					{
						dDiff = dateDiff(fc->day, fc->mon, fc->year, currDay, currMon, currYear);

						if (dDiff >= fc->val2 && ((double) fc->val * (double) fc->val2 / (double) drawnDays) >= 0.49) {
							elim = 0;
							break;
						}
							
						fc = fc->next;
					}
				}

				if (!(noMatch || elim)) {
					if (i < ceil((double) numOfAttempts/4)) {
						found = search1BallXY(coupon, drawnBalls, drawBallCount);
					} else if (i < ceil((double) numOfAttempts/2)) {
						found = search2CombXY(coupon, drawnBalls, NULL);
					} else if (i < ceil(3* (double) numOfAttempts/4)) {
						if (drawBallCount == DRAW_BALL_EN) break;
						found = search3CombXY(coupon, drawnBalls, NULL);
					} else {
						found = search4CombXY(coupon, drawnBalls, NULL);
					}

					if(!found) break;
				}

				if (autoCalc && drawBallCount > DRAW_BALL_EN) {
					if (i < ceil((double) numOfAttempts/4)) {matchComb = 0; elimComb = 2;}
					else if (i < ceil((double) numOfAttempts/2)) {matchComb = 0; elimComb = 3;}
					else if (i < ceil(3* (double) numOfAttempts/4)) {matchComb = 3; elimComb = 4;}
					else {matchComb = 0; elimComb = 0;}
				}
			}

			appendList(coupon, drawnBalls);
			drawCountDown--;

			printPercentOfProgress(pLabel, (UINT32) (drawRowCount-drawCountDown), (UINT32) drawRowCount);
		}

		/* Lucky */
		if (drawByLucky && drawCountDown)
		{
			if (drawBallCount > DRAW_BALL_EN) // winning numbers
			{
				luckyNum = (j%3)+1;

				if (luckyNum == 1) strcpy(label, "(lucky 3)");
				else if (luckyNum == 2) strcpy(label, "(2 of lucky 3)");
				else strcpy(label, "(lucky 2)");
			}
			else // euroNumber
			{
					luckyNum = 3;
					strcpy(label, "euro numbers");
			}

			drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0, 0);
			drawnBalls = drawBallsByLucky(drawnBalls, luckyNum, totalBall, drawBallCount);

			appendList(coupon, drawnBalls);
			drawCountDown--;

			printPercentOfProgress(pLabel, (UINT32) (drawRowCount-drawCountDown), (UINT32) drawRowCount);
		}
	}
}



UINT16 getDrawnBallsList(struct ListXY *ballList, char *fileName)
{
	UINT16 i=0;
	int d1, m1, y1, eu1, eu2;
	int n1, n2, n3, n4, n5;
	UINT8 keys[DRAW_BALL+1];
	UINT8 size;
	char ioBuf[50];

	FILE *fp;

	struct ListX *drawList = NULL;

	if ((fp = fopen(fileName, "r")) == NULL) {
		return 0;
	}

	for (i=0; fgets(ioBuf, 50, fp) && i<UINT16MAX; i++)
	{
		sscanf(ioBuf, "%d-%d-%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", &y1, &m1, &d1, &n1, &n2, &n3, &n4, &n5, &eu1, &eu2);

		if (ballList == winningDrawnBallsList) {
			keys[0] = (UINT8) n1;
			keys[1] = (UINT8) n2;
			keys[2] = (UINT8) n3;
			keys[3] = (UINT8) n4;
			keys[4] = (UINT8) n5;
			keys[5] = '\0';
			size = DRAW_BALL;
		} else if (ballList == euNumberDrawnBallsList) {
			keys[0] = (UINT8) eu1;
			keys[1] = (UINT8) eu2;
			keys[2] = '\0';
			size = DRAW_BALL_EN;
		}
		drawList = createListX(drawList, size, NULL, 0, 0, (UINT16) y1, (UINT8) m1, (UINT8) d1);

		appendItems(drawList, keys);
		appendList(ballList, drawList);
	}

	fclose(fp);

	return i;
}



UINT16 dateDiff(UINT8 d1, UINT8 m1, UINT16 y1, UINT8 d2, UINT8 m2, UINT16 y2)
{
	UINT16 x1, x2;

	m1 = (m1 + 9) % 12;
	y1 = y1 - m1 / 10;
	x1 = 365*y1 + y1/4 - y1/100 + y1/400 + (m1*306 + 5)/10 + (d1-1);

	m2 = (m2 + 9) % 12;
	y2 = y2 - m2 / 10;
	x2 = 365*y2 + y2/4 - y2/100 + y2/400 + (m2*306 + 5)/10 + (d2-1);

	return (x2-x1);
}



void formatDate(char* date, UINT8 day, UINT8 mon, UINT16 year)
{
	char s1[5], s2[3], s3[3];
	char tmp[2];

	sprintf(s1, "%d", (int) year);
	sprintf(s2, "%d", (int) mon);
	sprintf(s3, "%d", (int) day);

	if (strlen(s2)==1) {
		tmp[0] = s2[0];
		s2[0] = '0';
		s2[1] = tmp[0];
		s2[2] = '\0';
	}
	if (strlen(s3)==1) {
		tmp[0] = s3[0];
		s3[0] = '0';
		s3[1] = tmp[0];
		s3[2] = '\0';
	}

	sprintf(date, "%s-%s-%s", s1, s2, s3);
}



struct ListX * drawBallByRand(struct ListX *drawnBallsRand, struct ListX2 *ballSortOrder, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb)
{
	UINT16 i, j, k;
	UINT8 x, y, z;
	UINT8 index;
	UINT8 flyball, drawball;
	UINT16 shuffleGlobe;
	UINT8 noMatch, elim;
	UINT16 dDiff;

	struct ListX2 *globe = NULL;
	struct ListX *fc = NULL;
	struct ListXY *foundComb = NULL;

	foundComb = createListXY(foundComb);

	globe = createListX2(globe, totalBall);

	bubbleSortX2ByVal(ballSortOrder, 1);

	/* Fill inside the globe with balls */
	if (rand() % 2) // (blend1)
	{
		for (x=0, j=totalBall-1, k=0; k<totalBall; k++)
		{
			if (k%2) {
				appendItem2(globe, getKey2(ballSortOrder, j), 0);
				j--;
			}
			else {
				insertItem2(globe, getKey2(ballSortOrder, x));
				x++;
			}
		}
	}
	else // (blend2)
	{
		for (x=0, j=totalBall-1, k=0; k<totalBall; k++)
		{
			if (k%2) {
				addItemByIndex2(globe, totalBall-j, getKey2(ballSortOrder, j));
				j--;
			}
			else {
				addItemByIndex2(globe, x, getKey2(ballSortOrder, x));
				x++;
			}
		}
	}

	for (i=0; i < totalBall; i++)
	{
		removeAllX(drawnBallsRand);

		for (j=0; j<drawBallCount; j++)
		{
			/* shuffle globe */
			shuffleGlobe = (UINT16) ((ceil(totalBall/drawBallCount) * (rand() % 6 + (UINT16) ceil(totalBall/2) - 5) * (rand() % 6 + (UINT16) ceil(totalBall/3) - 5)));

			for (k=0; k<shuffleGlobe; k++)
			{
				index = rand() % (totalBall-j);

				flyball = removeItemByIndex2(globe, index);

				if (index < (UINT8) ceil(totalBall/3)) {
					insertItem2(globe, flyball);
				}
				else if (index < (UINT8) ceil(totalBall/2)) {
					addItemByIndex2(globe, (UINT8) ceil(2*totalBall/3), flyball);
				}
				else if (index < (UINT8) ceil(2*totalBall/3)) {
					addItemByIndex2(globe, (UINT8) ceil(totalBall/3), flyball);
				}
				else {
					appendItem2(globe, flyball, 0);
				}
			}

			/* draw a ball */
			drawball = removeItemByIndex2(globe, (UINT8) ceil((totalBall-j)/2));
			appendItem(drawnBallsRand, drawball);
		}

		for (x=0, y=drawBallCount-1, z=0; z<drawBallCount; z++)
		{
			if (z%2) {
				appendItem2(globe, getKey(drawnBallsRand, y), 0);
				y--;
			}
			else {
				insertItem2(globe, getKey(drawnBallsRand, x));
				x++;
			}
		}

		if (drawBallCount == DRAW_BALL_EN) break;

		noMatch = 0;

		if (matchComb == 3) {
			noMatch = !search3CombXY(luckyBalls3, drawnBallsRand, NULL);
		}

		elim = 0;

		switch (elimComb)
		{
		case 4 : elim = search4CombXY(luckyBalls4, drawnBallsRand, foundComb);
		case 3 : if (!elim) elim = search3CombXY(luckyBalls3, drawnBallsRand, foundComb);
		case 2 : if (!elim) elim = search2CombXY(luckyBalls2, drawnBallsRand, foundComb);
		default: break;
		}

		if (elim)
		{
			fc = foundComb->list;

			for (x=0; fc && x<lengthY(foundComb); x++) 
			{
				dDiff = dateDiff(fc->day, fc->mon, fc->year, currDay, currMon, currYear);

				if (dDiff >= fc->val2 && ((double) fc->val * (double) fc->val2 / (double) drawnDays) >= 0.49) {
					elim = 0;
					break;
				}
							
				fc = fc->next;
			}
		}

		if (!(noMatch || elim)) break;
	}

	bubbleSortXByKey(drawnBallsRand);
	removeAllX2(globe);
	free(globe);

	return drawnBallsRand;
}



struct ListX * drawBallByLeft(struct ListX *drawnBallsLeft, struct ListX2 *ballSortOrder, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb)
{
	UINT8 i, k;
	UINT8 noMatch, elim;
	UINT8 drawball;
	UINT16 dDiff;

	struct ListX2 *globe = NULL;
	struct ListX *fc = NULL;
	struct ListXY *foundComb = NULL;

	foundComb = createListXY(foundComb);

	globe = createListX2(globe, totalBall);

	bubbleSortX2ByVal(ballSortOrder, 1);

	for (i=0; i<totalBall; i++)	{
		appendItem2(globe, getKey2(ballSortOrder, i), 0);
	}

	for (k=0; k<totalBall; k++)
	{
		removeAllX(drawnBallsLeft);

		for (i=0; i<drawBallCount; i++)
		{
			do {
				drawball = getKey2(globe, gaussIndex(totalBall)-1);
			} while(seqSearchX1(drawnBallsLeft, drawball) >= 0);

			appendItem(drawnBallsLeft, drawball);
		}

		if (drawBallCount == DRAW_BALL_EN) break;

		noMatch = 0;

		if (matchComb == 3) {
			noMatch = !search3CombXY(luckyBalls3, drawnBallsLeft, NULL);
		}

		elim = 0;

		switch (elimComb)
		{
		case 4 : elim = search4CombXY(luckyBalls4, drawnBallsLeft, foundComb);
		case 3 : if (!elim) elim = search3CombXY(luckyBalls3, drawnBallsLeft, foundComb);
		case 2 : if (!elim) elim = search2CombXY(luckyBalls2, drawnBallsLeft, foundComb);
		default: break;
		}

		if (elim)
		{
			fc = foundComb->list;

			for (i=0; fc && i<lengthY(foundComb); i++) 
			{
				dDiff = dateDiff(fc->day, fc->mon, fc->year, currDay, currMon, currYear);

				if (dDiff >= fc->val2 && ((double) fc->val * (double) fc->val2 / (double) drawnDays) >= 0.49) {
					elim = 0;
					break;
				}
							
				fc = fc->next;
			}
		}

		if (!(noMatch || elim)) break;
	}

	bubbleSortXByKey(drawnBallsLeft);
	removeAllX2(globe);
	free(globe);

	return drawnBallsLeft;
}



struct ListX * drawBallByBlend1(struct ListX *drawnBallsBlend1, struct ListX2 *ballSortOrder, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb)
{
	UINT8 i, j, k;
	UINT8 noMatch, elim;
	UINT8 drawball;
	UINT16 dDiff;

	struct ListX2 *globe = NULL;
	struct ListX *fc = NULL;
	struct ListXY *foundComb = NULL;

	foundComb = createListXY(foundComb);

	globe = createListX2(globe, totalBall);

	bubbleSortX2ByVal(ballSortOrder, 1);

	for (i=0, j=totalBall-1, k=0; k<totalBall; k++)
	{
		if (k%2) {
			appendItem2(globe, getKey2(ballSortOrder, j), 0);
			j--;
		}
		else {
			insertItem2(globe, getKey2(ballSortOrder, i));
			i++;
		}
	}

	for (k=0; k<totalBall; k++)
	{
		removeAllX(drawnBallsBlend1);

		for (i=0; i<drawBallCount; i++)
		{
			do {
				drawball = getKey2(globe, gaussIndex(totalBall)-1);
			} while(seqSearchX1(drawnBallsBlend1, drawball) >= 0);

			appendItem(drawnBallsBlend1, drawball);
		}

		if (drawBallCount == DRAW_BALL_EN) break;

		noMatch = 0;

		if (matchComb == 3) {
			noMatch = !search3CombXY(luckyBalls3, drawnBallsBlend1, NULL);
		}

		elim = 0;

		switch (elimComb)
		{
		case 4 : elim = search4CombXY(luckyBalls4, drawnBallsBlend1, foundComb);
		case 3 : if (!elim) elim = search3CombXY(luckyBalls3, drawnBallsBlend1, foundComb);
		case 2 : if (!elim) elim = search2CombXY(luckyBalls2, drawnBallsBlend1, foundComb);
		default: break;
		}

		if (elim)
		{
			fc = foundComb->list;

			for (i=0; fc && i<lengthY(foundComb); i++) 
			{
				dDiff = dateDiff(fc->day, fc->mon, fc->year, currDay, currMon, currYear);

				if (dDiff >= fc->val2 && ((double) fc->val * (double) fc->val2 / (double) drawnDays) >= 0.49) {
					elim = 0;
					break;
				}
							
				fc = fc->next;
			}
		}

		if (!(noMatch || elim)) break;
	}

	bubbleSortXByKey(drawnBallsBlend1);
	removeAllX2(globe);
	free(globe);

	return drawnBallsBlend1;
}



struct ListX * drawBallByBlend2(struct ListX *drawnBallsBlend2, struct ListX2 *ballSortOrder, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb)
{
	UINT8 i, j, k;
	UINT8 noMatch, elim;
	UINT8 drawball;
	UINT16 dDiff;

	struct ListX2 *globe = NULL;
	struct ListX *fc = NULL;
	struct ListXY *foundComb = NULL;

	foundComb = createListXY(foundComb);

	globe = createListX2(globe, totalBall);

	bubbleSortX2ByVal(ballSortOrder, 1);

	for (i=0, j=totalBall-1, k=0; k<totalBall; k++)
	{
		if (k%2) {
			addItemByIndex2(globe, totalBall-j, getKey2(ballSortOrder, j));
			j--;
		}
		else {
			addItemByIndex2(globe, i, getKey2(ballSortOrder, i));
			i++;
		}
	}

	for (k=0; k<totalBall; k++)
	{
		removeAllX(drawnBallsBlend2);

		for (i=0; i<drawBallCount; i++)
		{
			do {
				drawball = getKey2(globe, gaussIndex(totalBall)-1);
			} while(seqSearchX1(drawnBallsBlend2, drawball) >= 0);

			appendItem(drawnBallsBlend2, drawball);
		}

		if (drawBallCount == DRAW_BALL_EN) break;

		noMatch = 0;

		if (matchComb == 3) {
			noMatch = !search3CombXY(luckyBalls3, drawnBallsBlend2, NULL);
		}

		elim = 0;

		switch (elimComb)
		{
		case 4 : elim = search4CombXY(luckyBalls4, drawnBallsBlend2, foundComb);
		case 3 : if (!elim) elim = search3CombXY(luckyBalls3, drawnBallsBlend2, foundComb);
		case 2 : if (!elim) elim = search2CombXY(luckyBalls2, drawnBallsBlend2, foundComb);
		default: break;
		}

		if (elim)
		{
			fc = foundComb->list;

			for (i=0; fc && i<lengthY(foundComb); i++) 
			{
				dDiff = dateDiff(fc->day, fc->mon, fc->year, currDay, currMon, currYear);

				if (dDiff >= fc->val2 && ((double) fc->val * (double) fc->val2 / (double) drawnDays) >= 0.49) {
					elim = 0;
					break;
				}
							
				fc = fc->next;
			}
		}

		if (!(noMatch || elim)) break;
	}

	bubbleSortXByKey(drawnBallsBlend2);
	removeAllX2(globe);
	free(globe);

	return drawnBallsBlend2;
}



struct ListX * drawBallBySide(struct ListX *drawnBallsSide, struct ListX2 *ballSortOrder, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb)
{
	UINT8 i, k;
	UINT8 noMatch, elim;
	UINT8 drawball;
	UINT16 dDiff;

	struct ListX2 *globe = NULL;
	struct ListX *fc = NULL;
	struct ListXY *foundComb = NULL;

	foundComb = createListXY(foundComb);

	globe = createListX2(globe, totalBall);

	bubbleSortX2ByVal(ballSortOrder, 1);

	for (k=0; k<totalBall; k++)
	{
		if (k%2) {
			insertItem2(globe, getKey2(ballSortOrder, k));
		}
		else {
			appendItem2(globe, getKey2(ballSortOrder, k), 0);
		}
	}

	for (k=0; k<totalBall; k++)
	{
		removeAllX(drawnBallsSide);

		for (i=0; i<drawBallCount; i++)
		{
			do {
				drawball = getKey2(globe, gaussIndex(totalBall)-1);
			} while(seqSearchX1(drawnBallsSide, drawball) >= 0);

			appendItem(drawnBallsSide, drawball);
		}

		if (drawBallCount == DRAW_BALL_EN) break;

		noMatch = 0;

		if (matchComb == 3) {
			noMatch = !search3CombXY(luckyBalls3, drawnBallsSide, NULL);
		}

		elim = 0;

		switch (elimComb)
		{
		case 4 : elim = search4CombXY(luckyBalls4, drawnBallsSide, foundComb);
		case 3 : if (!elim) elim = search3CombXY(luckyBalls3, drawnBallsSide, foundComb);
		case 2 : if (!elim) elim = search2CombXY(luckyBalls2, drawnBallsSide, foundComb);
		default: break;
		}

		if (elim)
		{
			fc = foundComb->list;

			for (i=0; fc && i<lengthY(foundComb); i++) 
			{
				dDiff = dateDiff(fc->day, fc->mon, fc->year, currDay, currMon, currYear);

				if (dDiff >= fc->val2 && ((double) fc->val * (double) fc->val2 / (double) drawnDays) >= 0.49) {
					elim = 0;
					break;
				}
							
				fc = fc->next;
			}
		}

		if (!(noMatch || elim)) break;
	}

	bubbleSortXByKey(drawnBallsSide);
	removeAllX2(globe);
	free(globe);

	return drawnBallsSide;
}



struct ListX * drawBallByNorm(struct ListX *drawnBallsNorm, struct ListX2 *ballSortOrder, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb)
{
	UINT8 i, k;
	UINT8 noMatch, elim;
	UINT8 drawball;
	UINT16 dDiff;

	struct ListX2 *globe = NULL;
	struct ListX *fc = NULL;
	struct ListXY *foundComb = NULL;

	foundComb = createListXY(foundComb);

	globe = createListX2(globe, totalBall);

	bubbleSortX2ByVal(ballSortOrder, -1);

	for (k=0; k<totalBall; k++)
	{
		if (k%2) {
			insertItem2(globe, getKey2(ballSortOrder, k));
		}
		else {
			appendItem2(globe, getKey2(ballSortOrder, k), 0);
		}
	}

	for (k=0; k<totalBall; k++)
	{
		removeAllX(drawnBallsNorm);

		for (i=0; i<drawBallCount; i++)
		{
			do {
				drawball = getKey2(globe, gaussIndex(totalBall)-1);
			} while(seqSearchX1(drawnBallsNorm, drawball) >= 0);

			appendItem(drawnBallsNorm, drawball);
		}

		if (drawBallCount == DRAW_BALL_EN) break;

		noMatch = 0;

		if (matchComb == 3) {
			noMatch = !search3CombXY(luckyBalls3, drawnBallsNorm, NULL);
		}

		elim = 0;

		switch (elimComb)
		{
		case 4 : elim = search4CombXY(luckyBalls4, drawnBallsNorm, foundComb);
		case 3 : if (!elim) elim = search3CombXY(luckyBalls3, drawnBallsNorm, foundComb);
		case 2 : if (!elim) elim = search2CombXY(luckyBalls2, drawnBallsNorm, foundComb);
		default: break;
		}

		if (elim)
		{
			fc = foundComb->list;

			for (i=0; fc && i<lengthY(foundComb); i++) 
			{
				dDiff = dateDiff(fc->day, fc->mon, fc->year, currDay, currMon, currYear);

				if (dDiff >= fc->val2 && ((double) fc->val * (double) fc->val2 / (double) drawnDays) >= 0.49) {
					elim = 0;
					break;
				}
							
				fc = fc->next;
			}
		}

		if (!(noMatch || elim)) break;
	}

	bubbleSortXByKey(drawnBallsNorm);
	removeAllX2(globe);
	free(globe);

	return drawnBallsNorm;
}



void getDrawnBallCount()
{
/* readable code but very fast code below

	UINT8 i;
	UINT16 j;
	UINT8 n1, n2, n3, n4, n5;
	UINT8 keys[DRAW_BALL+1];
	UINT8 eu1, eu2;
	int index;

	struct ListX *aPrvDrawn = NULL;

	for (i=0; i<TOTAL_BALL; i++) {
		appendItem2(winningBallsDrawCount, i+1, 0);
	}

	for (j=0; j<winningBallRows; j++)
	{
		aPrvDrawn = getListXByIndex(winningDrawnBallsList, j);

		getKeys(aPrvDrawn, keys, 0, DRAW_BALL);

		n1 = keys[0];
		n2 = keys[1];
		n3 = keys[2];
		n4 = keys[3];
		n5 = keys[4];
		
		if ((index = seqSearchX2(winningBallsDrawCount,n1)) != -1) incVal2(winningBallsDrawCount, index);
		if ((index = seqSearchX2(winningBallsDrawCount,n2)) != -1) incVal2(winningBallsDrawCount, index);
		if ((index = seqSearchX2(winningBallsDrawCount,n3)) != -1) incVal2(winningBallsDrawCount, index);
		if ((index = seqSearchX2(winningBallsDrawCount,n4)) != -1) incVal2(winningBallsDrawCount, index);
		if ((index = seqSearchX2(winningBallsDrawCount,n5)) != -1) incVal2(winningBallsDrawCount, index);
	}

	for (i=0; i<TOTAL_BALL_EN; i++) {
		appendItem2(euNumberBallsDrawCount, i+1, 0);
	}

	for (j=0; j<euNumberBallRows; j++) 
	{
		aPrvDrawn = getListXByIndex(euNumberDrawnBallsList, j);

		getKeys(aPrvDrawn, keys, 0, DRAW_BALL_EN);

		eu1 = keys[0];
		eu2 = keys[1];

		if ((index = seqSearchX2(euNumberBallsDrawCount,eu1)) != -1) incVal2(euNumberBallsDrawCount, index);
		if ((index = seqSearchX2(euNumberBallsDrawCount,eu2)) != -1) incVal2(euNumberBallsDrawCount, index);
	}
*/

	UINT8 i;
	UINT16 j;

	struct ListX *aPrvDrawn = NULL;

	for (i=0; i<TOTAL_BALL; i++) 
	{
		if (winningBallsDrawCount->index < winningBallsDrawCount->size) 
		{
			winningBallsDrawCount->balls[winningBallsDrawCount->index] = i+1;
			winningBallsDrawCount->vals[winningBallsDrawCount->index] = 0;
			winningBallsDrawCount->index++;
		}
	}

	aPrvDrawn = winningDrawnBallsList->list;

	for (j=0; (aPrvDrawn) && j<winningBallRows; j++) 
	{
		winningBallsDrawCount->vals[aPrvDrawn->balls[0]-1]++;
		winningBallsDrawCount->vals[aPrvDrawn->balls[1]-1]++;
		winningBallsDrawCount->vals[aPrvDrawn->balls[2]-1]++;
		winningBallsDrawCount->vals[aPrvDrawn->balls[3]-1]++;
		winningBallsDrawCount->vals[aPrvDrawn->balls[4]-1]++;
		
		aPrvDrawn = aPrvDrawn->next;
	}
	
	for (i=0; i<TOTAL_BALL_EN; i++) 
	{
		if (euNumberBallsDrawCount->index < euNumberBallsDrawCount->size) 
		{
			euNumberBallsDrawCount->balls[euNumberBallsDrawCount->index] = i+1;
			euNumberBallsDrawCount->vals[euNumberBallsDrawCount->index] = 0;
			euNumberBallsDrawCount->index++;
		}
	}

	aPrvDrawn = euNumberDrawnBallsList->list;

	for (j=0; (aPrvDrawn) && j<euNumberBallRows; j++) 
	{
		euNumberBallsDrawCount->vals[aPrvDrawn->balls[0]-1]++;
		euNumberBallsDrawCount->vals[aPrvDrawn->balls[1]-1]++;
		
		aPrvDrawn = aPrvDrawn->next;
	}
}



void clearScreen()
{
	#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
		system("clear");
	#elif __MSDOS__
		clrscr();
	#else
		system("cls");
	#endif
}



void printPercentOfProgress(char *label, UINT32 completed, UINT32 all)
{
#if defined(__MSDOS__)
	union REGS regs;
	regs.x.ax = 2;
	int86(0x33, &regs, &regs);
#elif defined(WIN32)
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cci;
	GetConsoleCursorInfo(handle, &cci);
	cci.bVisible = 0;
	SetConsoleCursorInfo(handle, &cci);
#else
	printf("\e[?25l");
#endif

#if defined(__MSDOS__) || defined(WIN32)
	printf("%s: %2d%% Completed\r", label, (UINT32) ceil(100*completed/all));
#else
	fputs("\0337", stdout);
	printf("%s: %2d%% Completed", label, (UINT32) ceil(100*completed/all));
	fputs("\0338", stdout);
	fflush(stdout);
#endif

#if defined(__MSDOS__)
	regs.x.ax = 1;
	int86(0x33, &regs, &regs);
#elif defined(WIN32)
	GetConsoleCursorInfo(handle, &cci);
	cci.bVisible = 1;
	SetConsoleCursorInfo(handle, &cci);
#else
	printf("\e[?25h");
#endif
}



int get_app_path (char *pname, size_t pathsize)
{
	long result;

#if defined(__linux__)
	pathsize --;
	result = readlink("/proc/self/exe", pname, pathsize);
	if (result > 0)
	{
		pname[result] = 0;
		if ((access(pname, 0) == 0)) return 0;
	}
#elif WIN32
	result = GetModuleFileName(NULL, pname, pathsize);
	if (result > 0)
	{
		int len = strlen(pname);
		int idx;
		for (idx = 0; idx < len; idx++) {
			if (pname[idx] == '\\') pname[idx] = '/';
		}
		if ((access(pname, 0) == 0)) return 0;
	}
#elif SOLARIS
	char *p = getexecname();
	if (p)
	{
		if (p[0] == '/')
		{
			strncpy(pname, p, pathsize);
			if ((access(pname, 0) == 0)) 	return 0;
		}
		else
		{
			getcwd(pname, pathsize);
			result = strlen(pname);
			strncat(pname, "/", (pathsize - result));
			result ++;
			strncat(pname, p, (pathsize - result));
			if ((access(pname, 0) == 0)) return 0;
		}
	}
#elif __APPLE__
	/*
	extern int _NSGetExecutablePath(char *buf, uint32_t *bufsize);
	_NSGetExecutablePath copies the path of the executable
	into the buffer and returns 0 if the path was successfully
	copied in the provided buffer. If the buffer is not large
	enough, -1 is returned and the expected buffer size is
	copied in *bufsize. Note that _NSGetExecutablePath will
	return "a path" to the executable not a "real path" to the
	executable. That is the path may be a symbolic link and
	not the real file. And with deep directories the total
	bufsize needed could be more than MAXPATHLEN.
	*/
	int status = -1;
	uint32_t ul_tmp = pathsize;
	char *given_path = malloc(MAXPATHLEN * 2);
	if (!given_path) return status;
	pathsize = MAXPATHLEN * 2;
	result = _NSGetExecutablePath(given_path, &ul_tmp);
	if (result == 0)
	{
		if (realpath(given_path, pname) != NULL) {
			if ((access(pname, 0) == 0))	status = 0;
		}
	}
	free (given_path);
	return status;
#endif
	return -1; /* Path Lookup Failed */
}



void pressAnyKeyToExit()
{
	puts("Press any key to exit.");
	#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
		system("stty raw -echo");
		getchar();
		system("stty -raw echo");
	#else
		getch();
	#endif
}



UINT8 isIntString(char* input) 
{
	int i;
	for (i = 0; i < strlen(input); i++) {
		if(input[i] < '0' || input[i] > '9') {
			return 0;
		}
	}
	return 1;
}



int main(void)
{
	struct ListXY *coupon = NULL;
	struct ListXY *coupon_en = NULL;
	FILE *fp;

	char input[100], *endptr;
	UINT16 keyb = 0, keyb2;

	srand((unsigned) time(NULL));

	if (!init()) {
		puts("Initialization failed!\n");
		pressAnyKeyToExit();
		return -1;
	}

	coupon = createListXY(coupon);
	coupon_en = createListXY(coupon_en);

mainMenu:

	puts("EuroJackpot Lotto 2.0 Copyright ibrahim Tipirdamaz (c) 2023\n");
	printf("Includes draws between dates %s - %s\n", dateStart, dateEnd);
	printf("If the %s file is out of date, update it.\n\n", FILESTATS);
	puts("Which number drawn how many times?\n");

	bubbleSortX2ByVal(winningBallsDrawCount, -1);
	printDrawnBallCount(winningBallsDrawCount);
	printf("\n");

	#ifndef __MSDOS__
	printf("\n");
	#endif

	puts("Number of draws of EuroNumbers\n");

	bubbleSortX2ByVal(euNumberBallsDrawCount, -1);
	printDrawnBallCount(euNumberBallsDrawCount);
	printf("\n\n");

	printf("1- Draw Ball\n");
	printf("2- Matched 2 combinations: %lu\n", (unsigned long) match2comb);
	printf("3- Matched 3 combinations: %lu\n", (unsigned long) match3comb);
	printf("4- Matched 4 combinations: %lu\n", (unsigned long) match4comb);
	printf("5- Matched 5 combinations: %lu", (unsigned long) match5comb);
	#ifdef __MSDOS__
	gotoxy(41, wherey()-4);
	#else
	printf("\n");
	#endif
	printf("6- 2 Numbers that drawn together");
	#ifdef __MSDOS__
	gotoxy(41, wherey()+1);
	#else
	printf("\n");
	#endif
	printf("7- 3 Numbers that drawn together");
	#ifdef __MSDOS__
	gotoxy(41, wherey()+1);
	#else
	printf("\n");
	#endif
	printf("8- 4 Numbers that drawn together");
	#ifdef __MSDOS__
	gotoxy(41, wherey()+1);
	#else
	printf("\n");
	#endif
	printf("9- Euro Numbers that drawn together");
	#ifdef __MSDOS__
	gotoxy(41, wherey()+1);
	#else
	printf("\n");
	#endif
	printf("99-Exit\n");
	printf("\nPlease input your selection and press enter: ");

keybCommand:

	do {
		scanf("%s",input);
		keyb = strtod(input, &endptr);
        if (isIntString(input) && ((keyb >= 0 && keyb < 10) || keyb == 99)) {
			break;
		} else {
			printf("incorrect input!\n");
		}
	} while (1);

	clearScreen();

	if (keyb == 0) {
		goto mainMenu;
	}
	else if (keyb == 99) {
		goto exitProgram;
	}

	if ((fp = fopen(outputFile, "w")) == NULL) {
		printf("Can't open file %s\n", OUTPUTFILE);
		pressAnyKeyToExit();
		return -1;
	}

	if (keyb == 1)
	{
		printf("\nInput draw count (between 1-50) : ");

		do {
			scanf("%s",input);
			keyb2 = strtod(input, &endptr);
			if (isIntString(input) && (keyb2 > 0 && keyb2 < 51)) {
				break;
			} else {
				printf("incorrect input!\n");
			}
		} while (1);

		puts("");

		/* coupon, totalBall, drawBallCount, drawRowCount, drawByNorm, left, blend1, blend2, side, rand, lucky */
		drawBalls(coupon, TOTAL_BALL, DRAW_BALL, keyb2, 1, 1, 1, 1, 1, 1, 1);

		/* draw euro numbers */
		drawBalls(coupon_en, TOTAL_BALL_EN, DRAW_BALL_EN, keyb2, 1, 1, 1, 1, 1, 1, 1);
		printListXYWithENByKey(coupon, coupon_en, fp);
		removeAllXY(coupon_en);
		removeAllXY(coupon);

	} else if (keyb == 2) {
		printf("Calculation results are writing to %s file...\n", OUTPUTFILE);
		calcMatchComb(2, fp);
	} else if (keyb == 3) {
		printf("Calculation results are writing to %s file...\n", OUTPUTFILE);
		calcMatchComb(3, fp);
	} else if (keyb == 4) {
		printf("Calculation results are writing to %s file...\n", OUTPUTFILE);
		calcMatchComb(4, fp);
	} else if (keyb == 5) {
		calcMatchComb(5, fp);
	} else if (keyb == 6) {
		printf("Numbers that drawn together (2 numbers):\n\n");
		fprintf(fp, "Numbers that drawn together (2 numbers):\n\n");
		#if defined(__MSDOS__)
		luckyBalls2 = getLuckyBallsFromFile(luckyBalls2, 2);
		#endif
		bubbleSortYByVal(luckyBalls2, -1);
		printLuckyBalls(luckyBalls2, fp);
		#if defined(__MSDOS__)
		removeAllXY(luckyBalls2);
		#endif
	} else if (keyb == 7) {
		printf("Numbers that drawn together (3 numbers):\n\n");
		fprintf(fp, "Numbers that drawn together (3 numbers):\n\n");
		#if defined(__MSDOS__)
		luckyBalls3 = getLuckyBallsFromFile(luckyBalls3, 3);
		#endif
		bubbleSortYByVal(luckyBalls3, -1);
		printLuckyBalls(luckyBalls3, fp);
		#if defined(__MSDOS__)
		removeAllXY(luckyBalls3);
		#endif
	} else if (keyb == 8) {
		printf("Numbers that drawn together (4 numbers):\n\n");
		fprintf(fp, "Numbers that drawn together (4 numbers):\n\n");
		#if defined(__MSDOS__)
		luckyBalls4 = getLuckyBallsFromFile(luckyBalls4, 4);
		#endif
		bubbleSortYByVal(luckyBalls4, -1);
		printLuckyBalls(luckyBalls4, fp);
		#if defined(__MSDOS__)
		removeAllXY(luckyBalls4);
		#endif
	} else if (keyb == 9) {
		printf("Euro Numbers that drawn together:\n\n");
		fprintf(fp, "Euro Numbers that drawn together:\n\n");
		#if defined(__MSDOS__)
		euroNumbers = getEuroNumbersFromFile(euroNumbers);
		#endif
		printLuckyBalls(euroNumbers, fp);
		#if defined(__MSDOS__)
		removeAllXY(euroNumbers);
		#endif
	}

	printf("\nThe results are written to %s file.\n", OUTPUTFILE);

	fclose(fp);

	printf("\n\n");
	printf("0- Main Menu\n");
	if (keyb == 1) printf("1- Draw Again\n");
	printf("99-Exit\n");
	printf("\nPlease input your selection and press enter: ");

	goto keybCommand;

exitProgram:

	removeAllXY(winningDrawnBallsList);
	removeAllXY(euNumberDrawnBallsList);
	removeAllX2(winningBallsDrawCount);
	removeAllX2(euNumberBallsDrawCount);

	return 0;
}
