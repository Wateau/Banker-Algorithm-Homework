#include<stdio.h> 
#include<stdbool.h>
#include<pthread.h>
#include <math.h>
#include <stdlib.h>


#define MAX_COUNT 6
#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3


int available[NUMBER_OF_RESOURCES]; 
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = {{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = {{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};

int request_resource(int customer_num, int request[]);
int release_resources(int customer_num, int release[]);
void run(void* arg);
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
bool isSafe(int customer_num, int request[]);
void printAllocationTable();
void printAvailable();
void printNeedTable();
struct info{
	int consumer_No;
};

int main(int argc, char *argv[]){
	int i = 0;
	for(i = 1 ; i < argc ; i++){
		available[i-1] = atoi(argv[i]);
	} 
	pthread_t thread[NUMBER_OF_CUSTOMERS];
	struct info information[NUMBER_OF_CUSTOMERS]; 
	
	for(i = 0 ; i < NUMBER_OF_CUSTOMERS ; i++){
		information[i].consumer_No = i;
	}
	for(i = 0; i < NUMBER_OF_CUSTOMERS ; i++){
		 pthread_create(&thread[i], NULL, (void *)&run, &information[i]);
	}
	for(i = 0; i < NUMBER_OF_CUSTOMERS ; i++){
		 pthread_join(thread[i], NULL);
	}
	printAvailable();
	printAllocationTable();
	printNeedTable();
	return 0;
}

void run(void* arg){
	struct info* param = (struct info*)arg;
    int customer_No = param->consumer_No; 
    int resourceList[NUMBER_OF_RESOURCES];
    int count = 0;
	for(count = 0 ; count < MAX_COUNT ; count++){
	    bool allZero = true;
	    int i = 0;
	    if(count < MAX_COUNT/2){
		    for(i = 0 ; i < NUMBER_OF_RESOURCES ; i++){
		    	srand((unsigned)time(NULL));
		    	if(need[customer_No][i] == 0){
		    		resourceList[i] = 0;
				}	else{
					resourceList[i] = rand() % need[customer_No][i];
				}
		    	if(resourceList[i] != 0){
		    		allZero = false;
				}
			}
			if(allZero){
				int k = 0;
				for(k = 0 ; k < NUMBER_OF_RESOURCES ; k++){
					resourceList[k] = need[customer_No][k];
				}
			}
			
		} else {
			int k = 0;
				for(k = 0 ; k < NUMBER_OF_RESOURCES ; k++){
					resourceList[k] = need[customer_No][k];
				}
		}
	    pthread_mutex_lock(&lock);
	    printf("Customer %d requests resources [%d,%d,%d]",customer_No,resourceList[0],resourceList[1],resourceList[2]);
		int result = request_resource(customer_No,resourceList);
		if(result == 0){
			printf("  Successfully !\n");
		}else if(result == -1){
			printf("   Failed !\n");
		}
		pthread_mutex_unlock(&lock);
		sleep(1);
		bool judge = true;
		for(i = 0; i < NUMBER_OF_RESOURCES ; i++){
			if(need[customer_No][i] != 0)
				judge = false;
		}
		if(judge){
			pthread_mutex_lock(&lock);
			release_resources(customer_No,allocation[customer_No]);
			printf("Cuctomer %d completed the task and release the resources successfully!\n",customer_No);
			pthread_mutex_unlock(&lock);
			break;
		}
	} 
}

int request_resource(int customer_num, int request[]) {
	int i = 0;
	//判断此次请求的资源量是否超过最大需求 
	 for(i = 0 ; i < NUMBER_OF_RESOURCES; i++){
		if(request[i] + allocation[customer_num][i]> maximum[customer_num][i])
			return -1;
	}
	//判断请求的资源量是否超过当前可以提供的资源 
	for(i = 0 ; i < NUMBER_OF_RESOURCES; i++){
		if(request[i] > available[i]){
			return -1;	
		}
	}
	if(isSafe(customer_num, request)){
		int j = 0;
		for(j = 0 ; j < NUMBER_OF_RESOURCES ; j++) {
			available[j] = available[j] - request[j];
			allocation[customer_num][j] = allocation[customer_num][j] + request[j];
			need[customer_num][j] = need[customer_num][j] - request[j];
		}
		return 0;
	} else{
		return -1;
	}
	return -1;
}

int release_resources(int customer_num, int release[]) {
	int i = 0;
	bool judge = true;
	for(i = 0 ; i < NUMBER_OF_RESOURCES ; i++){
		if(release[i] != maximum[customer_num][i])
			return -1;
	}
	for(i = 0 ; i < NUMBER_OF_RESOURCES ; i++){
		available[i] += release[i];
		allocation[customer_num][i] = 0;
		need[customer_num][i] = 0;
	}
	return 0;
}

bool isSafe(int customer_num, int request[]){
	int available_copy[NUMBER_OF_RESOURCES];
	int allocation_copy[NUMBER_OF_RESOURCES];
	int need_copy[NUMBER_OF_RESOURCES];
	int i = 0;
	for(i = 0 ; i < NUMBER_OF_RESOURCES ; i++) {
		available_copy[i] = available[i];
		allocation_copy[i] = allocation[customer_num][i] + request[i];
		need_copy[i] = need[customer_num][i] - request[i];
	}
	
	//标记数组记录该客户是否已经完成分配 
	int flag[NUMBER_OF_CUSTOMERS];
	for(i = 0 ; i < NUMBER_OF_CUSTOMERS ; i++){
		flag[i] = 0;
	}
	
	for(i = 0 ; i < NUMBER_OF_RESOURCES ; i++){
		available_copy[i] = available_copy[i] - request[i];
	} 
	bool judge = false;//用来判断该轮检测中是否存在可分配的客户 
	for(i = 0 ; i < NUMBER_OF_CUSTOMERS ; i++){
		if(i == 0){
			int k = 0;
			judge = false;
			for(k = 0 ; k < NUMBER_OF_CUSTOMERS ; k++){
				if(flag[k] == 0){	
					break;
				}
				if(k == NUMBER_OF_CUSTOMERS - 1){
					return true; 
				}
			}
		} 
		if(flag[i] == 1){
			continue;
		}
		int j = 0;
		if(i != customer_num){
			for(j = 0 ; j < NUMBER_OF_RESOURCES ; j++){
				if(need[i][j] > available_copy[j]){
					break;
				}
				if(j == NUMBER_OF_RESOURCES - 1){
					int k = 0;
					for( k = 0 ; k < NUMBER_OF_RESOURCES ; k++ ){
						available_copy[k] = available_copy[k] + allocation[i][k]; 
					}
					flag[i] = 1;
					judge = true;
				}
			}
		} else {
			for(j = 0 ; j < NUMBER_OF_RESOURCES ; j++){
				if(need_copy[j] > available_copy[j]){
					break;
				}	
				if(j == NUMBER_OF_RESOURCES - 1){
					int k = 0;
					for( k = 0 ; k < NUMBER_OF_RESOURCES ; k++ ){
						available_copy[k] = available_copy[k] + allocation_copy[k]; 
					}
					flag[i] = 1;
					judge = true;
				}
			}
		}
		if(i == NUMBER_OF_CUSTOMERS - 1) {
			if(!judge){
				return false;
			}
			i = -1; 
		}
	} 
	int k = 0;
	for(k = 0 ; k < NUMBER_OF_CUSTOMERS ; k++){
		if(flag[k] == 0){	
			break;
		}
		if(k == NUMBER_OF_CUSTOMERS - 1){
			return true; 
		}
	}
	return false;
}
void printAllocationTable(){
	int i,j;
	printf("----Allocation Table----\n");
	printf("+----------------------+\n") ;
	printf("| Customer | Resources |\n");
	printf("+----------+-----------+\n") ;
	for(i = 0 ; i < NUMBER_OF_CUSTOMERS ; i ++){
		printf("|       %d  |",i);
		for(j = 0 ; j < NUMBER_OF_RESOURCES ; j++){
			printf(" %d |",allocation[i][j]);
		}
		printf("\n");
		printf("+----------+-----------+\n") ;
	}
}
void printAvailable(){
	printf("----Current Available----\n");
	int i;
	printf("[");
	for(i = 0 ; i < NUMBER_OF_RESOURCES ; i++){
		printf(" %d ",available[i]);
	}
	printf("]\n");
}
void printNeedTable(){
	int i,j;
	printf("-------Need Table-------\n");
	printf("+----------------------+\n") ;
	printf("| Customer | Resources |\n");
	printf("+----------+-----------+\n") ;
	for(i = 0 ; i < NUMBER_OF_CUSTOMERS ; i ++){
		printf("|       %d  |",i);
		for(j = 0 ; j < NUMBER_OF_RESOURCES ; j++){
			printf(" %d |",need[i][j]);
		}
		printf("\n");
		printf("+----------+-----------+\n") ;
	}
}
