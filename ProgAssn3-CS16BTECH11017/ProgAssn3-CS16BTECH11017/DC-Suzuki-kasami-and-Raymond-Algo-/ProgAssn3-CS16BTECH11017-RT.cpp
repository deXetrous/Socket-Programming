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

atomic <int> total_count{0};            // counting total messages to be received

atomic <int> check_sleep{0};      // to make the sender sleep until all connections are done

atomic <int> messageComplexity{0};
double timeComplexity;
time_t rawtime;

mutex print_lock;

FILE *fp;

ofstream outfile;

mutex mtx_global;

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

void receiver(int index, int max_clientSize, std::vector <int> nodes, int n, int k, std::mutex *mtx, int alpha, int beta, int* Holder, bool* Using, deque<int>*  requestQ, bool* Asked)
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
                        //printf("Message Received at process  = %d, msg = %s\n", index,buffer);
                        this_thread::sleep_for(chrono::milliseconds((int)run_exp(alpha*10)));
                        mtx->lock();

                        int pos = 0;
                        while(pos < strlen(buffer))
                        {
                            if(buffer[pos] == '#')
                            {
                                tm *ltm = localtime(&now);
                                print_lock.lock();
                                outfile << "Process " << index <<  " received privilege at " << ltm->tm_min << ":" <<ltm->tm_sec << endl;
                                print_lock.unlock();
                                *Holder = index;
                                pos++;
                            }
                            else
                            {
                                int t_val=0,val;
                                while(buffer[pos] != ':')
                                {
                                    t_val = t_val*10 + (int)buffer[pos]-48;
                                    pos++;
                                }
                                pos++;
                                val = t_val;
                                
                                tm *ltm = localtime(&now);
                                print_lock.lock();
                                outfile << "Process " << index <<  " received request message at " << ltm->tm_min << ":" <<ltm->tm_sec << endl;
                                print_lock.unlock();
                                requestQ->push_back(val);
                                
                            }
                        }

                        
                        
                        mtx->unlock();

                    }
                    //printf("Done parsing\n");

                }
                
            }
            
            if(total_count == 0)
            {
                //fprintf(fp,"receiver to exit= %d\n", index);
                break;
            }
            
        }
        print_lock.lock();
        outfile << "Exiting receiver = " << index << endl;
        print_lock.unlock();
        return;
}



void sender(int index, std::vector <int> nodes, int n, int k, std::mutex *mtx, int alpha, int beta, int* Holder, bool* Using, deque<int>*  requestQ, bool* Asked)
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

    int count = 0;
    int counter = 0;

    time_t now = time(0);
    
    int p1 = 1;
    while(p1 <= k)
    {
        if(total_count <= 0)
            break;
        
        this_thread::sleep_for(chrono::milliseconds((int)run_exp(alpha*1000)));
        mtx->lock();
        
        //requests for CS
        if(find(requestQ->begin(), requestQ->end(), index) == requestQ->end())
        {
            tm* ltm = localtime(&now);
            print_lock.lock();
            outfile << "Process " << index <<  " is doing local computation at " << ltm->tm_min << ":" <<ltm->tm_sec << endl;
            print_lock.unlock();

            time_t reqTime1 = time(0);
		    timeComplexity += (-reqTime1);

            ltm = localtime(&now);
            print_lock.lock();
            outfile << "Process " << index <<  " requests to enter CS at " << ltm->tm_min << ":" <<ltm->tm_sec << " for the " << p1 << " time " << endl;
            print_lock.unlock();

            requestQ->push_back(index);
        }

        //ASSIGN_PRIVILEGE
        if(*Holder == index && requestQ->size()>0 && requestQ->front() == index)
        {
            //entering CS
            *Using = true;

            tm *ltm = localtime(&now);
            time_t reqTime2 = time(0);
		    timeComplexity += reqTime2;
            print_lock.lock();
            outfile << "Process " << index <<  " enters CS at " << ltm->tm_min << ":" <<ltm->tm_sec << endl;
            print_lock.unlock();

            ltm = localtime(&now);
            print_lock.lock();
            outfile << "Process " << index <<  " leaves CS at " << ltm->tm_min << ":" <<ltm->tm_sec << endl;
            print_lock.unlock();

            requestQ->pop_front();
            p1++;
            mtx_global.lock();
            total_count--;
            mtx_global.unlock();
            //cout << "Total count left = " << total_count << endl;
            //exiting CS
            *Using = false;
        }
        else if(*Holder == index && *Using == false && requestQ->size()>0 && requestQ->front() != index)
        {
            //sending privilege
            int temp = requestQ->front();
            requestQ->pop_front();
            int ind;
            for(int j=0;j<max_connection;j++)
            {
                if(nodes[j] == temp)
                {
                    ind = j;
                    break;
                }
            }
            
            *Asked = false;
            char hello1[] = "#";

            print_lock.lock();
            outfile << "Process " << index << " sending privilege to process " <<  temp << endl;
            print_lock.unlock();

            messageComplexity++;
            write(sock[ind], hello1, strlen(hello1));
            *Holder = temp;
        }
        else
        {
            // make request
            if(requestQ->size() > 0 && *Asked == false)
            {
                int ind;
                for(int j=0;j<max_connection;j++)
                {
                    if(nodes[j] == *Holder)
                    {
                        ind = j;
                        break;
                    }
                }
    
                string s = to_string(index) + ":";
                char hello1[s.length()];
                strcpy(hello1, s.c_str());
                
                int source;
                if(index - (*Holder) == 1)
                    source = index+1;
                else
                    source = index-1;

                tm *ltm = localtime(&now);

                if(index != 1 && index != n)
                {
                    print_lock.lock();
                      outfile << "Process " << index << " forwards p" << source << "'s request to enter CS at " << ltm->tm_min << ":" << ltm->tm_sec << endl;
                      print_lock.unlock();
                }
               
                messageComplexity++;
                write(sock[ind], hello1, strlen(hello1));
                *Asked = true;   
            }

        }


        mtx->unlock();

        this_thread::sleep_for(chrono::milliseconds((int)run_exp(alpha)));

    }
   
    int looper = 0; 
    while(total_count>0)
    {


        this_thread::sleep_for(chrono::milliseconds((int)run_exp(alpha)));
        mtx->lock();

        looper++;
        if(looper == 100)
        {
            mtx_global.lock();
            total_count = 0;
            mtx_global.unlock();
  
        }

        if(*Holder == index && *Using == false && requestQ->size()>0 && requestQ->front() != index)
        {
            //sending privilege
            int temp = requestQ->front();
            requestQ->pop_front();
            int ind;
            for(int j=0;j<max_connection;j++)
            {
                if(nodes[j] == temp)
                {
                    ind = j;
                    break;
                }
            }
            
            *Asked = false;
            char hello1[] = "#";
            print_lock.lock();
             outfile << "Process " << index << " sending privilege to process " <<  temp << endl;
             print_lock.unlock();
            messageComplexity++;
            write(sock[ind], hello1, strlen(hello1));
            *Holder = temp;
        }

        if(requestQ->size() > 0 && *Asked == false)
        {
            int ind;
            for(int j=0;j<max_connection;j++)
            {
                if(nodes[j] == *Holder)
                {
                    ind = j;
                    break;
                }
            }

            
            string s = to_string(index) + ":";
            char hello1[s.length()];
            strcpy(hello1, s.c_str());
            

            int source;
            if(index - (*Holder) == 1)
                source = index+1;
            else
                source = index-1;

            tm *ltm = localtime(&now);

            if(index != 1 && index != n)
            {
                print_lock.lock();
                 outfile << "Process " << index << " forwards p" << source << "'s request to enter CS at " << ltm->tm_min << ":" << ltm->tm_sec << endl;
                 print_lock.unlock();
            }
            
            messageComplexity++;
            write(sock[ind], hello1, strlen(hello1));
            *Asked = true;   
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

    int Holder;
    bool Using = false;
    deque <int> requestQ;
    bool Asked;

    if(index == 1)
        Holder = index;
    else
        Holder = index-1;

    std::mutex mtx;

    // the receiver or client function is implemented
    std::thread r(receiver, index, maxClients, nodes,n, k, &mtx, alpha, beta, &Holder, &Using, &requestQ, &Asked);
    // the sender or server function is implemented
    std::thread s(sender, index, nodes, n, k, &mtx, alpha, beta, &Holder, &Using, &requestQ, &Asked);

    while(total_count>0);

    r.join();
    s.join();

    
}

int main()
{
    srand (time(NULL));

    outfile.open("output_RT.txt");
    
    
    cout << "Writing to output file - output_RT.txt.." << endl;
    ifstream inputfile;
    inputfile.open("inp-params.txt");

    int n, k, i, alpha, beta;

    inputfile >> n >> k >> i >> alpha >> beta;

    total_count = n*k;

    std::vector <int> graph[n+1]; 

    int position=0;
    std::string temp;
    
    for(int i=1;i<=n;i++)
    {
        if(i != 1)
            graph[i].push_back(i-1);

        if(i != n)
            graph[i].push_back(i+1);
        

    }
    
    outfile << "The graph topology assumed for these processes = " << endl;
    for(int i = 1;i<=n;i++)
    {
        for(int j=0;j<graph[i].size(); j++)
             outfile << graph[i][j] << "  ";
         outfile << endl;
    }

    messageComplexity = 0;
    timeComplexity = 0.0;
    
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