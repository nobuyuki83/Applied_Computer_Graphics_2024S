# Task07: Multiple Importance Sampling (Brdf sampling, Light sampling)

![preview](preview.png)

**Deadline: June 6th (Thu) at 15:00pm**

----

## Before Doing Assignment

If you have not done the [task01](../task01), [task02](../task02) do it first to set up the C++ development environment.

Follow [this document](../doc/submit.md) to submit the assignment, In a nutshell, before doing the assignment,  
- make sure you synchronized the `main ` branch of your local repository  to that of remote repository.
- make sure you created branch `task07` from `main` branch.
- make sure you are currently in the `task07` branch (use `git branch -a` command).

Now you are ready to go!

---

## Problem

- Implement light sampling by lighting a single line code around `line #364`
- Implement Brdf sampling by lighting a single line code around `line #383`
- Implement MIS sampling by lighting a few lines of code around `line #401` around `line #403`

Run the program with **Release mode** and it will generate three images that replace the images below.   

| Light sampling              | Brdf sampling       | MIS sampling       |
| ----------------------- | ------------------------ | ------------------------ |
| ![light](out_light.png) | ![brdf](out_brdf.png) | ![mis](out_mis.png) |

Observe that three image looks similar but the noise is reduced by MIS sampling. 




## After Doing the Assignment

After modify the code, push the code and submit a pull request. Make sure your pull request only contains the files you edited. Good luck!
