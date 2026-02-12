# SPH Fluid Simulator

Simulador de fluidos usando Smoothed Particle Hydrodynamics (SPH) en C con bindings Python.

## Estructura

- `src/` - Código C del motor SPH
- `python/` - Bindings y visualización Python
- `examples/` - Ejemplos de uso
- `tests/` - Tests unitarios

## Compilación

```bash
cd src
make
```

## Uso

```python
from python.sph_simulator import SPHSimulator

sim = SPHSimulator(n_particles=1000)
sim.run()
```

## Algoritmo SPH

SPH es un método lagrangiano donde el fluido se representa como partículas. Cada partícula tiene propiedades (posición, velocidad, densidad, presión) y las interacciones se calculan usando kernels de suavizado.

### Fórmulas clave

- Densidad: ρᵢ = Σⱼ mⱼ W(rᵢ - rⱼ, h)
- Presión: P = k(ρ - ρ₀)
- Fuerza de presión: fᵖ = -∇P/ρ
- Fuerza de viscosidad: fᵛ = μ∇²v
# particle-SPH
