#include <bits/stdc++.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <random>
#include <time.h>
#include <mutex>
#include <atomic>
#include <fstream>
using namespace std;

atomic <int> total_count{0};
atomic <int> check_sleep{0};
atomic <int> tokenProcess{0};
deque <int> processQ;
int* LN;
mutex mtx_global;

mutex print_lock;

atomic <int> messageComplexity{0};
double timeComplexity;
time_t rawtime;

ofstream outfile;

FILE *fp;

void error(const char* msg)        // for displaying errors if needed
{
	perror(msg);
	exit(1);
}

// returns the exponential decay for lambda
double run_exp(float lambda)
{
    default_random_engine generate;
    exponential_distribution <double> distribution(1.0/lambda);
    return distribution(generate);
}

void receiver(int index, int max_clientSize, std::vector <int> nodes, int RN[], int size_vClock, std::mutex *mtx, int alpha, int beta, bool* granted, int* counter)
{
	
		int PORT = 3000+index;
		
		int server_fd, new_socket, valread;

	    struct sockaddr_in address;

	    int opt = 1;

	    int addrlen = sizeof(address);


	    char hello[] = "Hello from receiver";
	      
	    
	    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	    {

	        perror("socket failed");

	        exit(EXIT_FAILURE);
	    }
	      

	    
	    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
	                                                  &opt, sizeof(opt)))
	    {

	        perror("setsockopt");

	        exit(EXIT_FAILURE);

	    }

	    address.sin_family = AF_INET;

	    address.sin_addr.s_addr = INADDR_ANY;

	    address.sin_port = htons( PORT );
	      
	   

	    if (bind(server_fd, (struct sockaddr *)&address, 
	                                 sizeof(address))<0)
	    {

	        perror("bind failed");

	        exit(EXIT_FAILURE);
	    }
	    // sockets start listening here
	    if (listen(server_fd, 4) < 0)
	    {

	        perror("listen");

	        exit(EXIT_FAILURE);
	    }


	    check_sleep--;

	    vector <int> client_list;
	    int max_client = max_clientSize;
	    int count = 0;

	    // while there are clients to be accepted.
	    while(count < max_client)
	    {

	    	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
	                       (socklen_t*)&addrlen))<0)
	    	{
	       		perror("accept");
	        	exit(EXIT_FAILURE);
	    	}
	    	else
	    	{
	    		client_list.push_back(new_socket);
	    		count++;
	    	}
	    }
	    //fprintf(fp,"All connection established for client = %d\n", index);

	    char buffer[5000] = {0};
	    time_t now = time(0);
	    
	    // this runs if there are still any messages left to receive.
	    while(total_count>0)
	    {
	    	
			
	    	for(int i=0; i<max_client ;i++)
	    	{
				
	    		bzero(buffer,5000);

	    		if(total_count == 0)
	    		{
	    			//fprintf(fp,"\nreturned receiver index = %d\n", index);
	    			return;
	    		}

	    		valread = read(client_list[i] , buffer, 5000);

		    	if(valread < 0)
		    	{

		    		error("error on reading..");
		    	}
		    	else
		    	{
		    		// message not null
		    		if(strlen(buffer)>0)
		    		{
			    		mtx->lock();
				    				    		
			    		int pos = 0;
						
						
                        while(pos < strlen(buffer))
                        {
							
							
                            int ptr = 0,value=0;
							
                            if(buffer[pos] == '#')
                            {
								print_lock.lock();
                                outfile << " Process " << index << " received token " << endl;
								print_lock.unlock();
                                tokenProcess = index;
								 pos++;
                            }
                            else
                            {
                                while(buffer[pos] != ' ')
                                {
                                    ptr = ptr*10 + (int)buffer[pos]-48;
                                    pos++;
                                }
                                pos++;
								
                                while(buffer[pos] != ':')
                                {
                                    value = value*10 + (int)buffer[pos]-48;
                                    pos++;
                                }
                                pos++;

								//printf(" The value parsed from string = %d and %d\n", ptr, value);

                                if(RN[ptr] > value)
                                {

                                }
                                else
                                {
                                    RN[ptr] = max(RN[ptr], value);
                                    
                                }

								
                            }
                        }
						
			   			mtx->unlock();

		    		}
		    	}
		    	
	    	}
	    	if(total_count == 0)
		    {
		    	//fprintf(fp,"receiver to exit= %d\n", index);
		    	break;
		    }
	    	
	    }
	    outfile << "Exiting receiver = " << index << endl;
	    return;
}

void sender(int index, std::vector <int> nodes, int n, int k, int RN[], std::mutex *mtx,  int alpha, int beta, bool *granted,  int* counter)
{

	while(check_sleep > 0);
	int max_connection = nodes.size();
	
	struct sockaddr_in address;
    int sock[max_connection], valread;
    //struct sockaddr_in serv_addr[max_connection];

    for(int i = 0;i<max_connection;i++)
    {
    	int PORT = 3000+nodes[i];
    
	    struct sockaddr_in serv_addr;
	    char buffer[5000] = {0};
	    if ((sock[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    {
	        printf("\n Socket creation error \n");
	    }
	  
	    memset(&serv_addr, '0', sizeof(serv_addr));
	  
	    serv_addr.sin_family = AF_INET;
	    serv_addr.sin_port = htons(PORT);
	      
	    // Converting IPv4 and IPv6 addresses from text to binary form
	    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	    {
	        printf("\nInvalid address/ Address not supported \n");
	    }
	  
	    if (connect(sock[i], (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	    {
	        printf("\nConnection Failed index = %d\n",index);
	    }
    }

   

    time_t now = time(0);
	
	for(int p1 = 1;p1<=k;p1++)
    {
		if(total_count <=0)break;

      	this_thread::sleep_for(chrono::milliseconds(alpha));
        
		tm *ltm = localtime(&now);
		print_lock.lock();
		outfile << "Process " << index << " requesting CS at "  << ltm->tm_min << ":" <<ltm->tm_sec << endl;
		print_lock.unlock();
		time_t reqTime = time(0);
		timeComplexity += (-reqTime);
	    mtx->lock();
	    RN[index]++;
		mtx->unlock();
        if(tokenProcess != index)
        {
			
			std::string s = std::to_string(index)+ " " +std::to_string(RN[index])+":";
        	char hello[s.length()];
        	strcpy(hello, s.c_str());
            for(int j = 0;j<max_connection;j++){
				messageComplexity++;
                write(sock[j], hello, strlen(hello));
				this_thread::sleep_for(chrono::milliseconds(alpha));
			}
			
			while(tokenProcess!=index)
			{
				if(total_count <=0)break;
			}
			
        }
        if(total_count <=0)break;
		
		this_thread::sleep_for(chrono::milliseconds((int)run_exp(beta)));
		mtx->lock();

		
		tokenProcess = index;

		ltm = localtime(&now);
		print_lock.lock();
		outfile << "Process " << index << " entering CS at "  << ltm->tm_min << ":" <<ltm->tm_sec << endl;
		print_lock.unlock();
		time_t reqTime1 = time(0);
		timeComplexity += reqTime1;
		
		ltm = localtime(&now);
		print_lock.lock();
		outfile << "Process " << index << " exiting CS at "  << ltm->tm_min << ":" <<ltm->tm_sec << endl;
		print_lock.unlock();

		total_count--;

		

		mtx_global.lock();

		LN[index] = RN[index];
		for(int j=1;j<=n;j++)
		{
			if(find(processQ.begin(), processQ.end(), j) == processQ.end())
			{
				if(RN[j] == LN[j]+1)
					processQ.push_back(j);
			}
		}

		
		if(processQ.size()>0)
		{
			
			int temp,ind=-1;
			temp = processQ.front();
			processQ.pop_front();

			for(int j=0;j<max_connection;j++)
			{
				if(nodes[j] == temp)
				{
					ind = j;
					break;
				}
			}
			if(ind == -1)
			{
				//printf("This should not happen \n\n\n\n");
			}

			print_lock.lock();
			outfile << "Process " << index << " sending token to process = " << temp  << endl;
            print_lock.unlock();
			
			

			char hello1[] = "#";
			messageComplexity++;
            write(sock[ind], hello1, strlen(hello1));
			tokenProcess = temp;
			
        }

		mtx_global.unlock();

		this_thread::sleep_for(chrono::milliseconds((int)run_exp(beta)));
    	mtx->unlock();
    	    

    }
	
	int looper = 0;
	while(total_count>0)
	{
		
		this_thread::sleep_for(chrono::seconds((int)run_exp(beta)));
		mtx->lock();
		looper++;
		if(looper == 100)
            total_count = 0;
		if(tokenProcess == index)
		{
			mtx_global.lock();

			for(int j=1;j<=n;j++)
			{
				if(find(processQ.begin(), processQ.end(), j) == processQ.end())
				{
					if(RN[j] == LN[j]+1)
						processQ.push_back(j);
				}
			}

			
			if(processQ.size()>0)
			{
				int temp,ind;
				temp = processQ.front();
				processQ.pop_front();

				for(int j=0;j<max_connection;j++)
				{
					if(nodes[j] == temp)
					{
						ind = j;
						break;
					}
				}
				print_lock.lock();
				outfile << "Process " << index << " sending token to process " << temp << endl;
				print_lock.unlock();
				char msg[] = "#";
				messageComplexity++;
				write(sock[ind], msg, strlen(msg));
				tokenProcess = temp;
				
			}

			mtx_global.unlock();

		}
		mtx->unlock();
	}
	
    

    // closing all the sockets
    for(int i=0;i<max_connection;i++)
    	close(sock[i]);

	print_lock.lock();
    outfile << "Sender exiting for index = " << index << endl;
	print_lock.unlock();
    return;
}

void fun(int index, std::vector <int> nodes, int n, int k, int maxClients, int alpha, int beta)
{
	
	
    int *RN;
    RN = new int[n+1];
    for(int i=0;i<=n;i++)
        RN[i] = 0;

   
    bool granted = false;
	int counter = k;
	std::mutex mtx;

	std::thread r(receiver, index, maxClients, nodes, RN, n, &mtx, alpha, beta, &granted, &counter);
	
	std::thread s(sender, index, nodes, n, k, RN, &mtx, alpha, beta, &granted, &counter);

	while(total_count>0);

	r.join();
	s.join();
	
}

int main()
{
	srand (time(NULL));
	
	outfile.open("output_SK.txt");
	cout << "Writing to output file - output_SK.txt.." << endl;
	ifstream inputfile;
	inputfile.open("inp-params.txt");
	int n, k, i, alpha, beta;

	inputfile >> n >> k >> i >> alpha >> beta;

	total_count = n*k;
    tokenProcess = i;
    LN = new int[n+1];
	std::vector <int> graph[n+1]; 

	int position=0;
	std::string temp;
	
	for(int i=1;i<=n;i++)
	{
        LN[i] = 0;
		for(int j=1;j<=n;j++)
		{
			if(j!=i)
				graph[i].push_back(j);
		}
	}
	
	outfile << "The graph topology used is - " << endl;
	for(int i = 1;i<=n;i++)
	{
		for(int j=0;j<graph[i].size(); j++)
			outfile << graph[i][j] << " ";
		outfile << endl;
	}

	messageComplexity = 0;
	messageComplexity = 0.0;

	int *fill_maxClients;
	fill_maxClients = new int[n+1];

	for(int i=1;i<=n;i++)
		fill_maxClients[i]=0;

	for(int i=1;i<=n;i++)
	{
		for(int j=0;j<graph[i].size();j++)
			fill_maxClients[graph[i][j]]++;
	}


	check_sleep = n;

	std::thread th[n];
	for(int i=0;i<n;i++)
		th[i] = std::thread(fun, i+1, graph[i+1], n, k, fill_maxClients[i+1], alpha, beta);

	for(int i=0;i<n;i++)
		th[i].join();

	outfile << "All the processes completed their CS requests..Exiting.." << endl;
	outfile << "Total message complexity = " << messageComplexity/n << endl;
	outfile << "Total time complexity = " << abs(timeComplexity)/n << endl;
    outfile.close();

	return 0;
}