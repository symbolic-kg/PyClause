
rules = dict(
    b = dict(
        active = True,
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
)

multithreading = dict(
    worker = 4,
    start_method = "fork", # chose between "fork" and "spawn", if possibe use "fork", "spawn" will automatically be used in windows systems
    brules_dataarray_size = 6000000
)


completions = dict(
    max = 100
)

hyperparams = dict(
    uce = 5 # unseen counter example, which is used to compute the aconfidence (applied confidence)
)




