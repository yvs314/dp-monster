args="$@"
docker run -it -u=$(id -u):$(id -g) -v $PWD/..:/work dpm bash -c "cd tools && python3 run.py $args"
