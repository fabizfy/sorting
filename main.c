#include "sort.c"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define DICT_MAX_WORD_LEN 256 /* maximum length of a word (+1) */

int fileOpen();
char *getNextWord(FILE *fd);

int main(int argc, char *argv[])
{
    clock_t begin = clock(); // Starts keeping track of the time

    fileOpen(); // This Function opens files and starts creating hash

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC; // Stop tracking time
    printf("\nTime: %f\n", time_spent);                         // Prints out the time the code took to run
}

char *getNextWord(FILE *fd)
{
    char ch;                            /* holds current character */
    char wordBuffer[DICT_MAX_WORD_LEN]; /* buffer for build a word */
    int putChar = 0;                    /* current pos in buffer   */

    assert(fd != NULL); /* the file descriptor better not be NULL */

    /* read characters until we find an alphabetic one (or an EOF) */
    while ((ch = fgetc(fd)) != EOF)
        if (isalpha(ch))
            break;
    if (ch == EOF)
        return NULL; /* if we hit an EOF, we're done */

    /* otherwise, we have found the first character of the next word */
    wordBuffer[putChar++] = tolower(ch);

    /* loop, getting more characters (unless there's an EOF) */
    while ((ch = fgetc(fd)) != EOF)
    {
        /* the word is ended if we encounter whitespace */
        /* or if we run out of room in the buffer       */
        if (isspace(ch) || putChar >= DICT_MAX_WORD_LEN - 1)
            break;

        /* otherwise, copy the (lowercase) character into the word   */
        /* but only if it is alphanumeric, thus dropping punctuation */
        if (isalnum(ch))
            wordBuffer[putChar++] = tolower(ch);
    }
    wordBuffer[putChar] = '\0'; /* terminate the word          */
    return strdup(wordBuffer);  /* re-allocate it off the heap */
}

int fileOpen()
{
    FILE *fp = fopen("poem.txt", "r");
    char *firstWord = "something";
    char *secondWord = NULL;

    char **array = (char **)malloc(1000000 * sizeof(char *));

    int flag = 0;
    int i = 0;

    while (!feof(fp))
    {
        firstWord = getNextWord(fp); // reads text file
        if (flag == 0)
        {
            printf("Words: %s\n", firstWord);
            flag = 1;
            array[i++] = strdup(firstWord);
        }
        if (firstWord != NULL && secondWord != NULL)
        { // For pairs
            // printf("Words: %s\n", firstWord);         // printing out the pairs of words
            array[i++] = strdup(firstWord);
        }
        secondWord = firstWord;
    }
    setSortThreads(1);
    sortThreaded(array, i);

    for (int j = 0; j < i; j++)
    {
        fprintf(stdout, "sort array[%d] = %s\n", j, array[j]);
    }
    fclose(fp);
    return 0;
}