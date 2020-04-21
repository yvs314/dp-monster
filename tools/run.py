import argparse
import csv
import os
import subprocess
from os.path import isfile, join as join_path

tasks = {
    "all": ["ESC07.sop", "ESC11.sop", "ESC12.sop", "ESC25.sop", "ESC47.sop", "ESC63.sop", "ESC78.sop", "br17.10.sop",
            "br17.12.sop", "ft53.1.sop", "ft53.2.sop", "ft53.3.sop", "ft53.4.sop", "ft70.1.sop", "ft70.2.sop",
            "ft70.3.sop", "ft70.4.sop", "kro124p.1.sop", "kro124p.2.sop", "kro124p.3.sop", "kro124p.4.sop", "p43.1.sop",
            "p43.2.sop", "p43.3.sop", "p43.4.sop", "prob.100.sop", "prob42.sop", "rbg048a.sop", "rbg050c.sop",
            "rbg109a.sop", "rbg150a.sop", "rbg174b.sop", "rbg253a.sop", "rbg323a.sop", "rbg341a.sop", "rbg358a.sop",
            "rbg378a.sop", "ry48p.1.sop", "ry48p.2.sop", "ry48p.3.sop", "ry48p.4.sop"],
    "130GB": ["ESC07.sop", "ESC11.sop", "ESC12.sop", "ESC25.sop", "br17.10.sop", "br17.12.sop", "ft53.4.sop",
              "ft70.4.sop", "p43.3.sop", "p43.4.sop", "rbg109a.sop", "rbg150a.sop", "rbg174b.sop", "rbg253a.sop",
              "ry48p.3.sop", "ry48p.4.sop"],
    "4gb-300sec": ["ESC07.sop", "ESC11.sop", "ESC12.sop", "ESC25.sop", "br17.10.sop", "br17.12.sop", "ft53.4.sop",
                   "ft70.4.sop", "p43.4.sop", "rbg109a.sop", "rbg150a.sop", "rbg174b.sop", "rbg253a.sop",
                   "ry48p.4.sop"],
    "5sec": ["ESC07.sop", "ESC11.sop", "ESC12.sop", "br17.10.sop", "br17.12.sop", "ft53.4.sop", "p43.4.sop",
             "rbg109a.sop", "rbg150a.sop", "ry48p.4.sop"],
    "largish": ["ESC25.sop", "ft70.4.sop", "rbg174b.sop", "rbg253a.sop"]
}


def run_task(task_dict, run):
    env = os.environ
    env['OMP_THREAD_LIMIT'] = "%s" % task_dict['threads']

    task = task_dict['task']
    data_filename = join_path(task_dict['data_dir'], task)
    out_dir = join_path(task_dict['out_dir'], "%srun%s" % (task_dict['prefix'], run))
    executable_ = os.path.relpath(task_dict['executable'], out_dir)
    data_filename_ = os.path.relpath(data_filename, out_dir)
    os.makedirs(out_dir, exist_ok=True)

    command = "%s %s" % (executable_, data_filename_)
    if task_dict['force']:
        command += " -f"
    command += " " + " ".join(task_dict['param'])

    print("Run DPM module on %s threads with command: %s" % (task_dict['threads'], command))

    if task_dict['docker']:
        parent_dir = os.path.abspath(os.path.join(os.getcwd(), os.pardir))
        thread_env = "OMP_THREAD_LIMIT=%s" % task_dict['threads']
        bash_cmd = "bash -c 'cd tools && cd %s && %s'" % (out_dir, command)
        docker_cmd = "docker run --env %s -it -v %s:/work dpm %s" % \
                     (thread_env, parent_dir, bash_cmd)
        command = docker_cmd

    if task_dict['slurm']:
        os.makedirs("./outs/", exist_ok=True)
        os.makedirs(os.path.join(out_dir, "./outs/"), exist_ok=True)
        srun_cmd = "srun -o ./outs/slurm-%%j.out -t %s --mem=%s -c %s" % (task_dict['time'], task_dict['mem'], task_dict['threads'])
        srun_cmd += " -p %s" % task_dict['part'] if task_dict['part'] is not None else ""
        srun_cmd += " --exclusive" if task_dict['exclusive'] else ""
        command = srun_cmd + ' ' + command
        # command = list(filter(len, command.split(' ')))
        cwd = out_dir if not task_dict['docker'] else None
        return subprocess.Popen(command, shell=True, env=env, cwd=cwd)
    else:
        try:
            subprocess.run(command, shell=True, check=True, cwd=out_dir, env=env)
        except Exception:
            # just do nothing
            pass


def get_arg_dict(task, common_task_dict):
    answer = common_task_dict.copy()
    answer['task'] = task
    return answer


def parse_batchfile(filename, common_task_dict):
    answer = []
    with open(filename) as csvfile:
        reader = csv.DictReader(csvfile, delimiter=';')
        for row in reader:
            task_dict = common_task_dict.copy()
            param = []
            for key in row:
                if row[key] is not None and len(row[key]) and key is not None:
                    if key in common_task_dict:
                        task_dict[key] = row[key]
                    else:
                        pref = '-' * min(len(key), 2)
                        param += [pref + key, row[key]]

            if 'param' in row and len(row['param']):
                param = row['param'].split(' ')
            task_dict['param'] = param
            answer.append(task_dict)
    return answer


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='DPM project runner')
    tasks_params = parser.add_mutually_exclusive_group(required=True)
    tasks_params.add_argument('--group', '-g', type=str, choices=list(tasks.keys()),
                              help='Select group of tasks for solving')
    tasks_params.add_argument('--task', '-t', type=str, choices=list(tasks["all"]), help='Select task for solving')
    tasks_params.add_argument('--batch', '-b', type=str, help='Select batch file with tasks for solving')

    parser.add_argument('--param', '-p', default="", help='Parameters for child dpm module spitted by comma',
                        type=lambda s: str(s).split(','))
    parser.add_argument('--out_dir', '-o', type=str, default=join_path("..", "results"),
                        help='Output directory. If not specified, ../results are used.')
    parser.add_argument('--force', '-f', action='store_true', help='Force run')
    parser.add_argument('--nruns', '-n', type=int, default=1, help='Number of runs')
    parser.add_argument('--prefix', type=str, default="", help='Prefix for run directories')
    parser.add_argument('--threads', type=int, default=1, help='Number of OMP_THREAD_LIMIT for subprocess')

    parser.add_argument('--slurm', '-s', action='store_true', help='Use slurm or not')
    parser.add_argument('--mem', type=str, default="251G", help='Memory limit for slurm')
    parser.add_argument('--time', type=str, default="20:0:0", help='Time limit for slurm')
    parser.add_argument('--exclusive', action='store_true', help='Exclusive run of slurm task')
    parser.add_argument('--part', type=str, default=None, help='Partition for slurm task')
    parser.add_argument('--docker', action='store_true', help='Use docker to run subprocess via slurm')
    args = parser.parse_args()

    #a dumb, hacky way to process t-SOPLIB06 would be to switch data_dir to the following:
    #data_dir = os.path.abspath(join_path("..", "data", "t-SOPLIB06"))
    data_dir = os.path.abspath(join_path("..", "data", "TSPLIB"))
    executable = os.path.abspath(join_path("..", "build", "dpm" + (".exe" if os.name == "nt" else "")))
    assert isfile(executable), "Executable file doesn't exist"

    common_task_dict = {
        'task': '',
        'threads': args.threads,
        'param': args.param,
        'prefix': args.prefix,
        'out_dir': args.out_dir,
        'force': args.force,
        'nruns': args.nruns,

        'slurm': args.slurm,
        'mem': args.mem,
        'time': args.time,
        'exclusive': args.exclusive,
        'part': args.part,
        'docker': args.docker,

        'data_dir': data_dir,
        'executable': executable
    }

    tasks_args = []
    if args.batch is not None:
        tasks_args = parse_batchfile(args.batch, common_task_dict)
    elif args.group is not None:
        tasks_args = list(map(lambda task: get_arg_dict(task, common_task_dict), tasks[args.group]))
    else:
        tasks_args = [get_arg_dict(args.task, common_task_dict)]

    proc = []
    for task_dict in tasks_args:
        for run in range(1, int(task_dict['nruns']) + 1):
            proc.append(run_task(task_dict, run))

    # wait subprocess we run with slurm
    for p in proc:
        if p is not None:
            p.wait()
