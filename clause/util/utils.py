import os
from os import path
import sys
import importlib.resources as pkg_resources


def read_jsonl(path):
    """Reads a json-line file and return a list of dicts"""
    import json
    with open(path, "r") as infile:
        out = []
        # check if valid jsonl is test enough for now
        for line in infile:
            out_line = json.loads(line)
            out.append(out_line)
    return out


def join_u(*paths):
    """Prevents no \\ escaping and writing dysfunctional paths to config files for windows"""
    join = path.join(*paths)
    return join.replace(os.sep, '/')


def get_base_dir():
    """Only use this for code examples, never for implementation."""
    abs_path = path.abspath(__file__)
    for i in range(3):
        abs_path = path.dirname(abs_path)
    if "site-packages" in abs_path:
        print(
            "It seems PyClause is installed as a package;"
            " get_base_dir() is only valid when installed in editable mode with pip install -e ."
            " I will output your current directory.",
            file=sys.stderr
        )
        return ("./")
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
    resource = "amie-anyburl-integration.jar"
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


def read_map(path):
    map = {}
    with open(path, 'r') as file:
        for line in file:
            key, value = line.strip().split('\t')
            map[key] = value
    return map




