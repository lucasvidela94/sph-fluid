import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import sys
sys.path.insert(0, '/home/sombi/sph_fluid_sim')

from python.sph_simulator import SPHSimulator

def run_animation(n_particles=500, n_steps=500, interval=20):
    """Ejecutar animación del fluido."""
    
    # Crear simulador
    sim = SPHSimulator(n_particles=n_particles, domain=(0, 0, 2, 1.5))
    
    # Setup de matplotlib
    fig, ax = plt.subplots(figsize=(10, 6))
    ax.set_xlim(0, 2)
    ax.set_ylim(0, 1.5)
    ax.set_aspect('equal')
    ax.set_title('SPH Fluid Simulation - Dam Break')
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    
    # Inicializar scatter plot
    positions = sim.get_positions()
    scatter = ax.scatter(positions[:, 0], positions[:, 1], s=20, c='blue', alpha=0.6)
    
    # Texto de info
    info_text = ax.text(0.02, 0.98, '', transform=ax.transAxes, 
                        verticalalignment='top', fontsize=10)
    
    def update(frame):
        sim.step()
        positions = sim.get_positions()
        scatter.set_offsets(positions)
        info_text.set_text(f'Step: {frame + 1}/{n_steps} | Particles: {n_particles}')
        return scatter, info_text
    
    anim = FuncAnimation(fig, update, frames=n_steps, interval=interval, blit=True)
    plt.tight_layout()
    plt.show()

def run_static(n_particles=500, n_steps=100):
    """Ejecutar simulación y mostrar resultado final."""
    
    sim = SPHSimulator(n_particles=n_particles, domain=(0, 0, 2, 1.5))
    
    print(f"Simulando {n_particles} partículas por {n_steps} pasos...")
    
    for i in range(n_steps):
        sim.step()
        if (i + 1) % 100 == 0:
            print(f"  Paso {i + 1}/{n_steps}")
    
    positions = sim.get_positions()
    
    # Plot final
    plt.figure(figsize=(10, 6))
    plt.scatter(positions[:, 0], positions[:, 1], s=20, alpha=0.6)
    plt.xlim(0, 2)
    plt.ylim(0, 1.5)
    plt.title(f'SPH Fluid Simulation - Final State ({n_steps} steps)')
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.axis('equal')
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig('/home/sombi/sph_fluid_sim/examples/dam_break_final.png', dpi=150)
    print(f"\nGuardado en: examples/dam_break_final.png")
    plt.show()

if __name__ == '__main__':
    import argparse
    
    parser = argparse.ArgumentParser(description='SPH Fluid Simulator')
    parser.add_argument('--particles', '-p', type=int, default=500, help='Número de partículas')
    parser.add_argument('--steps', '-s', type=int, default=1000, help='Número de pasos')
    parser.add_argument('--static', action='store_true', help='Modo estático (sin animación)')
    
    args = parser.parse_args()
    
    if args.static:
        run_static(args.particles, args.steps)
    else:
        run_animation(args.particles, args.steps)
