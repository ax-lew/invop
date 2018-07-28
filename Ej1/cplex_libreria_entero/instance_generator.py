import sys
import random

repeats, max_months, max_oils = map(lambda x: int(x), sys.stdin.read().split(" "))

max_stock = 500
max_ton_vegetable_used_per_month = 500.0
max_ton_non_vegetable_used_per_month = 550.0
benefit = 150.0
stock_cost_per_ton = 5.0
initial_stock = 500.0

def generate_input(months, oils):
    vegetable_oils = int(oils*2/5)
    non_vegetable_oils = oils - vegetable_oils
    print("{} {} {} {} {} {} {} {}".format(vegetable_oils, oils, months, max_ton_vegetable_used_per_month, max_ton_non_vegetable_used_per_month, benefit, stock_cost_per_ton, initial_stock))

    hardness = ""
    for _ in range(oils):
        hardness += "{:.2f} ".format(random.uniform(2, 8))
    print(hardness)

    for _ in range(months):
        prices = ""
        for _ in range(oils):
            prices += "{} ".format(random.randint(50, 150))
        print(prices)

print("{} {}".format(len(range(6, max_months, 5)) * len(range(5, max_oils, 5)), repeats))

for months in range(6, max_months, 5):
    for oils in range(5, max_oils, 5):
        generate_input(months, oils)