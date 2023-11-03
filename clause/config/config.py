
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



completions = dict(
    max = 100
)

hyperparams = dict(
    uce = 5 # unseen counter example, which is used to compute the aconfidence (applied confidence)
)




