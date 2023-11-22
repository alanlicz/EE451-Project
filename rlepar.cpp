
#include <ostream>
#include <vector>
#include <iostream>
#include <pthread.h>
#include <time.h>

const int numThreads = 2; // Adjust the number of threads as needed

struct RleData // RLE Compression/Decompression
{
	std::vector<int8_t> mData;	// Stores either compressed or decompressed data
	void Compress(const std::vector<int8_t>& input); // Compresses input data and stores it in mData
	// Decompresses input data and stores it in mData
	void Decompress(const std::vector<int8_t>& input, size_t outSize);  // Outputs mData
	friend std::ostream& operator<< (std::ostream& stream, const RleData& rhs);
	static size_t MaxRunSize() { return 127; }
};
struct ThreadInfo
{
    int start,end;
    std::vector<int8_t>* input;
    std::vector<int8_t>* output;
};

void* CompressThread(void* arg) 
{
    ThreadInfo* threadInfo = static_cast<ThreadInfo*>(arg);
    pthread_t threadId = pthread_self();
    char cnter = 1;
	char flag = 0; //meaning checking for continous different chars
	std::vector<int8_t> temp;

	for(int x=threadInfo->start; x<threadInfo->end-1; x++)
	{
        // std::cout << threadInfo->start << " " <<(threadInfo->input->at(x)) << " ";
		if(threadInfo->input->at(x) == threadInfo->input->at(x+1))
		{
			// std::cout << threadInfo->start << " " <<(threadInfo->input->at(x)) << " ";
			cnter++;
			if(cnter == -128)
			{
				temp.push_back(((cnter+1)*-1));
				cnter=1;
				temp.push_back(threadInfo->input->at(x));
			}
			if(x == threadInfo->input->size()-2 || x == (threadInfo->input->size()/numThreads)-2) // you're 1 b4 the end of the vector, so last element needs to be pushed
			{
				temp.push_back(cnter);
				temp.push_back(threadInfo->input->at(x));
				// std::cout << threadInfo->start << " " <<(threadInfo->input->at(x)) << "\n";
			}
		}
		else // enter here if next char doesn't equal current char
		{
		    temp.push_back(cnter);
			temp.push_back(threadInfo->input->at(x));
			cnter=1;
			if(x == (threadInfo->input->size())-2) // you're 1 b4 the end of the vector, so last element needs to be pushed
			{
				temp.push_back(cnter);
				temp.push_back(threadInfo->input->at(x+1));
			}
		}
	}
    // for(int8_t c : temp)
	// {
	//     std::cout << threadInfo->start << " " << c<< " ";
	// }
	char sum = 0;
	int placeholder = -1;
	char flag2 = 0;
	for(int y=0; y<temp.size()-1; y+=2)
	{
	    if(temp[y]>1)
	    {
	        if(flag2 == 1)
	        {
	            flag2 = 0;
	            if(sum == 1)
				{
					threadInfo->output->at(placeholder) = sum;
				}
				else
				{
					threadInfo->output->at(placeholder) = sum*-1;
				}
	            sum = 0;
	            placeholder = -1;
	        }
	        threadInfo->output->push_back(temp[y]);
	        threadInfo->output->push_back(temp[y+1]);
	    }
	    else if(temp[y] == 1)
	    {
	        sum+=temp[y];
	        if(placeholder == -1)
	        {
	            placeholder = threadInfo->output->size();
	            if(placeholder == -1)
	                placeholder = 0;
	            threadInfo->output->push_back('P');
	            flag2 = 1;
	        }
	        if(sum == -128)
	        {
	            threadInfo->output->at(placeholder) = (sum+1);
	            placeholder = threadInfo->output->size();
	            threadInfo->output->push_back('P');
	            sum=1;
	        }
	        threadInfo->output->push_back(temp[y+1]);
	        if(y == temp.size()-2)
	        {
				if(sum == 1)
				{
					threadInfo->output->at(placeholder) = sum;
				}
				else
				{
					threadInfo->output->at(placeholder) = sum*-1;
				}
	        }
	    }
    }
    pthread_exit(NULL);
}

void RleData::Compress(const std::vector<int8_t>& input)
{
    mData.clear();
    mData.reserve(input.size() * 2);
    pthread_t threads[numThreads];
    struct ThreadInfo threadInfo[numThreads];

    int chunkSize = input.size() / numThreads;
    int remainder = input.size() % numThreads;
    int start = 0;

    for (int i = 0; i < numThreads; ++i)
    {
        int chunkEnd = start + chunkSize + (i < remainder ? 1 : 0);
        threadInfo[i].start = start;
        threadInfo[i].end = chunkEnd;
        threadInfo[i].input = (std::vector<int8_t>*)&input;
        threadInfo[i].output = &mData;
        // std::cout << start << " " << chunkEnd << "\n";
        pthread_create(&threads[i], nullptr, CompressThread, &threadInfo[i]);
        start = chunkEnd;
    }

    for (int i = 0; i < numThreads; ++i)
	{
        pthread_join(threads[i], nullptr);
    }
}

void* DecompressThread(void* arg)
{
    ThreadInfo* threadInfo = static_cast<ThreadInfo*>(arg);
    pthread_t threadId = pthread_self();

	for(int x=threadInfo->start; x<threadInfo->end; x+=2)
	{
		if(threadInfo->input->at(x) > 0)
		{
			for(int y=0; y<threadInfo->input->at(x); y++)
			{
				threadInfo->output->push_back(threadInfo->input->at(x+1));
			}
		}
		else
		{
			for(int z=0; z<(-1*threadInfo->input->at(x+1));z++)
			{
				threadInfo->output->push_back(threadInfo->input->at(x+1+z));
			}
			x = x + (threadInfo->input->at(x)*-1);
			x--;
		}
	}
    pthread_exit(NULL);
}

void RleData::Decompress(const std::vector<int8_t>& input, size_t outSize)
{
    mData.clear();
    mData.reserve(outSize);
    pthread_t threads[numThreads];
    struct ThreadInfo threadInfo[numThreads];

    int chunkSize = input.size() / numThreads;
    int remainder = input.size() % numThreads;
    int start = 0;

    for (int i = 0; i < numThreads; ++i)
    {
        int chunkEnd = start + chunkSize + (i < remainder ? 1 : 0);
        threadInfo[i].start = start;
        threadInfo[i].end = chunkEnd;
        threadInfo[i].input = (std::vector<int8_t>*)&input;
        threadInfo[i].output = &mData;
        // std::cout << start << " " << chunkEnd << "\n";
        pthread_create(&threads[i], nullptr, DecompressThread, &threadInfo[i]);
        start = chunkEnd;
    }
    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], nullptr);
    }
}

int main()
{
    struct timespec start, stop; 
    double time;
    std::vector<int8_t> test = {
			'a','a','a','b','b','b','c','c','c','*','*','*',
			'a','a','a','b','b','b','c','c','c',42,42,42,
		};
	RleData comp;
    RleData decomp;
    if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}
	comp.Compress(test);
    decomp.Decompress(comp.mData, 24);
    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}		
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;
	
	for(int8_t c : comp.mData)
	{
	    std::cout << c << " ";
	}
    std::cout << "\n";
    for(int8_t d : decomp.mData)
	{
	    std::cout << d << " " << "";
	}
    std::cout << "\n" << "Execution time for compress and decompress: " << time << "\n";
    return 0;
}
