/*
Sophie Landver
CIS 314 Fall 2015 Lab 3
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>

#define FILE_NAME "address.txt" 

int misses = 0;
int hits = 0;
int totalProcessed = 0;
char list[] = "0123456789ABCDEF";

struct cacheLine {
	int index; 
	char tag[7+1];	 
	int valid; 		 
	char data_low[10+1]; 
	char data_high[10+1];

};

int LinesInFile(char * in_file_name, FILE * in_file)
{
	/* This function returns the number of lines in a file. 
	Inputs: char * in_file_name: the string name of the file 
           	FILE * in_file: a FILE pointer 
    Returns: the integer amount of lines in the file. 
	*/
	int line_count = 0;
	char character; 
	in_file = fopen(in_file_name, "r");

	for(character = getc(in_file); character != EOF; character = getc(in_file))
	{
		if (character == '\n') 
		{
			line_count = line_count + 1;
		}
	}
 
    fclose(in_file);

    return line_count;

}

void ReadFileIntoArray(char ** address_array, int size, char * in_file_name, FILE * in_file)
{
	/* This function reads string addresses from a file and stores them in an array. 
	Inputs:	char ** address_array: the array in which the string addresses are stored in. 
			int size: the number of string addresses in the file 
			char * in_file_name: the string name of the file 
           	FILE * in_file: a FILE pointer 
    Returns: nothing. 
	*/

   in_file = fopen(in_file_name, "r"); 
   int i;
   for(i=0; i<size; i++)   
   {
   		fscanf(in_file, "%s", address_array[i]);
   }
   fclose(in_file);
}


void PrintCacheInfo()
{	
	/* This function prints out the current value of number of hits, number of 
	misses, hit rate, and miss rate. 
	Inputs: none.
	Returns: nothing.
	*/
	printf("Number of hits: %d\n", hits);
	printf("Number of misses: %d\n", misses);
	printf("Hit Rate: %d/%d\n", hits, totalProcessed);
	printf("Miss Rate: %d/%d\n", misses, totalProcessed);
}

void PrintCurrentCacheState(struct cacheLine * cache)
{
	/* This functions prints out the current state of the cache. 
	Inputs: struct cacheLine * cache: the array of 8 cacheLine structs 
	Returns: nothing. 
	*/
	printf("<Cache State>:\n");
	printf("\n");
	printf("Index     Tag         Valid   Data\n");
	int i;
	for(i=0; i<8; i++)
	{
		printf("%d         %s     %d       MEM[%s : %s]\n",cache[i].index, cache[i].tag, cache[i].valid, cache[i].data_low, cache[i].data_high);
	}

}



long ToDecimal(char * in_number, int in_base) 
{
	/* This function converts a number in any base from binary to hexadecimal 
	to a decimal number. 
	Inputs: char * in_number: the number to be converted
			int in_base: the base of which in_number is to be converted to
	Returns: the decimal number.
	*/

	long decimal = 0;
	int j = 0;
	int i;
	for (i=(strlen(in_number)-1); i>=0; i--)
	{
		char *pointer = strchr(list, in_number[j]);
		int index = pointer - list; // index at which the cofficient is in list
		long newpart =  index * (pow(in_base, i));
		decimal = decimal + newpart;
		j++;
	}

	return decimal;
}

int DecimalToDesiredBase(long decimal, long out_base, char * out_number)
{
	/* This function converts a decimal number to any user-defined base (base can be any
	base from binary t hexadecimal). 
	Inputs: long decimal: the decimal number to be converted
			long out_base: the base of which decimal is to be converted to
			char * out_number: representation of the decimal number in base out_base
	Returns: 0 always.  
	*/

	if (decimal == 0) 
	{ return 0; } //base case

	long quotient = decimal/out_base;
	int rem = decimal % out_base; 
	char fixed_rem = list[rem]; 
	*out_number = fixed_rem; 
	return DecimalToDesiredBase(quotient, out_base, out_number-1);
}

void FillOutNumber(char * out_number, int size)
{
	/* This function fills the last element in out_number with a null character and fills
	the rest of the elements in out_number with zeros. 
	Inputs: char * out_number: the number to be filled 
			int size: the size of out_number
	Returns: nothing. 
	*/
	int i;
	for (i=0; i<(size); i++)
	{
		if (i == (size) -1)
		{
			out_number[i] = '\0';
		}
		else
		{
			out_number[i] = '0';
		}
	}
}

void AdjustDataLowAndDataHigh(char * data_low, char * data_high, long decimal_address)
{
	/* This function adjusts the strings data_high and data_low to be the correct high
	and low points of the chunk of data to be brought into the cache. 
	Inputs: char * data_low: the string in which the low point of the chuck of data will be stored
			char * data_high: the string in which the high point of the chuck of data will be stored
			long decimal_address: the address to be fetched, in base 10. 
	Returns: nothing. 
	*/
	long mem_block = decimal_address/16;
	long low_base_ten = mem_block * 16;
	long high_base_ten = low_base_ten + 15;

	char low_unformatted[8+1];
	FillOutNumber(low_unformatted, (8+1));
	DecimalToDesiredBase(low_base_ten, 16, (low_unformatted + 7));

	char high_unformatted[8+1];
	FillOutNumber(high_unformatted, (8+1));
	DecimalToDesiredBase(high_base_ten, 16, (high_unformatted + 7));

	data_low[0] = '0';
	data_low[1] = 'x';
	data_low[10] = '\0';
	data_high[0] = '0';
	data_high[1] = 'x';
	data_high[10] = '\0';
	int i;
	for(i=2; i<10; i++)
	{
		int j = i-2;
		data_low[i] = low_unformatted[j];
	}

	for(i=2; i<10; i++)
	{
		int j = i-2;
		data_high[i] = high_unformatted[j];
	}
}

void ProcessAddress(char * address, struct cacheLine * testcache)
{
	/* This function proccesses an address; i.e. this function calulates the index and tag 
	of the address and then finds out whether this address is already stored in the cache or not. 
	If the address is not stored in the cache already, then it places the appropriate chunk of data in
	the cache, with the correct tag and index. 
	Inputs:	char * address: the address 
			struct cacheLine * testcache: the array of 8 cacheLine structs
	Returns: nothing.
	*/
	totalProcessed = totalProcessed + 1;
	char fixed_address[8+1];
	strncpy(fixed_address, address+2, 8);
	fixed_address[8] = '\0';
	
	char *pointer = strchr(list, fixed_address[6]);
	int base_ten = pointer - list; // index at which the cofficient is in list, which is base 10 number of fixed_array[6]
	int cur_index = base_ten % 8;
	//HAVE INDEX NOW

	char cur_tag[7+1];
	FillOutNumber(cur_tag, (7+1)); 
	long decimal_address = ToDecimal(fixed_address, 16);
	long x = decimal_address/(pow(2, 7)); //
	DecimalToDesiredBase(x, 16, (cur_tag + 6));
	//HAVE TAG NOW 

	if ((testcache[cur_index]).valid == 0)
	{
		//not in cache 
		misses = misses + 1;
		testcache[cur_index].index = cur_index;
		testcache[cur_index].valid = 1;
		strcpy(testcache[cur_index].tag, cur_tag);
	
		//format data low and high, with the "0x"
		char data_low[10+1];
		char data_high[10+1];
		AdjustDataLowAndDataHigh(data_low, data_high, decimal_address);
		//now put data_low and data_high in cacheLine
		strcpy(testcache[cur_index].data_low, data_low);
		strcpy(testcache[cur_index].data_high, data_high);
	}

	else
	{
		//index is in cache
		int value = strcmp((testcache[cur_index].tag), cur_tag);
		if (value == 0) 
		{
			//tag matches
			hits = hits + 1;
			//DONE
		}
		else
		{
			//tag doesnt match 
			misses = misses + 1;
			strcpy(testcache[cur_index].tag, cur_tag); //replace tag
			//format data low and high, with the "0x"
			char data_low[10+1];
			char data_high[10+1];
			AdjustDataLowAndDataHigh(data_low, data_high, decimal_address);
			//now put data_low and data_high in cacheLine
			strcpy(testcache[cur_index].data_low, data_low);
			strcpy(testcache[cur_index].data_high, data_high);
		}
	}


}



int main(){
	// This function emulates a Direct Mapped Cache

	FILE *in_file; 
  	char in_file_name[] = FILE_NAME; 
  	int size;
  	size = LinesInFile(in_file_name, in_file); 

  	char ** address_array = malloc(size*sizeof(char*));
  	int i;
  	for(i=0; i<size; i++)
  	{
  		address_array[i] = malloc(11*sizeof(char));
  	}

  	ReadFileIntoArray(address_array, size, in_file_name, in_file);  

  	struct cacheLine cache[8]; //make an array of type cacheLine of size 8
  	for(i=0; i<8; i++)
	{
		cache[i].index = 0;
		cache[i].valid = 0;
		strcpy(cache[i].tag, "0000000");
		strcpy(cache[i].data_low, "0000000000");
		strcpy(cache[i].data_high, "0000000000");
	}
	printf("\n");
	printf("Performing the first 10 cache references...\n");


  	for(i=0; i<10; i++) 
  	{
  		ProcessAddress(address_array[i], cache); 
  	}
  	PrintCacheInfo();
  	printf("\n");
  	PrintCurrentCacheState(cache);

  	printf("\n");
  	printf("Performing the remaining cache references...\n");


  	for(i=10; i<size; i++)
  	{
  		ProcessAddress(address_array[i], cache); 
  	}
  	PrintCacheInfo();
  	printf("\n");
  	PrintCurrentCacheState(cache);


  	for(i=0; i<size; i++)
  	{
  		free(address_array[i]);
  	}

  	free(address_array);



return 0;
}