import pandas as pd
import matplotlib.pyplot as plt
import networkx as nx
import numpy as np

# Read the file specifying column names
df = pd.read_csv("toy_example/peer_report.txt", 
                 names=["IN", "OUT", "KEY", "CONNECTION", "TIME(s)"])
print(df)

# Configure plot style
plt.style.use('seaborn-v0_8')
fig = plt.figure(figsize=(16, 12))

# 1. Network graph with times as edge weights
ax1 = plt.subplot(2, 3, 1)
G = nx.DiGraph()

for _, row in df.iterrows():
    G.add_edge(row['IN'], row['OUT'], weight=row['TIME(s)'], key=row['KEY'])

pos = nx.spring_layout(G, seed=42)
edges = G.edges()
weights = [G[u][v]['weight'] * 10000 for u, v in edges]  # Scale for better visualization

# Create mappable for colorbar
edge_collection = nx.draw(G, pos, 
        with_labels=True, 
        node_color='lightblue', 
        node_size=1000,
        edge_color=weights,
        edge_cmap=plt.cm.viridis,
        width=2,
        arrows=True,
        arrowsize=20)

plt.title('P2P Network - Node Connections\n(Edge color = Connection time)')

# Create mappable for colorbar
sm = plt.cm.ScalarMappable(cmap=plt.cm.viridis, norm=plt.Normalize(vmin=min(weights), vmax=max(weights)))
sm.set_array([])
plt.colorbar(sm, ax=ax1, label=r'Time ($10^{-4}$ s)', shrink=0.8)

# 2. Bar chart - Connection times
plt.subplot(2, 3, 2)
connection_labels = [f"{row['IN']}→{row['OUT']}" for _, row in df.iterrows()]
times = df['TIME(s)'] * 1000  # Convertir a milisegundos

bars = plt.bar(range(len(times)), times, color='skyblue', alpha=0.7)
plt.xlabel('Connections')
plt.ylabel('Time (ms)')
plt.title('Key Derivation Time')
plt.xticks(range(len(connection_labels)), connection_labels, rotation=45)

# Add values on bars
for i, (bar, time) in enumerate(zip(bars, times)):
    plt.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.001,
             f'{time:.3f}', ha='center', va='bottom', fontsize=8)

plt.grid(axis='y', alpha=0.3)

# 3. Distribution of generated keys
plt.subplot(2, 3, 3)
plt.scatter(df['KEY'], df['KEY'].value_counts(), color='lightgreen', alpha=0.7, edgecolor='black')

plt.xlabel('Key Value')
plt.ylabel('Frequency')
plt.title('Distribution of Generated Keys')
plt.grid(axis='y', alpha=0.3)

# 4. Average time per source node
plt.subplot(2, 3, 4)
avg_time_by_node = df.groupby('IN')['TIME(s)'].mean() * 1000
nodes = avg_time_by_node.index
avg_times = avg_time_by_node.values

plt.bar(nodes, avg_times, color='coral', alpha=0.7)
plt.xlabel('Source Node')
plt.ylabel('Average Time (ms)')
plt.title('Average Derivation Time per Node')
plt.grid(axis='y', alpha=0.3)

for node, time in zip(nodes, avg_times):
    plt.text(node, time + 0.005, f'{time:.3f}', ha='center', va='bottom')

# 5. Scatter plot - Relationship between nodes and time
plt.subplot(2, 3, 5)
colors = ['red', 'blue', 'green', 'orange', 'purple', 'brown']
for i, in_node in enumerate(df['IN'].unique()):
    subset = df[df['IN'] == in_node]
    plt.scatter(subset['OUT'], subset['TIME(s)'] * 1000, 
               label=f'Nodo {in_node}', color=colors[i % len(colors)], s=100)

plt.xlabel('Destination Node')
plt.ylabel('Time (ms)')
plt.title('Connection Time by Node Pair')
plt.legend()
plt.grid(True, alpha=0.3)

# 6. Adjacency matrix with key values
ax6 = plt.subplot(2, 3, 6)
nodes_all = sorted(set(df['OUT'].tolist() + df['IN'].tolist()))
matrix = np.zeros((len(nodes_all), len(nodes_all)))

for _, row in df.iterrows():
    i = nodes_all.index(row['IN'])
    j = nodes_all.index(row['OUT'])
    matrix[i][j] = row['KEY']

im = plt.imshow(matrix, cmap='viridis', aspect='auto')
plt.colorbar(im, ax=ax6, label='Key Value')
plt.title('Key Matrix Between Nodes')
plt.xlabel('Destination Node')
plt.ylabel('Source Node')
plt.xticks(range(len(nodes_all)), nodes_all)
plt.yticks(range(len(nodes_all)), nodes_all)

# Add values in cells
for i in range(len(nodes_all)):
    for j in range(len(nodes_all)):
        if matrix[i][j] != 0:
            plt.text(j, i, f'{int(matrix[i][j])}', 
                    ha='center', va='center', color='white', fontweight='bold')

plt.tight_layout()
plt.show()

# Additional statistics
print("\n" + "="*50)
print("KEY EXCHANGE STATISTICS")
print("="*50)
print(f"Total number of connections: {len(df)}")
print(f"Average time: {df['TIME(s)'].mean()*1000:.3f} ms")
print(f"Minimum time: {df['TIME(s)'].min()*1000:.3f} ms")
print(f"Maximum time: {df['TIME(s)'].max()*1000:.3f} ms")
print(f"Standard deviation: {df['TIME(s)'].std()*1000:.3f} ms")
print(f"Key range: {df['KEY'].min()} - {df['KEY'].max()}")
print(f"Success rate: {df['CONNECTION'].mean()*100:.1f}%")
