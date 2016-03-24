/*
 * lab3.cpp
 *
 *  Created on: Feb 25, 2016
 *      Author: dinorahcarrion
 */
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <string.h>
#include <fstream>
#include <iostream>
#include <utility>
#include "ompi.h"
// Don't CHANGE This Code (you can add more functions)-----------------------------------------------------------------------------

const static int TOTAL_INPUT_LINES = 100000;
const static int MAX_CHARS = 15;

using SearchArray = char[TOTAL_INPUT_LINES][MAX_CHARS];
struct Result;
int main(int argc, char* argv[]);
std::pair<int,int> psubstring(std::string const& line, int begins, int ends, int sz);
void largestPalindrome(std::string const& line, int lineNum, Result& r);
void parseFile(SearchArray stringMatrix,std::ifstream& inputFile);

struct Result
{
	Result()
	: lineNumber(0), firstChar(0), length(0)
	{}

	Result(int lineNumber_, int firstChar_, int length_)
	: lineNumber(lineNumber_), firstChar(firstChar_), length(length_)
	{}

	// This allows you to compare results with the < (less then) operator, i.e. r1 < r2
	bool
	operator<(Result const& o)
	{
		// Line number can't be equal
		return length < o.length ||
				(length == o.length && lineNumber >  o.lineNumber) ||
				(length == o.length && lineNumber == o.lineNumber  && firstChar > o.firstChar);
	}

	int lineNumber, firstChar, length;
};

void
DoOutput(Result r)
{
	std::cout << "Result: " << r.lineNumber << " " << r.firstChar << " " << r.length << std::endl;
}
// CHANGE This Code (you can add more functions)-----------------------------------------------------------------------------
std::pair<int,int> psubstring(std::string const& line, int begins, int ends, int sz){
	while(!(begins < 0) && (ends <= sz - 1) && (std::tolower(line.at(begins)) == std::tolower(line.at(ends))) ){
		--begins; ++ends;
	}

	// line.substr(begins,(ends-begins)-1); actual string
	return std::pair<int,int>(begins+1,(ends-begins-1));
}


void largestPalindrome(std::string const& line, int lineNum, Result& r){
	if(line.empty()) { r.lineNumber = lineNum; return; }

	int sz = (line.size());
	std::pair<int,int> palindrome(0,1);

	for(int pos = 0; pos < sz; ++pos){
		std::pair<int,int> sub1 = psubstring(line,pos,pos,sz);
		std::pair<int,int> sub2 = psubstring(line,pos,pos+1,sz);


		if(sub1.second > palindrome.second){
			palindrome.first = sub1.first;
			palindrome.second = sub1.second;
		}
		if(sub2.second > palindrome.second){
			palindrome.first = sub2.first;
			palindrome.second = sub2.second;
		}
	}
	r.lineNumber = lineNum;
	r.firstChar = palindrome.first;
	r.length = palindrome.second;
	//Result currentResult(lineNum,palindrome.first,palindrome.second);
	//r = (r > currentResult)? r: currentResult;

	//return r;
	//return Result(lineNum,palindrome.first,palindrome.second);


}

void parseFile(SearchArray stringMatrix,std::ifstream& inputFile)
{

	if(!inputFile.is_open())
	{
		perror("Failed to open this input file.Please try again.");
		exit(-1);
	}

	else
	{
		int parsedLines = 0;
		std::string line;
		while(std::getline(inputFile,line))
		{
			strcpy(stringMatrix[parsedLines++],line.c_str());
			line.clear();
		}

	}
	inputFile.close();
}


int
main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cout << "ERROR: Incorrect number of arguments. Format is: <filename> <numThreads> " << std::endl;
		return 0;
	}

	// ....... Your OpenMP program goes here ............
	char inputLines[TOTAL_INPUT_LINES][MAX_CHARS];

	std::ifstream inputFile(argv[1]);
	parseFile(inputLines,inputFile);
	int numThreads = argv[2];
    int tid,i,offset;
	omp_set_num_threads(numThreads);
	Result results[numThreads][MAX_CHARS];
	Result localPalindrome(0,0,0);
	Result largestInChunk(0,0,0);
	Result resultA(0,0,0);
	int lineChunk = TOTAL_INPUT_LINES/numThreads;


	//#pragma omp parallel shared(lineChunk,inputLines) private(tid,localPalindrome,largestInChunk)
	//{
		//tid = omp_get_thread_num();
		//localPalindrome.lineNumber += (tid * lineChunk);
		#pragma omp parallel for shared(inputLines, resultA) private(localPalindrome,largestInChunk) schedule(static,lineChunk)
		{

			for(i = (omp_get_thread_num()*lineChunk); i < lineChunk; i++)
			{

				largestPalindrome(inputLines[i],i,localPalindrome);
				largestInChunk = ((largestInChunk < localPalindrome)? localPalindrome: largestInChunk);

			}


			#pragma omp atomic
			{
				resultA = ((resultA < largestInChunk)? largestInChunk: resultA);
			}

		}

		DoOutput(resultA);

	//}









	// Part B

	// ... Eventually..
	Result resultB(0,0,0);
	DoOutput(resultB);

	return 0;
}




