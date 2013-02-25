#include <iostream>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include <string>
#include <mutex>
#include <deque>
#include <queue>
#include <condition_variable>

#include "erl_nif.h"
#include <perl++/perl++.h>

using namespace perl;
extern "C" {
	static ERL_NIF_TERM hello(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
	static ERL_NIF_TERM nifInit(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
	static ERL_NIF_TERM nifRun(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
	static ERL_NIF_TERM hello2(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
	static ERL_NIF_TERM modTest(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);

	static ERL_NIF_TERM perlInit(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
	static ERL_NIF_TERM perlUse(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
	static ERL_NIF_TERM perlCall(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
	static ERL_NIF_TERM perlLoad(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
	static ERL_NIF_TERM perlRun(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);

	static ErlNifFunc nif_funcs[] = {
		{"nifInit", 0, nifInit},
		{"nifRun", 0, nifRun},
		{"perlInit", 0, perlInit},
		{"perlUse", 0, perlUse},
		{"perlCall", 0, perlCall},
	};
	ERL_NIF_INIT(niftest,nif_funcs,NULL,NULL,NULL,NULL)
}

// C++ structures and classes

struct request {
	std::string package;
	std::string sub;
	ErlNifEnv* env;
//	std::vector args;
//	request(std::string mod, std::string fun, perl::Array params) : package(mod), sub(fun), args(params){}
	request(ErlNifEnv* env, std::string mod, std::string fun) : env(env), package(mod), sub(fun){}
};

// C++ function declarations
void runPerl(Interpreter universe);
void callPerl(const char* mod, const char* sub);
void callPerl2(const char* mod, const char* sub);

void perlDo(Interpreter* uni, request* req);

// C++ function definitions

static Interpreter Universe;
static std::mutex pIntLock;
static Interpreter Universe2;
static std::mutex pIntLock2;

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
}
static ERL_NIF_TERM perlLoad(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {}
static ERL_NIF_TERM perlRun(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {}

static ERL_NIF_TERM nifInit(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	Universe.set_context();
	Universe.use("NifTest");
	Universe2.set_context();
	Universe2.use("NifTest");
	return enif_make_atom(env, "oky");
}

static ERL_NIF_TERM nifRun(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	std::thread perlThread(callPerl, "NifTest", "HelloErlang");
	std::thread perlThread2(callPerl2, "NifTest", "HelloErlang");
	perlThread.detach();
	perlThread2.detach();
	return enif_make_atom(env, "booo");
}

void callPerl(const char* mod, const char* sub) {
	Universe.set_context();
	pIntLock.lock();
	Package p = Universe.package(mod);
	p.call(sub);
	pIntLock.unlock();
}
void callPerl2(const char* mod, const char* sub) {
	Universe2.set_context();
	pIntLock2.lock();
	Package p = Universe2.package(mod);
	p.call(sub);
	pIntLock2.unlock();
}

static ERL_NIF_TERM modTest(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	Interpreter universe;
	Package NifTest = universe.use("NifTest");
	
	String out = NifTest.call("HelloErlang");

    return enif_make_string(env, out.get_raw(), ERL_NIF_LATIN1);
}

static ERL_NIF_TERM hello2(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	const char * args[] = { "test!!!", "./test.pl" };
	Interpreter universe(2, args);

	std::thread perlThread(runPerl, universe);
	perlThread.detach();

    return enif_make_string(env, "Hello perl!", ERL_NIF_LATIN1);
}

void runPerl(Interpreter un) {
	Interpreter universe = un.clone();
	universe.set_context();
	universe.run();
}
