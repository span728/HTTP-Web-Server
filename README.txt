
Stephanie Pan 
sp3507
Lab7 

Part 1
For part 1, I wgetted Jae's html file, then wgetted some images from Google
to create my own page.  Then, I made a directory structure that worked with
the webpage Jae provided.  Aunoy actually sent out an email with a hint, so
that helped.  Please go to www.clac.cs.columbia.edu/~sp3507/cs3157/tng/ to
see it.  

Part 2a 
For this part of the lab, I wrote an http-server that could connect with
clients and send them the files they requested.  This program is a
complement to the http-client from Lab6 Part 2.  To write it, I had to use
appropriate socket structure.  I also had to learn how to properly parse
requests from the client, and properly send back files in return with
correct html syntax. It works according to instructions.  

Part 2b
Finally, I added mdb-lookup functionalities, by turning  my http-server into
a client for the mdb-lookup-server we wrote in Lab6 Part 1 (although I used
Jae's version to test my code).  The hardest part was definitely string
parsing and making sure all the conversions were correct between client and
server, then between server and mdb-server, and vice versa.  This part of
the code also works properly, in that I can access the mdb-lookup form and
search for different terms, which will show up in a nice table on the
browser.  
There is one valgrind error, because the file descriptor I created for the
mdb socket is actually never closed - since its a longterm variable, I close
it right before the program exits.  However, the program is usually exitted
using ctrl-C, so it never gets to that line, thus producing some "still
reachable" memory.  


