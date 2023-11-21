#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <err.h>

#define MAX_EVENTS 5
#define BUFFER_SIZE 1024

void handle_message(const char *message) {
    if (strcmp(message, "ping!\n") == 0)
        printf("pong!\n");
    else if (strcmp(message, "pong!\n") == 0)
        printf("ping!\n");
    else if (strcmp(message, "quit\n") == 0)
        exit(EXIT_SUCCESS);
    else
        printf("Unknown: %s", message);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Bad usage: %s <pipe_name>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *pipe_name = argv[1];
    int pipe_fd = open(pipe_name, O_RDONLY | O_NONBLOCK);
    if (pipe_fd == -1)
        err(EXIT_FAILURE, "Error opening pipe");

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        err(EXIT_FAILURE, "Error creating epoll instance");

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = pipe_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_fd, &event) == -1)
        err(EXIT_FAILURE, "Error adding pipe to epoll");

    struct epoll_event events[MAX_EVENTS];
    char buffer[BUFFER_SIZE];

    while (1) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1)
            err(EXIT_FAILURE, "Error in epoll_wait");

        for (int i = 0; i < num_events; i++) {
            if (events[i].events & EPOLLIN) {
                ssize_t bytes_read = read(pipe_fd, buffer, sizeof(buffer));
                if (bytes_read == -1 && errno != EAGAIN)
                    err(EXIT_FAILURE, "Error reading from pipe");

                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    handle_message(buffer);
                }
            }
        }
    }

close(pipe_fd);
    close(epoll_fd);

    return EXIT_SUCCESS;
}


