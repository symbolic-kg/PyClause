import time
import c_clause
from clause.util.utils import get_base_dir
from clause.config.options import Options
from clause.learning.miner import Miner
from clause.data.triples import TripleSet



path_train = "./data/wikidata5m_v3/train.txt"


#loads data in frontend
triples = TripleSet(path_train)


options = Options()
# rule mining in python
options.set("learning.mode", "torm")

# shut of all rule types except u_c, B
options.set("learning.torm.ud.active", False)
options.set("learning.torm.z.active", False)
options.set("learning.torm.xx_uc.active", False)
options.set("learning.torm.xx_ud.active", False)

# cyclical (B-Rule) rules
options.set("learning.torm.b.active", False)
#options.set("learning.torm.b.support", 100)
#options.set("learning.torm.b.length", 1)

# u_c rule options (len=1 rules: head(X,a) <-- b(X,b))
options.set("learning.torm.uc.support", 15)


miner = Miner(options, triples.rels, triples)
outputf = "./local/wd5m/wd5m-rules-uc-s15.txt"
miner.mine_rules(outputf)




