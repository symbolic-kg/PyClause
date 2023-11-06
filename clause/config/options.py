import flatdict


class Options():

    def __init__(self):
        """
        Creates an option onject that contains all parameters in their default value.
        """
        self.options = {}
        # the core options that are relavent whenever the c++ backend is called
        self.options['core'] = {
            "aggregation_function": "maxplus",
            "num_preselect": 10000000,
            "topk": 100,
            "filter_w_train": True,
            "filter_w_target": True,
            "disc_at_least": 10, ## -1 for off, must not be bigger than topk
            "rule_b_max_branching_factor": -1,
            "use_zero_rules": True,
            "rule_zero_weight": 0.01,
            "use_u_c_rules": True,
            "use_b_rules": True,
            "use_u_d_rules": True,
            "rule_u_d_weight": 0.01,
            "use_u_xxc_rules": True,
            "use_u_xxd_rules": True,
            "tie_handling": "frequency"
        }

        # the options relevenat for rule learning
        self.options['torm'] = dict(rules = dict(
            tautology = False, # if set to false, rules that do not make any wrong prediction are surpressed
            b = dict(
                active = False,
                confidence = 0.0001,
                support = 2,
                length = 3,
                batchsize = 1000
            ),
            uc = dict(
                active = True,
                confidence = 0.0001,
                support = 2
            ),
            ud = dict(
                active = True,
                confidence = 0.0001,
                support = 2
            ),
            z = dict(
                active = True,
                confidence = 0.0001,
                support = 2
            ),
            xx_uc = dict(
                active = True,
                confidence = 0.0001,
                support = 2
            ),
            xx_ud = dict(
                active = True,
                confidence = 0.0001,
                support = 2
            )
        ))

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

