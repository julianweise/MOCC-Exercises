#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <math.h>
#include <stdbool.h>

const int C_MESSAGE_BUFFER_SIZE = 1024;
const int C_PIPE_READ_END = 0;
const int C_PIPE_WRITE_END = 1;

struct Interval {
    int lowerBound;
    int upperBound;
};

struct IntervalTuple {
    struct Interval lowerInterval;
    struct Interval upperInterval;
};

bool mainProcess = true;

int lowerChildPid = 0;
int upperChildPid = 0;

int lowerChildResult = 0;
int upperChildResult = 0;

int sendMessageToParentPipe[2];
int receiveMessageFromParentPipe[2];

int sendMessageToLowerChildPipe[2];
int receiveMessageFromLowerChildPipe[2];

int sendMessageToUpperChildPipe[2];
int receiveMessageFromUpperChildPipe[2];

void createPipe(int lPipe[]) {
    if (pipe(lPipe) == -1)
    {
        fprintf(stderr, "Pipe Failed\n" );
        exit(EXIT_FAILURE);
    }

    //fprintf(stderr, "[%d] created pipe (READ = %d, WRITE = %d)\n", getpid(), lPipe[C_PIPE_READ_END], lPipe[C_PIPE_WRITE_END]);
}

void writeToPipe(int pipe[], int message) {
    //fprintf(stderr, "[%d] writing to pipe (%d). message = %d\n", getpid(), pipe[C_PIPE_WRITE_END], message);

    char buffer[C_MESSAGE_BUFFER_SIZE];
    sprintf(buffer, "%d", message);
    write(pipe[C_PIPE_WRITE_END], buffer, C_MESSAGE_BUFFER_SIZE);
}

int readFromPipe(int pipe[]) {
    char buffer[C_MESSAGE_BUFFER_SIZE];
    read(pipe[C_PIPE_READ_END], buffer, C_MESSAGE_BUFFER_SIZE);

    int message = atoi(buffer);

    //fprintf(stderr, "[%d] reading from pipe (%d). message = %d\n", getpid(), pipe[C_PIPE_READ_END], message);

    return message;
}

int forkProcess() {
    int pid = 0;
    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "[ERROR] Unable to fork process\n");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {
        //fprintf(stderr, "[%d] forked new process. id = %d\n", getpid(), pid);
    }
    else {
        lowerChildPid = 0;
        upperChildPid = 0;
        mainProcess = false;
    }
    return pid;
}

void forkLowerChild() {
    createPipe(sendMessageToLowerChildPipe);
    createPipe(receiveMessageFromLowerChildPipe);

    lowerChildPid = forkProcess();

    if (lowerChildPid == 0) {
        sendMessageToParentPipe[C_PIPE_WRITE_END] = receiveMessageFromLowerChildPipe[C_PIPE_WRITE_END];
        close(receiveMessageFromLowerChildPipe[C_PIPE_READ_END]);

        receiveMessageFromParentPipe[C_PIPE_READ_END] = sendMessageToLowerChildPipe[C_PIPE_READ_END];
        close(sendMessageToLowerChildPipe[C_PIPE_WRITE_END]);
    }
}

void forkUpperChild() {
    createPipe(sendMessageToUpperChildPipe);
    createPipe(receiveMessageFromUpperChildPipe);

    upperChildPid = forkProcess();

    if (upperChildPid == 0) {
        sendMessageToParentPipe[C_PIPE_WRITE_END] = receiveMessageFromUpperChildPipe[C_PIPE_WRITE_END];
        close(receiveMessageFromUpperChildPipe[C_PIPE_READ_END]);

        receiveMessageFromParentPipe[C_PIPE_READ_END] = sendMessageToUpperChildPipe[C_PIPE_READ_END];
        close(sendMessageToUpperChildPipe[C_PIPE_WRITE_END]);
    }
}

int intervalWidth(struct Interval* interval) {
    return interval->upperBound - interval->lowerBound;
}

struct IntervalTuple splitInterval(struct Interval* interval) {
    int width = intervalWidth(interval);

    if (width < 1) {
        fprintf(stderr, "[%d] Can not split interval, since intervalWidth <= 1\n", getpid());
        exit(EXIT_FAILURE);
    }

    struct Interval lower = { interval->lowerBound, interval->lowerBound + (int)floor(width / 2) };
    struct Interval upper = { lower.upperBound + 1, interval->upperBound };
    struct IntervalTuple tuple = { lower, upper };

    return tuple;
}

void exitIfIntervalIsClosed(struct Interval* interval) {
    int width = intervalWidth(interval);

    if (width > 0) {
        return;
    }

    if (!mainProcess) {
        //fprintf(stderr, "[%d] submitting result: %d\n", getpid(), interval->lowerBound);
        writeToPipe(sendMessageToParentPipe, interval->lowerBound);
        close(sendMessageToParentPipe[C_PIPE_WRITE_END]);
        exit(EXIT_SUCCESS);
    }

    fprintf(stdout, "%d", interval->lowerBound);
    exit(EXIT_SUCCESS);
}

void readIntervalFromParentPipe(struct Interval* interval) {
    interval->lowerBound = readFromPipe(receiveMessageFromParentPipe);
    interval->upperBound = readFromPipe(receiveMessageFromParentPipe);
    close(receiveMessageFromParentPipe[C_PIPE_READ_END]);

    exitIfIntervalIsClosed(interval);
}

void writeIntervalToChildPipe(int sendMessageToChildPipe[], struct Interval* interval) {
    writeToPipe(sendMessageToChildPipe, interval->lowerBound);
    writeToPipe(sendMessageToChildPipe, interval->upperBound);
    close(sendMessageToChildPipe[C_PIPE_WRITE_END]);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "[USAGE]: forksum {LOWER_BOUND[int]} {UPPER_BOUND{int}]");
        exit(EXIT_FAILURE);
    }

    struct Interval myInterval = { atoi(argv[1]), atoi(argv[2]) };

    while (intervalWidth(&myInterval) > 0) {
        struct IntervalTuple tuple = splitInterval(&myInterval);
        bool isParent = true;
        forkLowerChild();

        if (lowerChildPid == 0) {
            isParent = false;
            readIntervalFromParentPipe(&myInterval);
        }

        if (lowerChildPid > 0) {
            writeIntervalToChildPipe(sendMessageToLowerChildPipe, &(tuple.lowerInterval));

            wait(NULL);
            lowerChildResult = readFromPipe(receiveMessageFromLowerChildPipe);
            close(receiveMessageFromLowerChildPipe[C_PIPE_READ_END]);

            forkUpperChild();
        }

        if (upperChildPid == 0 && isParent) {
            isParent = false;
            readIntervalFromParentPipe(&myInterval);
        }

        if (upperChildPid > 0) {
            writeIntervalToChildPipe(sendMessageToUpperChildPipe, &(tuple.upperInterval));

            wait(NULL);
            upperChildResult = readFromPipe(receiveMessageFromUpperChildPipe);
            close(receiveMessageFromUpperChildPipe[C_PIPE_READ_END]);
            break;
        }
    }

    exitIfIntervalIsClosed(&myInterval);

    if (mainProcess) {
        fprintf(stdout, "%d\n", lowerChildResult + upperChildResult);
        exit(EXIT_SUCCESS);
    }
    else {
        writeToPipe(sendMessageToParentPipe, lowerChildResult + upperChildResult);
        close(sendMessageToParentPipe[C_PIPE_WRITE_END]);
        exit(EXIT_SUCCESS);
    }

    fprintf(stderr, "[%d] did not terminate\n", getpid());
}