import numpy as np
import os
import h5py


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
        rotation_velocities: a dictionary of key-value pair {"PartTypeX": 1d numpy.ndarray}, where the arrays
        are rotation velocities of each component.
        """
        os.system(
            f"post {self.filename} .tmp_log.hdf5 {r_max} {r_bin_number} {phi_bin_number}"
        )
        data = h5py.File(".tmp_log.hdf5", "r")
        self.radius = data["Radius"][:]
        self.rotation_velocities = {}
        for key in data["Rotation Velocity"].keys():
            self.rotation_velocities[key] = np.nanmean(
                data["Rotation Velocity"][key][...], axis=1
            )
        self.rc_calculated = True
        print("Rotation curve calculation done, clean the temporary file.")
        os.system("rm .tmp_log.hdf5")
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
