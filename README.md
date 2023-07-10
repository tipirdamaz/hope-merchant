![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/preview.png)

# Hope Merchant
Hope Merchant - Lotto Statistics and Ball Draw Algorithms In C


# WARNING! THIS PROGRAM DOES NOT GUARANTEE YOU WINNING THE LOTTO.

This program provides statistics such as how many times the numbers in the previous draws came out, how many times the numbers that came out together in the draws came out together and how many days apart.

It allows you to draw a lotto by using the information of the past draws.

## The fact that some numbers came out a lot in the previous draws does not mean that these numbers will appear more likely in the next draws.

If a statistically infinite number of draws are made, the number of draws of all numbers may be equal.

If the numbers that came out a lot in the previous draws were due to a systematic error caused by the machines and the balls, then it may make sense to draw according to the statistics of the past draws.

For example, in the "Çılgın Sayısal Loto" game which is played by drawing 6 balls out of a total of 90 balls, in two draws made with 5 days apart, if 4 balls reappear due to a systematic error, it may be useful to draw according to the statistics of the past draws.

```
23.11.2020 :  8 22 42 52 55 71        18.11.2020 :  8 22 50 55 71 87       5 days   ( 8,22,55,71)
```

# gaussIndex() Function and Pascal's Triangle

```c
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
```

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/pascal-triangle.jpg)
```
 Figure: Pascal's triangle with 54 nodes
```

The **gaussIndex** function simulates that a ball dropped at node **0** of Pascal's triangle hits the nodes and moves left or right each time (just like tossing a coin) reaching the base of the triangle and selecting the number at the node it hits at the base.

For example, we have a lotto game with a total of **10** balls in a globe. Let's draw from these 10 balls.
Let's examine how the **gaussIndex(10)** function works:

A ball dropped at the top node **(0)** of Pascal's triangle falls to the left **(1)** or right **(2)**.

Suppose the ball follows the path **0-1-4-7-12-18**.

Ball hitting knot **18**, It will land on one of the nodes:

	node = node + level = 18 + 6 = 24

or

	node = node + level + 1 = 18 + 6 + 1 = 25


Suppose the ball then follows the path **25-32-40-49** and reaches the base of Pascal's triangle.

```
gaussIndex(10) = node - (leftnode-1) = 49 - (45-1) = 5
```
**gaussIndex(10) will return ball number ⑤**

The probability that a ball dropped on node 0 will always go to the left or always to the right is very low.
Usually the ball lands somewhere in the middle at the base of the triangle.
If more than one ball fell from above, the balls would pile up in the middle just like in a **normal distribution**.
Based on data from previous draws, the balls are sorted according to certain sorting orders (with the **ballSortOrder** array) to the base of the triangle.
The function was named **gaussIndex** because an index of this array is returned.

In the draws we will make, if we want the balls to be drawn mostly from the balls that came out the most in the past draws,
At the base of the triangle, we should place the balls that appear the most in the middle, and the balls that appear the least on the sides.


Let the number of balls appear in past draws be as follows:

- ① => 56 times
- ② => 14 times
- ③ => 97 times
- ④ => 48 times
- ⑤ => 27 times
- ⑥ => 21 times
- ⑦ => 41 times
- ⑧ => 33 times
- ⑨ => 16 times
- ⑩ => 47 times

If we order the balls according to the normal distribution, that is, the balls that come out the most are in the middle:

- ballSortOrder[0] = ② => 14 times
- ballSortOrder[1] = ⑥ => 21 times
- ballSortOrder[2] = ⑧ => 33 times
- ballSortOrder[3] = ⑩ => 47 times
- ballSortOrder[4] = ① => 56 times
- ballSortOrder[5] = ③ => 97 times
- ballSortOrder[6] = ④ => 48 times
- ballSortOrder[7] = ⑦ => 41 times
- ballSortOrder[8] = ⑤ => 27 times
- ballSortOrder[9] = ⑨ => 16 times

The gaussIndex function following the path **0-1-4-7-12-18-25-32-40-49**:

```
index = gaussIndex(10)-1 = 5-1 = 4
```
**ballSortOrder[index] = ballSortOrder[4] = ①**


# PACKAGE CONTENTS

- **source**				: C source files

	- **sayisal.c**			: Turkey Sayisal Lotto
	- **super.c**			: Turkey Super Lotto
	- **sanstopu.c**		: Turkey Sans Topu Lotto
	- **eumillions.c**		: EuroMillions Lotto
	- **eujackpot.c**		: EuroJackpot Lotto
	- **powerball.c**		: American PowerBall Lotto
	- **megamillions.c**		: American MegaMillions Lotto


- **dist**

	- **...-win32.exe ...-win64.exe**	: Windows 32 bit and 64 bit executable files. These files were compiled with the gcc compiler on the windows 10 operating system.


	- **...-mac**			: macOS executable files. These files were compiled with the gcc compiler on the macOS Catalina 10.15.7 operating system.

	- **...-linux**			: Linux executable files. These files were compiled with the gcc compiler on the centOS 7 Linux operating system.

	- **....txt**			: TXT files are statistics files containing the results of previous draws. These files must be located in the same directory as the executable files.


- **DOS**				: For DOS operating system. 

	- **....EXE**			: MS-DOS executable files. These files were compiled with the 16-bit Borland Turbo C++ compiler on the MS-DOS 5.0 operating system.



# COMPILER AND PLATFORM

### Source files were compiled with 

- gcc 11.2.0 compiler and successfuly tested on

	- Windows 10
	- macOS Catalina 10.15.7
	- centOS 7 Linux

- Borland Turbo C++ 3.0 compiler and successfuly tested on

	- MS-DOS 5.0


# HOW TO COMPILE FILES?

- **gcc sourcefile.c -o outputfile -lm**

# SCREENSHOTS

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/01.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/02.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/03.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/04.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/05.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/06.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/07.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/08.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/09.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/10.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/11.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/12.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/13.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/14.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/15.png)

![alt text](https://github.com/tipirdamaz/hope-merchant/blob/main/screenshots/16.png)


# Donate
### BTC : 1CASunNSibqCsR5Y6kniSW2t22Rt5Bwtgu
![alt text](https://github.com/tipirdamaz/migallery/blob/main/donate/btc.png)
### ETH : 0x3ea58816d52d3a18a447b41a9e65aefb32700551
![alt text](https://github.com/tipirdamaz/migallery/blob/main/donate/eth.png)
### USDT : 0x3ea58816d52d3a18a447b41a9e65aefb32700551
![alt text](https://github.com/tipirdamaz/migallery/blob/main/donate/usdt.png)
