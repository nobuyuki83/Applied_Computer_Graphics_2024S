from pathlib import Path
import json
#
import numpy
import pyrr


class ChannelData:

    def __init__(self, i_bone, times, values, path):
        self.i_bone = i_bone
        self.times = times
        self.values = values
        self.path = path  # translation, scale, rotation

    def get_value(self, time):
        num_time = len(self.times)
        for idx1 in range(num_time):
            if time < self.times[idx1]:
                break
        if idx1 == num_time:
            time = num_time - 1
        idx0 = idx1 - 1
        if idx0 == -1:
            idx0 = 0
        ratio = 0.0
        if idx1 != idx0:
            ratio = (time - self.times[idx0]) / (self.times[idx1] - self.times[idx0])
        return (1. - ratio) * self.values[idx0] + ratio * self.values[idx1]


def nparray_from_accessor(gltf, data, i_accessor):
    accessor = gltf['accessors'][i_accessor]
    #
    dtype = numpy.dtype('float32')
    if accessor['componentType'] == 5123:
        dtype = numpy.dtype('ushort')
    elif accessor['componentType'] == 5126:
        dtype = numpy.dtype('float32')
    #
    size = 1
    if accessor['type'] == 'VEC3':
        size = 3
    elif accessor['type'] == 'VEC4':
        size = 4
    elif accessor['type'] == 'MAT4':
        size = 16
    #
    n = accessor['count']
    bufferview = gltf['bufferViews'][accessor['bufferView']]
    i_start = bufferview['byteOffset'] + accessor['byteOffset']
    data_for_accessor = data[i_start:i_start + n * dtype.itemsize * size]
    out_data = numpy.frombuffer(data_for_accessor, dtype=dtype)
    #
    if accessor['type'] == 'VEC3':
        out_data = out_data.reshape(-1, 3)
    elif accessor['type'] == 'VEC4':
        out_data = out_data.reshape(-1, 4)
    elif accessor['type'] == 'MAT4':
        out_data = out_data.reshape(-1, 4, 4)
    #
    return out_data


def get_bones_from_gltf(gltf, inode0, i_bone_parent, node2bone, bone2parentbone):
    node = gltf['nodes'][inode0]
    i_bone0 = node2bone[inode0]
    bone2parentbone[i_bone0] = i_bone_parent
    if not 'children' in node:
        return
    for i_node_chilren in node['children']:
        get_bones_from_gltf(gltf, i_node_chilren, i_bone0, node2bone, bone2parentbone)


def load_data(path_gltf, path_bin):
    '''
    load gltf data
    :param path_gltf: file path for gltf
    :param path_bin: file path for binary data (coordinates, animation etc)
    :return: mesh data, blend skinning data, skeleton data, animation data
    '''
    file = open(path_gltf, 'r')
    gltf = json.load(file)
    data = Path(path_bin).read_bytes()

    i_acc_tri2vtx = gltf['meshes'][0]['primitives'][0]['indices']
    tri2vtx = nparray_from_accessor(gltf, data, i_acc_tri2vtx)
    #
    i_acc_vtx2xyz = gltf['meshes'][0]['primitives'][0]['attributes']['POSITION']
    vtx2xyz = nparray_from_accessor(gltf, data, i_acc_vtx2xyz)
    #
    i_acc_vtx2weights = gltf['meshes'][0]['primitives'][0]['attributes']['WEIGHTS_0']
    vtx2weights = nparray_from_accessor(gltf, data, i_acc_vtx2weights)
    #
    i_acc_vtx2joints = gltf['meshes'][0]['primitives'][0]['attributes']['JOINTS_0']
    vtx2joints = nparray_from_accessor(gltf, data, i_acc_vtx2joints)
    print(vtx2joints.shape)
    #
    for node in gltf['nodes']:
        print(node)

    print("******")

    i_acc_bone2invBindingMatrix = gltf['skins'][0]['inverseBindMatrices']
    bone2invBindingMatrix = nparray_from_accessor(gltf, data, i_acc_bone2invBindingMatrix).copy()
    for i_bone in range(bone2invBindingMatrix.shape[0]):
        bone2invBindingMatrix[i_bone] = bone2invBindingMatrix[i_bone].transpose()
    print("bone2invBindingMatrix", bone2invBindingMatrix.shape)

    bone2node = gltf['skins'][0]['joints']
    node2bone = dict((v, k) for k, v in dict(enumerate(bone2node)).items())
    # print(node2bone)

    bone2boneparent = [-1] * len(bone2node)
    i_node_skeleton_root = gltf['skins'][0]['skeleton']
    get_bones_from_gltf(gltf, i_node_skeleton_root, -1, node2bone, bone2boneparent)
    print(bone2boneparent)

    channels = []
    for channel in gltf['animations'][0]['channels']:
        path = channel['target']['path']  # rotation, scale target
        i_bone = node2bone[channel['target']['node']]
        sampler = gltf['animations'][0]['samplers'][channel['sampler']]
        assert (sampler['interpolation'] == 'LINEAR')
        i_acc_times = sampler['input']
        i_acc_values = sampler['output']
        value_times = nparray_from_accessor(gltf, data, i_acc_times)
        value_values = nparray_from_accessor(gltf, data, i_acc_values)
        channels.append(ChannelData(i_bone, value_times, value_values, path))

    return tri2vtx, vtx2xyz, vtx2joints, vtx2weights, bone2boneparent, bone2invBindingMatrix, channels


def get_relative_transformations(time: float, num_bone: int, channels):
    bone2relativeTransformation = numpy.zeros((num_bone, 4, 4))
    for i_bone in range(num_bone):
        bone2relativeTransformation[i_bone] = numpy.eye(4)
    for ch in channels:
        val = ch.get_value(time)
        i_bone = ch.i_bone
        if ch.path == "translation":
            m = pyrr.Matrix44.from_translation(val).transpose()
            bone2relativeTransformation[i_bone] = numpy.matmul(bone2relativeTransformation[i_bone], m)
        elif ch.path == "rotation":
            m = pyrr.Matrix44.from_quaternion(val)
            bone2relativeTransformation[i_bone] = numpy.matmul(bone2relativeTransformation[i_bone], m)
        else:
            pass
    return bone2relativeTransformation
