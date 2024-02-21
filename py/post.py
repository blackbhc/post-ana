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
        self.__radius = data["Radius"][:]
        self.__rotation_velocities = {}
        for key in data["Rotation Velocity"].keys():
            self.__rotation_velocities[key] = np.nanmean(
                data["Rotation Velocity"][key][...], axis=1
            )
        self.rc_calculated = True
        print("Rotation curve calculation done, clean the temporary file.")
        os.system("rm .tmp_log.hdf5")
        return self.__radius, self.__rotation_velocities

    @property
    def radius(self):
        """
        Get the radius of the rotation curve.
        """
        if not self.rc_calculated:
            raise ValueError("Rotation curve not calculated.")
        return self.__radius * 1.0

    @property
    def rotation_velocities(self):
        """
        Get the rotation velocities of the rotation curve.
        """
        if not self.rc_calculated:
            raise ValueError("Rotation curve not calculated.")
        # copy the dictionary to avoid modification
        rotation_velocities = {}
        for key in self.__rotation_velocities.keys():
            rotation_velocities[key] = self.__rotation_velocities[key] * 1.0
        return rotation_velocities

    @property
    def fdisk(self, disktypes=[2], Rd=None):
        """
        Calculate the disk mass fraction at  2.2*Rd.
        ----------------
        Arguments:
        disktypes: list of int, the types of disk particles in the snapshot, default is [2].
        Rd: float, the scale length of the disk. Optional, if not provided, the function will treat the
        radius of maximum disk rotation velocity as 2.2*Rd.
        ----------------
        Return values:
        fdisk: float, the disk mass fraction at 2.2*Rd.
        """
        if not self.rc_calculated:
            raise ValueError("Rotation curve not calculated.")

        fdisk = 0
        # get all particle types
        all_types = [keys[8:] for keys in self.__rotation_velocities.keys()]
        # [8:] is to remove the "PartType" prefix

        vcs_all = self.__synthesize_rcs(all_types)
        vcs_disk = self.__synthesize_rcs(disktypes)

        if Rd is None:
            Rc_id = np.argmax(vcs_disk)
            fdisk = (vcs_disk[Rc_id] / vcs_all[Rc_id]) ** 2
        else:
            Rc = 2.2 * Rd
            id_left = np.where(self.__radius < Rc)[0][-1]
            delta = Rc - self.__radius[id_left]
            # get the disk mass fraction around 2.2*Rd
            fdisk1 = (vcs_disk[id_left] / vcs_all[id_left]) ** 2
            fdisk2 = (vcs_disk[id_left + 1] / vcs_all[id_left + 1]) ** 2
            # linear interpolation
            fdisk = fdisk1 + (fdisk2 - fdisk1) * delta / (
                self.__radius[id_left + 1] - self.__radius[id_left]
            )
        return fdisk

    def __synthesize_rcs(self, types):
        """
        Synthesize the rotation velocities of different particle types.
        ----------------
        Arguments:
        types: list of int, the types of particles to be considered.
        ----------------
        Return values:
        rcs: 1d numpy.ndarray, the synthesized rotation velocities.
        """
        rcs = []
        # go through all given particle types
        for type in types:
            type = "PartType" + str(type)  # get the key name
            if type not in self.__rotation_velocities.keys():
                raise ValueError(
                    f"Disk type {type} not found in the rotation curve data."
                )
            rcs.append(self.__rotation_velocities[type])
        rcs = np.array(rcs)
        rcs = np.sum(rcs**2, axis=0) ** 0.5
        return rcs
