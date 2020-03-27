
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
    ngx_uint_t       alg_ftp:1;
} ngx_stream_alg_srv_conf_t;


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

    conf = ngx_palloc(cf->pool, sizeof(ngx_stream_alg_srv_conf_t));
    if (conf == NULL) {
        return NULL;
    }
    
    conf->alg_ftp = 0;
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
#if 0
    u_char                             *last, *p;
    size_t                              len;
    ngx_int_t                           rc;
    ngx_connection_t                   *c;
    ngx_stream_ssl_preread_ctx_t       *ctx;
    ngx_stream_ssl_preread_srv_conf_t  *sscf;

    c = s->connection;

    ngx_log_debug0(NGX_LOG_DEBUG_STREAM, c->log, 0, "ssl preread handler");

    sscf = ngx_stream_get_module_srv_conf(s, ngx_stream_ssl_preread_module);

    if (!sscf->enabled) {
        return NGX_DECLINED;
    }

    if (c->type != SOCK_STREAM) {
        return NGX_DECLINED;
    }

    if (c->buffer == NULL) {
        return NGX_AGAIN;
    }

    ctx = ngx_stream_get_module_ctx(s, ngx_stream_ssl_preread_module);
    if (ctx == NULL) {
        ctx = ngx_pcalloc(c->pool, sizeof(ngx_stream_ssl_preread_ctx_t));
        if (ctx == NULL) {
            return NGX_ERROR;
        }

        ngx_stream_set_ctx(s, ctx, ngx_stream_ssl_preread_module);

        ctx->pool = c->pool;
        ctx->log = c->log;
        ctx->pos = c->buffer->pos;
    }

    p = ctx->pos;
    last = c->buffer->last;

    while (last - p >= 5) {

        if ((p[0] & 0x80) && p[2] == 1 && (p[3] == 0 || p[3] == 3)) {
            ngx_log_debug0(NGX_LOG_DEBUG_STREAM, ctx->log, 0,
                           "ssl preread: version 2 ClientHello");
            ctx->version[0] = p[3];
            ctx->version[1] = p[4];
            return NGX_OK;
        }

        if (p[0] != 0x16) {
            ngx_log_debug0(NGX_LOG_DEBUG_STREAM, ctx->log, 0,
                           "ssl preread: not a handshake");
            ngx_stream_set_ctx(s, NULL, ngx_stream_ssl_preread_module);
            return NGX_DECLINED;
        }

        if (p[1] != 3) {
            ngx_log_debug0(NGX_LOG_DEBUG_STREAM, ctx->log, 0,
                           "ssl preread: unsupported SSL version");
            ngx_stream_set_ctx(s, NULL, ngx_stream_ssl_preread_module);
            return NGX_DECLINED;
        }

        len = (p[3] << 8) + p[4];

        /* read the whole record before parsing */
        if ((size_t) (last - p) < len + 5) {
            break;
        }

        p += 5;

        rc = ngx_stream_ssl_preread_parse_record(ctx, p, p + len);

        if (rc == NGX_DECLINED) {
            ngx_stream_set_ctx(s, NULL, ngx_stream_ssl_preread_module);
            return NGX_DECLINED;
        }

        if (rc != NGX_AGAIN) {
            return rc;
        }

        p += len;
    }

    ctx->pos = p;
#endif
    return NGX_DECLINED;
}

char *
ngx_stream_alg_alg(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_stream_alg_srv_conf_t *acf = conf;
    acf->alg_ftp = 1;
    return NGX_CONF_OK;
}
