import yaml
import collections


##


def flatten_dict(d, parent_key='', sep='.'):
    items = []
    for k, v in d.items():
        new_key = parent_key + sep + k if parent_key else k
        if isinstance(v, collections.MutableMapping):
            items.extend(flatten_dict(v, new_key, sep=sep).items())
        else:
            items.append((new_key, v))
    return dict(items)


class Config:
    def __init__(self, load_path: str = None):
        self.options = None
        if load_path:
            self.load(load_path)

    def load(self, load_path: str):
        with open(load_path, "rb") as f:
            c = yaml.load(f, Loader=yaml.SafeLoader)
        self.options = flatten_dict(c)

    def get(self, key: str):
        if key in self.options:
            return self.options.get(key)
        else:
            raise Exception(f"Could not find {key} in config.")