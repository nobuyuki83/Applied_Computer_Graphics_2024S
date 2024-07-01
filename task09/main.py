import os
import math

import numpy
#
import pyrr
import numpy as np
import moderngl
import moderngl_window as mglw
from PIL import Image, ImageOps
from scipy import sparse
from scipy.sparse.linalg import spsolve
#
import util_for_task09


def matrix_graph_laplacian(tri2vtx, num_vtx):
    '''
    function to make graph laplacian matrix
    :param tri2vtx: index of vertex for triangles
    :param num_vtx: number of vertex
    :return: sparse matrix using scipy.sparse
    '''
    tri2ones = np.ones((tri2vtx.shape[0],))
    W = np.empty(0, np.float32)
    I = np.empty(0, np.uint32)
    J = np.empty(0, np.uint32)
    #
    for (i, j) in ((0, 0), (0, 1), (0, 2), (1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2)):
        tri2vi = tri2vtx[:, i]
        tri2vj = tri2vtx[:, j]
        if i == j:
            coeff = 1.
        else:
            coeff = -0.5
        W = np.append(W, tri2ones * coeff)
        I = np.append(I, tri2vi)
        J = np.append(J, tri2vj)
    return sparse.csr_matrix((W, (I, J)), shape=(num_vtx, num_vtx))


class HelloWorld(mglw.WindowConfig):
    '''
    Window to show the mesh deformation
    '''
    gl_version = (3, 3)
    title = "task09: laplacian mesh deformation"
    window_size = (500, 500)
    aspect_ratio = float(window_size[0]) / float(window_size[1])
    resizable = False

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.is_screenshot_taken = False

        # initialize mesh
        asset_path = os.path.normpath(os.path.join(__file__, '../../asset/bunny.obj'))
        self.tri2vtx, self.vtx2xyz_ini = util_for_task09.load_triangle_mesh_from_wavefront_obj_file(asset_path)
        util_for_task09.normalize_vtx2xyz(self.vtx2xyz_ini)
        self.vtx2xyz_def = self.vtx2xyz_ini.copy()

        # make a list for fixed vertices
        self.fixbase2vtx = (self.vtx2xyz_ini[:, 2] < -0.48).nonzero()[0].tolist()
        self.fixear2vtx = (self.vtx2xyz_ini[:, 2] > +0.48).nonzero()[0].tolist()
        self.fixback2vtx = (np.logical_and(
            self.vtx2xyz_ini[:, 0] > +0.15,
            self.vtx2xyz_ini[:, 2] > +0.115)).nonzero()[0].tolist()

        # make laplacian, bi-laplacian matrices
        num_vtx = self.vtx2xyz_ini.shape[0]
        self.matrix_laplace = matrix_graph_laplacian(self.tri2vtx, num_vtx)
        self.matrix_bilaplace = self.matrix_laplace * self.matrix_laplace

        # make matrix for fixed points
        coeff_penalty = 100.0
        fix2vtx = numpy.concatenate([self.fixback2vtx, self.fixear2vtx, self.fixbase2vtx])
        self.matrix_fix = sparse.csr_matrix((
            np.ones(fix2vtx.shape[0], np.float32) * coeff_penalty, (fix2vtx, fix2vtx)),
            shape=(num_vtx, num_vtx))

        # -----------------------------
        # below: visualization related

        self.prog = self.ctx.program(
            vertex_shader='''
                #version 330
                uniform mat4 matrix;                
                in vec3 in_vert;
                in vec3 in_nrm;
                out vec3 out_nrm;
                void main() {
                    out_nrm = normalize((matrix * vec4(in_nrm, 0.0)).xyz);
                    gl_Position = matrix * vec4(in_vert, 1.0);
                }
            ''',
            fragment_shader='''
                #version 330
                uniform bool is_shading; // variable of the program
                uniform vec3 color;                
                out vec4 f_color;
                in vec3 out_nrm;
                void main() {                
                    float ratio = abs(out_nrm.z);
                    if( !is_shading ){ ratio = 1.0; } 
                    f_color = vec4(color*ratio, 1.0);
                }
            ''',
        )

        # initialize visualization for mesh
        ebo = self.ctx.buffer(self.tri2vtx)  # send triangle index data to GPU (element buffer object)
        self.vbo_vtx2xyz_def = self.ctx.buffer(self.vtx2xyz_def)  # send initial vertex coordinates data to GPU
        vtx2nrm = util_for_task09.vtx2nrm(self.tri2vtx, self.vtx2xyz_def)
        self.vbo_vtx2nrm_def = self.ctx.buffer(vtx2nrm)  # send initial vertex coordinates data to GPU
        self.vao_ini = self.ctx.vertex_array(
            self.prog, [
                (self.vbo_vtx2xyz_def, '3f', 'in_vert'),
                (self.vbo_vtx2nrm_def, '3f', 'in_nrm')],
            ebo, 4, mode=moderngl.TRIANGLES)  # tell gpu about the mesh information and how to draw it

        # initialize visualization for points
        tri2vtx_octa, vtx2xyz_octa = util_for_task09.octahedron()
        self.vao_octa = self.ctx.vertex_array(
            self.prog, [
                (self.ctx.buffer(vtx2xyz_octa), '3f', 'in_vert')],
            self.ctx.buffer(tri2vtx_octa), 4, mode=moderngl.TRIANGLES)

    def render(self, time, frame_time):
        self.vtx2xyz_def[:] = self.vtx2xyz_ini[:]
        # set fixed deformation
        self.vtx2xyz_def[self.fixear2vtx, 0] += -0.5 * math.sin(time)
        self.vtx2xyz_def[self.fixback2vtx, 2] += -0.3 * math.sin(2. * time)

        # write a few line code below to implement laplacian mesh deformation
        # Problem 2: Laplacian deformation, which minimizes (x-x_def)D(x-x_def) + (x-x_ini)L(x-x_ini) w.r.t x,
        # Problem 3: Bi-Laplacian deformation, which minimizes (x-x_def)D(x-x_def) + (x-x_ini)L^2(x-x_ini) w.r.t x,
        # where D is the diagonal matrix with entry 1 if the vertex is fixed, 0 if the vertex is free, a.k.a `self.matrix_fix`
        # L is the graph Laplacian matrix a.k.a `self.matrix_laplace`
        # you may use `spsolve` to solve the liner system
        # spsolve: https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.linalg.spsolve.html#scipy.sparse.linalg.spsolve


        # do not edit beyond here
        # above: deformation
        # ---------------------
        # below: visualization

        self.ctx.clear(1.0, 1.0, 1.0)  # initizlize frame buffer with white
        self.ctx.enable(moderngl.DEPTH_TEST)

        self.vbo_vtx2xyz_def.write(self.vtx2xyz_def)  # send vertex information to GPU
        vtx2nrm = util_for_task09.vtx2nrm(self.tri2vtx, self.vtx2xyz_def)
        self.vbo_vtx2nrm_def.write(vtx2nrm)  # send normal information to GPU

        # make view transformation
        view_rot_z = pyrr.Matrix44.from_z_rotation(-np.pi * 0.2)
        view_rot_x = pyrr.Matrix44.from_x_rotation(np.pi * 0.4)
        view_z_flip = pyrr.Matrix44.from_scale((1.3, 1.3, -1.3, 1.))
        view_transf = view_z_flip * view_rot_x * view_rot_z

        # render triangle mesh
        self.prog['matrix'].value = tuple(view_transf.flatten())
        self.prog['color'].value = (0.8, 0.8, 0.8)
        self.prog['is_shading'].value = 1
        self.vao_ini.render()

        r = 0.02
        self.prog['is_shading'].value = 0

        # render fixed bases
        for i_vtx in self.fixbase2vtx:
            pos = self.vtx2xyz_def[i_vtx].copy()
            model_transf = pyrr.Matrix44.from_translation(pos) * pyrr.Matrix44.from_scale((r, r, r, 1.))
            self.prog['matrix'].value = tuple((view_transf * model_transf).flatten())
            self.prog['color'].value = (0.0, 0.0, 1.0)
            self.vao_octa.render()

        # render fixed ear
        for i_vtx in self.fixear2vtx:
            pos = self.vtx2xyz_def[i_vtx].copy()
            model_transf = pyrr.Matrix44.from_translation(pos) * pyrr.Matrix44.from_scale((r, r, r, 1.))
            self.prog['matrix'].value = tuple((view_transf * model_transf).flatten())
            self.prog['color'].value = (0.0, 1.0, 0.0)
            self.vao_octa.render()

        # render fixed back
        for i_vtx in self.fixback2vtx:
            pos = self.vtx2xyz_def[i_vtx].copy()
            model_transf = pyrr.Matrix44.from_translation(pos) * pyrr.Matrix44.from_scale((r, r, r, 1.))
            self.prog['matrix'].value = tuple((view_transf * model_transf).flatten())
            self.prog['color'].value = (1.0, 0.0, 0.0)
            self.vao_octa.render()

        # take a screenshot
        if not self.is_screenshot_taken and time > 2.2:
            self.is_screenshot_taken = True
            rgb = np.frombuffer(self.ctx.fbo.read(), dtype=np.uint8)
            rgb = rgb.reshape(self.ctx.fbo.size[0], self.ctx.fbo.size[1], 3)
            if rgb.shape[0] == 1000:
                rgb = rgb[::2, ::2, :].copy('C')
            rgb = Image.fromarray(rgb)
            ImageOps.flip(rgb).save("out.png")


def main():
    HelloWorld.run()


if __name__ == "__main__":
    main()
