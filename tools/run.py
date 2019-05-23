import argparse
import os
import subprocess
from os.path import isfile, join as join_path

tasks = {
    "all" : ["ESC07.sop","ESC11.sop","ESC12.sop","ESC25.sop","ESC47.sop","ESC63.sop","ESC78.sop","br17.10.sop","br17.12.sop","ft53.1.sop","ft53.2.sop","ft53.3.sop","ft53.4.sop","ft70.1.sop","ft70.2.sop","ft70.3.sop","ft70.4.sop","kro124p.1.sop","kro124p.2.sop","kro124p.3.sop","kro124p.4.sop","p43.1.sop","p43.2.sop","p43.3.sop","p43.4.sop","prob.100.sop","prob42.sop","rbg048a.sop","rbg050c.sop","rbg109a.sop","rbg150a.sop","rbg174b.sop","rbg253a.sop","rbg323a.sop","rbg341a.sop","rbg358a.sop","rbg378a.sop","ry48p.1.sop","ry48p.2.sop","ry48p.3.sop","ry48p.4.sop"],
    "130GB" : ["ESC07.sop","ESC11.sop","ESC12.sop","ESC25.sop","br17.10.sop","br17.12.sop","ft53.4.sop","ft70.4.sop","p43.3.sop","p43.4.sop","rbg109a.sop","rbg150a.sop","rbg174b.sop","rbg253a.sop","ry48p.3.sop","ry48p.4.sop"],
    "4gb-300sec" : ["ESC07.sop","ESC11.sop","ESC12.sop","ESC25.sop","br17.10.sop","br17.12.sop","ft53.4.sop","ft70.4.sop","p43.4.sop","rbg109a.sop","rbg150a.sop","rbg174b.sop","rbg253a.sop","ry48p.4.sop"],
    "5sec" : ["ESC07.sop","ESC11.sop","ESC12.sop","br17.10.sop","br17.12.sop","ft53.4.sop","p43.4.sop","rbg109a.sop","rbg150a.sop","ry48p.4.sop"]
}

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='DPM project runner')
    tasks_params = parser.add_mutually_exclusive_group(required=True)
    tasks_params.add_argument('--group', '-g', type=str, choices=list(tasks.keys()), help='Select group of tasks for solving')
    tasks_params.add_argument('--task', '-t', type=str, choices=list(tasks["all"]), help='Select task for solving')
    parser.add_argument('--param', '-p', default="", help='Parameters for child dpm module spitted by comma', type=lambda s: str(s).split(','))
    parser.add_argument('--slurm', '-s', action='store_true', help='Use slurm or not')
    parser.add_argument('--out_dir', '-o', type=str, default=None,  help='Output directory. If not specified, ../results are used.')
    parser.add_argument('--force', '-f', action='store_true', help='Force run')
    parser.add_argument('--nruns', '-n', type=int, default=5, help='Number of runs')
    parser.add_argument('--prefix', type=str, default="", help='Prefix for run directories')
    args = parser.parse_args()

    if args.group is not  None:
        subtasks = tasks[args.group]
    else:
        subtasks = [args.task]

    data_dir = os.path.abspath(join_path("..", "data", "TSPLIB"))
    base_out_dir = join_path("..", "results") if args.out_dir is None else args.out_dir
    executable = os.path.abspath(join_path("..", "build", "dpm" + (".exe" if os.name == "nt" else "")))
    assert isfile(executable), "Executable file doesn't exist"

    proc = []
    for task in subtasks:
        data_filename = join_path(data_dir, task)
        assert isfile(data_filename), "Data file doesn't exists"
        for i in range(1, args.nruns+1):
            out_dir = join_path(base_out_dir, "%srun%s" % (args.prefix, i))
            executable_ = os.path.relpath(executable, out_dir)
            data_filename_ = os.path.relpath(data_filename, out_dir)
            os.makedirs(out_dir, exist_ok=True)

            command = "%s %s" % (executable_, data_filename_)
            if args.force:
                command += " -f"
            command += " " + " ".join(args.param)

            print("Run DPM module with command: %s" % command)
            if args.slurm:
                command = "srun -t 20:0:0 --mem=251G %s" % command
                command = list(filter(len, command.split(' '))) + [file_name]
                proc.append(subprocess.Popen(command, stdout=subprocess.PIPE))
            else:
                subprocess.run(command, shell=True, check=True, cwd=out_dir)

    # wait subprocess we run with slurm
    for p in proc:
        p.wait()
