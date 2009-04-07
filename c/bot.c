#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

#include "memagent.h"

int main(int argc, char **argv) {

    agent_config_t conf;
    agent_handle_t handle;

    if (argc != 3) {
        fprintf(stderr, "Usage: bot <jid> <pass>\n");
        exit(EX_USAGE);
    }

    conf.jid = argv[1];
    conf.pass = argv[2];
    conf.software = "agent sample bot";
    conf.version = "1.0";
    conf.save_path = "/tmp/test.list";

    if(start_agent(conf, &handle)) {
        /* Doesn't much matter, but wait here. */
        pthread_join(handle.thread, NULL);
    } else {
        fprintf(stderr, "Couldn't initialize the agent.\n");
    }

}
