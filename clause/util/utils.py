import os
from os import path


def join_u(*paths):
    """prevents no \\ escaping and writing dysfunctional paths to config files"""
    join = path.join(*paths)
    return join.replace(os.sep, '/')


def get_base_dir():
    abs_path = path.abspath(__file__)
    for i in range(3):
        abs_path = path.dirname(abs_path)
    return abs_path.replace(os.sep, '/')


def get_ab_dir():
    base_dir = get_base_dir()
    return path.abspath(path.join(base_dir, join_u("bin", "AnyBURL-23-1.jar")))




