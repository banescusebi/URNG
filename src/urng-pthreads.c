/* Unpredictable Random Number Generator (URNG)
 * implemented using POSIX threads and PAPI
 * Copyright (c) 2012, Sebastian Banescu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or 
 * other materials provided with the distribution.
 * 
 * Neither the name of the author nor the names of its contributors may
 * be used to endorse or promote products derived from this software without 
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <papi.h>
#include <pthread.h>
 
#define NO_OF_THREADS 32
#define NO_OF_EVENTS 107
#define NO_OF_VALID_EVENTS 1

/* Variable declarations */
long_long values[NO_OF_EVENTS];
char ***filen;
unsigned char ***byte;
int num_hwcntrs;
int nrThreads;
int size; 
int rc;
int i = PAPI_TOT_IIS;
		
void handle_error(int retval)
{
	printf("Error %d\n", retval);
}

void *thread_routine(void *pt) {
	int EventSet = PAPI_NULL;
	int j = 0;
	int k;
	int t = (int) pt;
	char thId[2];
	PAPI_event_info_t info;
	
	int retCreate = PAPI_create_eventset(&EventSet);
	
	if (retCreate == PAPI_EINVAL)
		printf("The argument handle has not been initialized to PAPI_NULL or the argument is a NULL pointer.\n");
	else if (retCreate == PAPI_ENOMEM)
		printf("Insufficient memory to complete the operation.\n");
	else if (retCreate != PAPI_OK)
		printf("Error creating EventSet\n");
        
        	int retval = PAPI_get_event_info(i, &info);
	  	
	if (retval == PAPI_OK) 
	{  
	    	printf("%-30s 0x%-10x\n%s\n", info.symbol, info.event_code, info.long_descr);
	    	if (PAPI_add_event(EventSet, i) == PAPI_OK)
		{
		// Start counting events
		int retStart =  PAPI_start(EventSet);
		if (retStart == PAPI_EINVAL)
			printf("Invalid argument\n");
		else if (retStart == PAPI_ESYS)
			printf("C library call failed\n");
		else if (retStart == PAPI_ENOEVST)
			printf("The EventSet specified does not exist. \n");
		else if (retStart == PAPI_EISRUN)
			printf("The EventSet is currently counting events.\n");
		else if (retStart == PAPI_ECNFLCT)
			printf("The underlying counter hardware can not count this event and other events in the EventSet simultaneously.\n");
		else if (retStart == PAPI_ENOEVNT)
			printf("The PAPI preset is not available on the underlying hardware\n");	
		else if (retStart != PAPI_OK)	
			handle_error(1);
		else
			printf("** Started counting\n");
		
		for(k=0; k<size; k++)
		{
			int retRead = PAPI_read(EventSet, values);
		
			if (retRead == PAPI_EINVAL)
				printf("One or more of the arguments is invalid.\n");
			else if (retRead == PAPI_ESYS)
				printf("A system or C library call failed inside PAPI, see the errno variable.\n");
			else if (retRead == PAPI_ENOEVST)
				printf("The event set specified does not exist. \n");
			else if (retRead != PAPI_OK)
				printf("Error reading counters\n");
			else
				byte[j][k][t] = values[0];
		}
		
		// Stop counting events 
		if (PAPI_stop(EventSet, values) != PAPI_OK)
			printf("Error stoping counters\n");
		
		int retRemove = PAPI_remove_event(EventSet, i);
		if (retRemove == PAPI_EISRUN)
			printf("The EventSet is currently counting events.\n");
		else if (retRemove == PAPI_ECNFLCT)
			printf("The underlying counter hardware can not count this event and other events in the EventSet simultaneously.\n");
		else if (retRemove == PAPI_EINVAL)
			printf("One or more of the arguments is invalid.\n");
		else if (retRemove == PAPI_ENOEVST)
			printf("The EventSet specified does not exist. \n");
		else if (retRemove != PAPI_OK)
			printf("Error on removing event\n");
		}
		else
			printf("Event not counted\n");
	}
	else if (retval == PAPI_EINVAL)
		printf("One or more of the arguments is invalid.\n");
	else if (retval == PAPI_ENOTPRESET)
		printf("The PAPI preset mask was set, but the hardware event specified is not a valid PAPI preset.\n");
	else if (retval == PAPI_ENOEVNT)
		printf("The PAPI preset is not available on the underlying hardware.\n");
	pthread_exit(NULL);
}


int main(int argc, char *argv[])
{	
	/* Variable declaration */
	int j, k, t, rc;
	int retval;
	unsigned char bytexor;
	pthread_t threads[NO_OF_THREADS];
	
	/* Initialize the library */
	retval = PAPI_library_init(PAPI_VER_CURRENT);
	
	if (retval != PAPI_VER_CURRENT && retval > 0) 
	{
		fprintf(stderr,"PAPI library version mismatch!\n");
		exit(1); 
	}
	
	if (retval < 0)
		handle_error(retval);
	
	retval = PAPI_is_initialized();
	
	if (retval != PAPI_LOW_LEVEL_INITED)
		handle_error(retval);
	
	/*  The installation does not support PAPI */
	if ((num_hwcntrs = PAPI_num_counters()) < 0 )
		handle_error(1);
	
	/*  The installation supports PAPI, but has no counters */
	if ((num_hwcntrs = PAPI_num_counters()) == 0 )
		fprintf(stderr,"Info:: This machine does not provide hardware counters.\n");

	printf("The number of counters is %d\n", num_hwcntrs);
	
	/* Variable initialization */
	num_hwcntrs = 2;
	size = atoi(argv[1]);
	nrThreads = atoi(argv[2]);
	i = atoi(argv[4]) + 0x80000000; 	
	byte = malloc(sizeof(unsigned char**)*NO_OF_EVENTS); 
	filen = malloc(sizeof(char**)*NO_OF_VALID_EVENTS);
	
	PAPI_multiplex_init();
	
	/* Memory allocation for byte and filen */
	for (j=0; j<NO_OF_VALID_EVENTS; j++)
	{
		filen[j] = malloc(sizeof(char*)*nrThreads); 
		for (t=0; t<nrThreads; t++)
			filen[j][t] = malloc(sizeof(char)*30); 
		byte[j] = malloc(sizeof(unsigned char*)*size);
		for (k=0; k<size; k++)
		{
			byte[j][k] = malloc(sizeof(unsigned char)*nrThreads);
			for (t=0; t<nrThreads; t++)
				byte[j][k][t] = 0;
		}	
	}

	/* Span worker threads */
	for (t=0; t<nrThreads; t++)
	{
		printf("Starting thread %d\n", t);
		rc = pthread_create(&threads[t], NULL, thread_routine, (void *)t);
		if (rc)
		{
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
	
	/* Wait for threads to finish */
	for (t=0; t<nrThreads; t++)
	{
		pthread_join(threads[t], NULL);
		printf("Thread %d joined\n", t);
	}
	
	printf("OUT OF THE PARALLEL REGION\n");
	/* Write bytes in output file */
	for (j=0; j<NO_OF_VALID_EVENTS; j++)
	{
		FILE *fd = fopen(argv[3], "w");
		
		for (k=0; k<size; k++)
		{	
			for (t=0; t<nrThreads; t++)
				bytexor = bytexor ^ byte[j][k][t];
					
			fwrite(&bytexor,sizeof(unsigned char),1,fd);
		}
		fclose(fd);
	}
		
	return 0;
}
