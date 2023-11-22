#include <ostream>
#include <vector>
#include <iostream>
#include <pthread.h>

// RLE Compression/Decompression
struct RleData
{
	// Stores either compressed or decompressed data
	std::vector<int8_t> mData;
	
	// Compresses input data and stores it in mData
	void Compress(const std::vector<int8_t>& input);

	// Decompresses input data and stores it in mData
	void Decompress(const std::vector<int8_t>& input, size_t outSize);
    
	// Outputs mData
	friend std::ostream& operator<< (std::ostream& stream, const RleData& rhs);
	
	static size_t MaxRunSize() { return 127; }
};

void RleData::Compress(const std::vector<int8_t>& input)
{
	mData.clear();
	mData.reserve(input.size()*2);
	char cnter = 1;
	char flag = 0; //meaning checking for continous different chars
	std::vector<int8_t> temp;
	temp.reserve(input.size()*2);
	for(int x=0; x<input.size()-1; x++)
	{
		
		if(input[x] == input[x+1])
		{
			cnter++;
			if(cnter == -128)
			{
				temp.push_back(((cnter+1)*-1));
				cnter=1;
				temp.push_back(input[x]);
			}
			if(x==input.size()-2) // you're 1 b4 the end of the vector, so last element needs to be pushed
			{
				temp.push_back(cnter);
				temp.push_back(input[x]);
			}
		}
		else // enter here if next char doesn't equal current char
		{
		    temp.push_back(cnter);
			temp.push_back(input[x]);
			cnter=1;
			if(x==input.size()-2) // you're 1 b4 the end of the vector, so last element needs to be pushed
			{
				temp.push_back(cnter);
				temp.push_back(input[x+1]);
			}
		}
	}
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
					mData[placeholder] = sum;
				}
				else
				{
					mData[placeholder] = sum*-1;
				}
	            sum = 0;
	            placeholder = -1;
	        }
	        mData.push_back(temp[y]);
	        mData.push_back(temp[y+1]);
	    }
	    else if(temp[y] == 1)
	    {
	        sum+=temp[y];
	        if(placeholder == -1)
	        {
	            placeholder = mData.size();
	            if(placeholder == -1)
	                placeholder = 0;
	            mData.push_back('P');
	            flag2 = 1;
	        }
	        if(sum == -128)
	        {
	            mData[placeholder] = (sum+1);
	            placeholder = mData.size();
	            mData.push_back('P');
	            sum=1;
	        }
	        mData.push_back(temp[y+1]);
	        if(y == temp.size()-2)
	        {
				if(sum == 1)
				{
					mData[placeholder] = sum;
				}
				else
				{
					mData[placeholder] = sum*-1;
				}
	        }
	    }
	}
	
}

void RleData::Decompress(const std::vector<int8_t>& input, size_t outSize)
{
	mData.clear();
	mData.reserve(outSize);
	for(int x=0; x<input.size(); x+=2)
	{
		if(input[x] > 0)
		{
			for(int y=0; y<input[x]; y++)
			{
				mData.push_back(input[x+1]);
			}
		}
		else
		{
			for(int z=0; z<(-1*input[x]);z++)
			{
				mData.push_back(input[x+1+z]);
			}
			x = x + (input[x]*-1);
			x--;
		}

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
	    std::cout << c << " " << "";
	}
    std::cout << "\n";
    for(int8_t d : decomp.mData)
	{
	    std::cout << d << " " << "";
	}
    std::cout << "\n" << "Execution time for compress and decompress: " << time << "\n";
    return 0;
}
