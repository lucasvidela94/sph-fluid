#ifndef SPH_H
#define SPH_H

#include <stdint.h>

// Constantes físicas
#define SPH_KERNEL_RADIUS 0.05f
#define SPH_PARTICLE_MASS 0.02f
#define SPH_REST_DENSITY 1000.0f
#define SPH_GAS_CONSTANT 200.0f
#define SPH_VISCOSITY 0.1f
#define SPH_GRAVITY -9.81f
#define SPH_TIME_STEP 0.001f

// Estructura de partícula
typedef struct {
    float x, y;           // Posición
    float vx, vy;         // Velocidad
    float fx, fy;         // Fuerza
    float density;        // Densidad
    float pressure;       // Presión
} Particle;

// Estructura del simulador
typedef struct {
    Particle* particles;
    int n_particles;
    float kernel_radius;
    float particle_mass;
    float rest_density;
    float gas_constant;
    float viscosity;
    float gravity;
    float time_step;
    float domain_min_x, domain_min_y;
    float domain_max_x, domain_max_y;
} SPHSystem;

// Funciones de inicialización
SPHSystem* sph_create(int n_particles);
void sph_destroy(SPHSystem* system);
void sph_init_dam_break(SPHSystem* system);

// Funciones de simulación
void sph_compute_density_pressure(SPHSystem* system);
void sph_compute_forces(SPHSystem* system);
void sph_integrate(SPHSystem* system);
void sph_step(SPHSystem* system);

// Kernel SPH
float sph_kernel_poly6(float r, float h);
float sph_kernel_spiky_gradient(float r, float h);
float sph_kernel_viscosity_laplacian(float r, float h);

// Utilidades
void sph_get_positions(SPHSystem* system, float* positions);
void sph_set_domain(SPHSystem* system, float min_x, float min_y, float max_x, float max_y);

#endif // SPH_H
