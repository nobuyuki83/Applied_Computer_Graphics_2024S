import numpy
import numpy.typing
import math


def load_triangle_mesh_from_wavefront_obj_file(file_path: str):
    try:
        vtx2xyz = []
        tri2vtx = []
        with open(file_path) as f:
            for line in f:
                if line[0] == "v":
                    xyz = list(map(float, line[2:].strip().split()))
                    vtx2xyz.append(xyz)
                elif line[0] == "f":
                    vtx = list(map(int, line[2:].strip().split()))
                    tri2vtx.append(vtx)
        tri2vtx = numpy.array(tri2vtx, dtype=numpy.uint32) - 1
        vtx2xyz = numpy.array(vtx2xyz, dtype=numpy.float32)
        return (tri2vtx, vtx2xyz)

    except FileNotFoundError:
        print(f"{file_path} not found.")
    except:
        print("An error occurred while loading the shape.")


def normalize_vtx2xyz(vtx2xyz):
    """
    fit the points inside unit cube [0,1]^3
    :param tri2center:
    :return:
    """
    vmin = numpy.min(vtx2xyz, axis=0)
    vmax = numpy.max(vtx2xyz, axis=0)
    vtx2xyz -= (vmin+vmax)*0.5
    vtx2xyz *= 1.0/(vmax - vmin).max()


def vtx2nrm(tri2vtx, vtx2xyz) -> numpy.typing.NDArray:
    v0 = tri2vtx[:, 0]
    v1 = tri2vtx[:, 1]
    v2 = tri2vtx[:, 2]
    u = vtx2xyz[v1] - vtx2xyz[v0]
    v = vtx2xyz[v2] - vtx2xyz[v0]
    c = numpy.cross(u, v)
    c = c / numpy.linalg.norm(c, axis=1)[:, numpy.newaxis]
    vtx2nrm = numpy.zeros_like(vtx2xyz)
    vtx2nrm[v0] += c
    vtx2nrm[v1] += c
    vtx2nrm[v2] += c
    vtx2nrm = vtx2nrm / numpy.linalg.norm(vtx2nrm, axis=1)[:, numpy.newaxis]
    return vtx2nrm


def octahedron() -> (numpy.typing.NDArray, numpy.typing.NDArray):
    tri2vtx = [
        [0, 1, 2],
        [0, 2, 3],
        [0, 3, 4],
        [0, 4, 1],
        [5, 4, 3],
        [5, 3, 2],
        [5, 2, 1],
        [5, 1, 4]]
    vtx2xyz = [
        [0., 0., -1.],
        [1., 0., 0.],
        [0., 1., 0.],
        [-1., 0., 0.],
        [0., -1., 0.],
        [0., 0., 1.]]
    tri2vtx = numpy.array(tri2vtx, dtype=numpy.uint32)
    vtx2xyz = numpy.array(vtx2xyz, dtype=numpy.float32)

    return (tri2vtx, vtx2xyz)
