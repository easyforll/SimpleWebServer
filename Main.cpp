#include "config/config.h"

int main(int argc, char* argv[]){
    //database information need to be modified
    string user = "root";
    string passwd = "liuliang";
    string databasename = "lldb";
    Config config;
    config.parse_arg(argc,argv);

    WebServer server;
    server.init(config.PORT,user,passwd,databasename,config.LOGWrite,
                config.OPT_LINGER,config.TRIGMode,config.sql_num,config.thread_num,config.close_log,config.actor_model);

    //log
    server.log_write();

    //database
    server.sql_pool();

    //thread_pool
    server.thread_pool();

    //triger mode 
    server.trig_mode();

    //listen
    server.eventListen();

    server.eventLoop();

    return 0;            
}