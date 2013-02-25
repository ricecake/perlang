#include <thread>
#include <string>
#include <mutex>
#include <deque>
#include <queue>
#include <condition_variable>

#include <iostream>

#include "erl_nif.h"
#include <perl++/perl++.h>

using namespace perl;
extern "C" {
	static ERL_NIF_TERM perlInit(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
	static ERL_NIF_TERM perlUse(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
	static ERL_NIF_TERM perlCall(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
	static ERL_NIF_TERM perlLoad(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
	static ERL_NIF_TERM perlRun(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);

	static ErlNifFunc nif_funcs[] = {
		{"perlInit", 0, perlInit},
		{"perlUse", 0, perlUse},
		{"perlCall", 0, perlCall},
	};

	int load(ErlNifEnv* env, void* priv[], ERL_NIF_TERM load_info){return 0;}
	int reload(ErlNifEnv* env, void* priv[], ERL_NIF_TERM load_info){return 0;}
	int upgrade(ErlNifEnv* env, void* priv[], void* old_priv[], ERL_NIF_TERM load_info){return 0;}
	void unload(ErlNifEnv* env, void* priv){return;}

	ERL_NIF_INIT(perlang, nif_funcs, &load, &reload, &upgrade, &unload);
}

ERL_NIF_TERM
mk_error(ErlNifEnv* env, const char* mesg)
{
    return enif_make_tuple(env, enif_make_atom(env, "error"), enif_make_atom(env, mesg));
}

// C++ structures and classes

struct request {
	std::string package;
	std::string sub;
	ErlNifEnv* env;
	ErlNifPid* pid;
//	std::vector args;
//	request(std::string mod, std::string fun, perl::Array params) : package(mod), sub(fun), args(params){}
	request(ErlNifEnv* cenv, std::string mod, std::string fun) : package(mod), sub(fun){
		ErlNifEnv * Env  = enif_alloc_env();
		env = Env;
		pid = (ErlNifPid*) enif_alloc(sizeof(ErlNifPid));
		enif_self(cenv, pid);
	}
	~request(){
		enif_free_env(env);
	}
};

// C++ function declarations
void perlDo(Interpreter* uni, request* req);

// C++ function definitions
static std::condition_variable MS;
static std::mutex MSlock;

static std::deque<Interpreter*> Multiverse;
static std::queue<request*> requestQueue;
static std::mutex qLock;

static ERL_NIF_TERM perlInit(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	for(int i=0;i<16;i++){
		Multiverse.push_back(new Interpreter);
	}

	std::thread queueRunner([]{
		auto predicate = []{return !(Multiverse.empty()||requestQueue.empty());};
		while(1){
			std::unique_lock<std::mutex> iq(MSlock);
			MS.wait(iq, predicate);
			while(predicate()){
				qLock.lock();
				std::thread perlCall(perlDo, Multiverse.front(), requestQueue.front());
				perlCall.detach();
				Multiverse.pop_front();
				requestQueue.pop();
				qLock.unlock();
			}
		}
	});
	queueRunner.detach();
	return enif_make_atom(env, "ok");
}
static ERL_NIF_TERM perlUse(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	for(int i=0; i<argc; i++){
		
	}
	std::for_each(Multiverse.begin(), Multiverse.end(), [](Interpreter* i){
		i->set_context();
		i->use("NifTest");
	});

	return enif_make_atom(env, "ok");
}
static ERL_NIF_TERM perlCall(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	qLock.lock();
	requestQueue.push(new request(env, "NifTest", "HelloErlang"));
	qLock.unlock();
	MS.notify_one();
	return enif_make_atom(env, "ok");
}

void perlDo(Interpreter* uni, request* req) {
	uni->set_context();
	Package p = uni->package(req->package.c_str());

	p.call(req->sub.c_str());

	qLock.lock();
	Multiverse.push_back(uni);
	qLock.unlock();
	MS.notify_one();
	enif_send(NULL, req->pid, req->env, enif_make_atom(req->env, "ok"));
	delete req;
}

static ERL_NIF_TERM perlLoad(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {}
static ERL_NIF_TERM perlRun(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {}
