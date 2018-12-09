#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>


struct Interval {
    int lower;
    int higher;
};

struct IntervalTuple {
	struct Interval lower;
	struct Interval higher;
};

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

	struct Interval firstInterval = {start, (int) (start + floor((end - start) / 2))};
	struct Interval secondInterval = {firstInterval.higher + 1, end};

	struct IntervalTuple newIntervals = { firstInterval, secondInterval };

	if (start == end) {
		newIntervals.higher =  firstInterval;
	}

	return newIntervals;
}

void writeToPipe(int pipe[], int message) {
	write(pipe[1], &message, sizeof(message) + 1);
}

int main(int argc, char *argv[]) 
{ 
	if (argc != 3) {
		fprintf(stdout, "[USAGE] forksum start end");
		return 1;
	}

	struct Interval interval = {atoi(argv[1]), atoi(argv[2])};

	if (interval.lower == interval.higher) {
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

            close(parentWritePipeLower[1]);
            read(parentWritePipeLower[0], &interval.lower, sizeof(interval.lower));
            read(parentWritePipeLower[0], &interval.higher, sizeof(interval.higher));
            close(parentWritePipeLower[0]);

            close(parentReadPipeLower[0]);
            dup2(parentReadPipeLower[1], 1);

            if (interval.lower <= interval.higher) {
                fprintf(stdout, "%d", interval.lower);
                close(parentReadPipeLower[1]);
                return 0;
            }
        }

        if (pidLowerChild > 0) {

            struct IntervalTuple intervals = getNewIntervals(interval.lower, interval.higher);
            close(parentWritePipeLower[0]);
            writeToPipe(parentWritePipeLower, intervals.lower.lower);
            writeToPipe(parentWritePipeLower, intervals.lower.higher);
            close(parentWritePipeLower[1]);
            pidHigherChild = forkProccess();
        }

        if (pidLowerChild != 0 && pidHigherChild == 0) {
            close(parentWritePipeHigher[1]);
            read(parentWritePipeHigher[0], &interval.lower, sizeof(interval.lower));
            read(parentWritePipeHigher[0], &interval.higher, sizeof(interval.higher));
            close(parentWritePipeHigher[0]);

            close(parentReadPipeHigher[0]);
            dup2(parentReadPipeHigher[1], 1);

            if (interval.lower <= interval.higher) {
                fprintf(stdout, "%d", interval.lower);
                close(parentReadPipeHigher[1]);
                return 0;
            }
        }

        if (pidHigherChild > 0) {

            struct IntervalTuple intervals = getNewIntervals(interval.lower, interval.higher);
            close(parentWritePipeHigher[0]);
            writeToPipe(parentWritePipeHigher, intervals.higher.lower);
            writeToPipe(parentWritePipeHigher, intervals.higher.higher);
            close(parentWritePipeHigher[1]);
            break;
        }
	}

    int result = 0;

    if (pidLowerChild > 0) {
        wait(NULL);
        int lowerResult;
        read(parentReadPipeLower[0], &lowerResult, sizeof(lowerResult));
        result += lowerResult;
    }

    if (pidLowerChild > 0) {
        wait(NULL);
        int higherResult;
        read(parentReadPipeHigher[0], &higherResult, sizeof(higherResult));
        result += higherResult;
    }

    fprintf(stdout, "%d", result);
} 