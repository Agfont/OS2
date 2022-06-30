# Maxim nombre de processos: 32
# Maxim nombre de cpus que es faran servir: 1
 
docker build -t fork-bomb .
docker run --ulimit nproc=32:64 --cpus 1 -ti fork-bomb

docker container ls
docker kill <container>
