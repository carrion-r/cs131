/*
 * CS131: Custom Gather, Lab2
 *
 *
 * Author: Dinorah Carrion Rodriguez
 */

#include <mpi.h>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <fstream>
#include <string>
#include <string.h>
#include <iostream>
#include <chrono>
#include <utility>
/*input parameters*/

const static int TOTAL_INPUT_LINES = 100000;
const static int MAX_CHARS = 15;

/*constants for debugging of mpi run. They represent the
 * process control functions of mpirun
 * --debug
 * -v
 * */

const static int DEBUG = 0;
const static int VERBOSE_DEBUG = 0;
const static int SHOULD_TIME = 0;



/*http://en.cppreference.com/w/cpp/chrono/high_resolution_clock*/
using LineMatrix = char[TOTAL_INPUT_LINES][MAX_CHARS];
using Timer = std::chrono::high_resolution_clock;

// Don't CHANGE This Code (you can add more functions)-----------------------------------------------------------------------------

struct Result {
	Result() :
		lineNumber(0), firstChar(0), length(0) {
	}

	Result(int lineNumber_, int firstChar_, int length_) :
		lineNumber(lineNumber_), firstChar(firstChar_), length(length_) {
	}

	// This allows you to compare results with the < (less then) operator, i.e. r1 < r2
	bool operator<(Result const& o) {
		// Line number can't be equal
		return length < o.length
				|| (length == o.length && lineNumber > o.lineNumber)
				|| (length == o.length && lineNumber == o.lineNumber
						&& firstChar > o.firstChar);
	}

	int lineNumber, firstChar, length;
};

void DoOutput(Result r) {
	std::cout << "Result: " << r.lineNumber << " " << r.firstChar << " "
			<< r.length << std::endl;
}

// CHANGE This Code (you can add more functions)-----------------------------------------------------------------------------

void parseFile(LineMatrix matrix, std::ifstream& inputFile) {


	if (!inputFile.is_open()) {
		perror("Failed to open this input file.Please try again.");
		exit(-1);
	}

	else {
		int parsedLines = 0;
		std::string line;

		while ((parsedLines <= TOTAL_INPUT_LINES) && std::getline(inputFile,line)) {
			strcpy(matrix[parsedLines++], line.c_str());
			line.clear();
		}
	}
	inputFile.close();

}

std::pair<int, int> psubstring(std::string const& line, int begins, int ends,int sz) {
	while (!(begins < 0) && (ends <= sz - 1)
			&& (std::tolower(line.at(begins)) == std::tolower(line.at(ends)))) {
		--begins;
		++ends;
	}

	// line.substr(begins,(ends-begins)-1); actual string
	return std::pair<int, int>(begins + 1, (ends - begins - 1));
}

Result calculateLargestPalindrome(int lineNum, const std::string& line) {
	if (line.empty())
		return Result(0, 0, 0);

	int sz = (line.size());
	std::pair<int, int> palindrome(0, 1);

	/*palindrome.first corresponds to Result.firstChar
	 *palindrome.second corresponds to Result.length
	 */

	for (int pos = 0; pos < sz; ++pos) {
		std::pair<int, int> sub1 = psubstring(line, pos, pos, sz);
		std::pair<int, int> sub2 = psubstring(line, pos, pos + 1, sz);

		if (sub1.second > palindrome.second) {
			palindrome.first = sub1.first;
			palindrome.second = sub1.second;
		}
		if (sub2.second > palindrome.second) {
			palindrome.first = sub2.first;
			palindrome.second = sub2.second;
		}
	}

	return Result(lineNum, palindrome.first, palindrome.second);
}

int main(int argc, char* argv[]) {
	int processId;
	int numberOfProcesses;

	// Setup MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &processId);
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);

	// Two arguments, the program name and the input file. The second should be the input file
	if (argc != 2) {
		if (processId == 0) {
			std::cout
			<< "ERROR: Incorrect number of arguments. Format is: <filename>"
			<< std::endl;
		}
		MPI_Finalize();
		return 0;
	}

	// ....... Your SPMD program goes here ......

	/*   Master node processID value: 0
	 *Populates our matrix
	 *Flushes cout buffer
	 */
	char fileLines[TOTAL_INPUT_LINES][MAX_CHARS];
	if (processId == 0) {
		std::ifstream inputFile(argv[1]);
		parseFile(fileLines,inputFile);

		if (VERBOSE_DEBUG) {
			for (int i = 0; i < TOTAL_INPUT_LINES; ++i) {
				printf("Line %d: %s", i, fileLines[i]);
				std::endl(std::cout);
			}
		}
	}

	/* Once we have parsed our file,built our matrix, and our master node is running
	 *  we start timing our program
	 */
	std::chrono::time_point<Timer> startTime;
	if (processId == 0)
		startTime = Timer::now();

	/*Scattering the set of lines that each node will process */
	int lineSubset = TOTAL_INPUT_LINES / numberOfProcesses;
	char lineBuffer[lineSubset][MAX_CHARS];

	if (MPI_Scatter(fileLines, (lineSubset*MAX_CHARS), MPI_CHAR, &lineBuffer,
			(lineSubset*MAX_CHARS), MPI_CHAR, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
		perror("MPI_Scatter was not able to complete.");
		exit(-1);
	}

	if (VERBOSE_DEBUG) {
		for (int i = 0; i < lineSubset; ++i) {
			printf("Process %d line buffer: %s", processId, lineBuffer[i]);
			std::endl(std::cout);
		}

	}

	/*largestPalindrome holds our final value.
	 *palindromeSubResult is the partial result we get from our node/process i
	 *to do if have time: create function that process buffer
	 * */
	Result largestPalindrome(0,0,0);
	for (int i = 0; i < lineSubset; ++i) {
		if (VERBOSE_DEBUG) {
			printf("Line %s located at position %d by process%d", lineBuffer[i],i, processId);
			std::endl(std::cout);
		}
		Result palindromeSubResult = calculateLargestPalindrome(i,lineBuffer[i]);
		if (largestPalindrome < palindromeSubResult)
			largestPalindrome = palindromeSubResult;
	}

	largestPalindrome.lineNumber = (processId*lineSubset) + largestPalindrome.lineNumber;

	if (DEBUG) {
		printf("Palindrome Result calculated by process %d: {%d, %d, %d}",
				processId, largestPalindrome.lineNumber,
				largestPalindrome.firstChar, largestPalindrome.length);
		std::endl(std::cout);
	}

	int resultBuffer[3] = { largestPalindrome.lineNumber, largestPalindrome.firstChar,largestPalindrome.length };
	int neighborsBuffer[3] = {0,0,0};
	/* Ring Network: point to point communication between all of our
	 * (numberOfProcess - 1) worker nodes.
	 * Worker node at the higher level i communicates with the node thats directly below
	 * Nearest-neighbor/Destination node: (i-1) node.
	 * */

	for (int i = (numberOfProcesses - 1); i > 0; --i) {

		if (processId == i)
			MPI_Send(&resultBuffer, 3, MPI_INT, (i-1), 0, MPI_COMM_WORLD);

		else if (processId == (i - 1))
		{
			MPI_Recv(&neighborsBuffer,3, MPI_INT, i, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);

			Result neighborsResult(neighborsBuffer[0], neighborsBuffer[1],neighborsBuffer[2]);
			if(largestPalindrome < neighborsResult)
				largestPalindrome = neighborsResult;

			resultBuffer[0] = largestPalindrome.lineNumber;
			resultBuffer[1] = largestPalindrome.firstChar;
			resultBuffer[2] = largestPalindrome.length;
		}
	}

	if (processId == 0)
	{
		if (SHOULD_TIME)
		{
			auto endTime = Timer::now();
			auto totalProcessingTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
			printf("Time to process with %d processes: %lld", numberOfProcesses,totalProcessingTime);
			std::endl(std::cout);
		}

		DoOutput(largestPalindrome);
	}

	MPI_Finalize();

	return 0;
}

