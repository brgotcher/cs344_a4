#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#define CHARACTERS 50050

bool stopped = false;

// buffer1 is a shared resource between the input thread and the line separator thread
char buffer1[CHARACTERS];
// number of characters in the buffer
int count1 = 0;
// index where the input will insert the next character
int prodIndx1 = 0;
// index where the line separator will pick up the next character
int consIndx1 = 0;
// Initialize the mutex for buffer1
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full1 = PTHREAD_COND_INITIALIZER;

// buffer2 is a shared resource between the line separator thread and the plus sign thread
char buffer2[CHARACTERS];
// number of characters in the buffer
int count2 = 0;
// index where the input will insert the next character
int prodIndx2 = 0;
// index where the line separator will pick up the next character
int consIndx2 = 0;
// Initialize the mutex for buffer1
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full2 = PTHREAD_COND_INITIALIZER;

// buffer3 is a shared resource between the plus sign thread and the output thread
char buffer3[CHARACTERS];
// number of characters in the buffer
int count3 = 0;
// index where the input will insert the next character
int prodIndx3 = 0;
// index where the line separator will pick up the next character
int consIndx3 = 0;
// Initialize the mutex for buffer1
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full3 = PTHREAD_COND_INITIALIZER;

void add_to_buffer1(char *input) {
	pthread_mutex_lock(&mutex1);
	strncat(buffer1, input, strlen(input));
	count1 += strlen(input);
	prodIndx1 += strlen(input);
	pthread_cond_signal(&full1);
	pthread_mutex_unlock(&mutex1);

}

// input thread to scan input and insert into buffer
void *input() {
	printf("Enter input\n");
	fflush(stdout);
	char in[10001] = "1111111";
	while (strcmp(in, "STOP\n")) {
		fgets(in, 1001, stdin);
		printf("IN: %s\n", in);
		if (strcmp(in, "STOP\n") == 0) {
			stopped = true;
			pthread_cond_signal(&full1);
			break;
		} else {
			add_to_buffer1(in);
		}
	}

	printf("Buffer1: %s\n", buffer1);

	return NULL;
}

void get_buff1(char buff[]) {
	pthread_mutex_lock(&mutex1);
	while (count1 == 0 && stopped == false) {
		pthread_cond_wait(&full1, &mutex1);
	}
	if (count1 > 0 ) {
		char temp[] = "";
		printf("Count: %i\n", count1);
		printf("%i\n", consIndx1);
		printf("%c\n", buffer1[consIndx1]);
		sprintf(buff, "%c", buffer1[consIndx1]);
		

		consIndx1 += 1;
		count1 -= 1;
	}
	pthread_mutex_unlock(&mutex1);



}

void set_buff2(char *c) {
	pthread_mutex_lock(&mutex2);
	strncat(buffer2, c, 2);
	count2 += 1;
	consIndx2 += 1;
	pthread_cond_signal(&full2);
	pthread_mutex_unlock(&mutex2);
}

// line separator thread to replace line separators with spaces
void *line_separator() {
	char ch[] = " ";
	// iterate through buffer1 until end marker is reached

	while (stopped == false) {
		get_buff1(ch);
		printf("ch after get_buff1: %s\n", ch);
		if (strcmp(ch, "\n") == 0) {
			strcpy(ch, " ");
		}
		set_buff2(ch);
	}

	
	printf("Buffer2: %s\n", buffer2);
	

	return NULL;
}

void *plus_sign() {
	char c1 = ' ';
	char c2 = ' ';
	char toAdd = ' ';
	//while (c2 != 'Ç') {
	while (strncmp(c2, "Ç", 1)) {
		pthread_mutex_lock(&mutex2);
		while (count2 < 2) {
			pthread_cond_wait(&full2, &mutex2);
		}
		c1 = buffer2[consIndx2];
		c2 = buffer2[consIndx2 + 1];
		if (c1 == c2 && c1 == '+') {
			toAdd = '^';
			count2 = count2 - 2;
			consIndx2 = consIndx2 + 2;
		} else {
			toAdd = c1;
			count2--;
			consIndx2++;
		}
		pthread_mutex_unlock(&mutex2);

		pthread_mutex_lock(&mutex3);
		buffer3[prodIndx3] = toAdd;
		prodIndx3++;
		count3++;
		pthread_cond_signal(&full3);
		pthread_mutex_unlock(&mutex3);
	}

	pthread_mutex_lock(&mutex3);
	buffer3[prodIndx3] = toAdd;
	prodIndx3++;
	count3++;
	pthread_cond_signal(&full3);
	pthread_mutex_unlock(&mutex3);

	return NULL;

}

void *output() {
	char c = ' ';
	char line[80];
	int count = 0;
	// while (c != 'Ç') {
	while(strncmp(c, "Ç", 1)) {
		pthread_mutex_lock(&mutex3);
		while (count3 == 0) {
			pthread_cond_wait(&full3, &mutex3);
		}
		c = buffer3[consIndx3];
		if (count < 78) {
			line[count] = c;
			count++;
			consIndx3++;
			count3--;
		} else if (count == 78) {
			line[count] = c;
			line[count+1] = '\n';
			count = 0;
			consIndx3++;
			count3--;
			printf("%s", line);

		}
		pthread_mutex_unlock(&mutex3);

	}
	return NULL;

}




int main() {
	srand(time(0));
    pthread_t input_t, line_separator_t, plus_sign_t, output_t;
    // Create the threads
    pthread_create(&input_t, NULL, input, NULL);
    pthread_create(&line_separator_t, NULL, line_separator, NULL);
	//pthread_create(&plus_sign_t, NULL, plus_sign, NULL);
    //pthread_create(&output_t, NULL, output, NULL);
    // Wait for the threads to terminate
    pthread_join(input_t, NULL);
	printf("main Buffer1: %s", buffer1);
	pthread_join(line_separator_t, NULL);
    //pthread_join(plus_sign_t, NULL);
    //pthread_join(output_t, NULL);
	//printf("Buffer 2: %s", buffer2);
    return EXIT_SUCCESS;
}
