# Compile process A
gcc src_mmap/processAmmap.c -lncurses -lbmp -lm -o bin/processAmmap &

# Compile process B
gcc src_mmap/processBmmap.c -lncurses -lbmp -lm -o bin/processBmmap &

# Compile master process
gcc src_mmap/mastermmap.c -o bin/mastermmap
