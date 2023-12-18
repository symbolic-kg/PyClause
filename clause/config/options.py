import flatdict
import yaml


class Options():

    def __init__(self, path_config_extra=None):
        """Creates an option onject that contains all parameters in their default value."""
        self.options = {}
        with open('config-default.yaml', 'r') as file:
            self.options  = yaml.safe_load(file)

        if path_config_extra != None:
            options_extra = {}
            with open(path_config_extra, 'r') as file:
                print(">>> using options in " + path_config_extra + " to overwrite some default options")
                options_extra  = yaml.safe_load(file)

            # note that the following code won't work correctly, in the case of nested dictionaries
            # self.options.update(options_extra)
            # instead, we have to go his more complicated way ... 
            options_extra_flat = dict(flatdict.FlatDict(options_extra, delimiter='.'))
            for param in options_extra_flat:
                self.set(param, options_extra_flat[param])

    def set(self, param, value):
        """Overwrites an entry in the options.

        They key is specified in e.g. the format 'torm.rules.b'.
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
                    # print(">>> setting param " + str(param) + " = " + str(value))
                    o[ktoken[-1]] = value
                else:
                    param_bad = True
        else:
            if param in self.options:
                self.options[param] = value
                # print(">>> setting param " + str(param) + " = " + str(value))
            else:
                param_bad = True
        if param_bad:
            raise Exception(f"Trying to set key parameter {str(param)} in the options but the key does not exist.")

    
    def flat(self, key = None):
        """Returns a flat dictionary representation (or value) of the dictionary referred to by the given key.
        
        All nested / structured keys below that key are mapped to strings as e.g. 'torm.rules.b'.
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
         
        All nested / structured keys below that key are mapped to strings
        as e.g. 'torm.rules.b'. If a key is not specified all options are returned in the flat dictionary representation.
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
            as_string (bool): If True, returns the value as a string (needed for backend options);
            otherwise, the type is preserved.
        """
        if as_string:
            return self.flatS(key)
        else:
            return self.flat(key)
