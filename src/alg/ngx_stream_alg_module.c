
/*
 * Copyright (C) Yichun Zhang (agentzh)
 */


#ifndef DDEBUG
#define DDEBUG 0
#endif
#include "ddebug.h"


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_stream.h>
#include <nginx.h>


typedef struct {
    ngx_flag_t       alg_ftp;
} ngx_stream_alg_srv_conf_t;

typedef struct {
    size_t          left;
    size_t          size;
    size_t          ext;
    u_char         *pos;
    u_char         *dst;
    u_char          buf[4];
    u_char          version[2];
    ngx_str_t       host;
    ngx_str_t       alpn;
    ngx_log_t      *log;
    ngx_pool_t     *pool;
    ngx_uint_t      state;
} ngx_stream_alg_ctx_t;

static ngx_int_t ngx_stream_alg_init(ngx_conf_t *cf);
static ngx_int_t ngx_stream_alg_handler(ngx_stream_session_t *s);
static char * ngx_stream_alg_alg(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void * ngx_stream_alg_create_srv_conf(ngx_conf_t *cf);

static ngx_command_t  ngx_stream_alg_commands[] = {

    { ngx_string("alg"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_stream_alg_alg,
      NGX_STREAM_SRV_CONF_OFFSET,
      0,
      NULL },
      
    ngx_null_command
};


static ngx_stream_module_t  ngx_stream_alg_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_stream_alg_init,           /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    ngx_stream_alg_create_srv_conf,       /* create server configuration */
    NULL,
};


ngx_module_t  ngx_stream_alg_module = {
    NGX_MODULE_V1,
    &ngx_stream_alg_module_ctx,           /* module context */
    ngx_stream_alg_commands,              /* module directives */
    NGX_STREAM_MODULE,                     /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static void *
ngx_stream_alg_create_srv_conf(ngx_conf_t *cf)
{
    ngx_stream_alg_srv_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_stream_alg_srv_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}

static ngx_int_t
ngx_stream_alg_init(ngx_conf_t *cf)
{
    ngx_stream_handler_pt        *h;
    ngx_stream_core_main_conf_t  *cmcf;

    cmcf = ngx_stream_conf_get_module_main_conf(cf, ngx_stream_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_STREAM_PREREAD_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_stream_alg_handler;

    return NGX_OK;
}

static ngx_int_t
ngx_stream_alg_handler(ngx_stream_session_t *s)
{

    ngx_stream_alg_srv_conf_t  *ascf;
    ngx_connection_t *c;
    ngx_stream_alg_ctx_t       *ctx;
    u_char                             *last, *p;
    ssize_t    len;
    ngx_listening_t             *ls;

    ascf = ngx_stream_get_module_srv_conf(s,ngx_stream_alg_module);
    if (ascf->alg_ftp != 1 ) {
        return NGX_DECLINED;
    }
    c = s->connection;
    
    if ( c->type != SOCK_STREAM ) {
        return NGX_DECLINED;
    }
    
    ls = c->listening;

    if (ls->child != 1) {
        ls->alg = 1;
    }

    if ( c->buffer == NULL ) {
        return NGX_DECLINED;
    }
    
    ctx = ngx_stream_get_module_ctx(s, ngx_stream_alg_module);
    if (ctx == NULL) {
        ctx = ngx_pcalloc(c->pool, sizeof(ngx_stream_alg_ctx_t));
        if (ctx == NULL) {
            return NGX_ERROR;
        }
        ngx_stream_set_ctx(s, ctx, ngx_stream_alg_module);
        ctx->pool = c->pool;
        ctx->log = c->log;
        ctx->pos = c->buffer->pos;
    }
   
    p = ctx->pos;
    last = c->buffer->last;
    /*Find the \r\n pattern*/
    len = last - p;
    if (len >= 2 ) {
        /*If find the "\r\n*/
        if (ngx_strncmp(p,CRLF,ngx_strlen(CRLF)) != 0) {
            p += len;
            ctx->pos = p;
            return NGX_DECLINED;
        } else {
            return NGX_AGAIN;
        }
    } 
    return NGX_AGAIN;
}

char *
ngx_stream_alg_alg(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{

    ngx_stream_alg_srv_conf_t *ascf = conf;
    ngx_str_t                       *value;
    value = cf->args->elts;
    if (ngx_strcmp(value[1].data,"ftp") == 0) {
        ascf->alg_ftp = 1;
    } else {
        return NGX_CONF_ERROR;
    }
    return NGX_CONF_OK;
}
