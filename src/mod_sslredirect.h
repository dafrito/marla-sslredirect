#ifndef mod_sslredirect_INCLUDED
#define mod_sslredirect_INCLUDED

#include "marla.h"
#include <parsegraph_Session.h>
#include <parsegraph_user.h>
#include <parsegraph_List.h>
#include <parsegraph_environment.h>
#include <apr_pools.h>
#include <time.h>
#include <string.h>
#include <dlfcn.h>
#include <apr_dso.h>

void mod_sslredirect_init(struct marla_Server* server, enum marla_ServerModuleEvent e);
void mod_sslredirect_route(struct marla_Request* req, void* hookData);

#endif // mod_sslredirect_INCLUDED
