import os
from os import path
import sys
import importlib.resources as pkg_resources


def join_u(*paths):
    """prevents no \\ escaping and writing dysfunctional paths to config files"""
    join = path.join(*paths)
    return join.replace(os.sep, '/')


def get_base_dir():
    abs_path = path.abspath(__file__)
    for i in range(3):
        abs_path = path.dirname(abs_path)
    if "site-packages" in abs_path:
        print(
            "It seems PyClause is installed as a package;"
            " get_base_dir() is only valid when installed in editable mode with pip install -e .",
            file=sys.stderr
        )
    return abs_path.replace(os.sep, '/')


def get_package_dir():
    abs_path = path.abspath(__file__)
    for i in range(2):
        abs_path = path.dirname(abs_path)
    return abs_path.replace(os.sep, '/')


def get_config_default_dir():
    """Returns file path to default config"""
    package = "clause"
    resource = "config-default.yaml"
    try:
        with pkg_resources.path(package, resource) as p:
            return str(p)
    except:
        return join_u(get_package_dir(), resource)


def get_ab_dir():
    """Returns file path to AnyBURL binary"""
    package = "clause.bin"
    resource = "AnyBURL-23-1.jar"
    try:
        with pkg_resources.path(package, resource) as p:
            return str(p)
    except:
        return join_u(get_package_dir(), "bin", resource)


def get_amie_dir():
    """Returns file path to AMIE binary"""
    package = "clause.bin"
    resource = "AMIE-dev.jar"
    try:
        with pkg_resources.path(package, resource) as p:
            return str(p)
    except:
        return join_u(get_package_dir(), "bin", resource)


def list_from_idx_file(filename):
    outlist = []
    with open(filename, 'r') as file:
        for line in file:
            category_type = line.split('\t')[1].strip()
            outlist.append(category_type)
    return outlist




