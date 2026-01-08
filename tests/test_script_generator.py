import random

memory_size = 16384
ram_size = 4096
page_size = 256
block_size = 32
cache1_size = 512
cache2_size = 2048
ass1 = 1
ass2 = 4
print("init memory", memory_size)
print("init ram", ram_size, page_size)
print("init cache 1", cache1_size, block_size, ass1)
print("init cache 2", cache2_size, block_size, ass2)

id_counter = 1
cmd = 0
allocated_ids = []
while cmd < 1000:
    action = random.choices(["malloc", "read", "free"], weights = [20,75,5])[0]
    cmd += 1
    if action == "malloc":
        size = random.randint(1, memory_size/64)
        print("malloc", size)
        allocated_ids.append(id_counter)
        id_counter += 1
    if action == "read":
        address = random.randint(0, memory_size-1)
        print("read", address)
    if action == "free":
        if allocated_ids:
            id = random.choice(allocated_ids)
            allocated_ids.remove(id)
            print("free", id)
        else:
            cmd -= 1
# for i in range(1000):
#     print("read", i)
print("dump")
print("stats")