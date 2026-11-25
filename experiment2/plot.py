import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
from matplotlib.lines import Line2D

markdown = """
| Compiler | Backend | Flags | Compile (s) | Run (s) | Status |
|----------|---------|-------|-------------|---------|--------|
| gcc | - | -O0 | 0.7507 | 132.7446 | OK |
| gcc | - | -O1 | 1.8708 | 57.3448 | OK |
| gcc | - | -O2 | 3.6382 | 52.1492 | OK |
| gcc | - | -O3 | 4.7649 | 50.5582 | OK |
| clang | LLVM | -O0 | 0.3468 | 142.7883 | OK |
| clang | LLVM | -O1 | 1.6639 | 54.0931 | OK |
| clang | LLVM | -O2 | 2.0476 | 51.1437 | OK |
| clang | LLVM | -O3 | 2.0862 | 51.3251 | OK |
| zig-cc | LLVM | -O3 | 2.8599 | 53.9506 | OK |
| tcc | - | (default) | 0.0207 | 147.9381 | OK |
| cproc | QBE | (default) | 0.203 | 88.9362 | OK |
| antcc | - | -O0 | 0.0702 | 131.1418 | OK |
| antcc | - | -O3 | 0.0854 | 74.2476 | OK |
| madc | MIR | -O0 | 0.5879 | 120.9439 | OK |
| madc | MIR | -O1 | 0.6291 | 105.7785 | OK |
| madc | MIR | -O2 | 0.7044 | 95.2722 | OK |
| mucc | - | -O0 | 0.1734 | 223.8188 | OK |
| cppgm | GM | -O0 | 0.592 | 121.0911 | OK |
| cppgm | GM | -O1 | 0.6287 | 105.8937 | OK |
| cppgm | GM | -O2 | 0.7068 | 95.2613 | OK |
| cppgm | GM | -O3 | 0.7066 | 95.0949 | OK |
| mycc | LLVM | (default) | 1.2636 | 60.8043 | OK |
| mycc | LLVM | --final | 2.4311 | 51.9936 | OK |
| mycc | C | (default) | 2.3307 | 57.0358 | OK |
| mycc | C | --final | 3.0653 | 52.6486 | OK |
| mycc | GM | (default) | 0.6971 | 57.4345 | OK |
| mycc | GM | --final | 0.81 | 52.6276 | OK |
| mycc | QBE | (default) | 0.6802 | 77.5978 | OK |
"""

lines = markdown.strip().split('\n')
data = []
for line in lines[2:]:
    parts = [p.strip() for p in line.split('|') if p.strip()]
    if len(parts) < 6:
        continue
    compiler, backend, flags_str = parts[0], parts[1], parts[2]
    try:
        compile_time = float(parts[3])
        runtime = float(parts[4])
    except ValueError:
        continue
    label_parts = [compiler]
    if backend and backend != '-':
        label_parts.append(backend)
    if flags_str and flags_str != '-':
        label_parts.append(flags_str)
    name = ' '.join(label_parts)
    data.append((name, compile_time, runtime))

def pareto_front(points):
    ordered = sorted(points, key=lambda p: (p[1], p[2]))
    front, best_y = [], float('inf')
    for p in ordered:
        if p[2] < best_y:
            front.append(p)
            best_y = p[2]
    return front

pareto = pareto_front(data)
px = [p[1] for p in pareto]
py = [p[2] for p in pareto]
print('Pareto front:', [n for n, _, _ in pareto])

colors = [
    '#FF6B6B', '#FF8E8E',
    '#FFA07A', '#FFB347',
    '#E74C3C', '#E67E22',
    '#D35400', '#F39C12',
    '#4ECDC4', '#45B7AA',
    '#3AAFA0', '#2E8B7F',
    '#9B59B6', '#A569BD',
    '#B07CC6', '#8E44AD',
    '#2ECC71', '#27AE60',
    '#1ABC9C', '#16A085',
    '#3498DB', '#2980B9',
    '#F1C40F',
]

fig, ax = plt.subplots(figsize=(16, 10), dpi=100)
fig.patch.set_facecolor('#1a1a2e')
ax.set_facecolor('#1a1a2e')

x_coords = [item[1] for item in data]
y_coords = [item[2] for item in data]
labels = [name for name, _, _ in data]

for i, (x, y) in enumerate(zip(x_coords, y_coords)):
    ax.scatter(x, y, c=colors[i % len(colors)], s=250,
               edgecolors='white', linewidth=1.2, zorder=5)

ax.set_yscale('log')

ax.yaxis.set_major_formatter(mticker.FuncFormatter(lambda v, _: f'{v:g}'))
ax.yaxis.set_minor_formatter(mticker.NullFormatter())
ax.yaxis.set_major_locator(mticker.FixedLocator([50, 60, 70, 80, 90, 100, 120, 140]))
ax.set_ylim(45, 160)

ax.set_xlim(-0.1, 5.2)

PARETO_COLOR = '#F1C40F'
ax.plot(px, py, linestyle='--', color=PARETO_COLOR,
        linewidth=1.6, alpha=0.9, zorder=4)
for x, y in zip(px, py):
    ax.scatter(x, y, s=420, facecolors='none',
               edgecolors=PARETO_COLOR, linewidth=2, zorder=5)

for i, (x, y, label) in enumerate(zip(x_coords, y_coords, labels)):
    if i % 2 == 1:
        offset, va = (0, 14), 'bottom'
    else:
        offset, va = (0, -14), 'top'
    ax.annotate(
        label,
        (x, y),
        xytext=offset, textcoords="offset points",
        ha='center', va=va,
        fontsize=8, color='white',
        zorder=6,
    )

ax.set_xlabel('Compile time (seconds)', color='white', fontsize=14)
ax.set_ylabel('Runtime (seconds, log scale)', color='white', fontsize=14)
ax.tick_params(colors='white', labelsize=11)
ax.spines['bottom'].set_color('gray')
ax.spines['left'].set_color('gray')
ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)
ax.grid(True, alpha=0.15, color='white', which='major')

legend_elements = []
for i, (name, _, _) in enumerate(data):
    legend_elements.append(Line2D([0], [0], marker='o', color='w',
                                  markerfacecolor=colors[i % len(colors)],
                                  markersize=10, label=name))
legend_elements.append(Line2D([0], [0], linestyle='--', color=PARETO_COLOR,
                              linewidth=1.6, label='Pareto front'))

ax.legend(handles=legend_elements, loc='upper right', facecolor='#2d2d44',
          edgecolor='gray', labelcolor='white', fontsize=9, ncol=1)

ax.set_title('C compilers on index-c benchmark: compile time vs. runtime for index.c',
             color='white', fontsize=16, fontweight='bold', pad=25)

plt.tight_layout()
plt.savefig('experiment2_plot1.png', dpi=150, facecolor='#1a1a2e', bbox_inches='tight')
plt.close(fig)