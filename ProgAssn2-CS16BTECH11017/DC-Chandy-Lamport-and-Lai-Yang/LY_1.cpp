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

using namespace std;
atomic <int> total{0};
atomic <int> cycles{1};
atomic <int> check_sleep{0};
atomic <int> tuples_counter{0};
atomic <int> true_colour{0};

atomic <bool> isterminated{false};
bool* termination;
bool* cond;
bool* taken;
int * totalNumClient;
int * totalNumClientConst;
FILE *fp;


int** channelSent;


void error(const char* msg)
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

void receiver(int index, int max_clientSize, std::vector <int> nodes, std::vector<int> parent_nodes, int n, std::mutex *mtx, int lambda, int* money, int* colour)
{
	
		int PORT = 3000+index;
		
		int server_fd, new_socket, valread;
	    struct sockaddr_in address;

	    int opt = 1;

	    int addrlen = sizeof(address);

	    char hello[] = "Hello from receiver";
	      
	    // Socket file descriptors are created here.
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
	      
	    // The sockets are binded to the Port
	    if (bind(server_fd, (struct sockaddr *)&address, 
	                                 sizeof(address))<0)
	    {

	        perror("bind failed");

	        exit(EXIT_FAILURE);
	    }

	    // sockets starts listening for incoming connection
	    if (listen(server_fd, 4) < 0)
	    {
	        perror("listen");

	        exit(EXIT_FAILURE);
	    }

	    check_sleep--;

	    vector <int> client_list;
	    int max_client = max_clientSize;
	    int count = 0;

	    while(count < max_client)
	    {

	    	// sockets accepts incoming connections
	    	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
	                       (socklen_t*)&addrlen))<0)
	    	{

	       		perror("accept");
	        	exit(EXIT_FAILURE);
	    	}


	    	else
	    	{
	    		//connected sockets are added to client list.
	    		client_list.push_back(new_socket);
	    		count++;
	    	}
	    }
	    fprintf(fp,"All connection established for client = %d\n", index);

	    char buffer[5000] = {0};
	    time_t now = time(0);
	    //int count_left = max_client;

	    while(isterminated == false)
	    {

	    	for(int i=0; i<max_client ;i++)
	    	{
	    		bzero(buffer,5000);
	    		
	    		if(isterminated == true)
	    		{
	    			fprintf(fp,"\nreturned receiver index = %d\n", index);
	    			return;
	    		}

	    		valread = read(client_list[i] , buffer, 5000);
	    		

		    	if(valread < 0)
		    	{
		    		
		    		error("error on reading..");
		    	}
		    	else
		    	{
		    		
		    		if(strlen(buffer)>0)
		    		{

			    		mtx->lock();
			    		
			    		fprintf(fp, "message received at index = %d mssg = %s\n", index, buffer);
			    		

			    		int offset = 15;
				     	int sum = 0, temp_sum=0;
				     	for(int i=15;i<strlen(buffer);i++)
				     	{
				     		if(buffer[i] == '#')
				     		{
				     			
				     			i++;
				     			temp_sum = 0;
				     			while(buffer[i] != '%')
				     			{
				     				temp_sum = temp_sum*10 + (int)buffer[i]-48;
				     				i++;
				     			}
				     			int index_parent = temp_sum;
				     			temp_sum = 0;
				     			i++;
				     			while(buffer[i] != ':')
				     			{
				     				temp_sum = temp_sum*10 + (int)buffer[i]-48;
				     				i++;
				     			}
				     			
				     			
				     			sum += temp_sum;

				     			totalNumClient[index]--;
				     			
				     			tm *ltm = localtime(&now);
				     			
				     			fprintf(fp, "P%d Receives marker from p%d at %d:%d\n", index, index_parent, ltm->tm_min,ltm->tm_sec);

				     			if(taken[index] == false)
				     				cond[index] = true;

				     			bool checking1 = true;
				     			if(totalNumClient[index] <= 0)
				     				termination[index] = true;

				     			for(int z = 1;z<=n;z++)
				     			{
				     				if(termination[z] == false)
				     					checking1 = false;
				     			}
				     			
				     			if(checking1)
				     			{
				     				isterminated = true;
				     				tm *ltm = localtime(&now);

				     				fprintf(fp,"Termination Detected at %d:%d\n", ltm->tm_min, ltm->tm_sec);
				     				

				     				
				     				if(total > 0)
				     				{

				     					cycles++;
				     					//cout << " resetting for next snapshot\n\n";	

				     					for(int z = 1;z<=n;z++)
				     					{

				     						taken[z] = false;
				     						cond[z] = false;
				     						termination[z] = false;
											totalNumClient[z] = totalNumClientConst[z];
				     					}

				     					true_colour = (true_colour+1)%2;
				     					isterminated = false;
				     				}
				     			}
				     			
				     		}

				     		else if(buffer[i] == '@')
				     		{
				     			
				     			i++;
				     			temp_sum = 0;
				     			int parent;
				     			while(buffer[i] != '%')
				     			{
				     				temp_sum = temp_sum*10 + (int)buffer[i]-48;
				     				i++;
				     			}
				     			parent =temp_sum;
				     			temp_sum = 0;
				     			i++;
				     			while(buffer[i] != ':')
				     			{
				     				temp_sum = temp_sum*10 + (int)buffer[i]-48;
				     				i++;
				     			}
				     			channelSent[parent][index] -= temp_sum;
				     			sum += temp_sum;	
				     			
				     			fprintf(fp, "money received = %d at index = %d\n",temp_sum,index );
				     			//printf("money received = %d at index = %d\n",temp_sum,index );
				     		}
				     		i += offset;
				     			
				     		
				     	}
				     	if(sum > 0)
				     	{
	
				     		//cout << " sum received = " << sum << " index = " << index << endl;	
					     	*money = *money + sum;
					     	//cout << " Incrementing money for process = " << index << " amount = " << *money << endl;
					     	
					     	tm *ltm = localtime(&now);
	    					fprintf(fp, "P%d receives Rs. %d at %d:%d...remaining amount = %d\n", index, sum, ltm->tm_min, ltm->tm_sec, *money);
	    					//printf("P%d receives Rs. %d at %d:%d...remaining amount = %d\n", index, sum, ltm->tm_min, ltm->tm_sec, *money);

					     	total -= sum;
				     	}
				     	

			   			mtx->unlock();
			   		}
		    	}
		    	
		    	this_thread::sleep_for(chrono::milliseconds((int)run_exp(lambda)));
	    	}

	    	//cout << " coming out " << endl;
	    	
	    	if(isterminated)
		    {
		    	fprintf(fp,"receiver to exit= %d\n", index);
		    	break;
		    }
	    	
	    }
	    fprintf(fp,"Exiting receiver = %d\n", index);
	    return;
}


void sender(int index, std::vector <int> nodes, std::vector<int> parent_nodes, int n, std::mutex *mtx, int lambda, int* money, int* colour)
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
	      
	    // Convert IPv4 and IPv6 addresses from text to binary form

	    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) 
	    {
	        printf("\nInvalid address/ Address not supported \n");
	    }

	  
	    if (connect(sock[i], (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
	    {
	        printf("\nConnection Failed index = %d\n",index);
	    }
    }


    time_t now = time(0);
    while(isterminated == false)
    {	
    	mtx->lock();
    	
    	//fprintf(fp, " index = %d cond[index] = %d taken[index] = %d\n", index, taken[index], cond[index]);
    	if(taken[index] == false && (cond[index] == true || index == 1))
	    {
	    	
    		tm *ltm = localtime(&now);

    		*colour = (*colour+1)%2;

    		//cout << " -------------- \t\t P" << index << " takes its local snapshot at " << ltm->tm_min << ":" << ltm->tm_sec << " as: " << " amount remaining = " << *money << endl;
    		
    		fprintf(fp, "P%d takes its local snapshot at %d:%d as: Amount remaining = %d\n", index,ltm->tm_min,ltm->tm_sec,*money);
    		//printf("P%d takes its local snapshot at %d:%d as: Amount remaining = %d\n", index,ltm->tm_min,ltm->tm_sec,*money);

    		taken[index] = true;
	    	
	    }

    	else if(*money > 0)
    	{
    		int random_index = rand()%max_connection;	
	    	int random_amount = rand()%(*money) + 1;
	    	*money = *money - random_amount;

	    	string s;
	    	if(*colour == true_colour)
	    	{
	    		s = "Hello from send@" + to_string(index) +"%"+ to_string(random_amount)+":";
	    		channelSent[index][nodes[random_index]] += random_amount;
	    	}
	    	else
	    		s = "Hello from send#" + to_string(index) +"%"+ to_string(random_amount)+":";

			char mssg[s.length()];
			strcpy(mssg, s.c_str());
				
	    	
	    	tm *ltm = localtime(&now);

	    	

	    	write(sock[random_index], mssg, strlen(mssg));

	    	fprintf(fp, "P%d sends Rs. %d to p%d at %d:%d...remaining amount = %d\n", index, random_amount, nodes[random_index],ltm->tm_min,ltm->tm_sec, *money);
	    	//printf( "P%d sends Rs. %d to p%d at %d:%d...remaining amount = %d\n", index, random_amount, nodes[random_index],ltm->tm_min,ltm->tm_sec, *money);
	    	
	    	

	    	this_thread::sleep_for(chrono::milliseconds((int)run_exp(lambda)));
    	}
    	//cout << " waiting here....... index = " << index << " total = " << total << endl;
    	mtx->unlock();
    	this_thread::sleep_for(chrono::milliseconds((int)run_exp(lambda)));

    }


    for(int i=0;i<max_connection;i++)
    	close(sock[i]);

    fprintf(fp,"Sender exiting for index = %d\n", index);
    return;
}

void fun(int index, std::vector <int> nodes, std::vector<int> parent_nodes, int n, int lambda, int maxClients, int A)
{

	int money = A;
	std::mutex mtx;
	int colour = 0;

	std::thread r(receiver, index, maxClients, nodes, parent_nodes, n, &mtx, lambda, &money, &colour);
	
	std::thread s(sender, index, nodes, parent_nodes, n, &mtx, lambda, &money, &colour);

	while(total>0 && isterminated == false);

	r.join();
	s.join();
	
}

void fill_initiator_distance(int initiator_distance[], std::vector<int> temp_graph[], bool visited[])
{
	queue<int> vertices;
	vertices.push(1);
	while(!vertices.empty())
	{
		int x = vertices.front();
		vertices.pop();
		for(int i = 0;i<temp_graph[x].size(); i++)
		{
			if(visited[temp_graph[x][i]] == false)
			{
				initiator_distance[temp_graph[x][i]] = initiator_distance[x]+1;
				visited[temp_graph[x][i]] = true;
				vertices.push(temp_graph[x][i]);
			}
		}
		
	}
}

int main(int argc, char *argv[])
{
	srand (time(NULL));
	fp = fopen("output_LY1.txt", "w+");
	ifstream inputfile;
	inputfile.open("inp-params.txt");

	int n, lambda, A, T;
	inputfile>> n >> A >> T >> lambda;
	int edges = 0;
	termination = new bool[n+1];
	cond = new bool[n+1];
	taken = new bool[n+1];

	channelSent = new int*[n+1];
	

	for(int i=1;i<=n;i++)
	{
		channelSent[i] = new int[n+1];
		for(int j=1;j<=n;j++)
		{
			channelSent[i][j] = 0;
		}
	}
	

	std::vector <int> graph[n+1]; 
	
	// total = m*n;
	total = T;
	int position=0,value;
	std::string temp;
	
	for(int i=0;i<=n;i++)
	{	
		taken[i] = false;
		cond[i] = false;
		termination[i] = false;
		getline(inputfile, temp);
		if(i>0)
		{
			position++;
			int temp_val = 0, check_val = 0;
			for(int j=0;j<temp.length();j++)
			{
				if(temp[j] == ' ' && check_val == 0)
				{
					check_val = 1;
					continue;
				}
				if(check_val == 1)
				{
					if(temp[j] == ' ')
					{
						graph[position].push_back(temp_val);
						temp_val = 0;
					}
					else
					{
						temp_val = temp_val*10 + (int)temp[j] - 48;
					}
				}
			}
			if(temp_val != 0)
				graph[position].push_back(temp_val);
		}
	}

	std::vector <int> temp_graph[n+1];
	for(int i = 1;i<=n;i++)
	{
		for(int j=0;j<graph[i].size(); j++)
		{
			edges++;
			temp_graph[graph[i][j]].push_back(i);
		}
	}
	
	int initiator_distance[n+1];
	bool visited[n+1];
	initiator_distance[1] = 0;
	visited[1] = true;

	for(int i=2;i<=n;i++)
		visited[i] = false;

	int totalMessages = 0;
	fill_initiator_distance(initiator_distance, temp_graph, visited);
	for(int i=1;i<=n;i++)
	{
		totalMessages += initiator_distance[i];
		
	}
	
	int *fill_maxClients;

	fill_maxClients = new int[n+1];
	totalNumClient = new int[n+1];
	totalNumClientConst = new int[n+1];

	for(int i=1;i<=n;i++)
		fill_maxClients[i]=0;

	for(int i=1;i<=n;i++)
	{
		for(int j=0;j<graph[i].size();j++)
			fill_maxClients[graph[i][j]]++;
	}

	for(int i=1;i<=n;i++)
	{
		totalNumClient[i] = fill_maxClients[i];
		totalNumClientConst[i] = fill_maxClients[i];
	}

	check_sleep = n;

	std::thread th[n];
	for(int i=0;i<n;i++)
		th[i] = std::thread(fun, i+1, graph[i+1], temp_graph[i+1], n, lambda, fill_maxClients[i+1], A);

	for(int i=0;i<n;i++)
		th[i].join();

	fprintf(fp, "Message complexity = %d\n", totalMessages * cycles + 2*edges );
	
	return 0;
}