-module(perlang).

-export([init/0, perlInit/0, perlUse/0, perlCall/0]).
-on_load(init/0).
init() ->
      erlang:load_nif("./perlang", 0),
	  perlInit().

perlInit() ->
      "NIF library not loaded".
perlUse() ->
      "NIF library not loaded".
perlCall() ->
      "NIF library not loaded".
