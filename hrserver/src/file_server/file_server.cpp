﻿/*
 * file_server.cpp
 *
 *  Created on: 2013-12-9
 *      Author: ziteng@mogujie.com
 */

#include "FileConn.h"
#include "netlib.h"
#include "ConfigFileReader.h"
#include "version.h"
#include "IM.BaseDefine.pb.h"

void file_serv_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	if (msg == NETLIB_MSG_CONNECT) {
		CFileConn* pConn = new CFileConn();
		pConn->OnConnect(handle);
	} else {
		log("!!!error msg: %d ", msg);
	}
}

int main(int argc, char* argv[])
{
    backup_core_file();
    will_be_daemon(argc, argv);

	if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
		printf("Server Version: FileServer/%s\n", VERSION);
		printf("Server Build: %s %s\n", __DATE__, __TIME__);
		return 0;
	}

	signal(SIGPIPE, SIG_IGN);

	CConfigFileReader config_file("fileserver.conf");

	char* listen_ip = config_file.GetConfigName("Address");
	char* str_listen_port = config_file.GetConfigName("ListenPort");
    char* str_task_timeout = config_file.GetConfigName("TaskTimeout");

	if (!listen_ip || !str_listen_port) {
		log("config item missing, exit... ");
		return -1;
	}

	uint16_t listen_port = atoi(str_listen_port);
    uint32_t task_timeout = atoi(str_task_timeout);

    CStrExplode listen_ip_list(listen_ip, ';');
    std::list<IM::BaseDefine::IpAddr> q;
    for (uint32_t i = 0; i < listen_ip_list.GetItemCnt(); i++) {
        IM::BaseDefine::IpAddr ip_addr;
        ip_addr.set_ip(listen_ip_list.GetItem(i));
        ip_addr.set_port(listen_port);
    	q.push_back(ip_addr);
    }
	init_file_conn(q, task_timeout);

	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;


//	for (uint32_t i = 0; i < listen_ip_list.GetItemCnt(); i++) {
		ret = netlib_listen("0.0.0.0", /*listen_ip_list.GetItem(i), */listen_port, file_serv_callback, NULL);
		if (ret == NETLIB_ERROR)
			return ret;
//	}

	printf("server start listen on %s:%d\n", listen_ip, listen_port);
	printf("now enter the event loop...\n");
    
    writePid();

	netlib_eventloop();

	printf("exiting.......\n");
	log("exit");

	return 0;
}
