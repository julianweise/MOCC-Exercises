#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/wait.h>


struct Interval {
    int lower;
    int higher;
};

struct IntervalTuple {
	struct Interval lower;
	struct Interval higher;
};

const int C_READ_END = 0;
const int C_WRITE_END = 1;

int forkProccess() {
	int pid = fork();

	if (pid < 0) {
		fprintf(stderr, "[ERROR] Unable to fork process");
		exit(EXIT_FAILURE);
	}
	return pid;
}

void createPipe(int lPipe[]) {
	if (pipe(lPipe) == -1)
    { 
        fprintf(stderr, "Pipe Failed" ); 
        exit(EXIT_FAILURE); 
    } 
}

struct IntervalTuple getNewIntervals(int start, int end) {

    int intervalWidth = end - start;

    if (intervalWidth < 1) {
        fprintf(stderr, "Can not create new intervals: Interval width < 1! (start: %d, end: %d)", start, end);
        exit(EXIT_FAILURE);
    }

	struct Interval firstInterval = {start, (int) (start + floor((float)intervalWidth * 0.5f))};
	struct Interval secondInterval = {firstInterval.higher + 1, end};

	struct IntervalTuple newIntervals = { firstInterval, secondInterval };

	if (start == end) {
		newIntervals.higher =  firstInterval;
	}

	return newIntervals;
}

void writeToPipe(int pipe[], int message) {
	write(pipe[C_WRITE_END], &message, sizeof(message) + 1);
}

int main(int argc, char *argv[]) 
{ 
	if (argc != 3) {
		fprintf(stdout, "[USAGE] forksum start end");
		return 1;
	}

	struct Interval interval = {atoi(argv[1]), atoi(argv[2])};

	if (interval.lower >= interval.higher) {
		printf("%d", interval.lower);
		return 0;
	} 

	int parentWritePipeLower[2];
	int parentWritePipeHigher[2];

	int parentReadPipeLower[2];
	int parentReadPipeHigher[2];

	createPipe(parentWritePipeLower);
	createPipe(parentWritePipeHigher);

	createPipe(parentReadPipeLower);
	createPipe(parentReadPipeHigher);

    int pidLowerChild = 0;
    int pidHigherChild = 0;

    while(1) {
        pidLowerChild = forkProccess();

        if (pidLowerChild == 0 && pidHigherChild == 0) {
            // lower child process
            close(parentWritePipeLower[C_WRITE_END]);
            read(parentWritePipeLower[C_READ_END], &interval.lower, sizeof(interval.lower));
            read(parentWritePipeLower[C_READ_END], &interval.higher, sizeof(interval.higher));
            close(parentWritePipeLower[C_READ_END]);

            close(parentReadPipeLower[C_READ_END]);
            dup2(1, parentReadPipeLower[C_WRITE_END]);

            // interval consists of same values ==> stop recursion
            if (interval.lower >= interval.higher) {
                fprintf(stdout, "%d", interval.lower);
                close(parentReadPipeLower[C_WRITE_END]);
                return 0;
            }
        }

        if (pidLowerChild > 0) {
            // parent process
            struct IntervalTuple intervals = getNewIntervals(interval.lower, interval.higher);
            close(parentWritePipeLower[C_READ_END]);
            writeToPipe(parentWritePipeLower, intervals.lower.lower);
            writeToPipe(parentWritePipeLower, intervals.lower.higher);
            close(parentWritePipeLower[C_WRITE_END]);
            pidHigherChild = forkProccess();
        }

        if (pidLowerChild != 0 && pidHigherChild == 0) {
            // higher child process
            close(parentWritePipeHigher[C_WRITE_END]);
            read(parentWritePipeHigher[C_READ_END], &interval.lower, sizeof(interval.lower));
            read(parentWritePipeHigher[C_READ_END], &interval.higher, sizeof(interval.higher));
            close(parentWritePipeHigher[C_READ_END]);

            close(parentReadPipeHigher[C_READ_END]);
            dup2(1, parentReadPipeHigher[C_WRITE_END]);

            if (interval.lower == interval.higher) {
                fprintf(stdout, "%d", interval.lower);
                close(parentReadPipeHigher[C_WRITE_END]);
                return 0;
            }
        }

        if (pidHigherChild > 0) {
            // parent process
            struct IntervalTuple intervals = getNewIntervals(interval.lower, interval.higher);
            close(parentWritePipeHigher[C_READ_END]);
            writeToPipe(parentWritePipeHigher, intervals.higher.lower);
            writeToPipe(parentWritePipeHigher, intervals.higher.higher);
            close(parentWritePipeHigher[C_WRITE_END]);
            break;
        }
	}

    if (pidLowerChild > 0) {
        // parent process
        wait(NULL);
        wait(NULL);
        int lowerResult;
        int higherResult;
        read(parentReadPipeLower[C_READ_END], &lowerResult, sizeof(lowerResult));
        read(parentReadPipeHigher[C_READ_END], &higherResult, sizeof(higherResult));

        fprintf(stdout, "%d", lowerResult + higherResult);
    }
} 