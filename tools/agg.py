import argparse
import pandas as pd
import os

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='DPM project aggregator')
    parser.add_argument('--out_dir', '-o', type=str, default=None,  help='Output directory. If not specified, ../results is used.')
    parser.add_argument('--in_dir', '-i', type=str, default=None,  help='Input directory. If not specified, ../results is used.')
    parser.add_argument('--file', '-f', type=str, default=None,  help='Output filename stored in output directory. If not specified, result.[EXT] is used.')
    parser.add_argument('--type', '-t', type=str, default="xlsx", choices=["xlsx","csv"],  help='Output filename format. If not specified, xlsx is used')
    parser.add_argument('--prefix', type=str, default="", help='Prefix run directories')
    args = parser.parse_args()


    in_dir = os.path.join("..", "results") if args.in_dir is None else in_dir
    out_dir = os.path.join("..", "results") if args.out_dir is None else out_dir
    out_file_name = "result" if args.file is None else args.file
    out_file_name += '.' + args.type

    col_names = ["task_name", "problem_type", "direction", "method"]
    mean_col = '%sruns_mean' % args.prefix

    out_file_name = os.path.join(out_dir, out_file_name)

    run_pref = "%srun" % args.prefix
    runs = list(filter(lambda s: s.startswith(run_pref), os.listdir(in_dir)))
    df = None
    for run in runs:
        run_dir = os.path.join(in_dir, run)
        run_logs = list(filter(lambda s: s.split('.')[-1].lower() == 'log', os.listdir(run_dir)))
        def extract_param(log_name):
            param = log_name.split('-')
            task_name = param[0]
            param[-1] = '.'.join(param[-1].split('.')[:-1])
            file_name = os.path.join(run_dir, log_name)
            param = dict(zip(range(1, len(param)+1), param))
            with open(file_name, "r") as f:
                run_time = float(f.read().split("BF + RECOVERY DURATION IN SECONDS:")[1])
                param[run] = (run_time)
            return param
        param_logs = list(map(extract_param, run_logs))
        new_df = pd.DataFrame(param_logs)
        if df is None:
            df = new_df
        else:
            df = pd.merge(df, new_df, how='outer')

    run_columns = sorted(list(filter(lambda s: isinstance(s, str) and s.startswith(run_pref), df.columns)))
    df[mean_col] = df[run_columns].mean(axis=1, skipna=True)
    df.columns = list(map(lambda c: c if (not isinstance(c, int)) or (c > len(col_names)) else col_names[c-1], df.columns))
    not_run_cols = list(filter(lambda s: not(isinstance(s, str) and s.startswith(run_pref)) and s != mean_col, df.columns))

    df = df[not_run_cols + [mean_col] + run_columns]
    if args.type == "xlsx":
        df.to_excel(out_file_name, index=False)
    if args.type == "csv":
        df.to_csv(out_file_name, index=False, sep=';')
