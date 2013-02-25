/* niftest.c */
#include "erl_nif.h"
#include <perl++/perl++.h>

using namespace perl;
extern "C" {
static ERL_NIF_TERM hello(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    return enif_make_string(env, "Hello world!", ERL_NIF_LATIN1);
}


static ERL_NIF_TERM hello2(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	const char * args[] = { "test!!!", "./test.pl\0" };
	Interpreter universe(2, args);
	Interpreter un = universe.clone();
	for(int i=0;i<100;i++) {
		universe.set_context();
		universe.run();
		un.set_context();
		un.run();
	}
    return enif_make_string(env, "Hello perl!", ERL_NIF_LATIN1);
}

static ErlNifFunc nif_funcs[] =
{
    {"hello", 0, hello},
    {"hello2", 0, hello2}
};


ERL_NIF_INIT(niftest,nif_funcs,NULL,NULL,NULL,NULL)
}
