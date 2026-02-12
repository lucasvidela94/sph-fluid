import ctypes
import numpy as np
from pathlib import Path

# Cargar la librería compartida
lib_path = Path(__file__).parent.parent / 'src' / 'libsph.so'
lib = ctypes.CDLL(str(lib_path))

# Definir estructuras C
class Particle(ctypes.Structure):
    _fields_ = [
        ("x", ctypes.c_float),
        ("y", ctypes.c_float),
        ("vx", ctypes.c_float),
        ("vy", ctypes.c_float),
        ("fx", ctypes.c_float),
        ("fy", ctypes.c_float),
        ("density", ctypes.c_float),
        ("pressure", ctypes.c_float),
    ]

class SPHSystem(ctypes.Structure):
    pass

# Definir tipos de funciones
lib.sph_create.argtypes = [ctypes.c_int]
lib.sph_create.restype = ctypes.POINTER(SPHSystem)

lib.sph_destroy.argtypes = [ctypes.POINTER(SPHSystem)]
lib.sph_destroy.restype = None

lib.sph_init_dam_break.argtypes = [ctypes.POINTER(SPHSystem)]
lib.sph_init_dam_break.restype = None

lib.sph_step.argtypes = [ctypes.POINTER(SPHSystem)]
lib.sph_step.restype = None

lib.sph_get_positions.argtypes = [ctypes.POINTER(SPHSystem), ctypes.POINTER(ctypes.c_float)]
lib.sph_get_positions.restype = None

lib.sph_set_domain.argtypes = [ctypes.POINTER(SPHSystem), ctypes.c_float, ctypes.c_float, 
                                ctypes.c_float, ctypes.c_float]
lib.sph_set_domain.restype = None


class SPHSimulator:
    """Simulador de fluidos SPH de alto rendimiento."""
    
    def __init__(self, n_particles=1000, domain=(0, 0, 1, 1)):
        """
        Inicializar simulador.
        
        Args:
            n_particles: Número de partículas
            domain: (min_x, min_y, max_x, max_y) del dominio
        """
        self.n_particles = n_particles
        self.system = lib.sph_create(n_particles)
        if not self.system:
            raise MemoryError("No se pudo crear el sistema SPH")
        
        lib.sph_set_domain(self.system, *domain)
        lib.sph_init_dam_break(self.system)
        
        self._positions_buffer = np.zeros(n_particles * 2, dtype=np.float32)
    
    def __del__(self):
        if hasattr(self, 'system') and self.system:
            lib.sph_destroy(self.system)
    
    def step(self):
        """Ejecutar un paso de simulación."""
        lib.sph_step(self.system)
    
    def get_positions(self):
        """Obtener posiciones actuales de las partículas.
        
        Returns:
            numpy array de shape (n_particles, 2) con posiciones (x, y)
        """
        lib.sph_get_positions(self.system, self._positions_buffer.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))
        return self._positions_buffer.reshape(-1, 2)
    
    def run(self, n_steps=100, callback=None):
        """Ejecutar simulación por n_steps.
        
        Args:
            n_steps: Número de pasos a simular
            callback: Función opcional que se llama cada paso con (step, positions)
        """
        for i in range(n_steps):
            self.step()
            if callback:
                callback(i, self.get_positions())
