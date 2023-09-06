/**
 * @file
 * @brief main rendering program for various layouts of graphs and output formats
 */

/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

/*
 * Written by Stephen North and Eleftherios Koutsofios.
 */

#include "config.h"

#include <cgraph/cgraph.h>
#include <cgraph/exit.h>
#include <gvc/gvc.h>
#include <gvc/gvio.h>

#include <common/globals.h>

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

static int callcnt = 0;

ssize_t getrandom(void * buf, size_t buflen, unsigned int flags){
    return 42+callcnt;
    callcnt += 1;
}



#include <gvc/gvconfig.h>


#include "emscripten.h"

#define PROTOCOL_FUNCTION __attribute__((import_module("typst_env"))) extern

// ===
// Functions for the protocol

PROTOCOL_FUNCTION void wasm_minimal_protocol_send_result_to_host(const uint8_t *ptr, size_t len);
PROTOCOL_FUNCTION void wasm_minimal_protocol_write_args_to_buffer(uint8_t *ptr);

char * tsrc = "strict digraph {a -> d}";
char * tformat = "json";
char * tengine = "dot";

char lastErrorStr[1024];
int vizErrorf(char *buf)
{
    strncpy(lastErrorStr, buf, sizeof(lastErrorStr) - 1);
    return 0;
}


char m_result[1024];

//int CUSTOMFUNK(const char *src, const char *format, const char *engine)
int render(size_t srclen)
{
    int err = 0;
    // char * src = "strict digraph {a -> d}";
    char * src = (char *)calloc( srclen+1 , sizeof(char));
    char * format = "svg";
    // char * format = (char *)calloc( formatlen+1 , sizeof(char));
    char * engine = "dot";
    // char * engine = (char *)calloc( enginelen+1 , sizeof(char));

    wasm_minimal_protocol_write_args_to_buffer((uint8_t *)src);
    // wasm_minimal_protocol_write_args_to_buffer((uint8_t *)format);
    // wasm_minimal_protocol_write_args_to_buffer((uint8_t *)engine);

    lastErrorStr[0] = '\0';

    // const auto demand_loading = false;
    // auto gvc = std::make_shared<GVC::GVContext>(lt_preloaded_symbols, demand_loading);
    // auto g = std::make_shared<CGraph::AGraph>(dot);
    // const auto layout = GVC::GVLayout(gvc, g, engine);
    // const auto result = layout.render(format);
    // m_result = result.string_view();

    // return m_result.c_str();

    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, true);

    agseterr(AGERR);
    agseterrf(vizErrorf);

    agreadline(1);

    Agraph_t *graph;
    char *data = NULL;
    unsigned int length;
    while ((graph = agmemread(src)))
    {
        if (data == NULL)
        {
            gvLayout(gvc, graph, engine);
            gvRenderData(gvc, graph, format, &data, &length);
            gvFreeLayout(gvc, graph);
        }

        agclose(graph);

        src = "";
    }

    if (lastErrorStr[0] == '\0'){
        wasm_minimal_protocol_send_result_to_host( (uint8_t*) data, strlen(data));
    }
    else {
        wasm_minimal_protocol_send_result_to_host( (uint8_t*) lastErrorStr, strlen(lastErrorStr));
        err = 1;
    }
    gvFreeRenderData(data);

    gvFinalize(gvc);
    gvFreeContext(gvc);
    return err; //all good

}
/**
 * @dir .
 * @brief main rendering program for various layouts of graphs and output formats
 */

