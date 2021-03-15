# SpotifIUL

All you need is a linux-based OS

A C Server-Client aplication that simulates a music streaming service (Does not Play Music), the administrator can load files into a shared memory, read the content of the shared
memory, flush the content form the shared memory into a file and put the server under maintnence (Sleeps for 10 secnds), the user can Log In, Log Out, Afiliate to Playlist, View
Playlist and Listen to playlist.

This application uses message queues and shared memory for communication  and semaphores to deal with concurrency. 
