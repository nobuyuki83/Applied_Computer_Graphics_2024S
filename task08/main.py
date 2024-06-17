import os
import math
#
import pyrr
import numpy as np
import moderngl
import moderngl_window as mglw
from PIL import Image, ImageOps
#
import parse_gltf
import util_for_task08



class HelloWorld(mglw.WindowConfig):
    '''
    Window to show the gltf animation
    '''
    gl_version = (3, 3)
    title = "task08: skeletal animation"
    window_size = (500, 500)
    aspect_ratio = float(window_size[0]) / float(window_size[1])
    resizable = False

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        asset_dir = os.path.normpath(os.path.join(__file__, '../../asset'))
        tri2vtx, self.vtx2xyz_ini, self.vtx2bones, self.vtx2weights, \
            self.bone2parentbone, self.bone2invBindingMatrix, \
            self.channels = parse_gltf.load_data(
            os.path.join(asset_dir, 'CesiumMan.gltf'),
            os.path.join(asset_dir, 'CesiumMan_data.bin'))
        tri2vtx = tri2vtx.astype(np.uint32).reshape(-1, 3)
        edge2vtx = util_for_task08.edge2vtx_from_tri2vtx(tri2vtx)  # extract edges from triangle mesh
        self.vtx2xyz_def = self.vtx2xyz_ini.copy()
        self.animation_duration = 0
        for ch in self.channels:
            self.animation_duration = max(self.animation_duration, ch.times.max())
        self. is_screenshot_taken = False

        self.prog = self.ctx.program(
            vertex_shader='''
                #version 330
                uniform mat4 matrix;                
                in vec3 in_vert;
                void main() {
                    gl_Position = matrix * vec4(in_vert, 1.0);
                }
            ''',
            fragment_shader='''
                #version 330
                uniform vec3 color;                
                out vec4 f_color;
                void main() {
                    f_color = vec4(color, 1.0);
                }
            ''',
        )

        ebo = self.ctx.buffer(edge2vtx)  # send triangle index data to GPU (element buffer object)
        vbo_ini = self.ctx.buffer(self.vtx2xyz_ini)  # send initial vertex coordinates data to GPU
        self.vao_ini = self.ctx.vertex_array(
            self.prog, [(vbo_ini, '3f', 'in_vert')],
            ebo, 4, mode=moderngl.LINES)  # tell gpu about the mesh information and how to draw it
        self.vbo_def = self.ctx.buffer(self.vtx2xyz_def)  # send deformed vertex coordinates data to GPU
        self.vao_def = self.ctx.vertex_array(
            self.prog, [(self.vbo_def, '3f', 'in_vert')],
            ebo, 4, mode=moderngl.LINES)  # tell gpu about the mesh information and how to draw it

        # cylinder mesh for frame visualization
        (tri2vtx_cyl, vtx2xyz_cyl) = util_for_task08.cylinder_mesh_zup(0.01, 0.05, 16)
        ebo = self.ctx.buffer(tri2vtx_cyl)  # send triangle index data to GPU (element buffer object)
        vbo_cyl = self.ctx.buffer(vtx2xyz_cyl)  # send deformed vertex coordinates data to GPU
        self.vao_cyl = self.ctx.vertex_array(
            self.prog, [(vbo_cyl, '3f', 'in_vert')],
            ebo, 4, mode=moderngl.TRIANGLES)

    def render(self, time, frame_time):
        time = time % self.animation_duration
        num_bone = self.bone2invBindingMatrix.shape[0]

        # bone2relativeTransformation is a numpy array with shape (#num_bone, 4, 4)
        # bone2relativeTransformation[i_bone] is a matrix representing 3D transformation from the parent bone of i_bone
        bone2relativeTransformation = parse_gltf.get_relative_transformations(time, num_bone, self.channels)

        # bone2globalTransformation is a numpy array with shape (#num_bone, 4, 4)
        # bone2globalTransformation[i_bone] is a matrix representing 3D transformation of each bone from the origin
        bone2globalTransformation = np.zeros((num_bone, 4, 4))
        for i_bone in range(num_bone):
            i_bone_parent = self.bone2parentbone[i_bone]
            if i_bone_parent == -1:  # root bone
                bone2globalTransformation[i_bone] = bone2relativeTransformation[i_bone]
                continue
            # below, write one or two lines of code to compute `bone2globalTransformation[i_bone]`
            # hint: use numpy.matmul for multiplying nd-array
            # bone2globalTransformation[i_bone] = ???

        for i_vtx in range(self.vtx2xyz_ini.shape[0]):  # for each point in mesh
            p0 = self.vtx2xyz_ini[i_vtx]
            p0 = np.append(p0, np.array([1.0]))  # homogeneous coordinate of undeformed point
            p1 = np.array([0., 0., 0., 1.], dtype=np.float32)  # p1 is the deformed point
            for idx in range(4):  # in gltf each vertex is associated with four bones
                w = self.vtx2weights[i_vtx][idx]  # rig weight
                i_bone = self.vtx2bones[i_vtx][idx]  # bone index
                inverseBindingMatrix = self.bone2invBindingMatrix[i_bone]  # 4x4 inverse binding matrix
                globalTransformation = bone2globalTransformation[i_bone]  # 4x4 transformation matrix to deformed bone
                # write a few lines of codes to compute p1 using the linear blend skinning
                # hint: use np.matmul for matrix multiplication
                # hint: assume that rig weights w add up to one

                # p1 += ???

            self.vtx2xyz_def[i_vtx] = p1[:3]  # from homogeneous coordinates to the Cartesian coordinates

        self.vbo_def.write(self.vtx2xyz_def)
        self.ctx.clear(1.0, 1.0, 1.0)

        # view transformation for undeformed character
        transform_to_center = pyrr.Matrix44.from_translation((-0.8, 0., -0.8))
        view_rot_x = pyrr.Matrix44.from_x_rotation(np.pi * 0.5)
        view_rot_y = pyrr.Matrix44.from_y_rotation(np.pi * 0.3)
        view_transf = view_rot_y * view_rot_x * transform_to_center

        # draw undeformed mesh in red
        self.prog['matrix'].value = tuple(view_transf.flatten())  # column major
        self.prog['color'].value = (1., 0., 0.)
        self.vao_ini.render()

        # view transformation for deformed character
        transform_to_center = pyrr.Matrix44.from_translation((+0.6, 0., -0.8))
        view_rot_x = pyrr.Matrix44.from_x_rotation(np.pi * 0.5)
        view_rot_y = pyrr.Matrix44.from_y_rotation(np.pi * 0.3)
        view_transf = view_rot_y * view_rot_x * transform_to_center

        # draw how the origin of each bone is transformed
        for i_bone in range(num_bone):
            transf = bone2globalTransformation[i_bone]
            transf = np.matmul(transf.transpose(), view_transf)
            # z_axis
            self.prog['matrix'].value = tuple(transf.flatten())  # column-major
            self.prog['color'].value = (0., 0.0, 0.8)
            self.vao_cyl.render()
            # x_axis
            x_rot = pyrr.Matrix44.from_y_rotation(math.pi*0.5)
            self.prog['matrix'].value = tuple((np.matmul(x_rot.transpose(), transf)).flatten())  # column major
            self.prog['color'].value = (0.8, 0., 0.)
            self.vao_cyl.render()
            # y_axis
            y_rot = pyrr.Matrix44.from_x_rotation(-math.pi*0.5)
            self.prog['matrix'].value = tuple((np.matmul(y_rot.transpose(), transf)).flatten())  # column major
            self.prog['color'].value = (0.0, 0.8, 0.)
            self.vao_cyl.render()

        # draw deformed mesh in black
        self.prog['matrix'].value = tuple(view_transf.flatten())  # column major
        self.prog['color'].value = (0., 0., 0.)
        self.vao_def.render()

        # take a screenshot
        if not self.is_screenshot_taken and time > 1.8:
            self.is_screenshot_taken = True
            rgb = np.frombuffer(self.ctx.fbo.read(), dtype=np.uint8)
            rgb = rgb.reshape(self.ctx.fbo.size[0], self.ctx.fbo.size[1], 3)
            rgb = Image.fromarray(rgb)
            ImageOps.flip(rgb).save("output.png")



def main():
    HelloWorld.run()


if __name__ == "__main__":
    main()
