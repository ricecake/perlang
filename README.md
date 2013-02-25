perlang
=======

embedded perl for erlang 

usage:

Eshell V5.8.3  (abort with ^G)
1> c(perlang).
{ok,perlang}
2> perlang:perlUse().
ok
3> perlang:perlCall().
ok
4> flush().
Shell got ok
ok

TODO:

needs to have ability to specify modules to load.
needs to have ability to specify functions to call in perl.
needs to have ability to load xs based packages.
needs to have ability to translate types erlang <-> perl 

needs proper make files.
needs tests.
needs to have request class spit into seperate file.
need to consider having multiverse/queue class for simplified interface.

need ass load of helper methods for erlang types/methods/everything.  
