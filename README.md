# WinMut

Artifact for ASE21 submission: Faster Mutation Analysis with Fewer Processes and Smaller Overheads.

### Install

You can build WinMut from source or use pre-built docker images.
Please refer to [REQUIREMENTS.md](REQUIREMENTS.md) and [INSTALL.md](INSTALL.md) for the installation.

### Run the compiler
The built clang compiler accepts the following command line arguments:
```shell
-accmut           AccMut algorithm
-winmut           WinMut algorithm
-winmut-no-opt    Turn of the optimized instrument algorithm
-winmut-runtime   Generate no mutations, run the program under WinMut runtime
-winmut-timing    Generate no mutations, only instrument the program to record running time
```
You can build a project with cmake/autoconf/Makefile with our compiler by setting the CC/CFLAGS environment variables.

Typically if you want to build a project with autoconf with WinMut algorithm, you should run
```bash
CC=<<<path-to-our-compiler>>> CFLAGS="-winmut -O2" ./configure
WINMUT_DISABLED=YES LD_PRELOAD=<<<path-to-winmut-runtime>>> make
```
We will describe why we need `WINMUT_DISABLED` and `LD_PRELOAD` environment variables later.

You can read the file `experiments/new-subjects/run.sh` for how we use these command line arguments. You can also read
`experiments/config/*` to see how we configure and build the tested projects.

#### Limitations:
1. The modified compiler is not handling instructions with debug infos correctly at this time and would crash.
This affects the building of binutils. It won't be fully built with our compiler because its build system adds the
`-g` flag for some submodules. This happens after building gas module, so we can still test it.
2. The modified compiler cannot compile C++ programs if `-accmut` or `-winmut` is specified.
3. The WinMut runtime library would not work with threads. Please turn of thread support for the projects.

### Running the compiled program
You can directly run the compiled program to do mutation analysis. You may need to define some environment variables.

#### `WINMUT_LOG_FILE_PREFIX`
You should set the environment variable `WINMUT_LOG_FILE_PREFIX` to the **absolute path** to some **existing empty** directory to let the program
record some logs for analysis.

#### `LD_PRELOAD`
If the program depends on any external library generated by other compiler,
you should add the path to `LLVMWinMutRuntime.so` library to `LD_PRELOAD`.
This is necessary for our library to override all file system calls.
The preloaded library would delegate the system calls back to the system for those non-WinMut-compiled programs,
so we can run `make` or `bash` correctly.

#### `WINMUT_DISABLED`
If the environment variable is set to YES, the compiled program won't perform mutation analysis. This is very important
for reducing build time for the systems executing some compiled programs during build phase.

#### `WINMUT_MAX_RUN_CASES`
If set to some positive number n, the system would only perform mutation analysis on the first n program invocations.
This works correctly only if `WINMUT_LOG_FILE_PREFIX` is set correctly.

#### `WINMUT_MEASURE_COUNT`
This controls how the WinMut determines the timeouts. Normally you don't need to set this.

### Running the experiments from the paper
We provided several scripts to set up the projects and run the experiments.

#### Set up the projects
1. Go to the `experiment/new-subjects` directory
```shell
cd experiments/new-subjects
```
2. Download the subject projects and patch them
```shell
./setup.sh
```

The subject projects will be directly downloaded from the project websites. Some of them are patched to make it work
with our compiler. You can read the patch files at `experiments/new-subjects`.

- `lz4` is patched to remove the `-g` flag passed to the compiler.
- `lua` is patched to add the correct CFLAGS.
- `ffmpeg` is patched to remove several test cases that won't work with our runtime.

#### Building the subject project and execute the test suites
We provided a script called `run.sh` to build and execute the test suites.

The usage is
```text
./run.sh subject-name <build/run> <WinMut/WinMutNo/AccMut/AccMutNo/timing/validate> [time-limit/process-limit]
```

It hardcoded `${PROJECT_ROOT}/cmake-build-release` as the root path to the compiler and runtime. If you installed
the library elsewhere, you can override this by setting the environment variable `WINMUT_BASE_DIR`.

Examples:

If you want to run pcre2 with WinMut and restrict the max cases to 10, you should run
```shell
./run.sh pcre2 build WinMut
./run.sh pcre2 run WinMut 10
```

If you want to run gmp with only the WinMut runtime and restrict the max cases to 10, you should run
```shell
./run.sh gmp build validate
./run.sh gmp run validate 10
```

If you want to run libsodium, only record the time and restrict the max running time to 2s, you should run
```shell
./run.sh libsodium build timing
./run.sh libsodium run timing 2
```

It is known that ffmpeg would hang if ran in our script. You can build ffmpeg with our script, but you should run the
test suites directly from the shell.

For example, if you want to execute the test suites for ffmpeg under WinMut algorithm, you should run
```shell
cd ffmpeg/WinMut
LD_PRELOAD=<<<path-to-winmut-runtime>>> ACCMUT_LOG_FILE_PREFIX="$(pwd)/winmut-log-dir" WINMUT_MAX_RUN_CASES=20 \
make check
```

#### Reproducing the main result

To evaluate and debug the artifact, the WinMut runtime would log some necessary information to the directory specified
with the environment variable `WINMUT_LOG_FILE_PREFIX`.

These log files are:
```
forked: records the running time, exit status and the mutation information for each subprocess
forked-simple: records the the mutation information for each subprocess
panic: records the information for the programs executing unsupported features in the runtime
ran: records how many programs are executed with the runtime. We use this file to decide when to stop mutation testing
threshold: record the timeout threshold for the programs
timeout: records the information for the timeout
trace: for debugging, records the call to the file system. Turned off by default.
```

We provided a python script to analyze the results:

`experiments/analysis/read_time_and_process_num.py` reads the `forked` log to calculate the time and process number
for the project.

e.g.
```shell
read_time_and_process_num.py pcre2/WinMut/winmut_log_dir/forked
```
reads and print the time and process number for pcre2 project with WinMut algorithm.

```shell
read_time_and_process_num.py pcre2/WinMut/winmut_log_dir/forked pcre2/AccMut/winmut_log_dir/forked
```
compares the time and process number for pcre2 project with WinMut and AccMut algorithm.

#### Reproducing other results

##### Program statistics
For the program statistics, please refer to `experiments/analysis/read_program_statistics.sh`. You should first compile the project with WinMut algorithm, then run this script with the project root folder as the argument.

**Example:**

If you want to collect the statistics of pcre2 project, you should run the following commands in the `new-subjects` folder
```
./run.sh pcre2 build WinMut
../analysis/read_program_statistics.sh pcre2/WinMut
```

