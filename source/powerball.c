/**
 * PowerBall Lotto Statistics and Ball Draw Algorithms
 * Author: İbrahim Tıpırdamaz  <itipirdamaz@gmail.com>
 * Copyright 2022
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


#define TOTAL_BALL 69			    // total ball count
#define DRAW_BALL 5		            // number of balls to be drawn
#define TOTAL_BALL_PB 26		    // total power ball count


#ifdef __MSDOS__
#define FILESTATS "powerbll.txt"	// statistics file (winning numbers, power ball numbers)
#else
#define FILESTATS "powerball.txt"	// statistics file (winning numbers, power ball numbers)
#endif

#define OUTPUTFILE "output.txt"     // file to write results


/* MS-DOS swap files for large FILESTATS due to lack of memory */
#define LBL2SWAPFILE "luckybl2.swp"	// luckyBalls2 swap file
#define LBL3SWAPFILE "luckybl3.swp"	// luckyBalls3 swap file
#define LBL4SWAPFILE "luckybl4.swp"	// luckyBalls4 swap file


#define UINT16MAX 65535	// max file rows



/* TYPE DEFINITIONS */


typedef unsigned char UINT8;

#if defined(__MSDOS__)
typedef unsigned int UINT16;
typedef unsigned long UINT32;
#else
typedef short unsigned int UINT16;
typedef unsigned int UINT32;
#endif



/* GLOBAL VARIABLES */


/* current working directory */
char *cwd = NULL;
char *fileStats = NULL;		// cwd + PATH_SEPARATOR + FILESTATS
char *outputFile = NULL;	// cwd + PATH_SEPARATOR + OUTPUTFILE



struct Item2 {			/* List item (ball) */
	struct Item2 *next;	// pointer to next item (ball) in list (draw)
	UINT8 key;		// ball number
	UINT16 val;		// other info such as how many times the ball has been drawn so far
};



struct ListX {			/* List (globe, drawn balls, statistics file row etc) */
	char *label;		// list label (for new drawns)
	UINT16 year;		// old drawn year
	UINT8 mon;		// old drawn month
	UINT8 day;		// old drawn day (year, month, day for statistics file rows)
	UINT16 val;		// other value (how many times the numbers that drawn together, for lucky numbers)
	UINT8 *balls;		// ball array
	UINT8 index;		// last item index in array (added items count, if index==0 array is empty)
	UINT8 size;			// allocated total size of array
	struct ListX *next;	// next list (next row, if two dimensions)
};



struct ListX2 {			/* List (ballStats) */
	struct Item2 *head;	// first item in the list
	struct ListX2 *next;	// next list (next row, if two dimensions)
};



struct ListXY {			/* 2 dimensions List. Multi draw (coupon or drawn balls from file has been drawn so far) */
	struct ListX *list;	// list
};



/* Drawn balls lists from file has been drawn so far */

struct ListXY *winningDrawnBallsList = NULL;
struct ListXY *powerBallDrawnBallsList = NULL;
UINT16 fileStatRows = 0;


/* Old drawn dates between dateStart and dateEnd */

char dateStart[11], dateEnd[11];


/* How many times were the winning numbers drawn in the previous draws? */

struct ListX2 *winningBallStats = NULL;
struct ListX2 *powerBallStats = NULL;


/* Numbers that drawn together (how many times the numbers drawn together) */

struct ListXY *luckyBalls2 = NULL;
struct ListXY *luckyBalls3 = NULL;
struct ListXY *luckyBalls4 = NULL;


/* Matched combinations of numbers from previous draws */

UINT32 match2comb = 0;
UINT32 match3comb = 0;
UINT32 match4comb = 0;
UINT32 match5comb = 0;



/* FUNCTION DEFINITIONS */


/**
 * Returns the difference between two dates in days
 * Date 1 : m1/d1/y1 (mm/dd/yyyy)
 * Date 2 : m2/d2/y2 (mm/dd/yyyy)
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
 * Date format mm/dd/yyyy
 *
 * @param {char *} date     : char array (date assign as string to this variable (mm/dd/yyyy))
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
 * Wait until press any key
 *
 */
void pressAnyKey();



/**
 * Create Empty List (1 dimension)
 * 
 * @param {struct ListX *} pl : refers to a ball list
 * @param {Integer} size      : list size
 * @param {char *} label      : list label (for new drawns lists, otherwise NULL)
 * @param {Integer} val       : other value
 * @param {Integer} year      : year
 * @param {Integer} mon       : month
 * @param {Integer} day       : day
 * @return {struct ListX *}   : refers to the ball list (memory allocated)
 */
struct ListX *createListX(struct ListX *pl, UINT8 size, char *label, UINT16 val, UINT16 year, UINT8 mon, UINT8 day);



/**
 * Create Empty List (1 dimension)
 *
 * @param {struct ListX2 *} pl : refers to a ball list
 * @return {struct ListX2 *}   : refers to the ball list (memory allocated)
 */
struct ListX2 *createListX2(struct ListX2 *pl);



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
 * @param {struct ListX *}     : refers to a ball list
 * @param {Integer} key        : the key to which the ball will be added
 */
void insertItem(struct ListX *pl, UINT8 key);



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
 * @param {struct ListX *}     : refers to a ball list
 * @param {Integer} ind        : the index to which the ball will be removed
 * @return {Integer}           : returns key (ball number)
 */
UINT8 removeItemByIndex(struct ListX *pl, UINT8 ind);



/** 
 * If the specified key is not in the list, it adds to the specified index and returns 1
 *
 * @param {struct ListX *}     : refers to a ball list
 * @param {Integer} ind        : the index to which the ball will be added
 * @param {Integer} key        : key (ball number)
 * @return {Integer}           : If the key is in the list, it does not add it to the list and return 0
 */
UINT8 addItemByIndex(struct ListX *pl, UINT8 ind, UINT8 key);



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
 * Find the pointer of the item (ball) at the end of the list
 * 
 * @param {struct ListX2 *}    : refers to a ball list
 * @return {struct Item2 *}    : returns pointer of the item at the end of the list
 */
struct Item2 *atEnd2(struct ListX2 *pl);



/** 
 * Find the pointer of the list at the end of the 2 dimensions list
 * 
 * @param {struct ListXY *}    : refers to a 2 dimensions ball list (multiple draws)
 * @return {struct ListX *}    : returns pointer of the list at the end of the 2 dimensions list
 */
struct ListX *atEndY(struct ListXY *pl);



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
int seqSearch2X1(struct ListX2 *pl, UINT8 key);



/** 
 * Searches for 2 key (ball number) from the list.
 *
 * @param {struct ListX *}          : refers to a ball list
 * @param {unsigned char *} keys    : keys (ball numbers) to be search
 * @return {Integer}                : It returns the index of the number if it finds it, or -1 if it doesn't.
 */
int seqSearchX2(struct ListX *pl, UINT8 *keys);



/** 
 * Searches for 3 key (ball number) from the list.
 *
 * @param {struct ListX *}          : refers to a ball list
 * @param {unsigned char *} keys    : keys (ball numbers) to be search
 * @return {Integer}                : It returns the index of the number if it finds it, or -1 if it doesn't.
 */
int seqSearchX3(struct ListX *pl, UINT8 *keys);



/** 
 * Searches for 4 key (ball number) from the list.
 * 
 * @param {struct ListX *}          : refers to a ball list
 * @param {unsigned char *} keys    : keys (ball numbers) to be search
 * @return {Integer}                : It returns the index of the number if it finds it, or -1 if it doesn't.
 */
int seqSearchX4(struct ListX *pl, UINT8 *keys);



/** 
 * Searches for 5 key (ball number) from the list.
 *
 * @param {struct ListX *}          : refers to a ball list
 * @param {unsigned char *} keys    : keys (ball numbers) to be search
 * @return {Integer}                : It returns the index of the number if it finds it, or -1 if it doesn't.
 */
int seqSearchX5(struct ListX *pl, UINT8 *keys);



/**
 * Searches for 2 key (ball number) from the 2 dimensions list.
 * 
 * @param {struct ListXY *}         : refers to a 2 dimensions ball list
 * @param {unsigned char *} keys    : keys (ball numbers) to be search
 * @return {Integer}                : It returns the index of the numbers if it finds it, or -1 if it doesn't.
 *                                    This is index of the first list containing the key in the 2 dimensions list
 */
int seqSearchXY2(struct ListXY *pl, UINT8 *keys);



/**
 * Searches for 3 key (ball number) from the 2 dimensions list.
 * 
 * @param {struct ListXY *}         : refers to a 2 dimensions ball list
 * @param {unsigned char *} keys    : keys (ball numbers) to be search
 * @return {Integer}                : It returns the index of the numbers if it finds it, or -1 if it doesn't.
 *                                    This is index of the first list containing the key in the 2 dimensions list
 */
int seqSearchXY3(struct ListXY *pl, UINT8 *keys);



/**
 * Searches for 4 key (ball number) from the 2 dimensions list.
 * 
 * @param {struct ListXY *}         : refers to a 2 dimensions ball list
 * @param {unsigned char *} keys    : keys (ball numbers) to be search
 * @return {Integer}                : It returns the index of the numbers if it finds it, or -1 if it doesn't.
 *                                    This is index of the first list containing the key in the 2 dimensions list
 */
int seqSearchXY4(struct ListXY *pl, UINT8 *keys);



/** 
 * Increments the value (the number of times the ball has drawn so far) of the item at the specified index by 1
 *
 * @param {struct ListX2 *}    : refers to a ball list (ballStats)
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
 * @param {struct ListX2 *}    : refers to a ball list (ballStats)
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
 * Bubble sort by key (ball number) from smallest to greager the items (ball) in the ball list
 *
 * @param {struct ListX *}    : refers to a ball list
 */
void bubbleSortXByKey(struct ListX *pl);



/**
 * Function to swap data of two item (ball) a and b
 * 
 * @param {struct Item2 *} a   : refers to a ball
 * @param {struct Item2 *} b   : refers to a ball
 */
void swapX2(struct Item2 *a, struct Item2 *b);



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
 * print ListXY With PowerBall ByKey
 * Print keys of the items (row by row) in the 2 dimensions lists
 * 
 * @param {struct ListXY *}    : refers to 2 dimensions ball list (winning numbers)
 * @param {struct ListXY *}    : refers to 2 dimensions ball list (powerBall)
 * @param {FILE *} fp          : refers to output file. If fp != NULL print to output file
 */
void printListXYWithPBByKey(struct ListXY *pl1, struct ListXY *pl2, FILE *fp);



/** 
 * How many times the balls has been drawn so far 
 * (assign values to winningBallStats and powerBallStats global variables)
 * 
 */
void getDrawnBallsStats();



/**
 * Print key-val pair of the items in the list by ball statistics (How many times the balls has been drawn so far)
 * 
 * @param {struct ListX2 *} ballStats      : refers to balls and the number of times each ball was drawn in previous draws.
 */
void printBallStats(struct ListX2 *ballStats);



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
 * The numbers that came out in the previous draws (ballStats) are arranged at the base of the pascal's triangle and the ball is dropped on 
 * (with gaussIndex function) it and the ball hit is drawn. The gaussIndex function returns the index (random) of one of the balls arranged 
 * at the base of pascal's triangle. If a ball is dropped from the top node of Pascal's triangle, it drops to the left or right each time 
 * it hits the node (like tossing a coin). The gaussIndex function moves towards the base of the triangle, making a random selection at
 * each node. When it reaches the base of the triangle, it returns the index whichever node (index) it hits. 
 * 
 * @param {Integer} ballCount      : Ball count in the globe
 * @return {Integer}               : Returns the random ball number (or ballStats index, you substract 1 from the return value for index)
 *                                   For example, ballCount = 90, it returns number between 1-90 (for index 0-89) (theoretically)
 *                                   Usually retuns the middle numbers
 */
UINT8 gaussIndex(UINT8 ballCount);



/**
 * Draw random numbers
 * 
 * @param {struct ListX *} drawnBallsRand : refers to the balls to be drawn. 
 * @param {struct ListX2 *} ballStats     : refers to balls and the number of times each ball was drawn in previous draws.
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
struct ListX * drawBallByRand(struct ListX *drawnBallsRand, struct ListX2 *ballStats, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb);



/**
 * The numbers that came out in the previous draws (ballStats) are arranged at the base of the pascal triangle
 * (stacked to the left) according to the number that comes out the most from the number that comes out the least, 
 * and the ball is dropped on (with gaussIndex function) it and the ball hit is drawn.
 *
 * @param {struct ListX *} drawnBallsLeft : refers to the balls to be drawn. 
 * @param {struct ListX2 *} ballStats     : refers to balls and the number of times each ball was drawn in previous draws.
 * @param {Integer} totalBall             : Total ball count in the globe
 * @param {Integer} drawBallCount      	  : Number of balls to be drawn
 * @param {Integer} matchComb             : Number of combinations in which the drawn numbers must match any of the previous draws.
 * @param {Integer} elimComb              : If a combination of the drawn numbers matched with any of the previous draws, specified by the
 *                                          elimComb parameter, the draw is renewed.
 * @return {struct ListX *} drawnBallsLeft: Returns new drawn balls.
 */
struct ListX * drawBallByLeft(struct ListX *drawnBallsLeft, struct ListX2 *ballStats, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb);



/** 
 * Blend 1
 * Dividing the list of numbers (ballStats) in the middle, inverting the left and right parts and combining them.
 * The numbers that came out in the previous draws (ballStats) are arranged at the base of the
 * pascal triangle and the ball is dropped on (with gaussIndex function) it and the ball hit is drawn.
 * 
 * @param {struct ListX *} drawnBallsBlend1	: refers to the balls to be drawn.
 * @param {struct ListX2 *} ballStats		: refers to balls and the number of times each ball was drawn in previous draws.
 * @param {Integer} totalBall			: Total ball count in the globe
 * @param {Integer} drawBallCount		: Number of balls to be drawn
 * @param {Integer} matchComb			: Number of combinations in which the drawn numbers must match any of the previous draws.
 * @param {Integer} elimComb			: If a combination of the drawn numbers matched with any of the previous draws, 
 *						  specified by the elimComb parameter, the draw is renewed.
 * @return {struct ListX *} drawnBallsBlend1	: Returns new drawn balls.
 */
struct ListX * drawBallByBlend1(struct ListX *drawnBallsBlend1, struct ListX2 *ballStats, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb);



/** 
 * Blend 2
 * The balls are taken from the left and right of the ballStats and placed from the middle of the globe 
 * (The base of the Pascal's triangle) to the edges
 * 
 * @param {struct ListX *} drawnBallsBlend2	: refers to the balls to be drawn. 
 * @param {struct ListX2 *} ballStats		: refers to balls and the number of times each ball was drawn in previous draws.
 * @param {Integer} totalBall			: Total ball count in the globe
 * @param {Integer} drawBallCount		: Number of balls to be drawn
 * @param {Integer} matchComb			: Number of combinations in which the drawn numbers must match any of the previous draws.
 * @param {Integer} elimComb			: If a combination of the drawn numbers matched with any of the previous draws, specified by the
 * 						  elimComb parameter, the draw is renewed.
 * @return {struct ListX *} drawnBallsBlend2	: Returns new drawn balls.
 */
struct ListX * drawBallByBlend2(struct ListX *drawnBallsBlend2, struct ListX2 *ballStats, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb);



/**
 * The balls that drawn the least are placed in the middle of the globe, and the balls that drawn the most are placed on the edges.
 * 
 * @param {struct ListX *} drawnBallsSide	: refers to the balls to be drawn. 
 * @param {struct ListX2 *} ballStats		: refers to balls and the number of times each ball was drawn in previous draws.
 * @param {Integer} totalBall			: Total ball count in the globe
 * @param {Integer} drawBallCount		: Number of balls to be drawn
 * @param {Integer} matchComb			: Number of combinations in which the drawn numbers must match any of the previous draws.
 * @param {Integer} elimComb			: If a combination of the drawn numbers matched with any of the previous draws, specified by the
 * 						  elimComb parameter, the draw is renewed.
 * @return {struct ListX *} drawnBallsSide	: Returns new drawn balls.
 */
struct ListX * drawBallBySide(struct ListX *drawnBallsSide, struct ListX2 *ballStats, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb);



/** 
 * The most drawn balls are placed in the center of the globe,
 * the least drawn balls are placed at the edges (normal distribution)
 * 
 * @param {struct ListX *} drawnBallsNorm	: refers to the balls to be drawn.
 * @param {struct ListX2 *} ballStats		: refers to balls and the number of times each ball was drawn in previous draws.
 * @param {Integer} totalBall			: Total ball count in the globe
 * @param {Integer} drawBallCount		: Number of balls to be drawn
 * @param {Integer} matchComb			: Number of combinations in which the drawn numbers must match any of the previous draws.
 * @param {Integer} elimComb			: If a combination of the drawn numbers matched with any of the previous draws, specified by the
 * 						  elimComb parameter, the draw is renewed.
 * 						  For example, a previous draw is [1, 2, 4, 6, 8, 9]
 * 						  the new draw (drawnBallsNorm) is [1, 2, 5, 7, 9, 10]
 * 						  If matchComb = 2 and elimComb = 0, draw is OK, because (1,2) (1,9) or (2,9) matched.
 * 						  If matchComb = 2 and elimComb = 3, draw is renewed, because (1,2,9) eliminated. 
 * @return {struct ListX *} drawnBallsNorm	: Returns new drawn balls.
 */
struct ListX * drawBallByNorm(struct ListX *drawnBallsNorm, struct ListX2 *ballStats, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb);



/** 
 * The findComb function searches for combinations specified by the comb parameter in previous draws. 
 * For example, the numbers [1, 2, 4, 6, 8, 9] were drawn in one of the previous draws. 
 * The newly drawn numbers are drawnBalls = [1, 2, 5, 7, 9, 10].
 * findComb(drawnBalls, 3) searches for 3-combinations in drawnBalls in previous draws.
 * The findComb function returns 1 because there are matching triple combination (1,2,9). 
 * The comb parameter can take values 2, 3, 4, 5 and 6. 
 *
 * @param {struct ListX *} drawnBalls : refers to balls drawn in a draw.
 * @param {Integer} comb              : ball combinations
 * @return {Integer}                  : Returns 1 if found, 0 if not. 
 */
UINT8 findComb(struct ListX *drawnBalls, UINT8 comb);



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
 * @return {Integer}                      : Returns 1 if found, 0 if not.
 */
UINT8 search4CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls);



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
 * @return {Integer}                      : Returns 1 if found, 0 if not. 
 */
UINT8 search3CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls);



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
 * @return {Integer}                      : Returns 1 if found, 0 if not. 
 */
UINT8 search2CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls);



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
 * @param {Integer} drawBallCount     	  : DRAW_BALL for normal ball list or 1 for powerBall list (coupon column count)
 * @return {Integer}                      : Returns 1 if found, 0 if not.
 */
UINT8 search1BallXY(struct ListXY *couponList, struct ListX *drawnBalls, UINT8 drawBallCount);



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
void calcCombMatch(UINT8 comb, FILE *fp);



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
 * @param {struct ListXY *} luckyBalls	: refers to the lucky balls list
 * @param {Integer} comb		: double, triple or quartet (2, 3 or 4) combinations
 * @return {Integer} 			: returns 1 if success, otherwise returns 0
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
 * @param {struct ListXY *} drawnBallsLucky	: refers to the balls to be drawn. 
 * @param {Integer} drawNum			: 1: lucky 3, 2: 2 of lucky3, 3: lucky 2
 * @param {Integer} totalBall			: Total ball count in the globe
 * @param {Integer} drawBallCount		: Number of balls to be drawn
 * @return {struct ListXY *} drawnBallsLucky	: Returns new drawn balls.
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
 * The lists is assigned to the global variables winningDrawnBallsList or powerBallDrawnBallsList
 * 
 * @param {struct ListXY *}      : refer to 2 dimensions list of winning numbers (winningDrawnBallsList or powerBallDrawnBallsList)
 * @param {char *}               : Drawn list file
 * @return {Integer}		 : returns 0 if fileName or record not found, otherwise returns the number of records.
 */
UINT16 getDrawnBallsList(struct ListXY *ballList, char *fileName);



/* FUNCTIONS */


struct ListX *createListX(struct ListX *pl, UINT8 size, char *label, UINT16 val, UINT16 year, UINT8 mon, UINT8 day)
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
	pl->year = year;
	pl->mon = mon;
	pl->day = day;

	return pl;
}



struct ListX2 *createListX2(struct ListX2 *pl)
{
	pl = (struct ListX2 *) malloc(sizeof(struct ListX2));
	pl->head = NULL;
	pl->next = NULL;

	return pl;
}



struct ListXY *createListXY(struct ListXY *pl)
{
	pl = (struct ListXY *) malloc(sizeof(struct ListXY));
	pl->list = NULL;

	return pl;
}



void insertItem(struct ListX *pl, UINT8 key)
{
	UINT8 i;

	if (pl->index < pl->size) {
		for (i=pl->index; i>0; i--) {
			pl->balls[i] = pl->balls[i-1];
		}
		pl->balls[0] = key;
		pl->index++;
	}
}



struct Item2 *atEnd2(struct ListX2 *pl)
{
	struct Item2 *curr;

	if (pl->head == NULL)
		return NULL;

	curr = pl->head;

	while (curr->next)
		curr = curr->next;

	return curr;
}



struct ListX *atEndY(struct ListXY *pl)
{
	struct ListX *curr;

	if (pl == NULL)
		return NULL;

	curr = pl->list;

	while (curr->next)
		curr = curr->next;

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
	struct Item2 *pt = (struct Item2 *) malloc(sizeof(struct Item2));
	pt->key = key;
	pt->val = val;
	pt->next = NULL;

	if (pl->head == NULL)
		pl->head = pt;
	else
		(atEnd2(pl))->next = pt;
}



void appendItems(struct ListX *pl, UINT8 *keys)
{
	UINT8 i, size = strlen((char *)keys);
	
	if (pl->index + size <= pl->size) {
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

	if (pl->list == NULL)
		pl->list = plNext;
	else {
		(atEndY(pl))->next = plNext;
		plNext->next = NULL;
	}
}



UINT8 removeItemByIndex(struct ListX *pl, UINT8 ind)
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

	if (pt) {
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



UINT8 addItemByIndex(struct ListX *pl, UINT8 ind, UINT8 key)
{
	UINT8 i;

	if (pl->index < pl->size) 
	{
		for(i=pl->index; i>ind; i--) {
			pl->balls[i] = pl->balls[i-1];
		}
		pl->balls[i] = key;
		pl->index++;
	} else return 0;

	return 1;
}



void addListByIndex(struct ListXY *pl, struct ListX *newList, UINT16 ind)
{
	UINT16 i;
	struct ListX *right = NULL, *left = pl->list;

	if (left == NULL || ind == 0) {
		pl->list = newList;
		newList->next = left;
	} else {
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
	struct Item2 *prv, *pt = pl->head;

	while (pt != NULL) {
		prv = pt;
		pt = pt->next;
		free(prv);
	} 

	pl->head = NULL;
}



void removeAllXY(struct ListXY *pl)
{
	struct ListX *prvList, *pList = pl->list;

	while (pList != NULL) {
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



int seqSearchX1(struct ListX *pl, UINT8 key)
{
	UINT8 i;

	for (i=0; i<pl->index; i++) {
		if (pl->balls[i] == key) return i;
	}

	return -1;
}



int seqSearch2X1(struct ListX2 *pl, UINT8 key)
{
	struct Item2 *t=pl->head;
	UINT8 i = 0;

	while ( (t) && (t->key!=key) ) {
		t=t->next;
		i++;
	}

	if (t) return i;
	else return -1;
}



int seqSearchX2(struct ListX *pl, UINT8 *keys)
{
	UINT8 found=0;
	UINT8 i;

	for (i=0; i < pl->index -1; i++) 
	{
		if (pl->balls[i] == keys[0]) found++;
		if (pl->balls[i+1] == keys[1]) found++;
		if (found == 2) return i;
	}

	return -1;
}



int seqSearchX3(struct ListX *pl, UINT8 *keys)
{
	UINT8 found=0;
	UINT8 i;

	for (i=0; i < pl->index -2; i++) 
	{
		if (pl->balls[i] == keys[0]) found++;
		if (pl->balls[i+1] == keys[1]) found++;
		if (pl->balls[i+2] == keys[2]) found++;
		if (found == 3) return i;
	}

	return -1;
}



int seqSearchX4(struct ListX *pl, UINT8 *keys)
{
	UINT8 found=0;
	UINT8 i;

	for (i=0; i < pl->index -3; i++) 
	{
		if (pl->balls[i] == keys[0]) found++;
		if (pl->balls[i+1] == keys[1]) found++;
		if (pl->balls[i+2] == keys[2]) found++;
		if (pl->balls[i+3] == keys[3]) found++;
		if (found == 4) return i;
	}

	return -1;
}



int seqSearchX5(struct ListX *pl, UINT8 *keys)
{
	UINT8 found=0;
	UINT8 i;

	for (i=0; i < pl->index -4; i++) 
	{
		if (pl->balls[i] == keys[0]) found++;
		if (pl->balls[i+1] == keys[1]) found++;
		if (pl->balls[i+2] == keys[2]) found++;
		if (pl->balls[i+3] == keys[3]) found++;
		if (pl->balls[i+4] == keys[4]) found++;
		if (found == 5) return i;
	}

	return -1;
}



int seqSearchXY2(struct ListXY *pl, UINT8 *keys)
{
	struct ListX *tmp = pl->list;
	UINT16 i=0;

	while (tmp)
	{
		if (seqSearchX2(tmp, keys) >= 0) break;
		tmp = tmp->next;
		i++;
	}

	if (tmp) return i;
	else return -1;
}



int seqSearchXY3(struct ListXY *pl, UINT8 *keys)
{
	struct ListX *tmp = pl->list;
	UINT16 i=0;

	while (tmp)
	{
		if (seqSearchX3(tmp, keys) >= 0) break;
		tmp = tmp->next;
		i++;
	}

	if (tmp) return i;
	else return -1;
}



int seqSearchXY4(struct ListXY *pl, UINT8 *keys)
{
	struct ListX *tmp = pl->list;
	UINT16 i=0;

	while (tmp)
	{
		if (seqSearchX4(tmp, keys) >= 0) break;
		tmp = tmp->next;
		i++;
	}

	if (tmp) return i;
	else return -1;
}



void incVal2(struct ListX2 *pl, UINT8 ind)
{
	struct Item2 *t=pl->head;
	UINT8 i;

	for (i=0; (t) && i<ind; i++) {
		t=t->next;
	}

	t->val++;
}



void incValXY(struct ListXY *pl, UINT16 ind)
{
	struct ListX *tmp = pl->list;
	UINT16 i;

	for (i=0; (tmp) && i<ind; i++) {
		tmp=tmp->next;
	}

	tmp->val++;
}



UINT16 getValXY(struct ListXY *pl, UINT16 ind)
{
	struct ListX *tmp = pl->list;
	UINT16 i;

	for (i=0; (tmp) && i<ind; i++) {
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
	struct Item2 *t=pl->head;
	UINT8 i;

	for (i=0; (t) && i<index; i++) {
		t=t->next;
	}

	return t->key;
}



void getKeys(struct ListX *pl, UINT8 *keys, UINT8 index, UINT8 count)
{
	UINT8 i, j;

	if (count == 0) count = pl->index;

	for (i=index, j=0; j<count; i++, j++) {
		keys[j] = pl->balls[i];
	}

	keys[j] = '\0';
}



struct ListX * getListXByIndex(struct ListXY *pl, UINT16 ind)
{
	struct ListX *l = pl->list;
	UINT16 i;

	for (i=0; (l) && i<ind; i++) {
		l = l->next;
	}

	return l;
}



void bubbleSortX2ByVal(struct ListX2 *pl, int inc)
{
	UINT8 swapped;
	struct Item2 *ptr1;
	struct Item2 *lptr = NULL;

	if (pl->head == NULL) return;

	do {
		swapped = 0;
		ptr1 = pl->head;

		while (ptr1->next != lptr)
		{
			if ((inc == 1 && ptr1->val > ptr1->next->val) || (inc == -1 && ptr1->val < ptr1->next->val)) {
				swapX2(ptr1, ptr1->next);
				swapped = 1;
			}
			ptr1 = ptr1->next;
		}
		lptr = ptr1;
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



void swapX2(struct Item2 *a, struct Item2 *b)
{
	UINT8 temp;
	UINT16 temp2;

	temp = a->key;
	a->key = b->key;
	b->key = temp;

	temp2 = a->val;
	a->val = b->val;
	b->val = temp2;
}



void swapY(struct ListX *a, struct ListX *b)
{
	char *label;
	UINT8 *balls;
	UINT8 day, mon;
	UINT16 year, val;

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
}



void printListXByKey(struct ListX *pl, UINT8 printTo, FILE *fp)
{
	UINT8 i;
	char buf[5];

	for (i=0; i < pl->index; i++) {
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



void printListXYWithPBByKey(struct ListXY *pl1, struct ListXY *pl2, FILE *fp)
{
	struct ListX *nl1 = pl1->list;
	struct ListX *nl2 = pl2->list;
	char ioBuf[60];
	char buf[30];
	UINT8 i, j;

	puts("     Numbers       PowerBall\n");
	if (fp != NULL) fputs("     Numbers       PowerBall\n\n", fp);

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



void printBallStats(struct ListX2 *ballStats)
{
	struct Item2 *t = ballStats->head;
	UINT8 i = 0;

#ifdef __MSDOS__
	UINT8 col = 10;
#else
	UINT8 col = 15;
#endif

	while (t) {
		printf("%2d:%3d", t->key, t->val);
		if ((i+1) % col == 0) puts("");
		else printf("  ");
		t=t->next;
		i++;
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

	 if (pl == NULL || pl->list == NULL) {
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

	for (n=0; sum>0; n++) {
		sum = sum - (n+1);
	}

	return n;
}



UINT8 gaussIndex(UINT8 ballCount)
{
	UINT16 node=0, leftnode=0, level;

	for (level=1; level<ballCount;)
	{
		if (rand() % 100 < 49 + rand() % 2)
			node = node + level;
		else
			node = node + level + 1;

		leftnode = leftnode + level;
		level++;
	}

	return node - (leftnode-1);
}



UINT8 findComb(struct ListX *drawnBalls, UINT8 comb)
{
	UINT8 found = 0;

	switch (comb)
	{
	case 2 : found = search2CombXY(winningDrawnBallsList, drawnBalls); break;
	case 3 : found = search3CombXY(winningDrawnBallsList, drawnBalls); break;
	case 4 : found = search4CombXY(winningDrawnBallsList, drawnBalls); break;
	case 5 : found = search5CombXY(winningDrawnBallsList, drawnBalls); break;
	default: break;
	}

	return found;
}



UINT8 search5CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls)
{
	UINT16 i, listRows;
	UINT8 balls[DRAW_BALL+1];
	struct ListX *aPrvDrawn = NULL;
	UINT8 found = 0;

	if (prvDrawnsList == winningDrawnBallsList) listRows = fileStatRows;
	else listRows = lengthY(prvDrawnsList);

	getKeys(drawnBalls, balls, 0, 0);

	for (i=0; i<listRows; i++) 
	{
		aPrvDrawn = getListXByIndex(prvDrawnsList, i);

		if (seqSearchX5(aPrvDrawn, balls) >= 0) {found=1; break;}
	}

	return found;
}



UINT8 search5CombX(struct ListX *aPrvDrawn, struct ListX *drawnBalls)
{
	UINT8 balls[DRAW_BALL+1];
	UINT8 found = 0;

	getKeys(drawnBalls, balls, 0, 0);

	if (seqSearchX5(aPrvDrawn, balls) >= 0) found=1;

	return found;
}



UINT8 search4CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls)
{
	UINT16 i, listRows;
	UINT8 b1[DRAW_BALL+1];
	UINT8 b2[5];
	struct ListX *aPrvDrawn = NULL;
	UINT8 found = 0;

	if (prvDrawnsList == winningDrawnBallsList) listRows = fileStatRows;
	else listRows = lengthY(prvDrawnsList);

	getKeys(drawnBalls, b1, 0, drawnBalls->size);

	for (i=0; i<listRows; i++)
	{
		aPrvDrawn = getListXByIndex(prvDrawnsList, i);

		b2[0] = b1[0];
		b2[1] = b1[1];
		b2[2] = b1[2];
		b2[3] = b1[3];
		b2[4] = '\0';

		if (seqSearchX4(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[0];
		b2[1] = b1[1];
		b2[2] = b1[2];
		b2[3] = b1[4];
		b2[4] = '\0';

		if (seqSearchX4(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[0];
		b2[1] = b1[1];
		b2[2] = b1[3];
		b2[3] = b1[4];
		b2[4] = '\0';

		if (seqSearchX4(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[0];
		b2[1] = b1[2];
		b2[2] = b1[3];
		b2[3] = b1[4];
		b2[4] = '\0';

		if (seqSearchX4(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[1];
		b2[1] = b1[2];
		b2[2] = b1[3];
		b2[3] = b1[4];
		b2[4] = '\0';

		if (seqSearchX4(aPrvDrawn, b2) >= 0) {found=1; break;}
	}

	return found;
}



UINT8 search4CombX(struct ListX *aPrvDrawn, struct ListX *drawnBalls, struct ListXY *luckyBalls, char *buf)
{
	struct ListX *lb = NULL;
	int index;
	UINT8 b1[DRAW_BALL+1];
	UINT8 b2[5];
	UINT8 found = 0;
	char fStr[14];

	getKeys(drawnBalls, b1, 0, drawnBalls->size);

	if (buf) buf[0] = '\0';

	b2[0] = b1[0];
	b2[1] = b1[1];
	b2[2] = b1[2];
	b2[3] = b1[3];
	b2[4] = '\0';

	if (seqSearchX4(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			sprintf(fStr, "(%2d,%2d,%2d,%2d)", b1[0], b1[1], b1[2], b1[3]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY4(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 4, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[0];
	b2[1] = b1[1];
	b2[2] = b1[2];
	b2[3] = b1[4];
	b2[4] = '\0';

	if (seqSearchX4(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d,%2d,%2d)", b1[0], b1[1], b1[2], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY4(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 4, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[0];
	b2[1] = b1[1];
	b2[2] = b1[3];
	b2[3] = b1[4];
	b2[4] = '\0';
		
	if (seqSearchX4(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d,%2d,%2d)", b1[0], b1[1], b1[3], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY4(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 4, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[0];
	b2[1] = b1[2];
	b2[2] = b1[3];
	b2[3] = b1[4];
	b2[4] = '\0';
		
	if (seqSearchX4(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d,%2d,%2d)", b1[0], b1[2], b1[3], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY4(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 4, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[1];
	b2[1] = b1[2];
	b2[2] = b1[3];
	b2[3] = b1[4];
	b2[4] = '\0';

	if (seqSearchX4(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d,%2d,%2d)", b1[1], b1[2], b1[3], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY4(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 4, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	return found;
}



UINT8 search3CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls)
{
	UINT16 i, listRows;
	UINT8 b1[DRAW_BALL+1];
	UINT8 b2[4];
	struct ListX *aPrvDrawn = NULL;
	UINT8 found = 0;

	if (prvDrawnsList == winningDrawnBallsList) listRows = fileStatRows;
	else listRows = lengthY(prvDrawnsList);

	getKeys(drawnBalls, b1, 0, drawnBalls->size);

	for (i=0; i<listRows; i++)
	{
		aPrvDrawn = getListXByIndex(prvDrawnsList, i);

		b2[0] = b1[0];
		b2[1] = b1[1];
		b2[2] = b1[2];
		b2[3] = '\0';

		if (seqSearchX3(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[0];
		b2[1] = b1[1];
		b2[2] = b1[3];
		b2[3] = '\0';

		if (seqSearchX3(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[0];
		b2[1] = b1[1];
		b2[2] = b1[4];
		b2[3] = '\0';

		if (seqSearchX3(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[0];
		b2[1] = b1[2];
		b2[2] = b1[3];
		b2[3] = '\0';

		if (seqSearchX3(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[0];
		b2[1] = b1[2];
		b2[2] = b1[4];
		b2[3] = '\0';

		if (seqSearchX3(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[0];
		b2[1] = b1[3];
		b2[2] = b1[4];
		b2[3] = '\0';

		if (seqSearchX3(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[1];
		b2[1] = b1[2];
		b2[2] = b1[3];
		b2[3] = '\0';

		if (seqSearchX3(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[1];
		b2[1] = b1[2];
		b2[2] = b1[4];
		b2[3] = '\0';

		if (seqSearchX3(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[1];
		b2[1] = b1[3];
		b2[2] = b1[4];
		b2[3] = '\0';

		if (seqSearchX3(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[2];
		b2[1] = b1[3];
		b2[2] = b1[4];
		b2[3] = '\0';

		if (seqSearchX3(aPrvDrawn, b2) >= 0) {found=1; break;}
	}

	return found;
}



UINT8 search3CombX(struct ListX *aPrvDrawn, struct ListX *drawnBalls, struct ListXY *luckyBalls, char *buf)
{
	struct ListX *lb = NULL;
	int index;
	UINT8 b1[DRAW_BALL+1];
	UINT8 b2[4];
	UINT8 found = 0;
	char fStr[11];

	getKeys(drawnBalls, b1, 0, drawnBalls->size);

	if (buf) buf[0] = '\0';

	b2[0] = b1[0];
	b2[1] = b1[1];
	b2[2] = b1[2];
	b2[3] = '\0';
		
	if (seqSearchX3(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			sprintf(fStr, "(%2d,%2d,%2d)", b1[0], b1[1], b1[2]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY3(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 3, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[0];
	b2[1] = b1[1];
	b2[2] = b1[3];
	b2[3] = '\0';
		
	if (seqSearchX3(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d,%2d)", b1[0], b1[1], b1[3]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY3(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 3, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[0];
	b2[1] = b1[1];
	b2[2] = b1[4];
	b2[3] = '\0';

	if (seqSearchX3(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d,%2d)", b1[0], b1[1], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY3(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 3, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[0];
	b2[1] = b1[2];
	b2[2] = b1[3];
	b2[3] = '\0';
		
	if (seqSearchX3(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d,%2d)", b1[0], b1[2], b1[3]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY3(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 3, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[0];
	b2[1] = b1[2];
	b2[2] = b1[4];
	b2[3] = '\0';

	if (seqSearchX3(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d,%2d)", b1[0], b1[2], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY3(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 3, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[0];
	b2[1] = b1[3];
	b2[2] = b1[4];
	b2[3] = '\0';
		
	if (seqSearchX3(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d,%2d)", b1[0], b1[3], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY3(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 3, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[1];
	b2[1] = b1[2];
	b2[2] = b1[3];
	b2[3] = '\0';

	if (seqSearchX3(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d,%2d)", b1[1], b1[2], b1[3]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY3(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 3, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[1];
	b2[1] = b1[2];
	b2[2] = b1[4];
	b2[3] = '\0';
		
	if (seqSearchX3(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d,%2d)", b1[1], b1[2], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY3(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 3, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[1];
	b2[1] = b1[3];
	b2[2] = b1[4];
	b2[3] = '\0';

	if (seqSearchX3(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d,%2d)", b1[1], b1[3], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY3(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 3, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[2];
	b2[1] = b1[3];
	b2[2] = b1[4];
	b2[3] = '\0';
		
	if (seqSearchX3(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d,%2d)", b1[2], b1[3], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY3(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 3, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	return found;
}



UINT8 search2CombXY(struct ListXY *prvDrawnsList, struct ListX *drawnBalls)
{
	UINT16 i, listRows;
	UINT8 b1[DRAW_BALL+1];
	UINT8 b2[3];
	struct ListX *aPrvDrawn = NULL;
	UINT8 found = 0;

	if (prvDrawnsList == winningDrawnBallsList) listRows = fileStatRows;
	else listRows = lengthY(prvDrawnsList);

	getKeys(drawnBalls, b1, 0, drawnBalls->size);

	for (i=0; i<listRows; i++) 
	{
		aPrvDrawn = getListXByIndex(prvDrawnsList, i);

		b2[0] = b1[0];
		b2[1] = b1[1];
		b2[2] = '\0';

		if (seqSearchX2(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[0];
		b2[1] = b1[2];
		b2[2] = '\0';

		if (seqSearchX2(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[0];
		b2[1] = b1[3];
		b2[2] = '\0';

		if (seqSearchX2(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[0];
		b2[1] = b1[4];
		b2[2] = '\0';

		if (seqSearchX2(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[1];
		b2[1] = b1[2];
		b2[2] = '\0';

		if (seqSearchX2(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[1];
		b2[1] = b1[3];
		b2[2] = '\0';

		if (seqSearchX2(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[1];
		b2[1] = b1[4];
		b2[2] = '\0';

		if (seqSearchX2(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[2];
		b2[1] = b1[3];
		b2[2] = '\0';

		if (seqSearchX2(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[2];
		b2[1] = b1[4];
		b2[2] = '\0';

		if (seqSearchX2(aPrvDrawn, b2) >= 0) {found=1; break;}

		b2[0] = b1[3];
		b2[1] = b1[4];
		b2[2] = '\0';

		if (seqSearchX2(aPrvDrawn, b2) >= 0) {found=1; break;}
	}

	return found;
}



UINT8 search2CombX(struct ListX *aPrvDrawn, struct ListX *drawnBalls, struct ListXY *luckyBalls, char *buf)
{
	struct ListX *lb = NULL;
	int index;
	UINT8 b1[DRAW_BALL+1];
	UINT8 b2[3];
	UINT8 found = 0;
	char fStr[8];

	getKeys(drawnBalls, b1, 0, drawnBalls->size);

	if (buf) buf[0] = '\0';

	b2[0] = b1[0];
	b2[1] = b1[1];
	b2[2] = '\0';
		
	if (seqSearchX2(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			sprintf(fStr, "(%2d,%2d)", b1[0], b1[1]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY2(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 2, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[0];
	b2[1] = b1[2];
	b2[2] = '\0';
		
	if (seqSearchX2(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d)", b1[0], b1[2]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY2(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 2, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[0];
	b2[1] = b1[3];
	b2[2] = '\0';
		
	if (seqSearchX2(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d)", b1[0], b1[3]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY2(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 2, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[0];
	b2[1] = b1[4];
	b2[2] = '\0';

	if (seqSearchX2(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d)", b1[0], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY2(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 2, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[1];
	b2[1] = b1[2];
	b2[2] = '\0';
		
	if (seqSearchX2(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d)", b1[1], b1[2]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY2(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 2, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[1];
	b2[1] = b1[3];
	b2[2] = '\0';
		
	if (seqSearchX2(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d)", b1[1], b1[3]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY2(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 2, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[1];
	b2[1] = b1[4];
	b2[2] = '\0';
		
	if (seqSearchX2(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d)", b1[1], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY2(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 2, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[2];
	b2[1] = b1[3];
	b2[2] = '\0';

	if (seqSearchX2(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d)", b1[2], b1[3]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY2(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 2, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[2];
	b2[1] = b1[4];
	b2[2] = '\0';
		
	if (seqSearchX2(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d)", b1[2], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY2(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 2, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
	}

	b2[0] = b1[3];
	b2[1] = b1[4];
	b2[2] = '\0';
		
	if (seqSearchX2(aPrvDrawn, b2) >= 0)
	{
		if (buf) {
			if (found) strcat(buf, ", ");
			sprintf(fStr, "(%2d,%2d)", b1[3], b1[4]);
			strcat(buf, fStr);
		}
		if (luckyBalls) {
			if ((index = seqSearchXY2(luckyBalls, b2)) < 0) {
				lb = createListX(lb, 2, NULL, 1, 0, 0, 0);
				appendItems(lb, b2);
				appendList(luckyBalls, lb);
			} else {
				incValXY(luckyBalls, index);
			}
		}
		found=1;
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

	for (i=0; i<lengthY(couponList); i++) 
	{
		aPrvDrawn = getListXByIndex(couponList, i);

		for (j=0; j<drawBallCount; j++) {
			if (seqSearchX1(aPrvDrawn, balls[j]) >= 0) {found=1; break;}
		}
		if (found) break;
	}

	return found;
}



UINT8 init()
{
	struct ListX *tmp = NULL;
	int err;
	char realPath[PATH_MAX];
	cwd = (char *) malloc(sizeof(char)*PATH_MAX);
	fileStats = (char *) malloc(sizeof(char)*PATH_MAX);
	outputFile = (char *) malloc(sizeof(char)*PATH_MAX);

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
	powerBallDrawnBallsList = createListXY(powerBallDrawnBallsList);

	if (!(fileStatRows = getDrawnBallsList(winningDrawnBallsList, fileStats))) {
		printf("%s file or record not found!\n", fileStats);
		return 0;
	}

	#ifndef __MSDOS__
	if (!(fileStatRows = getDrawnBallsList(powerBallDrawnBallsList, fileStats))) {
		printf("%s file or record not found!\n", fileStats);
		return 0;
	}
	#endif

	tmp = getListXByIndex(winningDrawnBallsList, fileStatRows-1);
	formatDate(dateStart, tmp->day, tmp->mon, tmp->year);
	tmp = winningDrawnBallsList->list;
	formatDate(dateEnd, tmp->day, tmp->mon, tmp->year);

	luckyBalls2 = createListXY(luckyBalls2);
	luckyBalls2 = getLuckyBalls(luckyBalls2, 2);
	bubbleSortYByVal(luckyBalls2, -1);

	#if defined(__MSDOS__)
	removeAllXY(winningDrawnBallsList);
	if (!saveLuckyBallsToFile(luckyBalls2, 2)) return 0;
	removeAllXY(luckyBalls2);
	if (!(fileStatRows = getDrawnBallsList(winningDrawnBallsList, fileStats))) {
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
	if (!(fileStatRows = getDrawnBallsList(winningDrawnBallsList, fileStats))) {
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
	if (!(fileStatRows = getDrawnBallsList(winningDrawnBallsList, fileStats))) {
		printf("%s file or record not found!\n", fileStats);
		return 0;
	}
	if (!(fileStatRows = getDrawnBallsList(powerBallDrawnBallsList, fileStats))) {
		printf("%s file or record not found!\n", fileStats);
		return 0;
	}
	#endif

	winningBallStats = createListX2(winningBallStats);
	powerBallStats = createListX2(powerBallStats);
	getDrawnBallsStats();

	removeAllXY(powerBallDrawnBallsList);

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



struct ListXY * getLuckyBalls(struct ListXY *luckyBalls, UINT8 comb)
{
	UINT16 i, j;
	struct ListX *aPrvDrawn1 = NULL;
	struct ListX *aPrvDrawn2 = NULL;
	struct ListX *list = NULL;

	if (comb == 2 || comb == 3 || comb == 4)
	{
		for (i=0; i<fileStatRows; i++)
		{
			aPrvDrawn1 = getListXByIndex(winningDrawnBallsList, i);

			for (j=i+1; j<fileStatRows; j++)
			{
				aPrvDrawn2 = getListXByIndex(winningDrawnBallsList, j);

				if (comb == 2) {
					search2CombX(aPrvDrawn2, aPrvDrawn1, luckyBalls, NULL);
				}
				else if (comb == 3) {
					search3CombX(aPrvDrawn2, aPrvDrawn1, luckyBalls, NULL);
				}
				else if (comb == 4) {
					search4CombX(aPrvDrawn2, aPrvDrawn1, luckyBalls, NULL);
				}
			}
		}
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

	if (comb == 2) {
		strcpy(lbFile, LBL2SWAPFILE);
	}
	else if (comb == 3) {
		strcpy(lbFile, LBL3SWAPFILE);
	}
	else if (comb == 4) {
		strcpy(lbFile, LBL4SWAPFILE);
	}


	if ((fp = fopen(lbFile, "w")) == NULL) {
		printf("Can't open file %s\n", lbFile);
		pressAnyKey();
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
	UINT8 keys[DRAW_BALL+1];
	char lbFile[14];
	char ioBuf[20];
	char s[6];

	FILE *fp;

	struct ListX *drawList = NULL;

	if (comb == 2) {
		strcpy(lbFile, LBL2SWAPFILE);
	}
	else if (comb == 3) {
		strcpy(lbFile, LBL3SWAPFILE);
	}
	else if (comb == 4) {
		strcpy(lbFile, LBL4SWAPFILE);
	}

	if ((fp = fopen(lbFile, "r")) == NULL) {
		printf("Can't open file %s\n", lbFile);
		pressAnyKey();
		return NULL;
	}

	for (i=0; fgets(ioBuf, 20, fp) && i<UINT16MAX; i++)
	{
		drawList = createListX(drawList, comb, NULL, 0, 0, 0, 0);

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
			
			keys[0] = (UINT8) n1;
			keys[1] = (UINT8) n2;
			keys[2] = '\0';
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

			keys[0] = (UINT8) n1;
			keys[1] = (UINT8) n2;
			keys[2] = (UINT8) n3;
			keys[3] = '\0';
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

			keys[0] = (UINT8) n1;
			keys[1] = (UINT8) n2;
			keys[2] = (UINT8) n3;
			keys[3] = (UINT8) n4;
			keys[4] = '\0';
		}

		drawList->val = (UINT16) val;
		appendItems(drawList, keys);
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

	for (i=0; i<fileStatRows; i++)
	{
		aPrvDrawn1 = getListXByIndex(winningDrawnBallsList, i);

		for (j=i+1; j<fileStatRows; j++) 
		{
			aPrvDrawn2 = getListXByIndex(winningDrawnBallsList, j);

			match2comb += search2CombX(aPrvDrawn2, aPrvDrawn1, NULL, NULL);
			match3comb += search3CombX(aPrvDrawn2, aPrvDrawn1, NULL, NULL);
			match4comb += search4CombX(aPrvDrawn2, aPrvDrawn1, NULL, NULL);
			match5comb += search5CombX(aPrvDrawn2, aPrvDrawn1);
		}
	}
}



void calcCombMatch(UINT8 comb, FILE *fp)
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
	char lbBuf[120];
	char pLabel[10];
	char date1[11], date2[11];
	char lastDate[11];

	strcpy(pLabel, "Progress");

	if (comb == 2 || comb == 3 || comb == 4 || comb == 5) 
	{
		puts("Matched combinations of numbers from previous draws:\n");
		if (fp != NULL) fprintf(fp, "Matched combinations of numbers from previous draws:\n\n");

		if (comb == 2) {
			printf("Matched 2 combinations: %lu\n\n", match2comb);
			if (fp != NULL) fprintf(fp, "Matched 2 combinations: %lu\n\n", match2comb);
    		matchComb = match2comb;
		}
		else if (comb == 3) {
			printf("Matched 3 combinations: %lu\n\n", match3comb);
			if (fp != NULL) fprintf(fp, "Matched 3 combinations: %lu\n\n", match3comb);
    		matchComb = match3comb;
		}
		else if (comb == 4) {
			printf("Matched 4 combinations: %lu\n\n", match4comb);
			if (fp != NULL) fprintf(fp, "Matched 4 combinations: %lu\n\n", match4comb);
    		matchComb = match4comb;
		}
		else if (comb == 5) {
			printf("Matched 5 combinations: %lu\n\n", match5comb);
			if (fp != NULL) fprintf(fp, "Matched 5 combinations: %lu\n\n", match5comb);
    		matchComb = match5comb;
		}

		for (i=0, k=0, x=0; x<matchComb && i<fileStatRows; i++) 
		{
			if (comb == 2 || comb == 3 || comb == 4) {
					 printPercentOfProgress(pLabel, k, (UINT32) ceil((UINT32) fileStatRows*((UINT32) fileStatRows-1)/2));
			}

			aPrvDrawn1 = getListXByIndex(winningDrawnBallsList, i);

			for (j=i+1; j<fileStatRows; j++, k++) 
			{
				aPrvDrawn2 = getListXByIndex(winningDrawnBallsList, j);

				if (comb == 2) {
					found = search2CombX(aPrvDrawn2, aPrvDrawn1, NULL, lbBuf);
				}
				else if (comb == 3) {
					found = search3CombX(aPrvDrawn2, aPrvDrawn1, NULL, lbBuf);
				}
				else if (comb == 4) {
					found = search4CombX(aPrvDrawn2, aPrvDrawn1, NULL, lbBuf);
				}
				else if (comb == 5) {
					found = search5CombX(aPrvDrawn2, aPrvDrawn1);
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
			}
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
	if (drawNum == 1) {
		luckyBalls3 = getLuckyBallsFromFile(luckyBalls3, 3);
	}
	else if (drawNum == 2) {
		luckyBalls3 = getLuckyBallsFromFile(luckyBalls3, 3);
	}
	else {
		luckyBalls2 = getLuckyBallsFromFile(luckyBalls2, 2);
	}
	#endif

	if (drawNum == 1) {
		  luckyBalls = luckyBalls3;
	}
	else if (drawNum == 2) {
        luckyBalls = luckyBalls3;
	}
	else {
        luckyBalls = luckyBalls2;
	}

	lbsLen = lengthY(luckyBalls);

	bubbleSortYByVal(luckyBalls, -1);

	luckyRow2 = getListXByIndex(luckyBalls, lbsLen-1);
	lucky2MinVal = luckyRow2->val;
	luckyRow2 = getListXByIndex(luckyBalls, 0);
	lucky2MaxVal = luckyRow2->val;

	for (index=0; index < (UINT16) ceil(2*lbsLen/3) && getValXY(luckyBalls, index) > (UINT16) ceil((lucky2MinVal+lucky2MaxVal)/2); index++);

	if (index < 10) index = lbsLen;

	if (drawNum == 1) x = 3;
	else x = 2;

	if (drawNum == 1) 
	{
		luckyBalls = shuffleListXY(luckyBalls);
		luckyRow3 = getListXByIndex(luckyBalls, (UINT16) rand() % lengthY(luckyBalls));
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

		ball2 = getKey(luckyRow2, ind2);
		appendItem(drawnBallsLucky, ball2);
	}

	bubbleSortYByVal(luckyBalls, -1);

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
	UINT8 i, j;
	UINT8 found = 0;
	UINT8 matchComb = 0;
	UINT8 elimComb = 0;
	UINT8 autoCalc = 1;
	UINT8 noMatch, noElim;
	char label[22];
	char pLabel[11];
	UINT8 drawCountDown = drawRowCount;
	UINT8 luckyNum = 0;

	struct ListX *drawnBalls = NULL;
	struct ListX2 *ballStats = NULL;

	UINT8 numOfAttempts = (UINT8) ceil(5*totalBall/drawBallCount);

	if (drawBallCount == 1) {
		ballStats = powerBallStats;
		strcpy(pLabel, "PowerBall");
	}
	else {
		ballStats = winningBallStats;
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
				drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0);

				if (autoCalc && drawBallCount > 1) {
					matchComb = 0;
					elimComb = 2;
				}

				for (i=0; i < numOfAttempts; i++)
				{
					drawnBalls = drawBallByNorm(drawnBalls, ballStats, totalBall, drawBallCount, matchComb, elimComb);

					if (matchComb) noMatch = !findComb(drawnBalls, matchComb);
					else noMatch = 0;

					if (elimComb) noElim = findComb(drawnBalls, elimComb);
					else noElim = 0;

					if (!(noMatch || noElim)) {
						if (i < ceil(numOfAttempts/4)) {
							found = search1BallXY(coupon, drawnBalls, drawBallCount);
						} else if (i < ceil(numOfAttempts/2)) {
							if (drawBallCount == 1) break;
							found = search2CombXY(coupon, drawnBalls);
						} else if (i < ceil(3*numOfAttempts/4)) {
							found = search3CombXY(coupon, drawnBalls);
						} else {
							found = search4CombXY(coupon, drawnBalls);
						}

						if(!found) break;
					}

					if (autoCalc && drawBallCount > 1) {
						if (i < ceil(numOfAttempts/4)) {matchComb = 0; elimComb = 2;}
						else if (i < ceil(numOfAttempts/2)) {matchComb = 0; elimComb = 3;}
						else if (i < ceil(3*numOfAttempts/4)) {matchComb = 3; elimComb = 4;}
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
				drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0);

				if (autoCalc && drawBallCount > 1) {
					matchComb = 0;
					elimComb = 2;
				}

				for (i=0; i < numOfAttempts; i++)
				{
					drawnBalls = drawBallByLeft(drawnBalls, ballStats, totalBall, drawBallCount, matchComb, elimComb);

					if (matchComb) noMatch = !findComb(drawnBalls, matchComb);
					else noMatch = 0;

					if (elimComb) noElim = findComb(drawnBalls, elimComb);
					else noElim = 0;

					if (!(noMatch || noElim)) {
						if (i < ceil(numOfAttempts/4)) {
							found = search1BallXY(coupon, drawnBalls, drawBallCount);
						} else if (i < ceil(numOfAttempts/2)) {
							if (drawBallCount == 1) break;
							found = search2CombXY(coupon, drawnBalls);
						} else if (i < ceil(3*numOfAttempts/4)) {
							found = search3CombXY(coupon, drawnBalls);
						} else {
							found = search4CombXY(coupon, drawnBalls);
						}

						if(!found) break;
					}

					if (autoCalc && drawBallCount > 1) {
						if (i < ceil(numOfAttempts/4)) {matchComb = 0; elimComb = 2;}
						else if (i < ceil(numOfAttempts/2)) {matchComb = 0; elimComb = 3;}
						else if (i < ceil(3*numOfAttempts/4)) {matchComb = 3; elimComb = 4;}
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
				drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0);

				if (autoCalc && drawBallCount > 1) {
					matchComb = 0;
					elimComb = 2;
				}

				for (i=0; i < numOfAttempts; i++)
				{
					drawnBalls = drawBallByBlend1(drawnBalls, ballStats, totalBall, drawBallCount, matchComb, elimComb);

					if (matchComb) noMatch = !findComb(drawnBalls, matchComb);
					else noMatch = 0;

					if (elimComb) noElim = findComb(drawnBalls, elimComb);
					else noElim = 0;

					if (!(noMatch || noElim)) {
						if (i < ceil(numOfAttempts/4)) {
							found = search1BallXY(coupon, drawnBalls, drawBallCount);
						} else if (i < ceil(numOfAttempts/2)) {
							if (drawBallCount == 1) break;
							found = search2CombXY(coupon, drawnBalls);
						} else if (i < ceil(3*numOfAttempts/4)) {
							found = search3CombXY(coupon, drawnBalls);
						} else {
							found = search4CombXY(coupon, drawnBalls);
						}

						if(!found) break;
					}

					if (autoCalc && drawBallCount > 1) {
						if (i < ceil(numOfAttempts/4)) {matchComb = 0; elimComb = 2;}
						else if (i < ceil(numOfAttempts/2)) {matchComb = 0; elimComb = 3;}
						else if (i < ceil(3*numOfAttempts/4)) {matchComb = 3; elimComb = 4;}
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
				drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0);

				if (autoCalc && drawBallCount > 1) {
					matchComb = 0;
					elimComb = 2;
				}

				for (i=0; i < numOfAttempts; i++)
				{
					drawnBalls = drawBallByBlend2(drawnBalls, ballStats, totalBall, drawBallCount, matchComb, elimComb);

					if (matchComb) noMatch = !findComb(drawnBalls, matchComb);
					else noMatch = 0;

					if (elimComb) noElim = findComb(drawnBalls, elimComb);
					else noElim = 0;

					if (!(noMatch || noElim)) {
						if (i < ceil(numOfAttempts/4)) {
							found = search1BallXY(coupon, drawnBalls, drawBallCount);
						} else if (i < ceil(numOfAttempts/2)) {
							if (drawBallCount == 1) break;
							found = search2CombXY(coupon, drawnBalls);
						} else if (i < ceil(3*numOfAttempts/4)) {
							found = search3CombXY(coupon, drawnBalls);
						} else {
							found = search4CombXY(coupon, drawnBalls);
						}

						if(!found) break;
					}

					if (autoCalc && drawBallCount > 1) {
						if (i < ceil(numOfAttempts/4)) {matchComb = 0; elimComb = 2;}
						else if (i < ceil(numOfAttempts/2)) {matchComb = 0; elimComb = 3;}
						else if (i < ceil(3*numOfAttempts/4)) {matchComb = 3; elimComb = 4;}
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
			drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0);

			if (autoCalc && drawBallCount > 1) {
				matchComb = 0;
				elimComb = 2;
			}

			for (i=0; i < numOfAttempts; i++)
			{
				drawnBalls = drawBallBySide(drawnBalls, ballStats, totalBall, drawBallCount, matchComb, elimComb);

				if (matchComb) noMatch = !findComb(drawnBalls, matchComb);
				else noMatch = 0;

				if (elimComb) noElim = findComb(drawnBalls, elimComb);
				else noElim = 0;

				if (!(noMatch || noElim)) {
					if (i < ceil(numOfAttempts/4)) {
						found = search1BallXY(coupon, drawnBalls, drawBallCount);
					} else if (i < ceil(numOfAttempts/2)) {
						if (drawBallCount == 1) break;
						found = search2CombXY(coupon, drawnBalls);
					} else if (i < ceil(3*numOfAttempts/4)) {
						found = search3CombXY(coupon, drawnBalls);
					} else {
						found = search4CombXY(coupon, drawnBalls);
					}

					if(!found) break;
				}

				if (autoCalc && drawBallCount > 1) {
					if (i < ceil(numOfAttempts/4)) {matchComb = 0; elimComb = 2;}
					else if (i < ceil(numOfAttempts/2)) {matchComb = 0; elimComb = 3;}
					else if (i < ceil(3*numOfAttempts/4)) {matchComb = 3; elimComb = 4;}
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
			drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0);

			if (autoCalc && drawBallCount > 1) {
				matchComb = 0;
				elimComb = 2;
			}

			for (i=0; i < numOfAttempts; i++)
			{
				drawnBalls = drawBallByRand(drawnBalls, ballStats, totalBall, drawBallCount, matchComb, elimComb);

				if (matchComb) noMatch = !findComb(drawnBalls, matchComb);
				else noMatch = 0;

				if (elimComb) noElim = findComb(drawnBalls, elimComb);
				else noElim = 0;

				if (!(noMatch || noElim)) {
					if (i < ceil(numOfAttempts/4)) {
						found = search1BallXY(coupon, drawnBalls, drawBallCount);
					} else if (i < ceil(numOfAttempts/2)) {
						if (drawBallCount == 1) break;
						found = search2CombXY(coupon, drawnBalls);
					} else if (i < ceil(3*numOfAttempts/4)) {
						found = search3CombXY(coupon, drawnBalls);
					} else {
						found = search4CombXY(coupon, drawnBalls);
					}

					if(!found) break;
				}

				if (autoCalc && drawBallCount > 1) {
					if (i < ceil(numOfAttempts/4)) {matchComb = 0; elimComb = 2;}
					else if (i < ceil(numOfAttempts/2)) {matchComb = 0; elimComb = 3;}
					else if (i < ceil(3*numOfAttempts/4)) {matchComb = 3; elimComb = 4;}
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
			if (drawBallCount > 1) // winning numbers
			{
				luckyNum = (j%3)+1;

				if (luckyNum == 1) strcpy(label, "(lucky 3)");
				else if (luckyNum == 2) strcpy(label, "(2 of lucky 3)");
				else strcpy(label, "(lucky 2)");

				drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0);
    	        drawnBalls = drawBallsByLucky(drawnBalls, luckyNum, totalBall, drawBallCount);
			}
			else // powerBall
			{
				strcpy(label, "power ball");
				drawnBalls = createListX(drawnBalls, drawBallCount, label, 0, 0, 0, 0);

				for (i=0; i < numOfAttempts; i++) 
				{
					if (i < ceil(numOfAttempts/4)) {
    					if (j%2 == 0) drawnBalls = drawBallByNorm(drawnBalls, ballStats, totalBall, drawBallCount, 0, 0);
						else drawnBalls = drawBallByLeft(drawnBalls, ballStats, totalBall, drawBallCount, 0, 0);
					} else if (i < ceil(numOfAttempts/2)) {
    					if (j%2 == 0) drawnBalls = drawBallByBlend1(drawnBalls, ballStats, totalBall, drawBallCount, 0, 0);
						else drawnBalls = drawBallByBlend2(drawnBalls, ballStats, totalBall, drawBallCount, 0, 0);
					} else if (i < ceil(3*numOfAttempts/4)) {
    					drawnBalls = drawBallBySide(drawnBalls, ballStats, totalBall, drawBallCount, 0, 0);
					} else {
    					drawnBalls = drawBallByRand(drawnBalls, ballStats, totalBall, drawBallCount, 0, 0);
					}

					if (lengthY(coupon) > ceil(1.25*totalBall/drawBallCount)) break;
    				found = search1BallXY(coupon, drawnBalls, drawBallCount);
					if(!found) break;
				}
			}

			appendList(coupon, drawnBalls);
			drawCountDown--;

			printPercentOfProgress(pLabel, (UINT32) (drawRowCount-drawCountDown), (UINT32) drawRowCount);
		}
	}
}



UINT16 getDrawnBallsList(struct ListXY *ballList, char *fileName)
{
	UINT16 i=0;
	int d1, m1, y1, pb;
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
		sscanf(ioBuf, "%d/%d/%d\t%d\t%d\t%d\t%d\t%d\t%d\n", &m1, &d1, &y1, &n1, &n2, &n3, &n4, &n5, &pb);

		if (ballList == winningDrawnBallsList) {
			keys[0] = (UINT8) n1;
			keys[1] = (UINT8) n2;
			keys[2] = (UINT8) n3;
			keys[3] = (UINT8) n4;
			keys[4] = (UINT8) n5;
			keys[5] = '\0';
			size = DRAW_BALL;
		} else if (ballList == powerBallDrawnBallsList) {
			keys[0] = (UINT8) pb;
			keys[1] = '\0';
			size = 1;
		}
		drawList = createListX(drawList, size, NULL, 0, (UINT16) y1, (UINT8) m1, (UINT8) d1);

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
	char s1[3], s2[3], s3[5];
	char tmp[2];

	sprintf(s1, "%d", (int) mon);
	sprintf(s2, "%d", (int) day);
	sprintf(s3, "%d", (int) year);

	if (strlen(s1)==1) {
		tmp[0] = s1[0];
		s1[0] = '0';
		s1[1] = tmp[0];
		s1[2] = '\0';
	}
	if (strlen(s2)==1) {
		tmp[0] = s2[0];
		s2[0] = '0';
		s2[1] = tmp[0];
		s2[2] = '\0';
	}

	sprintf(date, "%s/%s/%s", s1, s2, s3);
}



struct ListX * drawBallByRand(struct ListX *drawnBallsRand, struct ListX2 *ballStats, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb)
{
	UINT16 i, j, k;
	UINT8 x, y, z;
	UINT8 index;
	UINT8 flyball, drawball;
	UINT16 shuffleGlobe;
	UINT8 noMatch, noElim;

	struct ListX *globe = NULL;

	globe = createListX(globe, totalBall, NULL, 0, 0, 0, 0);

	bubbleSortX2ByVal(ballStats, 1);

	/* Fill inside the globe with balls */
	if (rand() % 2) // (blend1)
	{
		for (x=0, j=totalBall-1, k=0; k<totalBall; k++)
		{
			if (k%2) {
				appendItem(globe, getKey2(ballStats, j));
				j--;
			}
			else {
				insertItem(globe, getKey2(ballStats, x));
				x++;
			}
		}
	}
	else // (blend2)
	{
		for (x=0, j=totalBall-1, k=0; k<totalBall; k++)
		{
			if (k%2) {
				addItemByIndex(globe, totalBall-j, getKey2(ballStats, j));
				j--;
			}
			else {
				addItemByIndex(globe, x, getKey2(ballStats, x));
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

				flyball = removeItemByIndex(globe, index);

				if (index < (UINT8) ceil(totalBall/3)) {
					insertItem(globe, flyball);
				}
				else if (index < (UINT8) ceil(totalBall/2)) {
					addItemByIndex(globe, (UINT8) ceil(2*totalBall/3), flyball);
				}
				else if (index < (UINT8) ceil(2*totalBall/3)) {
					addItemByIndex(globe, (UINT8) ceil(totalBall/3), flyball);
				}
				else {
					appendItem(globe, flyball);
				}
			}

			/* draw a ball */
			drawball = removeItemByIndex(globe, (UINT8) ceil((totalBall-j)/2));
			appendItem(drawnBallsRand, drawball);
		}

		for (x=0, y=drawBallCount-1, z=0; z<drawBallCount; z++)
		{
			if (z%2) {
				appendItem(globe, getKey(drawnBallsRand, y));
				y--;
			}
			else {
				insertItem(globe, getKey(drawnBallsRand, x));
				x++;
			}
		}

		noMatch = 0;
		if (matchComb) {
			if (!findComb(drawnBallsRand, matchComb)) noMatch = 1;
		}

		noElim = 0;
		if (elimComb) {
			if (findComb(drawnBallsRand, elimComb)) noElim = 1;
		}

		if (noMatch || noElim) {
		} else break;
	}

	bubbleSortXByKey(drawnBallsRand);
	removeAllX(globe);
	free(globe);

	return drawnBallsRand;
}



struct ListX * drawBallByLeft(struct ListX *drawnBallsLeft, struct ListX2 *ballStats, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb)
{
	UINT8 i, k;
	UINT8 noMatch, noElim;
	UINT8 drawball;
	struct ListX *globe = NULL;

	globe = createListX(globe, totalBall, NULL, 0, 0, 0, 0);

	bubbleSortX2ByVal(ballStats, 1);

	for (i=0; i<totalBall; i++)	{
		appendItem(globe, getKey2(ballStats, i));
	}

	for (k=0; k<totalBall; k++)
	{
		removeAllX(drawnBallsLeft);

		for (i=0; i<drawBallCount; i++)
		{
			do {
				drawball = getKey(globe, gaussIndex(totalBall)-1);
			} while(seqSearchX1(drawnBallsLeft, drawball) >= 0);
			appendItem(drawnBallsLeft, drawball);
		}

		noMatch = 0;
		if (matchComb) {
			if (!findComb(drawnBallsLeft, matchComb)) noMatch = 1;
		}

		noElim = 0;
		if (elimComb) {
			if (findComb(drawnBallsLeft, elimComb)) noElim = 1;
		}

		if (noMatch || noElim) {
		} else break;
	}

	bubbleSortXByKey(drawnBallsLeft);
	removeAllX(globe);
	free(globe);

	return drawnBallsLeft;
}



struct ListX * drawBallByBlend1(struct ListX *drawnBallsBlend1, struct ListX2 *ballStats, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb)
{
	UINT8 i, j, k;
	UINT8 noMatch, noElim;
	UINT8 drawball;
	struct ListX *globe = NULL;

	globe = createListX(globe, totalBall, NULL, 0, 0, 0, 0);

	bubbleSortX2ByVal(ballStats, 1);

	for (i=0, j=totalBall-1, k=0; k<totalBall; k++)
	{
		if (k%2) {
			appendItem(globe, getKey2(ballStats, j));
			j--;
		}
		else {
			insertItem(globe, getKey2(ballStats, i));
			i++;
		}
	}

	for (k=0; k<totalBall; k++)
	{
		removeAllX(drawnBallsBlend1);

		for (i=0; i<drawBallCount; i++)
		{
			do {
				drawball = getKey(globe, gaussIndex(totalBall)-1);
			} while(seqSearchX1(drawnBallsBlend1, drawball) >= 0);
			appendItem(drawnBallsBlend1, drawball);
		}

		noMatch = 0;
		if (matchComb) {
			if (!findComb(drawnBallsBlend1, matchComb)) noMatch = 1;
		}

		noElim = 0;
		if (elimComb) {
			if (findComb(drawnBallsBlend1, elimComb)) noElim = 1;
		}

		if (noMatch || noElim) {
		} else break;
	}

	bubbleSortXByKey(drawnBallsBlend1);
	removeAllX(globe);
	free(globe);

	return drawnBallsBlend1;
}



struct ListX * drawBallByBlend2(struct ListX *drawnBallsBlend2, struct ListX2 *ballStats, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb)
{
	UINT8 i, j, k;
	UINT8 noMatch, noElim;
	UINT8 drawball;
	struct ListX *globe = NULL;

	globe = createListX(globe, totalBall, NULL, 0, 0, 0, 0);

	bubbleSortX2ByVal(ballStats, 1);

	for (i=0, j=totalBall-1, k=0; k<totalBall; k++)
	{
		if (k%2) {
			addItemByIndex(globe, totalBall-j, getKey2(ballStats, j));
			j--;
		}
		else {
			addItemByIndex(globe, i, getKey2(ballStats, i));
			i++;
		}
	}

	for (k=0; k<totalBall; k++)
	{
		removeAllX(drawnBallsBlend2);

		for (i=0; i<drawBallCount; i++)
		{
			do {
				drawball = getKey(globe, gaussIndex(totalBall)-1);
			} while(seqSearchX1(drawnBallsBlend2, drawball) >= 0);
			appendItem(drawnBallsBlend2, drawball);
		}

		noMatch = 0;
		if (matchComb) {
			if (!findComb(drawnBallsBlend2, matchComb)) noMatch = 1;
		}

		noElim = 0;
		if (elimComb) {
			if (findComb(drawnBallsBlend2, elimComb)) noElim = 1;
		}

		if (noMatch || noElim) {
		} else break;
	}

	bubbleSortXByKey(drawnBallsBlend2);
	removeAllX(globe);
	free(globe);

	return drawnBallsBlend2;
}



struct ListX * drawBallBySide(struct ListX *drawnBallsSide, struct ListX2 *ballStats, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb)
{
	UINT8 i, k;
	UINT8 noMatch, noElim;
	UINT8 drawball;
	struct ListX *globe = NULL;

	globe = createListX(globe, totalBall, NULL, 0, 0, 0, 0);

	bubbleSortX2ByVal(ballStats, 1);

	for (k=0; k<totalBall; k++)
	{
		if (k%2) {
			insertItem(globe, getKey2(ballStats, k));
		}
		else {
			appendItem(globe, getKey2(ballStats, k));
		}
	}

	for (k=0; k<totalBall; k++)
	{
		removeAllX(drawnBallsSide);

		for (i=0; i<drawBallCount; i++)
		{
			do {
				drawball = getKey(globe, gaussIndex(totalBall)-1);
			} while(seqSearchX1(drawnBallsSide, drawball) >= 0);
			appendItem(drawnBallsSide, drawball);
		}

		noMatch = 0;
		if (matchComb) {
			if (!findComb(drawnBallsSide, matchComb)) noMatch = 1;
		}

		noElim = 0;
		if (elimComb) {
			if (findComb(drawnBallsSide, elimComb)) noElim = 1;
		}

		if (noMatch || noElim) {
		} else break;
	}

	bubbleSortXByKey(drawnBallsSide);
	removeAllX(globe);
	free(globe);

	return drawnBallsSide;
}



struct ListX * drawBallByNorm(struct ListX *drawnBallsNorm, struct ListX2 *ballStats, UINT8 totalBall, UINT8 drawBallCount, UINT8 matchComb, UINT8 elimComb)
{
	UINT8 i, k;
	UINT8 noMatch, noElim;
	UINT8 drawball;
	struct ListX *globe = NULL;

	globe = createListX(globe, totalBall, NULL, 0, 0, 0, 0);

	bubbleSortX2ByVal(ballStats, -1);

	for (k=0; k<totalBall; k++)
	{
		if (k%2) {
			insertItem(globe, getKey2(ballStats, k));
		}
		else {
			appendItem(globe, getKey2(ballStats, k));
		}
	}

	for (k=0; k<totalBall; k++)
	{
		removeAllX(drawnBallsNorm);

		for (i=0; i<drawBallCount; i++)
		{
			do {
				drawball = getKey(globe, gaussIndex(totalBall)-1);
			} while(seqSearchX1(drawnBallsNorm, drawball) >= 0);
			appendItem(drawnBallsNorm, drawball);
		}

		noMatch = 0;
		if (matchComb) {
			if (!findComb(drawnBallsNorm, matchComb)) noMatch = 1;
		}

		noElim = 0;
		if (elimComb) {
			if (findComb(drawnBallsNorm, elimComb)) noElim = 1;
		}

		if (noMatch || noElim) {
		} else break;
	}

	bubbleSortXByKey(drawnBallsNorm);
	removeAllX(globe);
	free(globe);

	return drawnBallsNorm;
}



void getDrawnBallsStats()
{
	UINT8 i;
	UINT16 j;
	UINT8 n1, n2, n3, n4, n5, pb;
	UINT8 keys[DRAW_BALL+1];
	int index;

	struct ListX *aPrvDrawn = NULL;

	for (i=0; i<TOTAL_BALL; i++) {
		appendItem2(winningBallStats, i+1, 0);
	}

	for (j=0; j<fileStatRows; j++)
	{
		aPrvDrawn = getListXByIndex(winningDrawnBallsList, j);

		getKeys(aPrvDrawn, keys, 0, DRAW_BALL);

		n1 = keys[0];
		n2 = keys[1];
		n3 = keys[2];
		n4 = keys[3];
		n5 = keys[4];

		if ((index = seqSearch2X1(winningBallStats,n1)) != -1) incVal2(winningBallStats, index);
		if ((index = seqSearch2X1(winningBallStats,n2)) != -1) incVal2(winningBallStats, index);
		if ((index = seqSearch2X1(winningBallStats,n3)) != -1) incVal2(winningBallStats, index);
		if ((index = seqSearch2X1(winningBallStats,n4)) != -1) incVal2(winningBallStats, index);
		if ((index = seqSearch2X1(winningBallStats,n5)) != -1) incVal2(winningBallStats, index);
	}

	for (i=0; i<TOTAL_BALL_PB; i++) {
		appendItem2(powerBallStats, i+1, 0);
	}

	for (j=0; j<fileStatRows; j++) 
	{
		aPrvDrawn = getListXByIndex(powerBallDrawnBallsList, j);

		pb = getKey(aPrvDrawn, 0);

		if ((index = seqSearch2X1(powerBallStats,pb)) != -1) incVal2(powerBallStats, index);
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



void pressAnyKey()
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



int main(void)
{
	UINT16 keyb = 0, keyb2;
	struct ListXY *coupon = NULL;
	struct ListXY *coupon_pb = NULL;
	FILE *fp;

	srand((unsigned) time(NULL));

	if (!init()) {
		pressAnyKey();
		return -1;
	}

	coupon = createListXY(coupon);
	coupon_pb = createListXY(coupon_pb);

mainMenu:

	bubbleSortX2ByVal(winningBallStats, -1);
	bubbleSortX2ByVal(powerBallStats, -1);

	printf("PowerBall Lotto 1.0 Copyright ibrahim Tipirdamaz (c) 2022\n");
	#ifndef __MSDOS__
	printf("\n");
	#endif
	printf("Includes draws between dates %s - %s\n", dateStart, dateEnd);
	printf("If the %s file is out of date, update it.\n\n", FILESTATS);
	puts("Which number drawn how many times?");
	#ifndef __MSDOS__
	printf("\n");
	#endif
	printBallStats(winningBallStats);
	printf("\n\n");

	puts("Number of draws of PowerBalls");
	#ifndef __MSDOS__
	printf("\n");
	#endif
	printBallStats(powerBallStats);
	printf("\n\n");

	printf("1- Draw Ball\n");
	printf("2- Matched 2 combinations: %lu\n", match2comb);
	printf("3- Matched 3 combinations: %lu\n", match3comb);
	printf("4- Matched 4 combinations: %lu\n", match4comb);
	printf("5- Matched 5 combinations: %lu", match5comb);
	#ifdef __MSDOS__
	gotoxy(41, wherey()-3);
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
	printf("9- Exit\n");
	printf("\nPlease make your selection: ");

keybCommand:

	do {
		scanf("%d", &keyb);
	} while(!(keyb >= 0 && keyb < 10));

	clearScreen();

	if (keyb == 0) {
		goto mainMenu;
	}
	else if (keyb == 9) {
		goto exitProgram;
	}

	if ((fp = fopen(outputFile, "w")) == NULL) {
		printf("Can't open file %s\n", OUTPUTFILE);
		pressAnyKey();
		return -1;
	}

	if (keyb == 1)
	{
		printf("\nInput draw count (between 1-50) : ");

		do {
			scanf("%d", &keyb2);
		} while(!(keyb2 > 0 && keyb2 < 51));

		puts("");

		/* coupon, totalBall, drawBallCount, drawRowCount, drawByNorm, left, blend1, blend2, side, rand, lucky */
		drawBalls(coupon, TOTAL_BALL, DRAW_BALL, keyb2, 1, 1, 1, 1, 1, 1, 1);

		/* draw power balls */
		drawBalls(coupon_pb, TOTAL_BALL_PB, 1, keyb2, 1, 1, 1, 1, 1, 1, 1);
		printListXYWithPBByKey(coupon, coupon_pb, fp);
		removeAllXY(coupon_pb);
		removeAllXY(coupon);

	} else if (keyb == 2) {
		printf("Calculation results are writing to %s file...\n", OUTPUTFILE);
		calcCombMatch(2, fp);
	} else if (keyb == 3) {
		printf("Calculation results are writing to %s file...\n", OUTPUTFILE);
		calcCombMatch(3, fp);
	} else if (keyb == 4) {
		printf("Calculation results are writing to %s file...\n", OUTPUTFILE);
		calcCombMatch(4, fp);
	} else if (keyb == 5) {
		calcCombMatch(5, fp);
	} else if (keyb == 6) {
		printf("Numbers that drawn together (2 numbers):\n\n");
		fprintf(fp, "Numbers that drawn together (2 numbers):\n\n");
		#if defined(__MSDOS__)
		luckyBalls2 = getLuckyBallsFromFile(luckyBalls2, 2);
		#endif
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
		printLuckyBalls(luckyBalls4, fp);
		#if defined(__MSDOS__)
		removeAllXY(luckyBalls4);
		#endif
	}

	printf("\nThe results are written to %s file.\n", OUTPUTFILE);

	fclose(fp);

	printf("\n\n");
	printf("0- Main Menu\n");
	if (keyb == 1) printf("1- Draw Again\n");
	printf("9- Exit\n");
	printf("\nPlease make your selection: ");

	goto keybCommand;

exitProgram:

	removeAllXY(winningDrawnBallsList);
	removeAllX2(winningBallStats);
	removeAllX2(powerBallStats);

	return 0;
}
