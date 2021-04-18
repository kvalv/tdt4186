Answers to task b):

Values found on WSL2, running on a Ryzen 7 5800X with 8 cores/16 threads

MAX BLOCKSIZE:  12198 bytes, bandwith drops to about 4GB/s at larger block sizes

MAX BANDWITH:   8694424000 @ blocksize =~ 8KB

Bandwidth seems to be shared between instances running on the same processor thread. I.e. running two instances with blocksize set to 8KB results in a bandwith at about 4GB/s on each.