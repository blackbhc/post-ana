import numpy as np
import ctypes

from numpy.core.multiarray import dtype


class info(ctypes.Structure):  # the structure for data transfer between C and Python
    _fields_ = [
        ("comp", ctypes.c_int),
        ("rs", ctypes.POINTER(ctypes.c_double)),
        ("rv", ctypes.POINTER(ctypes.c_double)),
        ("nums", ctypes.POINTER(ctypes.c_int)),
    ]


dyn_lib = ctypes.CDLL("/home/bhchen/.local/lib/libpost.so")
dyn_lib.get_the_info.restype = info  # set the return type of the function


class single_snapshot:
    def __init__(self, filename):
        self.filename = filename
        self.rc_calculated = False

    def cal_rc(self, r_max, r_bin_number, phi_bin_number):
        """
        Calculate the rotation velocities of a galactic simulation snapshot.
        ----------------
        Arguments:
        r_max: float, the maximum radius to calculate the rotation curve.
        r_bin_number: int, the number of radial bins.
        phi_bin_number: int, the number of azimuthal bins.
        ----------------
        Return values:
        radius: 1d numpy.ndarray, length = r_bin_number+1, the radial positions of the rotation curve.
        rotation_velocities: a dictionary of key-value pair {"PartTypeX": 2d numpy.ndarray}, where the arrays'
        shape=(r_bin_number+1, phi_bin_number). The rotation velocities of the component of each type.
        """
        info = dyn_lib.get_the_info(
            ctypes.c_char_p(self.filename.encode("utf-8")),
            ctypes.c_double(r_max),
            ctypes.c_int(r_bin_number),
            ctypes.c_int(phi_bin_number),
        )  # call the C function to get the data
        comp = info.contents.comp
        self.radius = np.array(
            info.contents.rs[: r_bin_number + 1], dtype=np.float64
        )  # the radial positions
        types = np.array(
            info.types[:comp], dtype=np.int32
        )  # the number of particles of each type
        dim_of_grid = (r_bin_number + 1) * phi_bin_number
        rotation_velocities = np.array(
            info.contents.rv[: comp * dim_of_grid], dtype=np.float64
        ).reshape(comp, r_bin_number + 1, phi_bin_number)

        for i in range(comp):
            type_name = "PartType" + str(types[i])
            self.rotation_velocities[type_name] = rotation_velocities[i]

        # free the memory
        dyn_lib.free_the_info(ctypes.POINTER(info))

        self.rc_calculated = True
        return self.radius, self.rotation_velocities

    @property
    def cal_fdisk(self, Rd, disktypes):
        """
        Calculate the disk mass fraction at  2.2*Rd.
        ----------------
        Arguments:
        Rd: float, the scale length of the disk. Optional, if not provided, the function will treat the
        radius of maximum disk rotation velocity as 2.2*Rd.
        disktypes: list of int, the types of disk particles in the snapshot.
        ----------------
        Return values:
        fdisk: float, the disk mass fraction at 2.2*Rd.
        """
        if not self.rc_calculated:
            raise ValueError("Rotation curve not calculated.")
        if Rd is None:
            Rc = __cal_vc_max(disktypes)[1]
        else:
            Rc = 2.2 * Rd

        fdisk = 0
        return fdisk

    def __cal_vc_max(self, types):
        """
        Calculate and return the value and position of the maximum rotation velocity.
        ----------------
        Arguments:
        types: list of int, the types of particles to be considered.
        ----------------
        Return values:
        id: int, the index of the maximum rotation velocity.
        vc_max: float, the maximum rotation velocity.
        """
        if not self.rc_calculated:
            raise ValueError("Rotation curve not calculated.")
        id = np.argmax(self.rotation_velocities["PartType" + str(types[0])])
        return id, self.rotation_velocities["PartType" + str(types[0])][id]
