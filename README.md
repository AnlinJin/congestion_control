#  A BitTorrent-like file transfer application with congestion control
## File function
hupsim.pl - This file emulates a network topology using topo.map (see Section 7)
- sha.[c|h] - The SHA-1 hash generator
- input buffer.[c|h] - Handle user input
- debug.[c|h] - helpful utilities for debugging output
- bt parse.[c|h] - utilities for parsing commandline arguments.
- peer.c - A skeleton peer file. Handles some of the setup and processing for you.
- nodes.map - provides the list of peers in the network
- topo.map - the hidden network topology used by hupsim.pl. This should be interpreted only by the hupsim.pl,
your code should not read this file. You may need to modify this file when using hupsim.pl to test the congestion
avoidance part of your program.
- make-chunks - program to create new chunk files given an input file that contains chunk-id, hash pairs, useful
for creating more larger file download scenarios.
## How the file transfer works
For complete explanation, please refer to the course project page of CMU15441 project2 
> https://www.cs.cmu.edu/~prs/15-441-F16/assignments.html
