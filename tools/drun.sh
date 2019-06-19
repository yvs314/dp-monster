args="$@"
docker run -it -v $PWD/..:/work dpm bash -c "cd tools && python3 run.py $args"
