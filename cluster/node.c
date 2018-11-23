//
//  General-purpose Photovoltaic Device Model gpvdm.com- a drift diffusion
//  base/Shockley-Read-Hall model for 1st, 2nd and 3rd generation solarcells.
// 
//  Copyright (C) 2012 Roderick C. I. MacKenzie r.c.i.mackenzie at googlemail.com
//
//	https://www.gpvdm.com
//	Room B86 Coates, University Park, Nottingham, NG7 2RD, UK
//
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>

#include "util.h"
#include "inp.h"
#include <state.h>

struct state *local_sim;

void* alarm_thread(void *in)
{
	while(1)
	{
		if (local_sim->head_sock>=0)
		{
			send_node_load(local_sim->head_sock);
		}
		sleep(1);
	}
	return NULL;
}

/*void alarm_wakeup (int i)
{
		struct itimerval tout_val;

		signal(SIGALRM,alarm_wakeup);

		tout_val.it_interval.tv_sec = 0;
		tout_val.it_interval.tv_usec = 0;
		tout_val.it_value.tv_sec = 2;
		tout_val.it_value.tv_usec = 0;
		if (local_sim->head_sock>=0)
		{
			send_node_load(local_sim->head_sock);
		}

		setitimer(ITIMER_REAL, &tout_val,0);
		
}*/

void my_handler(int s)
{
	printf("Caught signal %d\n",s);
	send_delete_node(local_sim->head_sock);
	printf("quitting\n");
	exit(1); 
}

int login(char * master_ip, int port)
{


    int sockfd; // Socket file descriptor
    //struct sockaddr_in addr_remote;
    struct sockaddr_in remote_addr; 
    /* Get the Socket file descriptor */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
		printf(" %s\n", strerror(errno));
        return (0);
    }
    /* Fill the socket address struct */
    remote_addr.sin_family = AF_INET; 
    remote_addr.sin_port = htons(port); 
    inet_pton(AF_INET, master_ip, &remote_addr.sin_addr); 
    bzero(&(remote_addr.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
    {
        return -1;
    }
    else
	{
		printf("client: Connected to server at port:%d sock:%d...ok!\n", port,sockfd);
	}
return sockfd;
}

int node(struct state *sim)
{

	local_sim=sim;
	int port=0;
	char temp_path[200];
	char interface[200];
	char master_ip[200];
	char store_path[200];
	struct sigaction sigIntHandler;

	//struct itimerval tout_val;

	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	struct inp_file inp;
	inp_init(&inp);
	if (inp_load(&inp,"node.inp")!=0)
	{
		printf("can't find file config.inp");
		exit(0);
	}
	inp_check(&inp,1.0);
	inp_search_string(&inp,temp_path,"#store_path");
	inp_search_string(&inp,interface,"#interface");
	inp_search_string(&inp,master_ip,"#master_ip");
	inp_search_int(&inp,&port,"#port");
	inp_free(&inp);

	realpath(temp_path, store_path);
	calpath_set_store_path(store_path);
	set_porgress_max(20);
	pthread_create( &sim->alarm, NULL,alarm_thread,NULL);

	do
	{
		sim->head_sock=-1;
		double p=0.0;
		do
		{
			sim->head_sock=login(master_ip,port);

			if (sim->head_sock>=0)
			{
				break;
			}

			sleep(1);
			text_progress(p);
			fputs("\r", stdout);
			p=p+0.1;
			if (p>1.0)
			{
				p=0;
			}
		}while(sim->head_sock<0);

		cal_my_ip(sim->head_sock);

		register_node(sim->head_sock);

		send_node_load(sim->head_sock);

		struct tx_struct data;
		int ret=0;
		int processed=FALSE;
		while(1)
		{
			ret=rx_packet(sim->head_sock,&data);

			processed=FALSE;
			
			if (ret==-1)
			{
				break;
			}

			if (cmp_node_runjob(sim,&data)==0)
			{
				processed=TRUE;
			}else
			if (cmp_rxfile(sim->head_sock,&data,sim)==0)
			{
				processed=TRUE;
			}else
			if (cmp_node_killall(sim->head_sock,&data)==0)
			{
				processed=TRUE;
			}else
			if (cmp_node_sleep(sim->head_sock,&data)==0)
			{
				processed=TRUE;
			}else
			if (cmp_node_poweroff(sim->head_sock,&data)==0)
			{
				processed=TRUE;
			}else
			if (cmp_node_send_data(sim,sim->head_sock,&data)==0)
			{
				processed=TRUE;
			}else
			if (cmp_nodeload(sim->head_sock,&data)==0)
			{
				processed=TRUE;
			}else
			if (cmp_node_quit(sim->head_sock,&data)==0)
			{
				processed=TRUE;
			}else
			if (cmp_slave_clean(sim->head_sock,&data)==0)
			{
				processed=TRUE;
			}else
			if (cmp_sync_packet_one(sim->head_sock,&data)==0)
			{
				processed=TRUE;
			}else
			if (cmp_sync_packet_two(sim,sim->head_sock,&data)==0)
			{
				processed=TRUE;
			}else
			if (cmp_node_killjob(sim->head_sock,&data)==0)
			{
				processed=TRUE;
			}
			
			if (processed==FALSE)
			{
				printf("Command not understood!!!!!!!!!!!!!!!!! %s\n",data.id);
			}
		
		}

		close (sim->head_sock);
		printf("client: connection lost trying again.\n");

	}while(1);

    return (0);
}
