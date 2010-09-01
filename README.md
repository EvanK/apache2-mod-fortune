mod_fortune
===


Covering My Behind
--
This is my first foray into writing Apache2 modules, and it is meant as both a learning exercise for me and an entertaining piece of open source software for the world at large.  It is by no means optimized for high-availability environments and I make no guarantee that it is free of bugs or security flaws.  Use at your own risk!


What Is `fortune`?
--
Anyone that's made heavy use of Linux has probably come across the `fortune` program at some point.  It outputs "a random, hopefully interesting, adage" on demand, a sort of digital fortune cookie (hence the name).


What is `mod_fortune`?
--
It is a module for the Apache2 webserver that pipes the output of the `fortune` command into an environment variable.  This results in each and every request to the webserver having available a different fortune that can be dumped into a header, accessed by a CGI script, or any other number of things.


Why?
--
I wrote this mostly for fun and because it's been a while since I dabbled in the C language.


Licensing
--
This software is released as open source under the MIT license:
    http://www.opensource.org/licenses/mit-license.php


Usage Example
--
This is an example configuration that would insert an `X-Fortune` header to every successful request.
    # if mod_header is enabled...
    <IfModule mod_header.c>
        # load and configure mod_fortune (default path but custom max length)
        LoadModule fortune_module modules/mod_fortune.so
        
        <IfModule mod_fortune.c>
            FortuneMaxLength 1000
            #FortuneProgram /usr/games/fortune
        </IfModule>
        
        # set X-Fortune header for successful response, if env variable exists
        Header onsuccess set X-Fortune %{FORTUNE_COOKIE}e env=FORTUNE_COOKIE
    </IfModule>
