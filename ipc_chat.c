/*
 * ipc_chat.c
 *
 * OS Lab - IPC Chat Using Named Pipes (FIFOs)
 *
 * Usage:
 *      Terminal 1: ./ipc_chat A
 *      Terminal 2: ./ipc_chat B
 *
 * Process A sends first.
 * Process B receives the message and replies.
 *
 * Type:
 *      quit
 *
 * to end the conversation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define FIFO_A_TO_B "chat_a_to_b"
#define FIFO_B_TO_A "chat_b_to_a"

#define MAX_MESSAGE 64

/*
 * Reads one line from the terminal.
 *
 * The function reads ONE character at a time using read().
 *
 * It stops when:
 *      - a newline '\n' is read
 *      - MAX_MESSAGE characters have been read
 *      - end-of-file occurs
 *
 * Returns the number of characters stored in buffer.
 */
ssize_t read_line(char *buffer)
{
    ssize_t total = 0;
    ssize_t result;
    char c;

    while (total < MAX_MESSAGE)
    {
        result = read(STDIN_FILENO, &c, 1);
        if (result == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        /* End-of-file */
        if (result == 0)
        {
            break;
        }

        buffer[total] = c;
        total++;
        /* Stop when Enter is pressed */

        if (c == '\n')
        {
            break;
        }
    }
    return total;
}


/*
 * Returns 1 if the message is:
 *
 *      quit\n
 *
 * Otherwise returns 0.
 */
int is_quit_message(char *buffer, ssize_t count)
{
    if (count == 5 &&
        buffer[0] == 'q' &&
        buffer[1] == 'u' &&
        buffer[2] == 'i' &&
        buffer[3] == 't' &&
        buffer[4] == '\n')
    {
        return 1;
    }

    return 0;
}


int main(int argc, char *argv[])
{
    int read_fd;
    int write_fd;

    char send_buffer[MAX_MESSAGE];
    char receive_buffer[MAX_MESSAGE];

    ssize_t send_count;
    ssize_t receive_count;


    /*
     * ---------------------------------------------------------
     * Check command-line argument
     * ---------------------------------------------------------
     */

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s A|B\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "A") != 0 &&
        strcmp(argv[1], "B") != 0)
    {
        fprintf(stderr, "Usage: %s A|B\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    /*
     * =========================================================
     * PROCESS A
     * =========================================================
     */

    if (strcmp(argv[1], "A") == 0)
    {
        printf("Starting Process A...\n");


        /*
         * TODO 4:
         *
         * Create FIFO_A_TO_B using mkfifo().
         *
         * Use permissions:
         *
         *      0666
         *
         * IMPORTANT:
         * If mkfifo() returns -1 because the FIFO already exists,
         * the program should continue.
         */
        if (mkfifo(FIFO_A_TO_B, 0666) == -1 && errno != EEXIST)
        {
            perror("mkfifo " FIFO_A_TO_B);
            exit(EXIT_FAILURE);
        }


        /*
         * TODO 5:
         *
         * Create FIFO_B_TO_A.
         * This is almost identical to TODO 4.
         */
        if (mkfifo(FIFO_B_TO_A, 0666) == -1 && errno != EEXIST)
        {
            perror("mkfifo " FIFO_B_TO_A);
            exit(EXIT_FAILURE);
        }


        printf("Waiting for Process B...\n");


        /*
         * Process A WRITES to FIFO_A_TO_B.
         *
         * TODO 6:
         *
         * Open FIFO_A_TO_B using:
         *
         *      O_WRONLY
         *
         * Store the returned file descriptor in write_fd.
         *
         * Remember to check for an error.
         */
        write_fd = open(FIFO_A_TO_B, O_WRONLY);
        if (write_fd == -1)
        {
            perror("open " FIFO_A_TO_B);
            exit(EXIT_FAILURE);
        }


        /*
         * Process A READS from FIFO_B_TO_A.
         *
         * TODO 7:
         *
         * Open FIFO_B_TO_A using:
         *
         *      O_RDONLY
         *
         * Store the returned file descriptor in read_fd.
         */
        read_fd = open(FIFO_B_TO_A, O_RDONLY);
        if (read_fd == -1)
        {
            perror("open " FIFO_B_TO_A);
            exit(EXIT_FAILURE);
        }


        printf("Connected to Process B!\n\n");


        /*
         * -----------------------------------------------------
         * Main chat loop for Process A
         * -----------------------------------------------------
         *
         * Process A always sends FIRST.
         */

        while (1)
        {
            printf("You: ");
            fflush(stdout);


            /*
             * Read one line from the keyboard.
             *
             * This part is already done for you.
             */
            send_count = read_line(send_buffer);

            /* Ctrl-D (end-of-file on stdin) ends the chat */
            if (send_count == 0)
            {
                break;
            }


            /*
             * TODO 8:
             *
             * Send send_buffer to Process B.
             *
             * Use:
             *
             *      write_fd
             *      send_buffer
             *      send_count
             *
             * Remember to check if write() returns -1.
             */
            if (write(write_fd, send_buffer, send_count) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }


            /*
             * If the user typed "quit", stop.
             *
             * This part is already done for you.
             */
            if (is_quit_message(send_buffer, send_count))
            {
                break;
            }


            /*
             * TODO 9:
             *
             * Read Process B's response from read_fd.
             *
             * Store the result in:
             *
             *      receive_buffer
             *
             * Read at most MAX_MESSAGE bytes.
             *
             * Save the number of bytes returned by read()
             * in receive_count.
             */
            receive_count = read(read_fd, receive_buffer, MAX_MESSAGE);


            /*
             * Check for read error.
             */
            if (receive_count == -1)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }


            /*
             * If the other process closed the pipe,
             * read() returns 0.
             */
            if (receive_count == 0)
            {
                printf("\nProcess B disconnected.\n");
                break;
            }


            /*
             * Display the incoming message.
             *
             * We already print "Peer: ".
             *
             * TODO 10:
             *
             * Use write() to display receive_buffer on the screen.
             *
             * Hint:
             *
             * write(STDOUT_FILENO,
             *       receive_buffer,
             *       receive_count);
             */

            printf("Peer: ");
            fflush(stdout);

            if (write(STDOUT_FILENO, receive_buffer, receive_count) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }


            /*
             * If Process B sent "quit", stop.
             */
            if (is_quit_message(receive_buffer, receive_count))
            {
                break;
            }
        }
    }


    /*
     * =========================================================
     * PROCESS B
     * =========================================================
     */

    else
    {
        printf("Starting Process B...\n");
        printf("Waiting for Process A...\n");


        /*
         * Process B READS from FIFO_A_TO_B.
         *
         * TODO 11:
         *
         * Open FIFO_A_TO_B using O_RDONLY.
         *
         * Store the result in read_fd.
         */
        read_fd = open(FIFO_A_TO_B, O_RDONLY);
        if (read_fd == -1)
        {
            perror("open " FIFO_A_TO_B);
            exit(EXIT_FAILURE);
        }


        /*
         * Process B WRITES to FIFO_B_TO_A.
         *
         * TODO 12:
         *
         * Open FIFO_B_TO_A using O_WRONLY.
         *
         * Store the result in write_fd.
         */
        write_fd = open(FIFO_B_TO_A, O_WRONLY);
        if (write_fd == -1)
        {
            perror("open " FIFO_B_TO_A);
            exit(EXIT_FAILURE);
        }


        printf("Connected to Process A!\n\n");


        /*
         * -----------------------------------------------------
         * Main chat loop for Process B
         * -----------------------------------------------------
         *
         * Process B always RECEIVES first.
         */

        while (1)
        {
            /*
             * TODO 13:
             *
             * Read a message from Process A.
             *
             * Use:
             *
             *      read_fd
             *      receive_buffer
             *      MAX_MESSAGE
             *
             * Store the number of bytes read in receive_count.
             */
            receive_count = read(read_fd, receive_buffer, MAX_MESSAGE);


            if (receive_count == -1)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }


            if (receive_count == 0)
            {
                printf("\nProcess A disconnected.\n");
                break;
            }


            /*
             * Display the message.
             */

            printf("Peer: ");
            fflush(stdout);


            /*
             * TODO 14:
             *
             * Display receive_buffer using write().
             *
             * Write receive_count bytes to STDOUT_FILENO.
             */
            if (write(STDOUT_FILENO, receive_buffer, receive_count) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }


            /*
             * If Process A sent "quit", stop.
             */
            if (is_quit_message(receive_buffer, receive_count))
            {
                break;
            }


            /*
             * Now Process B gets to respond.
             */

            printf("You: ");
            fflush(stdout);

            send_count = read_line(send_buffer);

            /* Ctrl-D (end-of-file on stdin) ends the chat */
            if (send_count == 0)
            {
                break;
            }


            /*
             * TODO 15:
             *
             * Send send_buffer to Process A using write_fd.
             *
             * Send exactly send_count bytes.
             */
            if (write(write_fd, send_buffer, send_count) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }


            /*
             * If Process B typed "quit", stop.
             */
            if (is_quit_message(send_buffer, send_count))
            {
                break;
            }
        }
    }


    /*
     * =========================================================
     * CLEANUP
     * =========================================================
     */


    /*
     * TODO 16:
     *
     * Close read_fd.
     *
     * Example:
     *
     *      if (close(read_fd) == -1)
     *      {
     *          perror("close");
     *      }
     */
    if (close(read_fd) == -1)
    {
        perror("close read_fd");
    }


    /*
     * TODO 17:
     *
     * Close write_fd.
     */
    if (close(write_fd) == -1)
    {
        perror("close write_fd");
    }


    /*
     * Only Process A should remove the FIFO files.
     *
     * Process B should NOT unlink them.
     */
    if (strcmp(argv[1], "A") == 0)
    {
        /*
         * TODO 18:
         *
         * Remove FIFO_A_TO_B using unlink().
         */
        if (unlink(FIFO_A_TO_B) == -1)
        {
            perror("unlink " FIFO_A_TO_B);
        }


        /*
         * TODO 19:
         *
         * Remove FIFO_B_TO_A using unlink().
         */
        if (unlink(FIFO_B_TO_A) == -1)
        {
            perror("unlink " FIFO_B_TO_A);
        }
    }


    printf("\nChat ended.\n");

    return 0;
}