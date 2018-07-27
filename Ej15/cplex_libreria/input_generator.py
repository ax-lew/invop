import random


def generators(p):
    acum = 0
    print("{} {}\n6 15000\n3 30000\n6 25000\n3 40000\n6 27000".format(p, 5))

    for x in range(p):
        available = random.randint(1,30)
        min_mw = random.randint(1,3000)
        max_mw = random.randint(min_mw, min_mw+3000)
        min_cost = random.randint(1,3000)
        extra_cost = random.randint(1,3000)
        start_cost = random.randint(1,3000)

        acum = acum + min_mw * available
        if x == p-1 and acum < 40000:
            acum = acum - min_mw
            min_mw = 40000 - acum
            max_mw = min_mw + max_mw
        
        print("{} {} {} {} {} {}".format(available, min_mw, max_mw, min_cost, extra_cost, start_cost))


print("{} {}".format(len(range(1, 20, 3)), 5))

for g in range(1, 20, 3):
    generators(g)