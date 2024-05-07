# Task03: Perspective-Correct Texture Mapping

**Deadline: May. 10th (Fri) at 15:00pm**

This is the blurred preview of the expected result:

![preview](preview.png)

When you execute the program, it will output an PNG image file (output.png) replacing the image below. Try making the image below and the image above look similar after doing the assignment. 

![output](output.png)

----

## Before doing the assignment

If you have not done the [task02](../task02), do it first to set up the C++ development environment.

Follow the instruction same as task02 to submit the assignment. In a nutshell, before doing the assignment,
- make sure you synchronized the main  branch of your local repository to that of remote repository.
- make sure you created branch `task03` from main branch.
- make sure you are currently in the `task03` branch (use git branch -a command).

The command will be like below

```bash
$ cd acg-<username>  # go to the local repository
$ git checkout main  # set main branch as the current branch
$ git fetch origin main    # download the main branch from remote repository
$ git reset --hard origin/main  # reset the local main branch same as remote repository
$ git branch -a   # make sure you are in main branch
$ git branch task03   # create task03 branch from main branch
$ git checkout task03  # switch into the task03 branch
$ git branch -a   # make sure you are in the task03 branch
```

Additionally, make sure `eigen` is installed under `acg-<username>/external/eigen`. 
Specifically, check if there is a header file `Dense` at `acg-<username>/external/eigen/Eigen/Dense`. 
Look at [task02](../task02) to how to install eigen.

Now you are ready to go!


### Problem 1

Compile the code with **out-of-source build** by making a new directory for build task02/build and compile inside that directory

``` bash
$ cd task03  # you are in "acg-<username>/task03" directory
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release .. # configure Release mode for fast execution
$ cmake --build .
```

Now you will see the `output.png` is updated. 

### Problem 2

The code you run shows the texture mapping is wrongly computed, and thus you see unnatural distortion of the texture.
This is because the UV coordinates is interpolated using the ***Barycentric coordinate on the screen*** ignoring the perspective effect.
Instead, we need to compute the ***Barycentric coordinate on the object*** to correctly interpolate the corner point's UV coordinates.


Write some codes around `line #69` in the `main.cpp`. 


### Submit

Finally, you submit the document by pushing to the `task03` branch of the remote repository. 

```bash
cd acg-<username>    # go to the top of the repository
git status  # check the changes
git add .   # stage the changes
git status  # check the staged changes
git commit -m "task03 finished"   # the comment can be anything
git push --set-upstream origin task03  # up date the task03 branch of the remote repository
```

got to the GitHub webpage `https://github.com/ACG-2024S/acg-<username>`. If everything looks good on this page, make a pull request. 

![](../doc/pullrequest.png)

## Reference

- [Perspective Projection and Texture Mapping (by Keenan Crane)](https://www.youtube.com/watch?v=_4Q4O2Kgdo4)
- [Texture Mapping](https://en.wikipedia.org/wiki/Texture_mapping)
