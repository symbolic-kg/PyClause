import flatdict
import yaml


class Options():

    def __init__(self, path_config_extra=None):
        """
        Creates an option onject that contains all parameters in their default value.
        """
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
        """
        Overwrites an entry in the options. They key is specified in e.g. the format 'torm.rules.b'.
        A value can be anything (that ahs a string representation). It is not possible to add
        a new parameter/value pair with this method.
        """
        if "." in param:
            ktoken = param.split(".")
            o = self.options
            for t in ktoken[:-1]: o = o[t]
            o[ktoken[-1]] = value
        else: 
            self.options(param, value)

    
    def flat(self, key = None):
        """
        Returns a flat dictionary representation of the dictionary referred to by the given key.
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
            d =  flatdict.FlatDict(o, delimiter='.')
            fd = dict(d)
            return fd
    
    def flatS(self, key = None):
        """
        Returns a flat dictionary representation of the dictionary referred to by the given key,
        where all values are converted to string. All nested / structured keys below that key are mapped to strings
        as e.g. 'torm.rules.b'. If a key is not specified all options are returned in the flat dictionary representation.
        """
        fd = self.flat(key)
        for k in fd.keys(): fd[k] = str(fd[k])
        return fd

