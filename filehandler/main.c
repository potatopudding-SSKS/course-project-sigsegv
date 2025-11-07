#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <wait.h>
#include <errno.h>
#include <time.h>

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
        struct stat* buf = (struct stat*)calloc(1, sizeof(struct stat));
        if (stat(listing->d_name, buf) == -1 && l_flag)
        {
            printf("\nError printing the details!!\n");
            free(buf);
            return 1;
        }
        if ((int)(buf->st_mode & 07777) == 0 && !a_flag)
        {
            continue;
            // Pass;
        }
        printf("%s ", listing->d_name);
        if (l_flag)
        {
            if ((int)(buf->st_mode & 07777) == 0 && !a_flag)
            {
                continue;
                // Pass;
            }
            printf(" %o\t%lld", buf->st_mode & 07777, buf->st_size); // %512 to print lower 3 octets
        }
        printf("\n");
        free(buf);
    }
    if (listing == NULL)
    {
        printf("Error opening directory!\n");
        return 1;
    }
    closedir(cur_dir);
    return 0;
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
    while ((bytes_read = read(file_pointer, data, sizeof(data) - 1)) > 0)
    {
        write(STDOUT_FILENO, data, bytes_read); // Print the content using write cause binary data
    }
    close(file_pointer);
    printf("\n");
    return 0;
}

int createfile(char* filename)
{
    int file_pointer = open(filename, O_CREAT | O_RDWR | O_EXCL, 0777); // Creates a file with all permissions if it doesn't exist. What if it does??
    if (file_pointer == -1)
    {
        printf("Error creating file %s!!\n", filename); // Raises error
        return 1;
    }
    close(file_pointer);
    printf("File created successfully!\n");
    return 0;
}

// TODO: Implement WRITE

int delete(char* filename)
{
    if (remove(filename))
    {
        printf("Error deleting the file!\n");
        return 1;
    }
    printf("Deleted file successfully\n");
    return 0;
}

int undo(char* filename)
{
    char* backup_path = (char*)calloc(8192, sizeof(char));
    strcpy(backup_path, "/backup/");
    strcat(backup_path, filename);
    int f = open(filename, O_WRONLY | O_TRUNC);
    if (f == -1)
    {
        printf("Error opening file %s!\n", filename);
        return 1;
    }
    int bkp = open(backup_path, O_RDONLY);
    if (bkp == -1)
    {
        if (errno == ENOENT)
        {
            printf("No backup exists, cannot UNDO\n");
            close(f);
            return 0;
        }
        else
        {
            printf("Error opening backup!\n");
            close(f);
            return 1;
        }
    }
    ssize_t bytes_read = 0;
    char data[BUF_SIZE] = {0}; // Allocating statically
    while ((bytes_read = read(bkp, data, sizeof(data) - 1)) > 0)
    {
        if (write(f, data, bytes_read) == -1)
        {
            printf("Error undoing!\n");
            close(f);
            close(bkp);
            return 1;
        }
    }
    if (bytes_read == -1)
    {
        printf("Error undoing!\n");
        close(f);
        close(bkp);
        return 1;
    }
    printf("Restored from backup successfully!\n");
    if (delete(backup_path) == 1)
    {
        printf("Error deleting backup!\n");
        close(f);
        close(bkp);
        return 1;
    }
    free(backup_path);
    close(f);
    close(bkp);
    return 0;
}

int info(char* filename)
{
    struct stat* buf = (struct stat*)calloc(1, sizeof(struct stat));
    if (stat(filename, buf) == -1)
    {
        printf("\nError printing the details!!\n");
        return 1;
    }
    printf("Details:\n");
    printf("File Size: %d B | File Permissions: %d\n", buf->st_size, buf->st_mode);
    printf("File Type: ");
    if (S_ISREG(buf->st_mode))
		printf("Regular file");
	else if (S_ISDIR(buf->st_mode))
		printf("Directory\n");
	else if (S_ISLNK(buf->st_mode))
		printf("Symbolic link");
	else if (S_ISCHR(buf->st_mode))
		printf("Character device");
	else if (S_ISBLK(buf->st_mode))
		printf("Block device");
	else if (S_ISFIFO(buf->st_mode))
		printf("FIFO (named pipe)");
	else if (S_ISSOCK(buf->st_mode))
		printf("Socket");
	else
		printf("Unknown type\n");
    printf(" | Owner ID: %d | Group ID: %d\n", buf->st_uid, buf->st_gid);
    printf("Inode Number: %d | Device Number: %d | Number of Hard Links: %d\n", buf->st_ino, buf->st_dev, buf->st_nlink);
    printf("Block Size: %d | Blocks Allocated: %d\n", buf->st_blksize, buf->st_blocks);
    printf("Last Accessed: %s", ctime(&(buf->st_atime)));
    printf("Last Modified: %s", ctime(&(buf->st_mtime)));
    printf("Last Status Change: %s", ctime(&(buf->st_ctime)));
    return 0;
}

int main()
{
    // view(0, 0);
    while (1)
    {
        char* filename = (char*)calloc(1024, sizeof(char)); // Filename is at most 1024 long
        scanf("%s", filename);
        info(filename);
        // if(readfile(filename) == 1)
        // {
        //     printf("What did you do to get here TT\n");
        // }
    }
    return 0;
}