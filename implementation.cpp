/*
 *  Created on: Mar 6, 2016
 *  Author: dinorahcarrion
 */
#include "mpi.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
// Don't CHANGE This Code (you can add more functions)-----------------------------------------------------------------------------
using Timer =  std::chrono::high_resolution_clock;

void
CreateElectionArray(int numberOfProcesses,int electionArray[][2])
{

	std::vector<int> permutation;
	for(int i = 0; i < numberOfProcesses; ++i)
		permutation.push_back(i);
	std::random_shuffle(permutation.begin(), permutation.end());

	for(int i = 0; i < numberOfProcesses; ++i)
	{
		electionArray[i][0] = permutation[i];
		int chance = std::rand() % 4; // 25 % chance of inactive
		electionArray[i][1] = chance != 0; // 50% chance,
	}

	//Check that there is at least one active
	bool atLeastOneActive = false;
	for(int i = 0; i < numberOfProcesses; ++i)
	{
		if(electionArray[i][1] == 1)
			atLeastOneActive = true;
	}
	if(!atLeastOneActive)
	{
		electionArray[std::rand() % numberOfProcesses][1] = 1;
	}
}

void
PrintElectionArray(int numberOfProcesses, int electionArray[][2])
{
	for(int i = 0; i < numberOfProcesses; ++i)
	{
		std::printf("%-3d ", electionArray[i][0]);
	}
	std::cout << std::endl;
	for(int i = 0; i < numberOfProcesses; ++i)
	{
		std::printf("%-3d ", electionArray[i][1]);
	}
	std::cout << std::endl;
}
void
PrintElectionResult(int winnerMPIRank, int round, int numberOfProcesses, int electionArray[][2])
{
    auto endTime = Timer::now();
	auto processingTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
	std::cout << "Total to processing time: " << processingTime << std::endl;

	std::cout << "Round " << round << std::endl;
	std::cout << "ELECTION WINNER IS " << winnerMPIRank << "(" << electionArray[winnerMPIRank][0] << ")  !!!!!\n";
	std::cout << "Active nodes where: ";
	for(int i = 0; i < numberOfProcesses; ++i)
	{
		if(electionArray[i][1] == 1)
			std::cout << i << "(" << electionArray[i][0] << "), ";
	}
	std::cout << std::endl;
	PrintElectionArray(numberOfProcesses, electionArray);
	for(int i = 0; i < numberOfProcesses*4-2; ++i)
		std::cout << "_";
	std::cout << std::endl;
}

// CHANGE This Code (you can add more functions)-----------------------------------------------------------------------------

int calculateActiveNodes(const int electionArray[][2], int sz)
{
	
	int numActive = 0;
	for(int i = 0; i < sz; ++i)
	{
		if(electionArray[i][1] == 1) numActive++;
	}
	return numActive;
}

/*matrix[el_position/size][el_position%size];

			cout << *electionArray[1%4] <<endl;
			cout << electionArray[1%4][1] << endl;

			cout << *electionArray[2%4] <<endl;
			cout << electionArray[2%4][1] << endl;
			
			cout << *electionArray[3%4] <<endl;
			cout << electionArray[3%4][1] << endl;
 */

std::pair<int,int> calculateNeighbors(const int electionArray[][2], int eid, int totalNumNodes)
{

	std::pair<int,int> before_after_nodeID;
	before_after_nodeID.first = -1;
	before_after_nodeID.second = -1;
	bool found = false;
	int i = 1;
    do
    {
        
        int j = 0;
        int beforeID = (eid + totalNumNodes -i)%totalNumNodes;
        int afterID = (eid+i)%totalNumNodes;
        
        /*Will search for neighbors ids until it finds them or has examined all possible candidates */
        do
        {
            
            if(electionArray[j][1] == 1)
            {
                int nodeID = electionArray[j][0];
                if((before_after_nodeID.first == -1)&&(nodeID == beforeID))
                    before_after_nodeID.first = j;
                
                if((before_after_nodeID.second == -1)&&(nodeID == afterID))   
                    before_after_nodeID.second = j;
                    
                if(before_after_nodeID.first != -1 && before_after_nodeID.second != -1)
                    found=true;
            }
                        
            ++j;
            
        }while((j < totalNumNodes) && !found);
        
    
    	++i;
    /*Breaks out of the loop if we have found our neighbors*/
		    
   }while((i < totalNumNodes) && !found);

   return before_after_nodeID;

}

/*template<std::size_t SIZE>
int calculateWinner(std::array<int, SIZE>& w)
{
	for ( auto rit=w.rbegin() ; rit < w.rend(); ++rit )
		if(*rit != -2)
			return *rit;

}*/

int calculateWinner(std::vector<int> recv)
{
	for ( auto rit=recv.rbegin() ; rit < recv.rend(); ++rit )
		if(*rit != -2)
			return *rit;
}


bool errorDuringElection(std::vector<int> recv)
{
	bool errorOccurred = false;
   /*if recv vector contains the value -1 print out error message*/
   if(std::find(recv.begin(), recv.end(), -1) != recv.end())
   {
   
   	  errorOccurred = true;
   }
  
   return errorOccurred;
}
int election(const int electionArray[][2], int totalNumNodes, int numActive, int pid)
{

	int my_eid = electionArray[pid][0];
	std::pair<int,int> neighbors = calculateNeighbors(electionArray,my_eid,totalNumNodes);
	int beforeNodeID = neighbors.first;
	int afterNodeID = neighbors.second;
	
	/*
	std::array<int,totalNumNodes> sendArray, recvArray;
	sendArray.fill(-1);
	recvArray.fill(-1);
	sendArray.at(my_eid) = pid;
	*/
	std::vector<int> sendArray(totalNumNodes,-1), recvArray(totalNumNodes,-1);
	sendArray[my_eid] = pid;
	MPI_Request request;

	for(int i = 0; i < totalNumNodes ; ++i)
	{

		if(electionArray[i][1] == 0)
		{
			int eid = electionArray[i][0];
			sendArray[eid] = -2;
			recvArray[eid] = -2;
		}
	}

/* int MPI_Isend(void *buf,int count,MPI_Datatype datatype,int dest,int tag,MPI_Comm comm,MPI_Request *request);
* int MPI_Recv(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
 * */
	for(int n = 0; n < numActive; ++n)
	{
		
		MPI_Isend(&sendArray[0],totalNumNodes,MPI_INT,afterNodeID,0,MPI_COMM_WORLD, &request);

		MPI_Recv(&recvArray[0],totalNumNodes,MPI_INT,beforeNodeID,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		
		MPI_Wait(&request, MPI_STATUS_IGNORE);

		std::swap(sendArray,recvArray);
		sendArray[my_eid] = pid;
	}
	std::swap(sendArray,recvArray);
    if(errorDuringElection(recvArray) == false)
			return calculateWinner(recvArray);
	else 
		return election(electionArray, totalNumNodes, numActive, pid);
}

int
main(int argc, char* argv[])
{
	int processId;
	int numberOfProcesses;

	// Setup MPI
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &processId);
	MPI_Comm_size( MPI_COMM_WORLD, &numberOfProcesses);

	// Two arguments, the program name, the number of rounds, and the random seed
	if(argc != 3)
	{
		if(processId == 0)
		{
			std::cout << "ERROR: Incorrect number of arguments. Format is: <numberOfRounds> <seed>" << std::endl;
		}
		MPI_Finalize();
		return 0;
	}

	const int numberOfRounds = std::atoi(argv[1]);
	const int seed           = std::atoi(argv[2]);

	std::srand(seed);// Set the seed

	auto electionArray = new int[numberOfProcesses][2]; // Bcast with &electionArray[0][0]...

	for(int round = 0; round < numberOfRounds; ++round)
	{
		/*Ensures that previous election has finished*/
        	MPI_Barrier(MPI_COMM_WORLD);
		if(processId == 0)
			CreateElectionArray(numberOfProcesses, (int (*)[2])electionArray);
		
		std::chrono::time_point<Timer> startTime;
		if (processId == 0)
			startTime = Timer::now();
			
 		if(MPI_Bcast(&electionArray[0][0],numberOfProcesses*2,MPI_INT,0,MPI_COMM_WORLD) != MPI_SUCCESS)
		{
			perror("Failure with MPI_Bcast");
			exit(-1);
		}
		
		
		int winner = -1;
		
		/*If this process is participating in the election, calculate a winner */
		if(electionArray[processId][1] == 1)
		{
			int numActive = calculateActiveNodes((int (*)[2])electionArray,numberOfProcesses);
				
			/*Declares itself winner because its the only candidate. Otherwise it runs an election.*/
			if(numActive == 1)
				winner = processId;
			else	
				winner = election((int (*)[2])electionArray,numberOfProcesses,numActive,processId);
			
			/*if this process was the winner and its not the master, tell the master that it won*/
			if(winner == processId && winner != 0)
				MPI_Send(&winner,1,MPI_INT,0,0, MPI_COMM_WORLD);
		}

		/*Waits and imposses temporal order*/
	       MPI_Barrier(MPI_COMM_WORLD);
	
		/*Checks node is active before doing an election*/
		if(processId == 0)
		{
			int electionWinner = 0;
			/*if im not the winner, then I can  recv the winner's id from any of the processes*/
			if(winner != 0)
				MPI_Recv(&electionWinner,1,MPI_INT,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			
			PrintElectionResult(electionWinner, round, numberOfProcesses, (int (*)[2])electionArray);
		}
	}

	delete[] electionArray;

	MPI_Finalize();

	return 0;
}
