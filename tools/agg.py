import argparse
import pandas as pd
import os


def aggregate(args):
    in_dir, out_dir = args.in_dir, args.in_dir if args.out_dir is None else args.out_dir
    out_file_name = "%sresult" % args.prefix if args.file is None else args.file
    out_file_name += '.' + args.type

    col_names = ["task_name", "problem_type", "direction", "method"]
    avg_time_col = 'avg_time'
    avg_ram_col = 'avg_RAM'

    out_file_name = os.path.join(out_dir, out_file_name)

    run_pref = "%srun" % args.prefix
    runs = list(filter(lambda s: s.startswith(run_pref), os.listdir(in_dir)))
    df = None
    for run in runs:
        run_dir = os.path.join(in_dir, run)
        run_logs = list(filter(lambda s: s.split('.')[-1].lower() == 'log', os.listdir(run_dir)))

        def parse_ram(s):
            ram = s.split('~')
            xbytes = ''.join(filter(lambda c: not c.isdigit(), ram[0])).strip()
            ram = list(map(float, map(lambda s: "".join(filter(lambda c: c.isdigit(), s)), ram)))
            if len(ram) > 1:
                ram[0] += float(ram[1]) / 1024.
            return "%0.3f %s" % (ram[0], xbytes)

        def extract_param(log_name):
            param = log_name.split('-')
            task_name = param[0]
            param[-1] = '.'.join(param[-1].split('.')[:-1])
            log_file_name = os.path.join(run_dir, log_name)
            dump_file_name = log_file_name[:-3] + 'dump'
            i = run.split("run")[-1]

            param = dict(zip(range(1, len(param) + 1), param))

            param['success'] = int(os.path.isfile(dump_file_name))
            start, states, time, ram, layer = [None] * 5

            if os.path.isfile(dump_file_name):
                with open(dump_file_name, "r") as f:
                    param['value'] = int(f.read().split("VALUE:")[1].splitlines()[0])
                with open(log_file_name, "r") as f:
                    fl = f.read()
                    start = pd.to_datetime(fl.splitlines()[0].split("Started on")[-1].strip())
                    states = fl.split("TOTAL STATES PROCESSED:")[-1].splitlines()[0].strip()
                    time = float(fl.split("TOTAL DURATION IN SECONDS:").pop().splitlines()[0])
                    ram = parse_ram(fl.split("RAM USAGE AT LAST LAYER:")[-1].splitlines()[0])
                    layer = fl.split("B;")[-3].splitlines()[-1].split(';')[0]
            else:
                layer = 0
                with open(log_file_name, "r") as f:
                    fl = f.read()
                    start = pd.to_datetime(fl.splitlines()[0].split("Started on")[-1].strip())
                    last_line = fl.splitlines()[-1].split(';')
                    if last_line[0].isdigit():
                        layer = last_line[0]
                        states = last_line[-2]
                        ram = parse_ram(last_line[-4])
                        time = pd.to_timedelta(last_line[2]).total_seconds()

            param["start%s" % i] = start
            param["states%s" % i] = states
            param["time%s" % i] = time
            param["layer%s" % i] = layer
            param["RAM%s" % i] = ram

            return param

        param_logs = list(map(extract_param, run_logs))
        new_df = pd.DataFrame(param_logs)
        if df is None:
            df = new_df
        else:
            df = pd.merge(df, new_df, how='outer')

    time_columns = sorted(list(filter(lambda s: isinstance(s, str) and s.startswith('time'), df.columns)))
    df[avg_time_col] = df[time_columns].mean(axis=1, skipna=True)

    ram_columns = sorted(list(filter(lambda s: isinstance(s, str) and s.startswith('RAM'), df.columns)))
    # df[avg_ram_col] = df[ram_columns].mean(axis=1, skipna=True)

    df.columns = list(
        map(lambda c: c if (not isinstance(c, int)) or (c > len(col_names)) else col_names[c - 1], df.columns))
    not_run_cols = list(
        filter(lambda s: not (isinstance(s, str) and s.startswith('time')) and s != avg_time_col, df.columns))

    df = df[not_run_cols + [avg_time_col] + time_columns]
    if args.type == "xlsx":
        df.to_excel(out_file_name, index=False)
    if args.type == "csv":
        df.to_csv(out_file_name, index=False, sep=';')
    return df


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='DPM project aggregator')
    parser.add_argument('--in_dir', '-i', type=str, default=os.path.join("..", "results"),
                        help='Input directory. If not specified, ../results is used.')
    parser.add_argument('--out_dir', '-o', type=str, default=None,
                        help='Output directory. If not specified, --in_dir is used.')
    parser.add_argument('--file', '-f', type=str, default=None,
                        help='Output filename stored in output directory. If not specified, result.[EXT] is used.')
    parser.add_argument('--type', '-t', type=str, default="xlsx", choices=["xlsx", "csv"],
                        help='Output filename format. If not specified, xlsx is used')
    parser.add_argument('--prefix', type=str, default=None, help='Prefix run directories')
    args = parser.parse_args()

    if args.prefix is not None:
        aggregate(args)
        exit(0)

    # scan all available prefixes
    in_dir = args.in_dir
    exp_dirs = filter(lambda d: os.path.isdir(os.path.join(in_dir, d)), os.listdir(in_dir))
    prefixes = set(map(lambda s: 'run'.join(s.split('run')[:-1]), exp_dirs))

    dfs = []
    for prefix in sorted(prefixes):
        args.prefix = prefix
        df = aggregate(args)
        df.insert(0, 'pref', prefix, True)
        dfs.append(df)
    dfs = pd.concat(dfs, sort=False)

    out_dir = args.in_dir if args.out_dir is None else args.out_dir
    out_file_name = os.path.join(out_dir, "results." + args.type)

    if args.type == "xlsx":
        dfs.to_excel(out_file_name, index=False)
    if args.type == "csv":
        dfs.to_csv(out_file_name, index=False, sep=';')
