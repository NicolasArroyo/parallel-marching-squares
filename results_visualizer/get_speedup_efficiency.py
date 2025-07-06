import re
from collections import defaultdict

def parse_results(filename):
    results = defaultdict(lambda: [])
    with open(filename, 'r') as f:
        size = threads = None
        for line in f:
            line = line.strip()
            if not line:
                continue
            match = re.match(r'\[(\d+),\s*(\d+)\]', line)
            if match:
                size, threads = int(match.group(1)), int(match.group(2))
            else:
                if size is not None and threads is not None:
                    t = float(line.replace('ms.', '').replace('ms', '').strip())
                    results[(size, threads)].append(t)
    return results

def compute_speedup_efficiency(results):
    speedup = {}
    efficiency = {}
    ts_means = {}
    for (size, threads), times in results.items():
        if threads == 1:
            ts_means[size] = sum(times) / len(times)
    for (size, threads), times in results.items():
        tp = sum(times) / len(times)
        ts = ts_means[size]
        sp = ts / tp
        eff = sp / threads
        speedup[(size, threads)] = sp
        efficiency[(size, threads)] = eff
    return speedup, efficiency

def write_results(results, filename, label):
    with open(filename, 'w') as f:
        current_size = None
        for (size, threads) in sorted(results.keys()):
            if size != current_size:
                if current_size is not None:
                    f.write('\n')
                f.write(f'[{size}]\n')
                current_size = size
            f.write(f'{threads}: {results[(size, threads)]:.4f}\n')
        f.write('\n')

if __name__ == '__main__':
    results = parse_results('resultados_non_opt.txt')
    speedup, efficiency = compute_speedup_efficiency(results)
    write_results(speedup, 'speedup_non_opt.txt', 'Speedup')
    write_results(efficiency, 'efficiency_non_opt.txt', 'Efficiency')
    print('Done!')
