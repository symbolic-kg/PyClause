import flatdict
import yaml
import copy

from clause.util.utils import get_config_default_dir


class Options():

    def __init__(self, path=None):
        """Creates an option object that contains all parameters in their default value."""
        self.options = {}
        self.default_options = {}
        default_path = get_config_default_dir()
        with open(default_path, 'r') as file:
            self.options  = yaml.safe_load(file)
            self.default_options = copy.deepcopy(self.options)

        if path:
            options_extra = {}
            with open(path, 'r') as file:
                print(">>> using options in " + path + " to overwrite some default options")
                options_extra  = yaml.safe_load(file)

            options_extra_flat = dict(flatdict.FlatDict(options_extra, delimiter='.'))
            for param in options_extra_flat:
                self.set(param, options_extra_flat[param])
    

    def set(self, param, value):
        """Overwrites an entry in the options.

        They key is specified in e.g. the format 'learner.mode'.
        A value can be anything (that has a string representation). It is not possible to add new keys
        that do not exist in the default options. An exception is the parent key "raw". Under this
        key, new key, val pairs can be set. For instance, options.set("learning.amie.raw.notexistingkey", val).

        """
        param_bad = False
        if "." in param:
            ktoken = param.split(".")
            o = self.options
            prev_key = None
            for t in ktoken[:-1]:
                prev_key = t
                if t in o:
                    if type(o[t]) == dict: o = o[t]
                    else:
                        param_bad = True
                        break
                else:
                    param_bad = True
                    break
            if not param_bad:
                if ktoken[-1] in o or prev_key == "raw":
                    o[ktoken[-1]] = value
                else:
                    param_bad = True
        else:
            if param in self.options:
                self.options[param] = value
            else:
                param_bad = True
        if param_bad:
            raise Exception(f"Trying to set key parameter {str(param)} in the options but the key does not exist.")

    
    def flat(self, key = None):
        """Returns a flat dictionary representation (or value) of the dictionary referred to by the given key.
        
        All nested / structured keys below that key are mapped to strings separated by dots '.' .
        If a key is not specified all options are returned in the flat dictionary representation.
        """
        if key == None:
            d =  flatdict.FlatDict(self.options, delimiter='.')
            fd = dict(d)
            return fd
        else:
            ktoken = key.split(".")
            o = self.options
            for t in ktoken: o = o[t]
            try:
                d =  flatdict.FlatDict(o, delimiter='.')
                fd = dict(d)
                return fd
            except:
                return o
    
    def flatS(self, key = None):
        """Returns a flat dictionary representation (or value) of the dictionary referred to by the given key,
        where all values are converted to strings.
         
        All nested / structured keys below that key are mapped to strings separated by dots '.'.
        If a key is not specified all options are returned in the flat dictionary representation.
        """
        fd = self.flat(key)
        try:
            for k in fd.keys(): fd[k] = str(fd[k])
            return fd
        except:
            return str(fd)

    def get(self, key, as_string=True):
        """Returns a flat dictionary or a value for a given key.
            Args:
            key: The key for which to retrieve the value.
            as_string (bool): If True, returns the value as a string;
            otherwise, the type is preserved.
        """
        if as_string:
            return self.flatS(key)
        else:
            return self.flat(key)


    def _dict_diff(self, diff_dict, default_dict):
        """Rerturns a dict that contains all subdicts and items that are different in diff_dict from default_dict."""
        diff = {}
        for key in diff_dict:
            if key not in default_dict:
                diff[key] = diff_dict[key]
            elif isinstance(diff_dict[key], dict) and isinstance(default_dict[key], dict):
                nested_diff = self._dict_diff(diff_dict[key], default_dict[key])
                if nested_diff: 
                    diff[key] = nested_diff
            elif diff_dict[key] != default_dict[key]:
                diff[key] = diff_dict[key]
        return diff


    def write(self, file_path):
        """Writes user specified options to yaml file (ignores unchanged default options)."""
        diff = self._dict_diff(copy.deepcopy(self.options), copy.deepcopy(self.default_options))
        yaml.dump(diff)
        with open(file_path, 'w') as f:
            yaml.safe_dump(diff, f, default_flow_style=False)
     

