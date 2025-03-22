import random


def fill_kv(file, count, values):
    for i in range (0, count):
        file.write("SET " + str(i) + ' ' + str(i) + '\n')
    for i in range (0, min(count, 100)):
        values.append(str(random.randint(0, count - 1)))
        file.write("DEL " + values[i] + '\n')
    file.write("EXIT\n")

def fill_set(file, sets, count):
    for i in range(0, sets):
        for j in range(0, count):
            file.write(f"ZADD {i}set {j * count} {str(i * count + j)}" + '\n')
    for i in range(min(sets, 5)):
        set_ = random.randint(0, sets - 1)
        file.write(f"ZRANK {set_}set {set_ * count + random.randint(0, count - 1)}" + '\n')
    set_ = random.randint(0, sets - 1)
    file.write(f"ZQUERYGE {set_}set 0 {random.randint(0, count - 1)} 0 {min(count, 100)}" + '\n')
    set_ = random.randint(0, sets - 1)
    file.write(f"ZQUERYLE {set_}set {count} {random.randint(0, count - 1)} 0 {min(count, 100)}" + '\n')
    set_ = random.randint(0, sets - 1)
    file.write(f"ZCOUNT {set_}set {0} {min(count, 100)}" + '\n')
    for i in range(0, sets):
        for j in range(0, count):
            file.write("ZREM " + str(i) + "set" + ' ' + str(i * count + j) + '\n')
    file.write("EXIT\n")

vals = []
def kv_test(count):
    print(f"KV_test: count: {count}")
    with open("test_cases.txt", "w") as test_file:
        fill_kv(test_file, int(count), vals)
def set_test(sets, count):
    print(f"Set_test: sets: {sets} count: {count}")
    with open("test_cases.txt", "w") as test_file:
        fill_set(test_file, int(sets), int(count))