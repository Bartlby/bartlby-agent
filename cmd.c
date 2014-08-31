/* $Id$ */
/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2005-2009 Helmut Januschka - All Rights Reserved
 *   Contact: <helmut@januschka.com>, <contact@bartlby.org>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version; incorporated herein by reference.
 *
 *   visit: www.bartlby.org for support
 * ----------------------------------------------------------------------- */
/*
$Revision$
$HeadURL$
$Date$
$Author$ 
*/




#include "bartlby_agent.h"



unsigned long crc32_table[256];

char * new_passive_text;
int new_passive_state;

char * passive_action=NULL;
char * passive_host=NULL;
int passive_service=-1;
int passive_port=-1;

static sig_atomic_t portier_connection_timed_out=0;





#define _log(a,b,...) printf(__VA_ARGS__);



static void bartlby_conn_timeout(int signo) {
 	portier_connection_timed_out = 1;
}

//Caller has to free
char * bartlby_portier_fetch_reply(int sock) {
		char buffer[2048];
		portier_connection_timed_out=0;
		int recv_bytes;
		alarm(5);
		recv_bytes=read(sock, buffer, 2047);
		if(recv_bytes < 0) {
			_log(LH_TRIGGER, B_LOG_CRIT, "Portier: fetching reply failed\n");
			return NULL;
		}
		if(portier_connection_timed_out == 1) {
			return NULL;
		} else {
			buffer[recv_bytes]=0;
			return strdup(buffer);
		}
		
}

void bartlby_portier_disconnect(int sock) {
	close(sock);
}


int bartlby_portier_send(json_object * obj, int sock) {

	const char * json_package = json_object_to_json_string(obj);


	portier_connection_timed_out=0;
	alarm(5);
	if(write(sock, json_package, strlen(json_package)) < 0) {
			return -1;
	}
	alarm(0);
	if(portier_connection_timed_out == 1) {
		_log(LH_TRIGGER, B_LOG_DEBUG, "PORTIER: sending of json package timed out");
		return -1;
	}
	portier_connection_timed_out=0;
	return 1;
}

int bartlby_portier_connect(char *host_name,int port){
	int result;


	struct addrinfo hints, *res, *ressave;
	char ipvservice[20];
	int sockfd;
	
	sprintf(ipvservice, "%d",port);
	
	 memset(&hints, 0, sizeof(struct addrinfo));

   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
	
	 portier_connection_timed_out=0;
	
	
	 result = getaddrinfo(host_name, ipvservice, &hints, &res);
	 if(result < 0 || portier_connection_timed_out != 0) {
	 		return -7;
	}
	ressave = res;
	sockfd=-1;
	
	while (res) {
		alarm(5);
        sockfd = socket(res->ai_family,
                        res->ai_socktype,
                        res->ai_protocol);
        alarm(0);
       
        if (!(sockfd < 0)) {
        	alarm(5);
            if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) {
                break;
            }
            alarm(0);
        	    
            close(sockfd);
            sockfd=-1;
            
        }
    res=res->ai_next;
  }
  freeaddrinfo(ressave);
 
	return sockfd;
	
	
}


void cmd_get_passive() {
	int res;
	char verstr[2048];
	char cmdstr[2048];
	char result[2048];
	char * token, * token_t;
	int rc;
	json_object * jso, *rjso, *json_plugin, *json_args, *json_error, *json_errormsg;
	char * reply;

	res=bartlby_portier_connect(passive_host, passive_port);
	if(res > 0) {
		jso = json_object_new_object();
		json_object_object_add(jso, "method", json_object_new_string("plugin_info"));
    	json_object_object_add(jso, "service_id", json_object_new_int64(passive_service));				
    	if(bartlby_portier_send(jso, res) >0) {
    		reply=bartlby_portier_fetch_reply(res);
    		rjso=json_tokener_parse(reply);
    		if(rjso) {
    			json_object_object_get_ex(rjso, "plugin", &json_plugin);
    			json_object_object_get_ex(rjso, "args", &json_args);
    			json_object_object_get_ex(rjso, "error_code", &json_error);
    			json_object_object_get_ex(rjso, "error_msg", &json_errormsg);
				if(json_object_get_int(json_error) < 0) {
					printf("Failed with '%s'\n", json_object_get_string(json_errormsg));
					exit(4);
				}    else { 			
    				printf("%s %s\n", json_object_get_string(json_plugin), json_object_get_string(json_args));
    			}
    			json_object_put(rjso);
    		} else {
    			printf("JSON PARSE ERROR on '%s'", reply);
    		}
    		free(reply);
    	}
    	json_object_put(jso);
    	bartlby_portier_disconnect(res);
	} else {
		printf("Connect failed\n");
		exit(2);	
	}	
}

void cmd_get_server_id() {
	int res;
	char verstr[2048];
	char cmdstr[2048];
	char result[2048];
	char myhostname[255];
	
	int rc;
	json_object * jso, *rjso, *json_server_id, *json_error, *json_errormsg;
	char * reply;

	if(gethostname(myhostname, 255) != 0) {
		printf("gethostname() failed\n");
		exit(1);	
	}
	fprintf(stderr, "trying to get server id for: %s \n", myhostname);
	

	res=bartlby_portier_connect(passive_host, passive_port);
	if(res > 0) {
		jso = json_object_new_object();
		json_object_object_add(jso, "method", json_object_new_string("get_server_id"));
    	json_object_object_add(jso, "server_name", json_object_new_string(myhostname));				
    	
    	if(bartlby_portier_send(jso, res) >0) {
    		reply=bartlby_portier_fetch_reply(res);
    		rjso=json_tokener_parse(reply);
    		if(rjso) {
    			json_object_object_get_ex(rjso, "server_id", &json_server_id);
    			json_object_object_get_ex(rjso, "error_code", &json_error);
    			json_object_object_get_ex(rjso, "error_msg", &json_errormsg);
				if(json_object_get_int(json_error) < 0) {
					printf("Failed with '%s'\n", json_object_get_string(json_errormsg));
					exit(4);
				}    else { 			
    				printf("%ld\n", json_object_get_int64(json_server_id));
    			}
    			json_object_put(rjso);
    		} else {
    			printf("JSON PARSE ERROR on '%s'", reply);
    		}
    		free(reply);
    	} else {
    		printf("send failed()");
    	}
    	json_object_put(jso);
    	bartlby_portier_disconnect(res);
	} else {
		printf("Connect failed\n");
		exit(2);	
	}	
}

void cmd_get_services() {
	int res;
	char verstr[2048];
	char cmdstr[2048];
	char result[2048];
	
	int rc;
	
	int i;
	json_object * jso, *rjso, *json_services, *json_error, *json_errormsg;
	char * reply;

	

	res=bartlby_portier_connect(passive_host, passive_port);
	if(res > 0) {
		jso = json_object_new_object();
		json_object_object_add(jso, "method", json_object_new_string("server_checks_needed"));
    	json_object_object_add(jso, "server_id", json_object_new_int64(passive_service));				
    	
    	if(bartlby_portier_send(jso, res) >0) {
    		reply=bartlby_portier_fetch_reply(res);
    		rjso=json_tokener_parse(reply);
    		if(rjso) {
    			json_object_object_get_ex(rjso, "services", &json_services);
    			json_object_object_get_ex(rjso, "error_code", &json_error);
    			json_object_object_get_ex(rjso, "error_msg", &json_errormsg);
				if(json_object_get_int(json_error) < 0) {
					printf("Failed with '%s'\n", json_object_get_string(json_errormsg));
					exit(4);
				}    else { 			
    				for(i=0; i < json_object_array_length(json_services); i++) {
						json_object *obj = json_object_array_get_idx(json_services, i);
						printf("%ld ", json_object_get_int64(obj));
						json_object_put(obj);
					}
					printf("\n");
    			}
    			json_object_put(rjso);
    		} else {
    			printf("JSON PARSE ERROR on '%s'", reply);
    		}
    		free(reply);
    	} else {
    		printf("send failed()");
    	}
    	json_object_put(jso);
    	bartlby_portier_disconnect(res);
	} else {
		printf("Connect failed\n");
		exit(2);	
	}	

}

void cmd_set_passive() {
	int res;
	char verstr[2048];
	char cmdstr[2048];
	char result[2048];
	
	int rc;
	
	int i;
	json_object * jso, *rjso, *json_services, *json_error, *json_errormsg;
	char * reply;

	

	res=bartlby_portier_connect(passive_host, passive_port);
	if(res > 0) {
		jso = json_object_new_object();
		json_object_object_add(jso, "method", json_object_new_string("set_passive"));
    	json_object_object_add(jso, "service_id", json_object_new_int64(passive_service));				
    	json_object_object_add(jso, "state", json_object_new_int(new_passive_state));			
    	json_object_object_add(jso, "passive_text", json_object_new_string(new_passive_text));			
    	
    	if(bartlby_portier_send(jso, res) >0) {
    		reply=bartlby_portier_fetch_reply(res);
    		rjso=json_tokener_parse(reply);
    		if(rjso) {
    			json_object_object_get_ex(rjso, "error_code", &json_error);
    			json_object_object_get_ex(rjso, "message", &json_errormsg);
				if(json_object_get_int(json_error) < 0) {
					printf("Failed with '%s'\n", json_object_get_string(json_errormsg));
					exit(4);
				}    else { 			
    				printf("Return: %s\n", json_object_get_string(json_errormsg));
    				
    			}
    			json_object_put(rjso);
    		} else {
    			printf("JSON PARSE ERROR on '%s'", reply);
    		}
    		free(reply);
    	} else {
    		printf("send failed()");
    	}
    	json_object_put(jso);
    	bartlby_portier_disconnect(res);
	} else {
		printf("Connect failed\n");
		exit(2);	
	}	

}

void help() {
	printf("Bartlby cmd tool:\n");
	printf("-h             display help\n");
	printf("-i             host\n");
	printf("-p             port \n");
	printf("-s             service-id or server-id (if action == get_services)\n");
	printf("-m             new service text\n");
	printf("-e             new service state (0,1,2)\n");
	printf("-a             action\n");
	printf("               maybe: get_passive, set_passive, get_services, get_server_id\n");
	exit(1);	
}
void parse_options(int argc, char ** argv) {
	static struct option longopts[] = {
		{ "help",	0, NULL, 'h'},
		{ "host",	0, NULL, 'i'},
		{ "port", 0, NULL, 'p'},
		{ "service-id",	0, NULL, 's'},
		{ "action",	0, NULL, 'a'},
		
		{ NULL,		0, NULL, 0}
	};
	int c;
	
	
	if(argc < 2) {
		help();
			
	}

	for (;;) {
		c = getopt_long(argc, argv, "hi:e:m:s:p:a:S", longopts, (int *) 0);
		if (c == -1)
			break;
		switch (c) {
		case 'h':  /* --help */
			help();
		break;
		case 'e':
			new_passive_state=atoi(optarg);
		break;
		case 'm':
			new_passive_text=optarg;
		break;
		case 'i':
			
			passive_host=optarg;
		break;
		case 'p':
			passive_port=atoi(optarg);
		break;
		case 's':
			passive_service=atoi(optarg);
		break;
		case 'a':
			passive_action=optarg;
		break;
		
		
		default:
			help();
		}
	}
	
	
}

int main(int argc, char ** argv) {
	//printf("portier client\n");
	parse_options(argc, argv);
	//printf("Working on '%s:%d/%d' action(%s)\n", passive_host, passive_port, passive_service, passive_action);
	if(passive_action == NULL) {
		printf("Action unkown!\n");	
		exit(2);
	}
	if(passive_service <=0 || passive_port <= 0 || passive_host == NULL ){
		printf("Either port, service or host is missing\n");
		exit(3);	
	}
	
	
	if(strcmp(passive_action, "get_passive") == 0) {
		cmd_get_passive();		
	} else if (strcmp(passive_action, "set_passive") == 0) {
		cmd_set_passive();
	} else if(strcmp(passive_action, "get_services") == 0) {
		cmd_get_services();
	} else if(strcmp(passive_action, "get_server_id") == 0) {
		cmd_get_server_id();				
	} else {
		printf("Hmm action is pretty unusefull\n");
	}
	
	
	
	return 0;
}


