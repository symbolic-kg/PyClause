
paths = dict(
    input = dict(
        train = "E://code/eclipse-workspace/AnyBURL/data/FB237/train.txt",
        filter = (
            "E://code/eclipse-workspace/AnyBURL/data/WN18RR/train.txt",
            "E://code/eclipse-workspace/AnyBURL/data/WN18RR/valid.txt",
            "E://code/eclipse-workspace/AnyBURL/data/WN18RR/test.txt",
        ),
        tasks = "E://code/eclipse-workspace/AnyBURL/data/WN18RR/test.txt"
    ),
    output = dict(
        rules = "E://exp/pyclause/rules/rules-pctorm-wn18rr-b3"
    )
)

rules = dict(
    b = dict(
        active = True,
        confidence = 0.01,
        support = 5,
        pruning = (0,100), # (x,y) means that the search is stopped if after y attempts not more than x correct predictions have been found
        length = 3,
        timeout = 5, # =1seconds
        branches_per_level = ((500, (10,10,10)), (500, (10,10,10)))
    ),
    uc = dict(
        active = False,
        confidence = 0.0001,
        support = 2
    ),
    ud = dict(
        active = False,
        confidence = 0.0001,
        support = 2
    ),
    z = dict(
        active = False,
        confidence = 0.0001,
        support = 2
    ),
    xx_uc = dict(
        active = False,
        confidence = 0.0001,
        support = 2
    ),
    xx_ud = dict(
        active = False,
        confidence = 0.0001,
        support = 2
    )
)

multithreading = dict(
    worker = 4,
    start_method = "fork", # chose between "fork" and "spawn", if possibe use "fork", "spawn" will automaticall be used in windows systems
    brules_dataarray_size = 6000000
)


completions = dict(
    max = 100
)

hyperparams = dict(
    uce = 5 # unseen counter example, which is used to compute the aconfidence (applied confidence)
)




