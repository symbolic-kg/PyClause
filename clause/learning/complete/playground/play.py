

class Person:
    
    def __init__(self, name):
        self.name = name

class Professor(Person):
    def __init__(self,name):
        super().__init__(name)

p1 = Person("pete")
p2 = Person("cornelius")

print(p2.name)