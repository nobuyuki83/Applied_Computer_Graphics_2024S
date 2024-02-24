# How to Set Up Eigen Library

There are three ways to set up `Eigen`:

- install from package managers (for Mac and Ubuntu)
- download library release (for all OSs)
- download source and install (for all OSs)

Below, we discuss these options in detail

----

## Mac 

### Install from Package Manager

Use the Mac's package manager `brew` for installation

```bash
$ brew install eigen
```

### Download Library

The repository can be downloaded from

 http://eigen.tuxfamily.org/index.php?title=Main_Page#Download

1. Download the compressed file (e.g.,` eigen-3.*.*.zip`)  and extract it. This result in a directory `eigen-3.*.*`
2. Put the extracted directory under the `external` directory as `acg-<username>/external/eigen-3.*.*` .

Make sure you have a header file `Dense` at

```
acg-<username>/external/eigen-3.*.*/Eigen/Dense
```

### Clone Library

Instead of download, you can clone the `Eigen` repository. 

```bash
$ git submodule update --init external/eigen # clone the "eigen" repository
```

Make sure you have a header file `Dense` at

```
acg-<username>/external/eigen/Eigen/Dense
```



---
## Ubuntu or Windows Subsystem for Linux (WSL)

### Install from Package Manager

For ubuntu, install `eigen` using `apt-get` as

```bash
$ sudo apt-get install libeigen3-dev
```

### Download Library

The repository can be downloaded from here:

 http://eigen.tuxfamily.org/index.php?title=Main_Page#Download

1. Download the compressed file (e.g.,` eigen-3.*.*.zip`)  and extract it. This result in a directory `eigen-3.*.*`
2. Put the extracted file under the `external` directory as `acg-<username>/external/eigen-3.*.*` .

Make sure you have a header file `Dense` at

```
acg-<username>/external/eigen-3.*.*/Eigen/Dense
```

### Clone Repository

The only difference is that instead of download we use submodule to clone the `Eigen` repository. 

```bash
$ git submodule update --init external/eigen # clone the "eigen" repository (this may take one or two mins)
```

Make sure you have a header file `Dense` at
```
acg-<username>/external/eigen/Eigen/Dense
```


---
## Windows

### Download Library
Download the repository from here

 http://eigen.tuxfamily.org/index.php?title=Main_Page#Download

1. Download the compressed file (e.g.,` eigen-3.*.*.zip`)  and extract it. This result in a directory `eigen-3.*.*`
2. Put the extracted file under the `external` directory as `acg-<username>/external/eigen-3.*.*` .

Make sure you have a header file `Dense` at

```
acg-<username>/external/eigen-3.*.*/Eigen/Dense
```

### Clone Repository

The only difference is that instead of download we use submodule to clone the `Eigen` repository. 

```bash
$ git submodule update --init external/eigen # clone the "eigen" repository (this may take one or two mins)
```

Make sure you have a header file `Dense` at

```
acg-<username>/external/eigen/Eigen/Dense
```

---
## Configure Eigen Library

For this course, the installation procedure above is enough. But the `eigen` can be further optimized for your environment by configuring the library.

To configure the `eigen` library, type the commands below
```bash
$ cd external/eigen # move to the eigen repository
$ mkdir build # make a directory for "out-of-source build"
$ cd build    # move to the new directory
$ cmake ..    # configure (this may take one or two mins)
$ cmake --install . --prefix ../../eigenlib # install eigen into the "eigenlib" folder
```

Make sure you have a header file `Dense` at

```
acg-<username>/external/eigenlib/include/eigen3/Eigen/Dense
```