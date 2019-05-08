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
    parser.add_argument('--task', '-t', type=str, choices=list(tasks.keys()), required=True, help='Select subtasks for solving')
    parser.add_argument('--slurm', '-s', type=bool, help='Use slurm or not')
    parser.add_argument('--out_dir', '-o', type=str, default=None,  help='Output directory. If not specified, ../results are used.')
    parser.add_argument('--force', '-f', action='store_true',  help='Force run')
    args = parser.parse_args()

    subtasks = tasks[args.task]

    data_dir = join_path("..", "data", "TSPLIB")
    out_dir = join_path("..", "results") if args.out_dir is None else args.out_dir
    os.makedirs(out_dir, exist_ok=True)
    executable = join_path("..", "build", "dpm" + (".exe" if os.name == "nt" else ""))
    assert isfile(executable), "Executable file doesn't exist"

    for task in subtasks:
        data_filename = join_path(data_dir, task)
        assert isfile(data_filename), "Data file doesn't exists"
        suffix = "sop-TSP-BWD-DP"
        log_filename = join_path(out_dir, "%s.%s.log" % (task[:-4], suffix))
        dump_filename = join_path(out_dir, "%s.%s.dump" % (task[:-4], suffix))

        if args.force or (not isfile(log_filename) and not isfile(dump_filename)):
            if args.slurm:
                raise NotImplementedError()
            else:
                subprocess.run("%s %s" % (executable, data_filename), shell=True, check=True, cwd=out_dir)
        else:
            print("Task %s already computed" % task)
