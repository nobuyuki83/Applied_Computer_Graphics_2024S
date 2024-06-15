import numpy
import numpy.typing
import math

def edge2vtx_from_tri2vtx(tri2vtx) -> numpy.typing.NDArray:
    '''
    :param tri2vtx: triangle index of a mesh
    :return: edges in the triangle mesh (shape=[#edge,2], dtype=np.int32)
    '''
    v0 = tri2vtx[:, 0]
    v1 = tri2vtx[:, 1]
    v2 = tri2vtx[:, 2]
    edge2vtx = numpy.stack([v0, v1], axis=1)
    edge2vtx = numpy.vstack([edge2vtx, numpy.stack([v1, v2], axis=1)])
    edge2vtx = numpy.vstack([edge2vtx, numpy.stack([v2, v0], axis=1)])
    edge2vtx.sort(axis=1)
    edge2vtx = numpy.unique(edge2vtx, axis=0)
    return edge2vtx


def cylinder_mesh_zup(r, l, n) -> (numpy.typing.NDArray, numpy.typing.NDArray):
    '''

    :param r:
    :param l:
    :param n:
    :return: triangle index and vertex coordinates
    '''
    tri2vtx = numpy.zeros((4*n, 3), dtype=numpy.uint32)
    for i in range(0, n):
        tri2vtx[i, 0] = 0
        tri2vtx[i, 1] = 1+i
        tri2vtx[i, 2] = 1+(i+1)%n
        #
        tri2vtx[n+i, 0] = 1+i
        tri2vtx[n+i, 1] = 1+(i+1)%n
        tri2vtx[n+i, 2] = 1+(i+1)%n+n
        #
        tri2vtx[2*n+i, 0] = 1+i
        tri2vtx[2*n+i, 1] = 1+(i+1)%n+n
        tri2vtx[2*n+i, 2] = 1+i+n
        #
        tri2vtx[3*n+i, 0] = 2*n+1
        tri2vtx[3*n+i, 1] = 1+(i+1)%n+n
        tri2vtx[3*n+i, 2] = 1+i+n

    vtx2xyz = numpy.zeros((2*n+2, 3), dtype=numpy.float32)
    vtx2xyz[0, :] = [0., 0., 0.]
    for i in range(0, n):
        theta = math.pi * 2.0 * float(i) / float(n)
        vtx2xyz[1+i, :] = [r*math.cos(theta), r*math.sin(theta), 0.]
        vtx2xyz[1+i+n, :] = [r*math.cos(theta), r*math.sin(theta), l]
    vtx2xyz[2*n+1, :] = [0., 0., l]

    return (tri2vtx, vtx2xyz)
