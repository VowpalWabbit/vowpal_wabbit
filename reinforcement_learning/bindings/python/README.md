# Python Bindings

## Build + Install

1. `sudo add-apt-repository ppa:deadsnakes/ppa`
2. `sudo apt-get update`
3. `sudo apt-get install python3.6`
4. [Install pip for python3.6](https://askubuntu.com/questions/889535/how-to-install-pip-for-python-3-6-on-ubuntu-16-10)
5. `sudo python3.6 -m pip install -r ./reinforcement_learning/bindings/python/dev-requirements.txt`
6. `export RL_PYTHON_EXT_DEPS=<dir> # Dependencies will be installed to <dir>`
7. `./reinforcement_learning/bindings/python/build_linux/restore.sh`
8. `make clean`
9. `make rl_python`
10. `sudo python3.6 -m pip install reinforcement_learning/bindings/python/dist/rl_client-0.0.9-cp36-cp36m-linux_x86_64.whl`

## Usage
To use the client Python bindings please use [these instructions](https://microsoft.github.io/vowpal_wabbit/reinforcement_learning/doc/python/html/index.html).
