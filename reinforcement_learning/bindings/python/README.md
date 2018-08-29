# Python Bindings

## Build + Install
1. [Install pip for python3.6](https://askubuntu.com/questions/889535/how-to-install-pip-for-python-3-6-on-ubuntu-16-10)
2. `sudo python3.6 -m pip install wheel`
3. `sudo python3.6 -m pip install setuptools`
4. Edit reinforcement_learning/bindings/python/setup.py to point to a directory on your machine (Will install dependencies here)
5. `./reinforcement_learning/bindings/python/build_linux/restore.sh`
6. `make clean`
7. `make rl_python`
8. `sudo pip3.6 install reinforcement_learning/bindings/python/dist/rl_client-0.0.5-cp36-cp36m-linux_x86_64.whl`

## Usage
To use the client Python bindings please use [these instructions](https://microsoft.github.io/vowpal_wabbit/reinforcement_learning/doc/html/Python.html).
