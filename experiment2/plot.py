import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
from matplotlib.lines import Line2D

markdown = """
| Compiler | Backend | Flags | Compile (s) | Run (s) | Status |
|----------|---------|-------|-------------|---------|--------|
| gcc | - | -O0 | 0.8071 | 132.7883 | OK |
| gcc | - | -O1 | 1.8752 | 57.2677 | OK |
| gcc | - | -O2 | 3.6452 | 52.3576 | OK |
| gcc | - | -O3 | 4.7808 | 50.5865 | OK |
| clang | - | -O0 | 0.3434 | 142.3183 | OK |
| clang | - | -O1 | 1.6552 | 53.911 | OK |
| clang | - | -O2 | 2.0434 | 51.2614 | OK |
| clang | - | -O3 | 2.0987 | 51.2867 | OK |
| zig-cc | - | -O3 | 2.9528 | 53.8927 | OK |
| tcc | - | (default) | 0.0208 | 147.7219 | OK |
| cproc | qbe | (default) | 0.2068 | 89.001 | OK |
| antcc | - | -O0 | 0.0694 | 132.4988 | OK |
| antcc | - | -O3 | 0.0844 | 74.2026 | OK |
| madc | - | -O0 | 0.5937 | 120.8796 | OK |
| madc | - | -O1 | 0.6301 | 105.7359 | OK |
| madc | - | -O2 | 0.7064 | 95.2928 | OK |
| mycc | llvm | (default) | 1.2673 | 60.5642 | OK |
| mycc | llvm | --final | 2.4541 | 51.9651 | OK |
| mycc | c | (default) | 2.3358 | 56.8405 | OK |
| mycc | c | --final | 3.0562 | 52.711 | OK |
| mycc | gm | (default) | 0.693 | 58.1035 | OK |
| mycc | gm | --final | 0.8034 | 52.803 | OK |
| mycc | qbe | (default) | 0.6859 | 77.6846 | OK |
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

for i, (x, y, label) in enumerate(zip(x_coords, y_coords, labels)):
    if i % 2 == 0:
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

ax.legend(handles=legend_elements, loc='upper right', facecolor='#2d2d44',
          edgecolor='gray', labelcolor='white', fontsize=9, ncol=1)

ax.set_title('C compilers on index-c benchmark: compile time vs. runtime for index.c',
             color='white', fontsize=16, fontweight='bold', pad=25)

plt.tight_layout()
plt.savefig('experiment2_plot1.png', dpi=150, facecolor='#1a1a2e', bbox_inches='tight')
plt.close(fig)
print(f"Plotted {len(data)} points.")