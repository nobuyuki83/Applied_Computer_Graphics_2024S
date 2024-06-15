# Task08: Skeletal Animation

![preview](preview.png)

**Deadline: May 20th (Thu) at 15:00pm**

----

## Before Doing Assignment

### Install Python (if necessary)
We use Python for this assignment. 
This assigment only supports Python ver. 3.

To check if Python 3.x is installed, launch a command prompt and type `python3 --version` and see the version.

For MacOS and Ubuntu you have Python installed by default. 
For Windows, you may need to install the Python by yourself.
[This document](https://docs.python.org/3/using/windows.html) show how to install Python 3.x on Windows.


### Virtual environment

We want to install dependency ***locally*** for this assignment.

```bash
cd acg-<username> 
python3 -m venv venv  # make a virtual environment named "venv"
```

Then, start the virtual environment.
For Mac or Linux, type

```bash
source venv/bin/activate  # start virtual environment 
```

For Windows, type  

```bash
venv\Scripts\activate.bat  # start virtual environment
```

In the command prompt, you will see `(venv)` at the beginning of each line.
There will be `venv` folder under `acg-<username>`.     

### Install dependency

In this assignment we use many external library. We use `pip` to install these.

```bash
pip3 install numpy
pip3 install moderngl
pip3 install moderngl_window
```

Alternatively, you can install above dependency at once by

```bash
cd acg-<username>/task08
pip3 install -r requirements.txt
```

type `pip3 list` and then confirm you have libraries such as `moderngl`, `numpy`, `pillow`, `pyglet`, `pyrr` etc.

### Make branch

Follow [this document](../doc/submit.md) to submit the assignment, In a nutshell, before doing the assignment,  
- make sure you synchronized the `main ` branch of your local repository  to that of remote repository.
- make sure you created branch `task08` from `main` branch.
- make sure you are currently in the `task08` branch (use `git branch -a` command).

Now you are ready to go!

---

## Problem 1 (Python execution practice)

Run the code with `python3 main.py` 

The program will output `output.png` that update the image below

![problem1](output.png)

## Problem 2 (Skeletal animation)

In this problem, we compute the 3D transformation (rotation and translation) of each bone.

Bones in a skeleton of a character ave a tree structure. Each bone has parent bone, except for the root bone (the bone with index 0).
For bone `i_bone`, the index of parent bone is `self.bone2parentbone[i_bone]`.
3D Affine transformation from the parent bone is written in `bone2relativeTransformation`.
Note that index of parent bone is always smaller than the child bone e.g., `ibone > bone2relativeTransformation[i_bone]`.

Write some code around line #??? to compute the transformations of all the bones in `bone2globalTransformation`.

This will animate the frames (red, blue, and green cylinders) of the bones.    

## Problem 3 (Linear Blend Skinning)

We now have the 3D transformation of each bone, let's animate the mesh using the linear blend skinning.

Write some code around line #??? to implement the linear blend skinning.

Each vertex of the mesh is associated with four bones.
Use `inverseBindingMatrix` which has the inverse transformation of bone from origin to the undeformed mesh. 

This will animate black mesh.

## After Doing the Assignment

After modify the code, push the code and submit a pull request. Make sure your pull request only contains the files you edited. Good luck!

BTW, You can exit the virtual environment by typing `deactivate` in the command prompt.

