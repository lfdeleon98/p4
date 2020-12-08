/*Lauren DeLeon
John Carroll
CS570
09/15/2020
*/
#include "getword.h"
#include <stdlib.h>
/*
This program parses through given inputs and depending on the character and the combination of 
metacharacters, returns the number of characters and a string. Depending on what the current 
and next characters are will determine the output string and the length of that string. 
*/
int getword(char *w) {
	
/*
length is the length of characters in the string before it is broke by a space or a metacharacter. 
i helps indicate if the number of characters will be negative or positive. check_next is the next 
character in the getword function. j is the incremental long integer to calculate the length and 
characters from the path. letter is the current character in the getword function. h is the 
characters from the path from the getenv function.
*/
    extern int amp_flag, till_flag;
    
    int length = 0, i = 0, check_next = 0;
    unsigned long j = 0; 
    int letter = getchar();
    char *h;
    h = getenv("HOME");
	amp_flag = 0, till_flag = 0;
/*
This part of the function checks to see if the first character to pass through the getword function 
is a space or tab. If the current character is a space or a tab, the getchar() function is used to 
iterate through the input stream until it reaches a character that is not a space or a tab. 
*/
    while (letter == '\t' || letter  == ' ') {
        letter = getchar();
    }
	
/*
This if statement checks if the first character is a newline (\n), if so, it will set the pointer to 
a null terminator (\0) and the length of the string will be 0. By returning 0, we are telling the 
program that we are done checking this line of input, and it is time to move onto the next line of input.
*/
    if (letter == '\n') {
        *w = '\0';
        return 0;
    }
	
/*
This if statement checks if the first character is a metacharacter (<, <<, >, |, or &). If the current 
character is < and the next one is also < (we check by iterating to the next character using getchar()), 
we add both the current and next characters into the string by using the *w++ = (char) check_next; 
command. After adding both to the string, we want to end the string by adding \0 and making the length 
equal to 2, in which we will return that value. If the first character is any of the other metacharacters, 
then only that character will be added to the string followed by \0 and the length will be returned with 
a value of 1.
*/
    if((letter == '<') || (letter == '|') || (letter == '>') || (letter == '&')){
			check_next = getchar();
			//checks if there is the case of << and then creates a string with both of them in it
			if (check_next == '<'){
				*w++ = (char) letter;
				check_next = getchar();
				length++;
			}
			if ((letter == '&') && ((check_next == EOF) || (check_next == ' ') || (check_next == '\n'))){
				amp_flag++;
            }
			
			ungetc(check_next, stdin);
			length++;
			*w++ = (char) letter;
			*w = '\0';
			return length;
		}
/*
This if statment checks to see if the first character in the line is a ~. If it is, then we use the 
for loop make the pointer point to each character from the path, which was stored in the value h. 
For each character that is pointed to, we increment the length of the string, or increment the number 
of characters in the string. Once the incremental value of j is equal to the length of the path, it 
exists out of the for loop. The following if statment checks if the next character (check_next) is 
the end of the file (EOF). If it is, we want to set the pointer to a null terminator and return the 
current length of the string. If the next character is not EOF, then we put the next character back 
into the string using the ungetc() function because we iterated over that character when we used 
check_next = getchar() to check if the next character was EOF. Now that the next character is in the 
string, we want to iterate to the next character using letter = getchar();
*/
    if (letter == '~') {
        if ((check_next = getchar()) != '\t' || check_next != ' ' || check_next != '\n') { //next to character
            till_flag++;
        } else if (check_next == EOF) {
            *w++ = '\0';
            return length;
        } else { 
            for(j = 0; j < strlen(h); j++){// if it is ~ and then white space or newline
                *w++ = h[j];
                length++;
            }
        }
        ungetc(check_next, stdin);
        letter = getchar();
    }
	
/*
This if statement checks to see if the first character is a $, if so iterates to the next character using 
the getchar() function and checks if the next character is EOF. If it is, then the string will be empty 
and the length size will be 0. Then we return 0 to move onto the next line of input. If the next character 
is not EOF, then we put the next character back into the string so that we can iterate correctly. If the 
first character is a $, then we will set i to -1 because we will use this value later to make the length 
of this specific string negative. If the first character is not -1, then i will be 1.
*/
    if (letter == '$') {
		/*checks if the next char after $ is eof*/
        if ((check_next = getchar()) == EOF) {
            *w++ = '\0';
            return 0;
        }
		/*checks if $ is the first char, if so, ignore the $ and return the next character*/
        ungetc(check_next, stdin);
        letter = getchar();
		/*this will set the length to a negative number if true*/
        i = -1;
        // dollar_flag++;
    } else {
        i = 1;
    }
	
/*
This while loop will check all the remaining characters in the input stream as long as EOF is not reached.
*/
    while (letter != EOF) {
		
/*
The first if statement checks if the input has exceeded the maximum storage size minus 1 (254 characters). 
If the length is 254, then the last character will be \0. We use ungetc() here to ensure that the last 
character is read into the string since getchar() will not return it. We break out of the while loop to 
go to the bottom if statements.
*/
		if (length == (STORAGE - 1)) {
            *w ='\0';
            ungetc(letter, stdin);
            return length;
        }
		
/*
This if statement will check if current character is a space, tab, or null terminator inside of the input, after 
the first character and if so, the input stream will end by making adding a \0 to the string and breaking out of 
the while loop.
*/
        if (letter == '\t' || letter == ' ') {
            *w = '\0';
            break;
        }
		

/*
In this if statement, we check if the current character is a metacharacter. If it is, we ungetc because we want the 
character to be put back into the input stream. We then add a \0 into the string and break out of the while loop. The 
metacharacter function above the while loop will handle adding the character into the string and the respective length.
*/
        if (letter == '<' || letter == '>' || letter == '|' || letter == '&') {
            ungetc(letter, stdin);
            *w = '\0';
            break;
        }
		
/*
In this if statement, we check if the current character is a backslash (\\) and if the next character is a newline. 
If both are true, then we add a null terminator to the string, and we break out of the while loop. Else, if the current 
character is a backslash and the next character is one of the metacharacters, a space, a $, or a ~, we add the next 
character into the string and increment the length. We then iterate the input using getchar.
*/
        if (letter == '\\') {
            check_next = getchar();
            if (check_next == '\n') {
                *w ='\0';
                break;
            } else if (check_next == '<' || check_next == '>' || check_next == '$' || check_next == '&' || check_next == '~' || check_next == '|' || check_next == ' ' || check_next == '\\') {
                ungetc(check_next, stdin);
                letter = getchar();
            } else {
                ungetc(check_next, stdin);
                letter = getchar();
            }
        }
/*
This if statement checks if the current character is a newline, and if so, we end the empty string by adding only 
adding \0 to it, and we ungetc() because we want the newline to be lengthed in our output. Afterwards, we break out 
of the while loop.
*/
		if (letter == '\n') {
            *w = '\0';
            ungetc(letter, stdin);
            break;
        }

/*This if statement checks if the next character is EOF, if so, put the second to last character into the string using 
*w++ = (char) letter; and increment the length by one. Then it returns the length times the value if the i value that 
indicates if the length will be negative or positve. 
*/
        check_next = getchar();
        if (check_next == EOF) {
            *w++ = (char) letter;
            *w++ = '\0';
            length++;
            return (length*i);
        } 
        ungetc(check_next, stdin);
		
/*
This bit of code will run if the current character does not meet any of those case conditions. It will be added to the string,
the length of the string will be incremented by 1, and then we will iterate through the input stream.
*/
        *w++ = (char) letter;
        length++;
        letter = getchar();
    }
/*
Once the current character is eqaul to EOF, it will exit out of the while loop. It will only add \0 to the string and it will return 
-255 as the length. The input stream will end and the getword.c program will end.
*/
	/*checks if the character is eof*/
    if (letter == EOF) {
        *w = '\0';
        return -255;
    }
/*
If the first character was $ of the string, then the string will return a negative length value.
*/
	if (i == -1) {
		return (length*i);
	}
/*This will return the length value.*/
    return length;
}
