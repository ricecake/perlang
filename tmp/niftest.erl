-module(niftest).

-export([init/0, hello/0, hello2/0, modTest/0, nifInit/0, nifRun/0, perlInit/0, perlUse/0, perlCall/0]).

init() ->
      erlang:load_nif("./niftest", 0).

hello() ->
      "NIF library not loaded".

hello2() -> "gah fuck".

modTest() ->
      "NIF library not loaded".
nifInit() ->
      "NIF library not loaded".
nifRun() ->
      "NIF library not loaded".
perlInit() ->
      "NIF library not loaded".
perlUse() ->
      "NIF library not loaded".
perlCall() ->
      "NIF library not loaded".
