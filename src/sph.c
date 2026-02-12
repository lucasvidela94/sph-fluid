#include "sph.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Kernel Poly6 para densidad
float sph_kernel_poly6(float r, float h) {
    if (r > h) return 0.0f;
    float h2 = h * h;
    float h9 = h2 * h2 * h2 * h2 * h;
    float diff = h2 - r * r;
    return (315.0f / (64.0f * M_PI * h9)) * diff * diff * diff;
}

// Gradiente del kernel Spiky para presión
float sph_kernel_spiky_gradient(float r, float h) {
    if (r > h || r < 1e-6f) return 0.0f;
    float h6 = h * h * h * h * h * h;
    float diff = h - r;
    return (-45.0f / (M_PI * h6)) * diff * diff;
}

// Laplaciano del kernel Viscosity
float sph_kernel_viscosity_laplacian(float r, float h) {
    if (r > h) return 0.0f;
    float h6 = h * h * h * h * h * h;
    return (45.0f / (M_PI * h6)) * (h - r);
}

// Crear sistema SPH
SPHSystem* sph_create(int n_particles) {
    SPHSystem* system = (SPHSystem*)malloc(sizeof(SPHSystem));
    if (!system) return NULL;
    
    system->particles = (Particle*)malloc(n_particles * sizeof(Particle));
    if (!system->particles) {
        free(system);
        return NULL;
    }
    
    system->n_particles = n_particles;
    system->kernel_radius = SPH_KERNEL_RADIUS;
    system->particle_mass = SPH_PARTICLE_MASS;
    system->rest_density = SPH_REST_DENSITY;
    system->gas_constant = SPH_GAS_CONSTANT;
    system->viscosity = SPH_VISCOSITY;
    system->gravity = SPH_GRAVITY;
    system->time_step = SPH_TIME_STEP;
    
    // Dominio por defecto
    system->domain_min_x = 0.0f;
    system->domain_min_y = 0.0f;
    system->domain_max_x = 1.0f;
    system->domain_max_y = 1.0f;
    
    memset(system->particles, 0, n_particles * sizeof(Particle));
    
    return system;
}

// Destruir sistema
void sph_destroy(SPHSystem* system) {
    if (system) {
        free(system->particles);
        free(system);
    }
}

// Inicializar dam break (colapso de presa)
void sph_init_dam_break(SPHSystem* system) {
    int nx = (int)sqrt(system->n_particles * 0.25f);
    int ny = system->n_particles / nx;
    
    float spacing = system->kernel_radius * 0.5f;
    int idx = 0;
    
    for (int i = 0; i < nx && idx < system->n_particles; i++) {
        for (int j = 0; j < ny && idx < system->n_particles; j++) {
            Particle* p = &system->particles[idx];
            p->x = system->domain_min_x + 0.1f + i * spacing;
            p->y = system->domain_min_y + 0.1f + j * spacing;
            p->vx = 0.0f;
            p->vy = 0.0f;
            p->density = system->rest_density;
            p->pressure = 0.0f;
            idx++;
        }
    }
}

// Calcular densidad y presión
void sph_compute_density_pressure(SPHSystem* system) {
    float h = system->kernel_radius;
    float h2 = h * h;
    
    for (int i = 0; i < system->n_particles; i++) {
        Particle* pi = &system->particles[i];
        float density = 0.0f;
        
        for (int j = 0; j < system->n_particles; j++) {
            float dx = pi->x - system->particles[j].x;
            float dy = pi->y - system->particles[j].y;
            float r2 = dx * dx + dy * dy;
            
            if (r2 < h2) {
                float r = sqrtf(r2);
                density += system->particle_mass * sph_kernel_poly6(r, h);
            }
        }
        
        pi->density = fmaxf(density, system->rest_density);
        pi->pressure = system->gas_constant * (pi->density - system->rest_density);
    }
}

// Calcular fuerzas
void sph_compute_forces(SPHSystem* system) {
    float h = system->kernel_radius;
    float h2 = h * h;
    
    for (int i = 0; i < system->n_particles; i++) {
        Particle* pi = &system->particles[i];
        float fpx = 0.0f, fpy = 0.0f;
        float fvx = 0.0f, fvy = 0.0f;
        
        for (int j = 0; j < system->n_particles; j++) {
            if (i == j) continue;
            
            Particle* pj = &system->particles[j];
            float dx = pi->x - pj->x;
            float dy = pi->y - pj->y;
            float r2 = dx * dx + dy * dy;
            
            if (r2 < h2 && r2 > 1e-12f) {
                float r = sqrtf(r2);
                
                // Fuerza de presión
                float pressure_avg = (pi->pressure + pj->pressure) / (2.0f * pj->density);
                float grad_w = sph_kernel_spiky_gradient(r, h);
                float factor = -system->particle_mass * pressure_avg * grad_w / r;
                fpx += factor * dx;
                fpy += factor * dy;
                
                // Fuerza de viscosidad
                float dvx = pj->vx - pi->vx;
                float dvy = pj->vy - pi->vy;
                float lap_w = sph_kernel_viscosity_laplacian(r, h);
                float visc_factor = system->viscosity * system->particle_mass * lap_w / pj->density;
                fvx += visc_factor * dvx;
                fvy += visc_factor * dvy;
            }
        }
        
        pi->fx = fpx + fvx;
        pi->fy = fpy + fvy + system->gravity * pi->density;
    }
}

// Integración temporal (Euler semi-implícito)
void sph_integrate(SPHSystem* system) {
    float dt = system->time_step;
    
    for (int i = 0; i < system->n_particles; i++) {
        Particle* p = &system->particles[i];
        
        // Actualizar velocidad
        p->vx += (p->fx / p->density) * dt;
        p->vy += (p->fy / p->density) * dt;
        
        // Actualizar posición
        p->x += p->vx * dt;
        p->y += p->vy * dt;
        
        // Colisiones con bordes (simple bounce)
        float damping = 0.5f;
        if (p->x < system->domain_min_x) {
            p->x = system->domain_min_x;
            p->vx *= -damping;
        }
        if (p->x > system->domain_max_x) {
            p->x = system->domain_max_x;
            p->vx *= -damping;
        }
        if (p->y < system->domain_min_y) {
            p->y = system->domain_min_y;
            p->vy *= -damping;
        }
        if (p->y > system->domain_max_y) {
            p->y = system->domain_max_y;
            p->vy *= -damping;
        }
    }
}

// Un paso completo de simulación
void sph_step(SPHSystem* system) {
    sph_compute_density_pressure(system);
    sph_compute_forces(system);
    sph_integrate(system);
}

// Obtener posiciones para Python
void sph_get_positions(SPHSystem* system, float* positions) {
    for (int i = 0; i < system->n_particles; i++) {
        positions[2 * i] = system->particles[i].x;
        positions[2 * i + 1] = system->particles[i].y;
    }
}

// Setear dominio
void sph_set_domain(SPHSystem* system, float min_x, float min_y, float max_x, float max_y) {
    system->domain_min_x = min_x;
    system->domain_min_y = min_y;
    system->domain_max_x = max_x;
    system->domain_max_y = max_y;
}
