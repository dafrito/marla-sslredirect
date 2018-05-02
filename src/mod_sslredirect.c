#include "mod_sslredirect.h"

static marla_WriteResult writeResponse(marla_Request* req, marla_WriteEvent* we)
{
    char buf[4096];

    char* portstart = strstr(req->host, ":");
    int needed;
    if(portstart) {
        // Host contains port.
        portstart[0] = 0;
        needed = snprintf(buf, sizeof buf,
            "HTTP/1.1 301 Moved Permanently\r\nLocation: https://%s%s\r\n\r\n", req->host, req->uri
        );
        portstart[0] = ':';
    }
    else {
        // Host contains no port.
        needed = snprintf(buf, sizeof buf,
            "HTTP/1.1 301 Moved Permanently\r\nLocation: https://%s%s\r\n\r\n", req->host, req->uri
        );
    }
    int nwritten = marla_Connection_write(req->cxn, buf, needed);
    if(nwritten < needed) {
        if(nwritten > 0) {
            marla_Connection_putbackWrite(req->cxn, nwritten);
        }
        return marla_WriteResult_DOWNSTREAM_CHOKED;
    }

    while(marla_Ring_size(req->cxn->output) > 0) {
        int nflushed;
        marla_WriteResult wr = marla_Connection_flush(req->cxn, &nflushed);
        if(wr != marla_WriteResult_CONTINUE) {
            return wr;
        }
    }
    req->writeStage = marla_CLIENT_REQUEST_AFTER_RESPONSE;
    return marla_WriteResult_CONTINUE;
}

static void redirectHandler(struct marla_Request* req, enum marla_ClientEvent ev, void* data, int dataLen)
{
    marla_WriteEvent* we;
    switch(ev) {
    case marla_EVENT_ACCEPTING_REQUEST:
        *((int*)data) = 1;
        break;
    case marla_EVENT_REQUEST_BODY:
        we = data;
        if(we->length > 0) {
            // Skip the body.
            we->index = we->length;
        }
        else {
            req->readStage = marla_CLIENT_REQUEST_DONE_READING;
        }
        break;
    case marla_EVENT_MUST_WRITE:
        we = data;
        we->status = writeResponse(req, we);
        break;
    case marla_EVENT_DESTROYING:
        req->handlerData = 0;
    }
}

void mod_sslredirect_route(struct marla_Request* req, void* hookData)
{
    req->handler = redirectHandler;
}

void mod_sslredirect_init(struct marla_Server* server, enum marla_ServerModuleEvent e)
{
    switch(e) {
    case marla_EVENT_SERVER_MODULE_START:
        marla_Server_addHook(server, marla_ServerHook_ROUTE, mod_sslredirect_route, 0);
        break;
    case marla_EVENT_SERVER_MODULE_STOP:
        break;
    }
}
