#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define BUF_SIZE 32768 // Very large buffer uwu

// Struct initialization
// File structure = SENTENCES -> CHARS -> CHARS...
//                      |         |->chr   |->chr
//                  SENTENCES...
// Where each sentence in a file is represented as a SENTENCES node, and each SENTENCES node stores a LL of CHARS nodes, each containing a character.
typedef struct CHARS
{
    char chr;
    struct CHARS* next;
} CHARS;

typedef struct SENTENCES
{
    CHARS* sen;
    struct SENTENCES* next;
} SENTENCES;

int view(int a_flag, int l_flag)
{
    DIR* cur_dir = opendir("."); // Open the current directory
    if (cur_dir == NULL)
    {
        printf("Cannot open directory\n");
        return 1;
    }
    struct dirent* listing;
    while ((listing = readdir(cur_dir)) != NULL)
    {
        if (listing->d_name[0] == '.' && !a_flag)
        {
            continue;
            // Pass;
        }
        printf("%s ", listing->d_name);
        if (l_flag)
        {
            struct stat buf[BUF_SIZE];
            if (stat(listing->d_name, buf) == -1)
            {
                printf("\nError printing the details!!\n");
                return 1;
            }
            printf(" %o\t%lld", buf->st_mode%512, buf->st_size);
        }
        printf("\n");
    }
}

int readfile(char* filename) // filename can be the path to the file as well
{
    int file_pointer = open(filename, O_RDONLY); // Opens file
    if (file_pointer == -1)
    {
        printf("Error opening file %s!!\n", filename); // Checks if file exists
        return 1;
    }
    char data[BUF_SIZE] = {0}; // Zero the buffer (Could use calloc free but ehhh)
    ssize_t bytes_read = 0;
    bytes_read = read(file_pointer, data, BUF_SIZE-1);
    data[bytes_read] = '\0';
    printf("%s", data); // Print data
    printf("\n");
}

int main()
{
    view(0, 1);
    // while (1)
    // {
    //     char* filename = (char*)calloc(1024, sizeof(char)); // Filename is at most 1024 long
    //     scanf("%s", filename);
    //     // if(readfile(filename) == 1)
    //     // {
    //     //     printf("What did you do to get here TT\n");
    //     // }
    // }
    return 0;
}