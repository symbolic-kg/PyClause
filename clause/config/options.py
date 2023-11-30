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
        """Overwrites an entry in the options.

        They key is specified in e.g. the format 'torm.rules.b'.
        A value can be anything (that has a string representation). It is not possible to add
        a new key hierarchy with this method. But new keys + values can be added on the level of
        the lowest key of an existing hierarchy.
         
        Example: existing hierarchy in config-default.yaml "learning.anyburl.raw.THRESHOLD_CONFIDENCE": 0.0001
        You can create new key, val by conf.set("learning.anyburl.raw.NEW_KEY", someVal).
       

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

